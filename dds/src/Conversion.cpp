/*
 * Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include "Conversion.hpp"
#include <fastrtps/types/TypeDescriptor.h>
#include <sstream>
#include <stack>

//using eprosima::fastrtps::types::MemberDescriptor;
using eprosima::fastrtps::types::MemberId;

namespace soss {
namespace dds {

using namespace eprosima::fastrtps;

std::map<std::string, ::xtypes::DynamicType::Ptr> Conversion::types_;
std::map<std::string, DynamicPubSubType*> Conversion::registered_types_;

bool Conversion::soss_to_dds(
        const ::xtypes::DynamicData& input,
        DynamicData* output)
{
    std::stack<DynamicData*> dyn_stack;
    DynamicData* current_data = output;
    DynamicData* parent = nullptr;

    std::stringstream ss;
    input.for_each([&](const ::xtypes::DynamicData::ReadableNode& node)
       {
           while (dyn_stack.size() > node.deep())
           {
               dyn_stack.pop();
               if (parent != nullptr)
               {
                   parent->return_loaned_value(current_data);
               }

               if (dyn_stack.empty())
               {
                   current_data = output;
                   parent = nullptr;
               }
               else
               {
                   parent = dyn_stack.top();
               }
           }
           if (parent != nullptr)
           {
               if (!node.has_parent())
               {
                   std::cout << "Parent isn't nullptr, but node has no parent..." << std::endl;
                   return;
               }
               uint32_t id = current_data->get_member_id_by_name(node.from_member()->name());
               current_data = parent->loan_value(id);
               if (current_data == nullptr)
               {
                   ss << "Cannot find member " << node.from_member()->name() << " in type " << node.type().name();
                   throw DDSMiddlewareException(ss.str());
               }
           }

           switch (node.type().kind())
           {
                // TODO: Current dtparser sets ID for primitive types to 0 instead of MEMBER_ID_INVALID.
                // If this parser is kept, we should fix this to use MEMBER_ID_INVALID (so we can omit it here).
                case ::xtypes::TypeKind::CHAR_8_TYPE:
                    current_data->set_char8_value(node.data().value<char>(), 0);
                    break;
                case ::xtypes::TypeKind::CHAR_16_TYPE:
                    current_data->set_char16_value(node.data().value<char32_t>(), 0);
                    break;
                case ::xtypes::TypeKind::UINT_8_TYPE:
                    current_data->set_uint8_value(node.data().value<uint8_t>(), 0);
                    break;
                case ::xtypes::TypeKind::INT_16_TYPE:
                    current_data->set_int16_value(node.data().value<int16_t>(), 0);
                    break;
                case ::xtypes::TypeKind::UINT_16_TYPE:
                    current_data->set_uint16_value(node.data().value<uint16_t>(), 0);
                    break;
                case ::xtypes::TypeKind::INT_32_TYPE:
                    current_data->set_int32_value(node.data().value<int32_t>(), 0);
                    break;
                case ::xtypes::TypeKind::UINT_32_TYPE:
                    current_data->set_uint32_value(node.data().value<uint32_t>(), 0);
                    break;
                case ::xtypes::TypeKind::INT_64_TYPE:
                    current_data->set_int64_value(node.data().value<int64_t>(), 0);
                    break;
                case ::xtypes::TypeKind::UINT_64_TYPE:
                    current_data->set_uint64_value(node.data().value<uint64_t>(), 0);
                    break;
                case ::xtypes::TypeKind::FLOAT_32_TYPE:
                    current_data->set_float32_value(node.data().value<float>(), 0);
                    break;
                case ::xtypes::TypeKind::FLOAT_64_TYPE:
                    current_data->set_float64_value(node.data().value<double>(), 0);
                    break;
                case ::xtypes::TypeKind::FLOAT_128_TYPE:
                    current_data->set_float128_value(node.data().value<long double>(), 0);
                    break;
                case ::xtypes::TypeKind::STRING_TYPE:
                    current_data->set_string_value(node.data().value<std::string>(), 0);
                    break;
                case ::xtypes::TypeKind::WSTRING_TYPE:
                {
                    current_data->set_wstring_value(node.data().value<std::wstring>(), 0);
                    break;
                }
                case ::xtypes::TypeKind::ARRAY_TYPE:
                    // TODO Implement
                    ss << "Array type unsupported. Type: " << node.type().name();
                    throw DDSMiddlewareException(ss.str());
                    break;
                case ::xtypes::TypeKind::SEQUENCE_TYPE:
                    // TODO Implement
                    ss << "Sequence type unsupported. Type: " << node.type().name();
                    throw DDSMiddlewareException(ss.str());
                    break;
                case ::xtypes::TypeKind::STRUCTURE_TYPE:
                {
                    if (!node.has_parent())
                    {
                        dyn_stack.push(output);
                        return;
                    }
                    dyn_stack.push(current_data);
                    break;
                }
                default:
                    ss << "Unsupported type: " << node.type().name();
                    throw DDSMiddlewareException(ss.str());

           }
       });

    return true;
}

// TODO: Can we receive a type without members as root?
bool Conversion::dds_to_soss(
        const std::string /*type*/,
        DynamicData* input,
        ::xtypes::DynamicData& output)
{
    std::stringstream ss;
    uint32_t id = 0;
    uint32_t i = 0;
    MemberDescriptor descriptor;

    while (id != MEMBER_ID_INVALID)
    {
        id = input->get_member_id_at_index(i);
        ResponseCode ret = types::RETCODE_ERROR;

        if (id != MEMBER_ID_INVALID)
        {
            DynamicDataSOSS* dd_soss = static_cast<DynamicDataSOSS*>(input);
            ret = dd_soss->GetDescriptorSOSS(descriptor, id);

            if (ret == types::RETCODE_OK)
            {
                types::TypeKind member_type = descriptor.get_kind();

                if (member_type == types::TK_BOOLEAN)
                {
                    bool value;
                    ret = input->get_bool_value(value, id);
                    output[descriptor.get_name()].value<bool>(value);
                }
                else if (member_type == types::TK_BYTE)
                {
                    uint8_t value;
                    ret = input->get_byte_value(value, id);
                    output[descriptor.get_name()].value<uint8_t>(value);
                }
                else if (member_type == types::TK_INT16)
                {
                    int16_t value;
                    ret = input->get_int16_value(value, id);
                    output[descriptor.get_name()].value<int16_t>(value);
                }
                else if (member_type == types::TK_INT32)
                {
                    int32_t value;
                    ret = input->get_int32_value(value, id);
                    output[descriptor.get_name()].value<int16_t>(value);
                }
                else if (member_type == types::TK_INT64)
                {
                    int64_t value;
                    ret = input->get_int64_value(value, id);
                    output[descriptor.get_name()].value<int64_t>(value);
                }
                else if (member_type == types::TK_UINT16)
                {
                    uint16_t value;
                    ret = input->get_uint16_value(value, id);
                    output[descriptor.get_name()].value<uint16_t>(value);
                }
                else if (member_type == types::TK_UINT32)
                {
                    uint32_t value;
                    ret = input->get_uint32_value(value, id);
                    output[descriptor.get_name()].value<uint32_t>(value);
                }
                else if (member_type == types::TK_UINT64)
                {
                    uint64_t value;
                    ret = input->get_uint64_value(value, id);
                    output[descriptor.get_name()].value<uint64_t>(value);
                }
                else if (member_type == types::TK_FLOAT32)
                {
                    float value;
                    ret = input->get_float32_value(value, id);
                    output[descriptor.get_name()].value<float>(value);
                }
                else if (member_type == types::TK_FLOAT64)
                {
                    double value;
                    ret = input->get_float64_value(value, id);
                    output[descriptor.get_name()].value<double>(value);
                }
                else if (member_type == types::TK_FLOAT128)
                {
                    long double value;
                    ret = input->get_float128_value(value, id);
                    output[descriptor.get_name()].value<long double>(value);
                }
                else if (member_type == types::TK_CHAR8)
                {
                    char value;
                    ret = input->get_char8_value(value, id);
                    output[descriptor.get_name()].value<char>(value);
                }
                else if (member_type == types::TK_CHAR16)
                {
                    wchar_t value;
                    ret = input->get_char16_value(value, id);
                    output[descriptor.get_name()].value<wchar_t>(value);
                }
                else if (member_type == types::TK_STRING8)
                {
                    std::string value;
                    ret = input->get_string_value(value, id);
                    output[descriptor.get_name()].value<std::string>(value);
                }
                else if (member_type == types::TK_STRING16)
                {
                    std::wstring value;
                    ret = input->get_wstring_value(value, id);
                    output[descriptor.get_name()].value<std::wstring>(value);
                }
                else if (member_type == types::TK_ARRAY)
                {
                    // TODO
                }
                else if (member_type == types::TK_SEQUENCE)
                {
                    // TODO
                }
                else if (member_type == types::TK_STRUCTURE)
                {
                    DynamicData* nested_msg_dds = input->loan_value(id);
                    ::xtypes::DynamicData nested_msg_soss = output[descriptor.get_name()][id];

                    if (nested_msg_dds != nullptr)
                    {
                        if (dds_to_soss(nested_msg_dds->get_name(), nested_msg_dds, nested_msg_soss))
                        {
                            ret = types::RETCODE_OK;
                        }
                        input->return_loaned_value(nested_msg_dds);
                    }
                }
                else
                {
                    ret = types::RETCODE_ERROR;
                }

                i++;
            }

            if (ret != types::RETCODE_OK)
            {
                std::stringstream ss;
                ss << "Error parsing from dynamic type '" << input->get_name();
                ss << "Error code: " << ret << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }
    }

    return true;
}

::xtypes::DynamicData Conversion::dynamic_data(
        const std::string& type_name)
{
    auto it = types_.find(type_name);

    if (it == types_.end())
    {
        auto pst_it = registered_types_.find(type_name);
        if (pst_it != registered_types_.end())
        {
            std::stringstream ss;
            ss << "Error getting data from dynamic type '" << type_name << " (not registered).";
            throw DDSMiddlewareException(ss.str());
        }
        auto pair = types_.emplace(
            type_name,
            convert_type(pst_it->second->GetDynamicType().get()));
        it = pair.first;
    }

    return ::xtypes::DynamicData(*it->second.get());
}

::xtypes::DynamicType::Ptr Conversion::convert_type(
        const DynamicType* type)
{
    auto it = types_.find(type->get_name());
    if (it != types_.end())
    {
        return it->second;
    }
    ::xtypes::DynamicType::Ptr ptr = create_type(type);
    types_.emplace(type->get_name(), ptr);
    return ptr;
}

::xtypes::DynamicType::Ptr Conversion::create_type(
        const DynamicType* type)
{
    const eprosima::fastrtps::types::TypeDescriptor* descriptor = type->get_descriptor();
    switch (descriptor->get_kind())
    {
        // Basic types
        case ::eprosima::fastrtps::types::TK_NONE:
            return ::xtypes::DynamicType::Ptr();
        case ::eprosima::fastrtps::types::TK_BOOLEAN:
            return ::xtypes::primitive_type<bool>();
        case ::eprosima::fastrtps::types::TK_BYTE:
            return ::xtypes::primitive_type<uint8_t>();
        case ::eprosima::fastrtps::types::TK_INT16:
            return ::xtypes::primitive_type<int16_t>();
        case ::eprosima::fastrtps::types::TK_INT32:
            return ::xtypes::primitive_type<int32_t>();
        case ::eprosima::fastrtps::types::TK_INT64:
            return ::xtypes::primitive_type<int64_t>();
        case ::eprosima::fastrtps::types::TK_UINT16:
            return ::xtypes::primitive_type<uint16_t>();
        case ::eprosima::fastrtps::types::TK_UINT32:
            return ::xtypes::primitive_type<uint32_t>();
        case ::eprosima::fastrtps::types::TK_UINT64:
            return ::xtypes::primitive_type<uint64_t>();
        case ::eprosima::fastrtps::types::TK_FLOAT32:
            return ::xtypes::primitive_type<float>();
        case ::eprosima::fastrtps::types::TK_FLOAT64:
            return ::xtypes::primitive_type<double>();
        case ::eprosima::fastrtps::types::TK_FLOAT128:
            return ::xtypes::primitive_type<long double>();
        case ::eprosima::fastrtps::types::TK_CHAR8:
            return ::xtypes::primitive_type<char>();
        case ::eprosima::fastrtps::types::TK_CHAR16:
            return ::xtypes::primitive_type<wchar_t>();
        // String TKs
        case ::eprosima::fastrtps::types::TK_STRING8:
        {
            if (descriptor->get_bounds() != LENGTH_UNLIMITED)
            {
                return ::xtypes::StringType(descriptor->get_bounds());
            }
            return ::xtypes::StringType();
        }
        case ::eprosima::fastrtps::types::TK_STRING16:
        {
            if (descriptor->get_bounds() != LENGTH_UNLIMITED)
            {
                return ::xtypes::WStringType(descriptor->get_bounds());
            }
            return ::xtypes::WStringType();
        }
        // Collection TKs
        case ::eprosima::fastrtps::types::TK_SEQUENCE:
        {
            ::xtypes::DynamicType::Ptr inner = convert_type(descriptor->get_element_type().get());
            return ::xtypes::SequenceType(*inner.get(), descriptor->get_bounds());
        }
        case ::eprosima::fastrtps::types::TK_ARRAY:
        {
            ::xtypes::DynamicType::Ptr inner = convert_type(descriptor->get_element_type().get());
            return ::xtypes::ArrayType(*inner.get(), descriptor->get_bounds());
        }
        case ::eprosima::fastrtps::types::TK_MAP:
        {
            // TODO
            break;
        }
        // Constructed/Named types
        case ::eprosima::fastrtps::types::TK_ALIAS:
        {
            // TODO
            break;
        }
        // Enumerated TKs
        case ::eprosima::fastrtps::types::TK_ENUM:
        {
            // TODO
            break;
        }
        case ::eprosima::fastrtps::types::TK_BITMASK:
        {
            // TODO
            break;
        }
        // Structured TKs
        case ::eprosima::fastrtps::types::TK_ANNOTATION:
        {
            // TODO
            break;
        }
        case ::eprosima::fastrtps::types::TK_STRUCTURE:
        {
            using ::eprosima::fastrtps::types::DynamicTypeMember;
            ::xtypes::StructType struct_type(descriptor->get_name());
            // TODO inheritance.
            // type.parent(convert_type(descriptor->get_base_type().get()));

            std::map<MemberId, DynamicTypeMember*> members_map;
            // It lacks a const version.
            const_cast<DynamicType*>(type)->get_all_members(members_map);

            for (auto& it : members_map)
            {
                struct_type.add_member(
                    it.second->get_descriptor()->get_name(),
                    *convert_type(it.second->get_descriptor()->get_type().get()).get());
            }
            return struct_type;
        }
        case ::eprosima::fastrtps::types::TK_UNION:
        {
            // TODO
            break;
        }
        case ::eprosima::fastrtps::types::TK_BITSET:
        {
            // TODO
            break;
        }
    }

    return ::xtypes::DynamicType::Ptr();
}

} // namespace dds
} // namespace soss

