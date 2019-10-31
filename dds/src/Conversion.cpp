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
        const ::xtypes::DynamicData& /*input*/,
        DynamicData* /*output*/)
{
    /*
    for (auto it = input.data.begin(); it != input.data.end() ;it++)
    {
        std::string soss_name = it->first;
        std::string soss_type = it->second.type();
        ResponseCode ret = types::RETCODE_ERROR;

        MemberDescriptor descriptor;
        DynamicDataSOSS* dd_soss = static_cast<DynamicDataSOSS*>(output);
        uint32_t id = output->get_member_id_by_name(soss_name);
        ret = dd_soss->GetDescriptorSOSS(descriptor, id);

        types::TypeKind dds_type = descriptor.get_kind();

        if (dds_type == types::TK_BOOLEAN)
        {
            bool val;
            soss::Convert<bool>::from_soss_field(it, val);
            ret = output->set_bool_value(val, id);
        }
        else if (dds_type == types::TK_BYTE)
        {
            uint8_t val;
            soss::Convert<uint8_t>::from_soss_field(it, val);
            ret = output->set_byte_value(val, id);
        }
        else if (dds_type == types::TK_UINT16)
        {
            uint16_t val;
            soss::Convert<uint16_t>::from_soss_field(it, val);
            ret = output->set_uint16_value(val, id);
        }
        else if (dds_type == types::TK_UINT32)
        {
            uint32_t val;
            soss::Convert<uint32_t>::from_soss_field(it, val);
            ret = output->set_uint32_value(val, id);
        }
        else if (dds_type == types::TK_UINT64)
        {
            uint64_t val;
            soss::Convert<uint64_t>::from_soss_field(it, val);
            ret = output->set_uint64_value(val, id);
        }
        else if (dds_type == types::TK_INT16)
        {
            int16_t val;
            soss::Convert<int16_t>::from_soss_field(it, val);
            ret = output->set_int16_value(val, id);
        }
        else if (dds_type == types::TK_INT32)
        {
            int32_t val;
            soss::Convert<int32_t>::from_soss_field(it, val);
            ret = output->set_int32_value(val, id);
        }
        else if (dds_type == types::TK_INT64)
        {
            int64_t val;
            soss::Convert<int64_t>::from_soss_field(it, val);
            ret = output->set_int64_value(val, id);
        }
        else if (dds_type == types::TK_FLOAT32)
        {
            float val;
            soss::Convert<float>::from_soss_field(it, val);
            ret = output->set_float32_value(val, id);
        }
        else if (dds_type == types::TK_FLOAT64)
        {
            double val;
            soss::Convert<double>::from_soss_field(it, val);
            ret = output->set_float64_value(val, id);
        }
        else if (dds_type == types::TK_FLOAT128)
        {
            long double val;
            soss::Convert<long double>::from_soss_field(it, val);
            ret = output->set_float128_value(val, id);
        }
        else if (dds_type == types::TK_CHAR8)
        {
            char val;
            soss::Convert<char>::from_soss_field(it, val);
            ret = output->set_char8_value(val, id);
        }
        else if (dds_type == types::TK_CHAR16)
        {
            wchar_t val;
            soss::Convert<wchar_t>::from_soss_field(it, val);
            ret = output->set_char16_value(val, id);
        }
        else if (dds_type == types::TK_STRING8)
        {
            std::string val;
            soss::Convert<std::string>::from_soss_field(it, val);
            ret = output->set_string_value(val, id);
        }
        else if (dds_type == types::TK_STRUCTURE)
        {
            DynamicData* nested_msg_dds = output->loan_value(id);
            if (nested_msg_dds != nullptr)
            {
                const soss::Message* nested_msg_soss = it->second.cast<soss::Message>();

                if (soss_to_dds(*nested_msg_soss, nested_msg_dds))
                {
                    ret = types::RETCODE_OK;
                }

                output->return_loaned_value(nested_msg_dds);
            }
        }

        if (ret != types::RETCODE_OK)
        {
            std::stringstream ss;
            ss << "Error parsing from soss message '" << output->get_name() << "' in member '" << soss_name << "'. ";
            ss << "Error code: " << ret << ".";
            throw DDSMiddlewareException(ss.str());
        }
    }

    return true;
    */
    return false;
}

bool Conversion::dds_to_soss(
        const std::string /*type*/,
        DynamicData* /*input*/,
        ::xtypes::DynamicData& /*output*/)
{
    /*
    uint32_t id = 0;
    uint32_t i = 0;
    MemberDescriptor descriptor;

    output.type = type;

    while (id != MEMBER_ID_INVALID)
    {
        id = input->get_member_id_at_index(i);
        ResponseCode ret = types::RETCODE_ERROR;

        if (id != MEMBER_ID_INVALID)
        {
            soss::Field field;

            DynamicDataSOSS* dd_soss = static_cast<DynamicDataSOSS*>(input);
            ret = dd_soss->GetDescriptorSOSS(descriptor, id);

            if (ret == types::RETCODE_OK)
            {
                types::TypeKind member_type = descriptor.get_kind();

                if (member_type == types::TK_BOOLEAN)
                {
                    bool value;
                    ret = input->get_bool_value(value, id);
                    field.set<bool>(std::move(value));
                }
                else if (member_type == types::TK_BYTE)
                {
                    uint8_t value;
                    ret = input->get_byte_value(value, id);
                    field.set<uint8_t>(std::move(value));
                }
                else if (member_type == types::TK_INT16)
                {
                    int16_t value;
                    ret = input->get_int16_value(value, id);
                    field.set<int64_t>(std::move(value));
                }
                else if (member_type == types::TK_INT32)
                {
                    int32_t value;
                    ret = input->get_int32_value(value, id);
                    field.set<int64_t>(std::move(value));
                }
                else if (member_type == types::TK_INT64)
                {
                    int64_t value;
                    ret = input->get_int64_value(value, id);
                    field.set<int64_t>(std::move(value));
                }
                else if (member_type == types::TK_UINT16)
                {
                    uint16_t value;
                    ret = input->get_uint16_value(value, id);
                    field.set<uint64_t>(std::move(value));
                }
                else if (member_type == types::TK_UINT32)
                {
                    uint32_t value;
                    ret = input->get_uint32_value(value, id);
                    field.set<uint64_t>(std::move(value));
                }
                else if (member_type == types::TK_UINT64)
                {
                    uint64_t value;
                    ret = input->get_uint64_value(value, id);
                    field.set<uint64_t>(std::move(value));
                }
                else if (member_type == types::TK_FLOAT32)
                {
                    float value;
                    ret = input->get_float32_value(value, id);
                    field.set<double>(static_cast<double>(std::move(value)));
                }
                else if (member_type == types::TK_FLOAT64)
                {
                    double value;
                    ret = input->get_float64_value(value, id);
                    field.set<double>(std::move(value));
                }
                else if (member_type == types::TK_FLOAT128)
                {
                    long double value;
                    ret = input->get_float128_value(value, id);
                    field.set<double>(static_cast<double>(std::move(value)));
                }
                else if (member_type == types::TK_CHAR8)
                {
                    char value;
                    ret = input->get_char8_value(value, id);
                    field.set<char>(std::move(value));
                }
                else if (member_type == types::TK_CHAR16)
                {
                    wchar_t value;
                    ret = input->get_char16_value(value, id);
                    field.set<wchar_t>(std::move(value));
                }
                else if (member_type == types::TK_STRING8)
                {
                    std::string value;
                    ret = input->get_string_value(value, id);
                    field.set<std::string>(std::move(value));
                }
                else if (member_type == types::TK_STRING16)
                {
                    std::wstring value;
                    ret = input->get_wstring_value(value, id);
                    field.set<std::wstring>(std::move(value));
                }
                else if (member_type == types::TK_STRUCTURE)
                {
                    DynamicData* nested_msg_dds = input->loan_value(id);
                    soss::Message nested_msg_soss;

                    if (nested_msg_dds != nullptr)
                    {
                        if (dds_to_soss(nested_msg_dds->get_name(), nested_msg_dds, nested_msg_soss))
                        {
                            ret = types::RETCODE_OK;
                        }
                        input->return_loaned_value(nested_msg_dds);
                    }

                    if (ret == types::RETCODE_OK)
                    {
                        output.data[descriptor.get_name()] = soss::make_field<soss::Message>(nested_msg_soss);
                    }
                }
                else
                {
                    ret = types::RETCODE_ERROR;
                }

                if (ret == types::RETCODE_OK &&
                    member_type != types::TK_STRUCTURE)
                {
                    output.data[descriptor.get_name()] = std::move(field);
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
    */
    return false;
}


} // namespace dds
} // namespace soss

