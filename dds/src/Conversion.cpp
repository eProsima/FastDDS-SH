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
#include <sstream>

using eprosima::fastrtps::types::MemberDescriptor;
using eprosima::fastrtps::types::MemberId;

namespace soss {
namespace dds {

using namespace eprosima::fastrtps;

bool Conversion::soss_to_dds(
        const ::xtypes::DynamicData& input,
        DynamicData* output)
{
    std::stringstream ss;
    input.for_each([&output, &ss](const ::xtypes::DynamicData::ReadableNode& node)
       {
           switch (node.data().type().kind())
           {
                case ::xtypes::TypeKind::CHAR_8_TYPE:
                    output->set_char8_value(node.data().value<char>());
                    break;
                case ::xtypes::TypeKind::CHAR_16_TYPE:
                    output->set_char16_value(node.data().value<char32_t>());
                    break;
                case ::xtypes::TypeKind::UINT_8_TYPE:
                    output->set_uint8_value(node.data().value<uint8_t>());
                    break;
                case ::xtypes::TypeKind::INT_16_TYPE:
                    output->set_int16_value(node.data().value<int16_t>());
                    break;
                case ::xtypes::TypeKind::UINT_16_TYPE:
                    output->set_uint16_value(node.data().value<uint16_t>());
                    break;
                case ::xtypes::TypeKind::INT_32_TYPE:
                    output->set_int32_value(node.data().value<int32_t>());
                    break;
                case ::xtypes::TypeKind::UINT_32_TYPE:
                    output->set_uint32_value(node.data().value<uint32_t>());
                    break;
                case ::xtypes::TypeKind::INT_64_TYPE:
                    output->set_int64_value(node.data().value<int64_t>());
                    break;
                case ::xtypes::TypeKind::UINT_64_TYPE:
                    output->set_uint64_value(node.data().value<uint64_t>());
                    break;
                case ::xtypes::TypeKind::FLOAT_32_TYPE:
                    output->set_float32_value(node.data().value<float>());
                    break;
                case ::xtypes::TypeKind::FLOAT_64_TYPE:
                    output->set_float64_value(node.data().value<double>());
                    break;
                case ::xtypes::TypeKind::FLOAT_128_TYPE:
                    output->set_float128_value(node.data().value<long double>());
                    break;
                case ::xtypes::TypeKind::STRING_TYPE:
                    output->set_string_value(node.data().value<std::string>());
                    break;
                case ::xtypes::TypeKind::WSTRING_TYPE:
                {
                    output->set_wstring_value(node.data().value<std::wstring>());
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
                    uint32_t id = output->get_member_id_by_name(node.from_member()->name());
                    DynamicData* nested_msg_dds = output->loan_value(id);
                    if (nested_msg_dds != nullptr)
                    {
                        ::xtypes::DynamicData nested_msg_soss = node.data();

                        if (soss_to_dds(nested_msg_soss, nested_msg_dds))
                        {
                            //ret = types::RETCODE_OK;
                        }

                        output->return_loaned_value(nested_msg_dds);
                    }
                    else
                    {
                        ss << "Cannot find member " << node.from_member()->name() << " in type " << node.type().name();
                        throw DDSMiddlewareException(ss.str());
                    }
                    break;
                }
                default:
                    ss << "Unsupported type: " << node.type().name();
                    throw DDSMiddlewareException(ss.str());

           }
       });

    return true;
}

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
                    output.value<bool>(value);
                }
                else if (member_type == types::TK_BYTE)
                {
                    uint8_t value;
                    ret = input->get_byte_value(value, id);
                    output.value<uint8_t>(value);
                }
                else if (member_type == types::TK_INT16)
                {
                    int16_t value;
                    ret = input->get_int16_value(value, id);
                    output.value<int16_t>(value);
                }
                else if (member_type == types::TK_INT32)
                {
                    int32_t value;
                    ret = input->get_int32_value(value, id);
                    output.value<int16_t>(value);
                }
                else if (member_type == types::TK_INT64)
                {
                    int64_t value;
                    ret = input->get_int64_value(value, id);
                    output.value<int64_t>(value);
                }
                else if (member_type == types::TK_UINT16)
                {
                    uint16_t value;
                    ret = input->get_uint16_value(value, id);
                    output.value<uint16_t>(value);
                }
                else if (member_type == types::TK_UINT32)
                {
                    uint32_t value;
                    ret = input->get_uint32_value(value, id);
                    output.value<uint32_t>(value);
                }
                else if (member_type == types::TK_UINT64)
                {
                    uint64_t value;
                    ret = input->get_uint64_value(value, id);
                    output.value<uint64_t>(value);
                }
                else if (member_type == types::TK_FLOAT32)
                {
                    float value;
                    ret = input->get_float32_value(value, id);
                    output.value<float>(value);
                }
                else if (member_type == types::TK_FLOAT64)
                {
                    double value;
                    ret = input->get_float64_value(value, id);
                    output.value<double>(value);
                }
                else if (member_type == types::TK_FLOAT128)
                {
                    long double value;
                    ret = input->get_float128_value(value, id);
                    output.value<long double>(value);
                }
                else if (member_type == types::TK_CHAR8)
                {
                    char value;
                    ret = input->get_char8_value(value, id);
                    output.value<char>(value);
                }
                else if (member_type == types::TK_CHAR16)
                {
                    wchar_t value;
                    ret = input->get_char16_value(value, id);
                    output.value<wchar_t>(value);
                }
                else if (member_type == types::TK_STRING8)
                {
                    std::string value;
                    ret = input->get_string_value(value, id);
                    output.value<std::string>(value);
                }
                else if (member_type == types::TK_STRING16)
                {
                    std::wstring value;
                    ret = input->get_wstring_value(value, id);
                    output.value<std::wstring>(value);
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
                    ::xtypes::DynamicData nested_msg_soss = output[id];

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


} // namespace dds
} // namespace soss

