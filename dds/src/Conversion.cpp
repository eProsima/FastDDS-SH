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
std::map<std::string, DynamicTypeBuilder_ptr> Conversion::builders_;

void Conversion::set_primitive_data(
        const xtypes::DynamicData& from,
        DynamicData* to)
{
    switch (from.type().kind())
    {
        // TODO 0 or MEMBER_ID_INVALID?
        case ::xtypes::TypeKind::BOOLEAN_TYPE:
            to->set_bool_value(from.value<bool>(), 0);
            break;
        case ::xtypes::TypeKind::CHAR_8_TYPE:
            to->set_char8_value(from.value<char>(), 0);
            break;
        case ::xtypes::TypeKind::CHAR_16_TYPE:
            to->set_char16_value(from.value<char32_t>(), 0);
            break;
        case ::xtypes::TypeKind::UINT_8_TYPE:
            to->set_uint8_value(from.value<uint8_t>(), 0);
            break;
        case ::xtypes::TypeKind::INT_8_TYPE:
            to->set_int8_value(from.value<int8_t>(), 0);
            break;
        case ::xtypes::TypeKind::INT_16_TYPE:
            to->set_int16_value(from.value<int16_t>(), 0);
            break;
        case ::xtypes::TypeKind::UINT_16_TYPE:
            to->set_uint16_value(from.value<uint16_t>(), 0);
            break;
        case ::xtypes::TypeKind::INT_32_TYPE:
            to->set_int32_value(from.value<int32_t>(), 0);
            break;
        case ::xtypes::TypeKind::UINT_32_TYPE:
            to->set_uint32_value(from.value<uint32_t>(), 0);
            break;
        case ::xtypes::TypeKind::INT_64_TYPE:
            to->set_int64_value(from.value<int64_t>(), 0);
            break;
        case ::xtypes::TypeKind::UINT_64_TYPE:
            to->set_uint64_value(from.value<uint64_t>(), 0);
            break;
        case ::xtypes::TypeKind::FLOAT_32_TYPE:
            to->set_float32_value(from.value<float>(), 0);
            break;
        case ::xtypes::TypeKind::FLOAT_64_TYPE:
            to->set_float64_value(from.value<double>(), 0);
            break;
        case ::xtypes::TypeKind::FLOAT_128_TYPE:
            to->set_float128_value(from.value<long double>(), 0);
            break;
        case ::xtypes::TypeKind::STRING_TYPE:
            to->set_string_value(from.value<std::string>(), 0);
            break;
        case ::xtypes::TypeKind::WSTRING_TYPE:
        {
            to->set_wstring_value(from.value<std::wstring>(), 0);
            break;
        }
        default:
        {
            std::stringstream ss;
            ss << "Expected primitive data, but found " << from.type().name();
            throw DDSMiddlewareException(ss.str());
        }
    }
}

void Conversion::set_array_data(
        const xtypes::DynamicData& from,
        DynamicData* to,
        const std::vector<uint32_t>& indexes)
{
    const xtypes::ArrayType& type = static_cast<const xtypes::ArrayType&>(from.type());
    const xtypes::DynamicType& inner_type = type.content_type();
    DynamicDataFactory* factory = DynamicDataFactory::get_instance();

    for (uint32_t idx = 0; idx < from.size(); ++idx)
    {
        std::vector<uint32_t> new_indexes = indexes;
        new_indexes.push_back(idx);
        MemberId id = to->get_array_index(new_indexes);
        switch (inner_type.kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
                to->set_bool_value(from[idx].value<bool>(), id);
                break;
            case ::xtypes::TypeKind::CHAR_8_TYPE:
                to->set_char8_value(from[idx].value<char>(), id);
                break;
            case ::xtypes::TypeKind::CHAR_16_TYPE:
                to->set_char16_value(from[idx].value<char32_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_8_TYPE:
                to->set_uint8_value(from[idx].value<uint8_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_8_TYPE:
                to->set_int8_value(from[idx].value<int8_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_16_TYPE:
                to->set_int16_value(from[idx].value<int16_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_16_TYPE:
                to->set_uint16_value(from[idx].value<uint16_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_32_TYPE:
                to->set_int32_value(from[idx].value<int32_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_32_TYPE:
                to->set_uint32_value(from[idx].value<uint32_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_64_TYPE:
                to->set_int64_value(from[idx].value<int64_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_64_TYPE:
                to->set_uint64_value(from[idx].value<uint64_t>(), id);
                break;
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
                to->set_float32_value(from[idx].value<float>(), id);
                break;
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
                to->set_float64_value(from[idx].value<double>(), id);
                break;
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
                to->set_float128_value(from[idx].value<long double>(), id);
                break;
            case ::xtypes::TypeKind::STRING_TYPE:
                to->set_string_value(from[idx].value<std::string>(), id);
                break;
            case ::xtypes::TypeKind::WSTRING_TYPE:
                to->set_wstring_value(from[idx].value<std::wstring>(), id);
                break;
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                set_array_data(from[idx], to, new_indexes);
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner sequence builder
                DynamicData* seq_data = factory->create_data(builder.get());
                set_sequence_data(from[idx], seq_data);
                to->set_complex_value(seq_data, id);
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner struct builder
                DynamicData* st_data = factory->create_data(builder.get());
                soss_to_dds(from[idx], st_data);
                to->set_complex_value(st_data, id);
                break;
            }
            default:
            {
                std::stringstream ss;
                ss << "Unexpected data type: " << from.type().name() << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }
    }
}

void Conversion::set_sequence_data(
        const xtypes::DynamicData& from,
        DynamicData* to)
{
    const xtypes::SequenceType& type = static_cast<const xtypes::SequenceType&>(from.type());
    MemberId id;
    DynamicDataFactory* factory = DynamicDataFactory::get_instance();
    for (uint32_t idx = 0; idx < from.size(); ++idx)
    {
        switch (type.content_type().kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
                to->insert_bool_value(from[idx].value<bool>(), id);
                break;
            case ::xtypes::TypeKind::CHAR_8_TYPE:
                to->insert_char8_value(from[idx].value<char>(), id);
                break;
            case ::xtypes::TypeKind::CHAR_16_TYPE:
                to->insert_char16_value(from[idx].value<char32_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_8_TYPE:
                //to->insert_uint8_value(from[idx].value<uint8_t>(), id);
                to->insert_byte_value(from[idx].value<uint8_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_8_TYPE:
                //to->insert_int8_value(from[idx].value<int8_t>(), id);
                to->insert_byte_value(from[idx].value<int8_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_16_TYPE:
                to->insert_int16_value(from[idx].value<int16_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_16_TYPE:
                to->insert_uint16_value(from[idx].value<uint16_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_32_TYPE:
                to->insert_int32_value(from[idx].value<int32_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_32_TYPE:
                to->insert_uint32_value(from[idx].value<uint32_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_64_TYPE:
                to->insert_int64_value(from[idx].value<int64_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_64_TYPE:
                to->insert_uint64_value(from[idx].value<uint64_t>(), id);
                break;
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
                to->insert_float32_value(from[idx].value<float>(), id);
                break;
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
                to->insert_float64_value(from[idx].value<double>(), id);
                break;
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
                to->insert_float128_value(from[idx].value<long double>(), id);
                break;
            case ::xtypes::TypeKind::STRING_TYPE:
                to->insert_string_value(from[idx].value<std::string>(), id);
                break;
            case ::xtypes::TypeKind::WSTRING_TYPE:
                to->insert_wstring_value(from[idx].value<std::wstring>(), id);
                break;
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner array builder
                DynamicData* array_data = factory->create_data(builder.get());
                set_array_data(from[idx], array_data, std::vector<uint32_t>());
                to->insert_complex_value(array_data, id);
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner sequence builder
                DynamicData* seq_data = factory->create_data(builder.get());
                set_sequence_data(from[idx], seq_data);
                to->insert_complex_value(seq_data, id);
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner struct builder
                DynamicData* st_data = factory->create_data(builder.get());
                soss_to_dds(from[idx], st_data);
                to->insert_complex_value(st_data, id);
                break;
            }
            default:
            {
                std::stringstream ss;
                ss << "Unexpected data type: " << from.type().name() << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }
    }
}

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
                case ::xtypes::TypeKind::BOOLEAN_TYPE:
                case ::xtypes::TypeKind::CHAR_8_TYPE:
                case ::xtypes::TypeKind::CHAR_16_TYPE:
                case ::xtypes::TypeKind::UINT_8_TYPE:
                case ::xtypes::TypeKind::INT_8_TYPE:
                case ::xtypes::TypeKind::INT_16_TYPE:
                case ::xtypes::TypeKind::UINT_16_TYPE:
                case ::xtypes::TypeKind::INT_32_TYPE:
                case ::xtypes::TypeKind::UINT_32_TYPE:
                case ::xtypes::TypeKind::INT_64_TYPE:
                case ::xtypes::TypeKind::UINT_64_TYPE:
                case ::xtypes::TypeKind::FLOAT_32_TYPE:
                case ::xtypes::TypeKind::FLOAT_64_TYPE:
                case ::xtypes::TypeKind::FLOAT_128_TYPE:
                case ::xtypes::TypeKind::STRING_TYPE:
                case ::xtypes::TypeKind::WSTRING_TYPE:
                {
                    set_primitive_data(node.data(), current_data);
                    break;
                }
                case ::xtypes::TypeKind::ARRAY_TYPE:
                    set_array_data(node.data(), current_data, std::vector<uint32_t>());
                    break;
                case ::xtypes::TypeKind::SEQUENCE_TYPE:
                    set_sequence_data(node.data(), current_data);
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

void Conversion::set_sequence_data(
        const DynamicData* c_from,
        xtypes::DynamicData& to)
{
    const xtypes::SequenceType& type = static_cast<const xtypes::SequenceType&>(to.type());
    DynamicData* from = const_cast<DynamicData*>(c_from);

    for (uint32_t idx = 0; idx < to.size(); ++idx)
    {
        MemberId id = from->get_member_id_at_index(idx);
        ResponseCode ret = ResponseCode::RETCODE_ERROR;

        switch (type.content_type().kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
            {
                bool value;
                ret = from->get_bool_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::CHAR_8_TYPE:
            {
                char value;
                ret = from->get_char8_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::CHAR_16_TYPE:
            {
                wchar_t value;
                ret = from->get_char16_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_8_TYPE:
            {
                uint8_t value;
                ret = from->get_uint8_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::INT_8_TYPE:
            {
                int8_t value;
                ret = from->get_int8_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::INT_16_TYPE:
            {
                int16_t value;
                ret = from->get_int16_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_16_TYPE:
            {
                uint16_t value;
                ret = from->get_uint16_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::INT_32_TYPE:
            {
                int32_t value;
                ret = from->get_int32_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_32_TYPE:
            {
                uint32_t value;
                ret = from->get_uint32_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::INT_64_TYPE:
            {
                int64_t value;
                ret = from->get_int64_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_64_TYPE:
            {
                uint64_t value;
                ret = from->get_uint64_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
            {
                float value;
                ret = from->get_float32_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
            {
                double value;
                ret = from->get_float64_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
            {
                long double value;
                ret = from->get_float128_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::STRING_TYPE:
            {
                std::string value;
                ret = from->get_string_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::WSTRING_TYPE:
            {
                std::wstring value;
                ret = from->get_wstring_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                DynamicData* array = from->loan_value(id);
                xtypes::DynamicData soss_array(type.content_type());
                set_array_data(array, soss_array, std::vector<uint32_t>());
                from->return_loaned_value(array);
                to.push(soss_array);
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                DynamicData* seq = from->loan_value(id);
                xtypes::DynamicData soss_seq(type.content_type());
                set_sequence_data(seq, soss_seq);
                from->return_loaned_value(seq);
                to.push(soss_seq);
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                DynamicData* st = from->loan_value(id);
                xtypes::DynamicData soss_st(type.content_type());
                dds_to_soss(st, soss_st);
                from->return_loaned_value(st);
                to.push(soss_st);
                break;
            }
            default:
            {
                std::stringstream ss;
                ss << "Unexpected data type: " << to.type().name() << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }

        if (ret != ResponseCode::RETCODE_OK)
        {
            std::stringstream ss;
            ss << "Error parsing from dynamic type '" << to.type().name();
            throw DDSMiddlewareException(ss.str());
        }
    }
}

void Conversion::set_array_data(
        const DynamicData* c_from,
        xtypes::DynamicData& to,
        const std::vector<uint32_t>& indexes)
{
    const xtypes::ArrayType& type = static_cast<const xtypes::ArrayType&>(to.type());
    const xtypes::DynamicType& inner_type = type.content_type();
    DynamicData* from = const_cast<DynamicData*>(c_from);

    for (uint32_t idx = 0; idx < type.dimension(); ++idx)
    {
        std::vector<uint32_t> new_indexes = indexes;
        new_indexes.push_back(idx);
        MemberId id = from->get_array_index(new_indexes);
        ResponseCode ret = ResponseCode::RETCODE_ERROR;
        switch (inner_type.kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
            {
                bool value;
                ret = from->get_bool_value(value, id);
                to[idx].value<bool>(value);
            }
            break;
            case ::xtypes::TypeKind::CHAR_8_TYPE:
            {
                char value;
                ret = from->get_char8_value(value, id);
                to[idx].value<char>(value);
            }
            break;
            case ::xtypes::TypeKind::CHAR_16_TYPE:
            {
                wchar_t value;
                ret = from->get_char16_value(value, id);
                to[idx].value<wchar_t>(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_8_TYPE:
            {
                uint8_t value;
                ret = from->get_uint8_value(value, id);
                to[idx].value<uint8_t>(value);
            }
            break;
            case ::xtypes::TypeKind::INT_8_TYPE:
            {
                int8_t value;
                ret = from->get_int8_value(value, id);
                to[idx].value<int8_t>(value);
            }
            break;
            case ::xtypes::TypeKind::INT_16_TYPE:
            {
                int16_t value;
                ret = from->get_int16_value(value, id);
                to[idx].value<int16_t>(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_16_TYPE:
            {
                uint16_t value;
                ret = from->get_uint16_value(value, id);
                to[idx].value<uint16_t>(value);
            }
            break;
            case ::xtypes::TypeKind::INT_32_TYPE:
            {
                int32_t value;
                ret = from->get_int32_value(value, id);
                to[idx].value<int32_t>(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_32_TYPE:
            {
                uint32_t value;
                ret = from->get_uint32_value(value, id);
                to[idx].value<uint32_t>(value);
            }
            break;
            case ::xtypes::TypeKind::INT_64_TYPE:
            {
                int64_t value;
                ret = from->get_int64_value(value, id);
                to[idx].value<int64_t>(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_64_TYPE:
            {
                uint64_t value;
                ret = from->get_uint64_value(value, id);
                to[idx].value<uint64_t>(value);
            }
            break;
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
            {
                float value;
                ret = from->get_float32_value(value, id);
                to[idx].value<float>(value);
            }
            break;
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
            {
                double value;
                ret = from->get_float64_value(value, id);
                to[idx].value<double>(value);
            }
            break;
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
            {
                long double value;
                ret = from->get_float128_value(value, id);
                to[idx].value<long double>(value);
            }
            break;
            case ::xtypes::TypeKind::STRING_TYPE:
            {
                std::string value;
                ret = from->get_string_value(value, id);
                to[idx].value<std::string>(value);
            }
            break;
            case ::xtypes::TypeKind::WSTRING_TYPE:
            {
                std::wstring value;
                ret = from->get_wstring_value(value, id);
                to[idx].value<std::wstring>(value);
            }
            break;
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                DynamicData* array = from->loan_value(id);
                xtypes::DynamicData soss_array(type.content_type());
                set_array_data(array, soss_array, new_indexes);
                from->return_loaned_value(array);
                to[idx] = soss_array;
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                DynamicData* seq = from->loan_value(id);
                xtypes::DynamicData soss_seq(type.content_type());
                set_sequence_data(seq, soss_seq);
                from->return_loaned_value(seq);
                to[idx] = soss_seq;
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                DynamicData* st = from->loan_value(id);
                xtypes::DynamicData soss_st(type.content_type());
                dds_to_soss(st, soss_st);
                from->return_loaned_value(st);
                to[idx] = soss_st;
                break;
            }
            default:
            {
                std::stringstream ss;
                ss << "Unexpected data type: " << to.type().name() << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }

        if (ret != ResponseCode::RETCODE_OK)
        {
            std::stringstream ss;
            ss << "Error parsing from dynamic type '" << to.type().name();
            throw DDSMiddlewareException(ss.str());
        }
    }

}

// TODO: Can we receive a type without members as root?
bool Conversion::dds_to_soss(
        const DynamicData* c_input,
        ::xtypes::DynamicData& output)
{
    std::stringstream ss;
    uint32_t id = 0;
    uint32_t i = 0;
    MemberDescriptor descriptor;

    // We promise to not modify it, but we need it non-const, so we can call loan_value freely.
    DynamicData* input = const_cast<DynamicData*>(c_input);

    while (id != MEMBER_ID_INVALID)
    {
        id = input->get_member_id_at_index(i);
        ResponseCode ret = ResponseCode::RETCODE_ERROR;

        if (id != MEMBER_ID_INVALID)
        {
            DynamicDataSOSS* dd_soss = static_cast<DynamicDataSOSS*>(input);
            ret = dd_soss->GetDescriptorSOSS(descriptor, id);

            if (ret == ResponseCode::RETCODE_OK)
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
                    DynamicData* array = input->loan_value(id);
                    xtypes::DynamicData data = output[descriptor.get_name()];
                    set_array_data(array, data, std::vector<uint32_t>());
                    input->return_loaned_value(array);
                    output[descriptor.get_name()] = data;
                }
                else if (member_type == types::TK_SEQUENCE)
                {
                    DynamicData* seq = input->loan_value(id);
                    xtypes::DynamicData data = output[descriptor.get_name()];
                    set_sequence_data(seq, data);
                    input->return_loaned_value(seq);
                    output[descriptor.get_name()] = data;
                }
                else if (member_type == types::TK_STRUCTURE)
                {
                    DynamicData* nested_msg_dds = input->loan_value(id);
                    ::xtypes::DynamicData nested_msg_soss = output[descriptor.get_name()][id];

                    if (nested_msg_dds != nullptr)
                    {
                        if (dds_to_soss(nested_msg_dds, nested_msg_soss))
                        {
                            ret = ResponseCode::RETCODE_OK;
                        }
                        input->return_loaned_value(nested_msg_dds);
                    }
                }
                else
                {
                    ret = ResponseCode::RETCODE_ERROR;
                }

                i++;
            }

            if (ret != ResponseCode::RETCODE_OK)
            {
                std::stringstream ss;
                ss << "Error parsing from dynamic type '" << input->get_name();
                //ss << "Error code: " << ret << ".";
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
        std::stringstream ss;
        ss << "Error getting data from dynamic type '" << type_name << " (not registered).";
        throw DDSMiddlewareException(ss.str());
    }

    return ::xtypes::DynamicData(*it->second.get());
}

DynamicTypeBuilder* Conversion::create_builder(
        const xtypes::DynamicType& type)
{
    if (builders_.count(type.name()) > 0)
    {
        /*
        std::cout << "[soss-dds]: Topic '" << topic_name << "' has a type already registered." << std::endl;
        std::cout << "[soss-dds]: Registed type: '" << registered_types_[topic_name]->getName()
                  << "' but trying to register type: '" << type.name() << "'." << std::endl;
        */
        return builders_[type.name()].get();
    }

    DynamicTypeBuilder_ptr builder = get_builder(type);
    builder->set_name(type.name());
    DynamicTypeBuilder* result = builder.get();
    builders_.emplace(type.name(), std::move(builder));
    return result;
}

DynamicTypeBuilder_ptr Conversion::get_builder(
        const xtypes::DynamicType& type)
{
    DynamicTypeBuilderFactory* factory = DynamicTypeBuilderFactory::get_instance();
    switch (type.kind())
    {
        case ::xtypes::TypeKind::BOOLEAN_TYPE:
        {
            return factory->create_bool_builder();
        }
        case ::xtypes::TypeKind::INT_8_TYPE:
        case ::xtypes::TypeKind::UINT_8_TYPE:
        {
            return factory->create_byte_builder();
        }
        case ::xtypes::TypeKind::INT_16_TYPE:
        {
            return factory->create_int16_builder();
        }
        case ::xtypes::TypeKind::UINT_16_TYPE:
        {
            return factory->create_uint16_builder();
        }
        case ::xtypes::TypeKind::INT_32_TYPE:
        {
            return factory->create_int32_builder();
        }
        case ::xtypes::TypeKind::UINT_32_TYPE:
        {
            return factory->create_uint32_builder();
        }
        case ::xtypes::TypeKind::INT_64_TYPE:
        {
            return factory->create_int64_builder();
        }
        case ::xtypes::TypeKind::UINT_64_TYPE:
        {
            return factory->create_uint64_builder();
        }
        case ::xtypes::TypeKind::FLOAT_32_TYPE:
        {
            return factory->create_float32_builder();
        }
        case ::xtypes::TypeKind::FLOAT_64_TYPE:
        {
            return factory->create_float64_builder();
        }
        case ::xtypes::TypeKind::FLOAT_128_TYPE:
        {
            return factory->create_float128_builder();
        }
        case ::xtypes::TypeKind::CHAR_8_TYPE:
        {
            return factory->create_char8_builder();
        }
        case ::xtypes::TypeKind::CHAR_16_TYPE:
        {
            return factory->create_char16_builder();
        }
        case ::xtypes::TypeKind::ENUMERATION_TYPE:
        {
            // TODO
        }
        case ::xtypes::TypeKind::BITSET_TYPE:
        {
            // TODO
        }
        case ::xtypes::TypeKind::ALIAS_TYPE:
        {
            // TODO
        }
        /*
        case ::xtypes::TypeKind::BITMASK_TYPE:
        {
            // TODO
        }
        */
        case ::xtypes::TypeKind::ARRAY_TYPE:
        {
            const xtypes::ArrayType& c_type = static_cast<const xtypes::ArrayType&>(type);
            std::pair<std::vector<uint32_t>, DynamicTypeBuilder_ptr> pair;
            get_array_specs(c_type, pair);
            DynamicTypeBuilder_ptr result = factory->create_array_builder(pair.second->build(), pair.first);
            return result;
        }
        case ::xtypes::TypeKind::SEQUENCE_TYPE:
        {
            const xtypes::SequenceType& c_type = static_cast<const xtypes::SequenceType&>(type);
            DynamicTypeBuilder_ptr content = get_builder(c_type.content_type());
            DynamicTypeBuilder_ptr result = factory->create_sequence_builder(content->build(), c_type.bounds());
            return result;
        }
        case ::xtypes::TypeKind::STRING_TYPE:
        {
            const xtypes::StringType& c_type = static_cast<const xtypes::StringType&>(type);
            return factory->create_string_builder(c_type.bounds());
        }
        case ::xtypes::TypeKind::WSTRING_TYPE:
        {
            const xtypes::WStringType& c_type = static_cast<const xtypes::WStringType&>(type);
            return factory->create_wstring_builder(c_type.bounds());
        }
        case ::xtypes::TypeKind::MAP_TYPE:
        {
            // TODO
        }
        case ::xtypes::TypeKind::UNION_TYPE:
        {
            // TODO
        }
        case ::xtypes::TypeKind::STRUCTURE_TYPE:
        {
            DynamicTypeBuilder_ptr result = factory->create_struct_builder();
            const xtypes::StructType& from = static_cast<const xtypes::StructType&>(type);

            for (size_t idx = 0; idx < from.members().size(); ++idx)
            {
                const xtypes::Member& member = from.member(idx);
                DynamicTypeBuilder_ptr member_builder = get_builder(member.type());
                result->add_member(idx, member.name(), member_builder->build());
            }
            return result;
        }
        default:
            break;
    }
    return nullptr;
}

void Conversion::get_array_specs(
        const xtypes::ArrayType& array,
        std::pair<std::vector<uint32_t>, DynamicTypeBuilder_ptr>& result)
{
    result.first.push_back(array.dimension());
    if (array.content_type().kind() == ::xtypes::TypeKind::ARRAY_TYPE)
    {
        get_array_specs(static_cast<const xtypes::ArrayType&>(array.content_type()), result);
    }
    else
    {
        result.second = get_builder(array.content_type());
    }
}
} // namespace dds
} // namespace soss

