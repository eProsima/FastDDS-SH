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
#include <soss/utilities.hpp>
#include <fastrtps/types/MemberDescriptor.h>

using eprosima::fastrtps::types::ResponseCode;
using eprosima::fastrtps::types::MemberDescriptor;
using eprosima::fastrtps::types::MemberId;

// Due to a problem with Fast-RTPS library, which by now does not export the "GetDescriptor" function for windows,
// this workaround is needed in order to access the protected member "mDescriptors".
// This problem will be solved in future fixes, but for now this workaround is the only way of introspecting
// dynamic types in windows.
class DynamicDataSOSS : public eprosima::fastrtps::types::DynamicData
{
public:
    ResponseCode GetDescriptorSOSS(MemberDescriptor& value, MemberId id) const
    {
        auto it = mDescriptors.find(id);
        if (it != mDescriptors.end())
        {
            value.CopyFrom(it->second);
            return ResponseCode::RETCODE_OK;
        }
        else
        {
            std::cerr << "Error getting MemberDescriptor. MemberId not found." << std::endl;
            return ResponseCode::RETCODE_BAD_PARAMETER;
        }
    }
};


namespace soss {
namespace dds {

bool Conversion::soss_to_dds(
        const soss::Message& input,
        types::DynamicData* output)
{
    for (auto it = input.data.begin(); it != input.data.end() ;it++)
    {
        std::string soss_name = it->first;
        std::string soss_type = it->second.type();
        types::ResponseCode ret = types::RETCODE_ERROR;

        types::MemberDescriptor descriptor;
        DynamicDataSOSS* dd_soss = static_cast<DynamicDataSOSS*>(output);
        uint32_t id = output->GetMemberIdByName(soss_name);
        ret = dd_soss->GetDescriptorSOSS(descriptor, id);

        types::TypeKind dds_type = descriptor.GetKind();

        if (dds_type == types::TK_BOOLEAN)
        {
            bool val;
            soss::Convert<bool>::from_soss_field(it, val);
            ret = output->SetBoolValue(val, id);
        }
        else if (dds_type == types::TK_BYTE)
        {
            u_char val;
            soss::Convert<u_char>::from_soss_field(it, val);
            ret = output->SetByteValue(val, id);
        }
        else if (dds_type == types::TK_UINT16)
        {
            uint16_t val;
            soss::Convert<uint16_t>::from_soss_field(it, val);
            ret = output->SetUint16Value(val, id);
        }
        else if (dds_type == types::TK_UINT32)
        {
            uint32_t val;
            soss::Convert<uint32_t>::from_soss_field(it, val);
            ret = output->SetUint32Value(val, id);
        }
        else if (dds_type == types::TK_UINT64)
        {
            uint64_t val;
            soss::Convert<uint64_t>::from_soss_field(it, val);
            ret = output->SetUint64Value(val, id);
        }
        else if (dds_type == types::TK_INT16)
        {
            int16_t val;
            soss::Convert<int16_t>::from_soss_field(it, val);
            ret = output->SetInt16Value(val, id);
        }
        else if (dds_type == types::TK_INT32)
        {
            int32_t val;
            soss::Convert<int32_t>::from_soss_field(it, val);
            ret = output->SetInt32Value(val, id);
        }
        else if (dds_type == types::TK_INT64)
        {
            int64_t val;
            soss::Convert<int64_t>::from_soss_field(it, val);
            ret = output->SetInt64Value(val, id);
        }
        else if (dds_type == types::TK_FLOAT32)
        {
            float val;
            soss::Convert<float>::from_soss_field(it, val);
            ret = output->SetFloat32Value(val, id);
        }
        else if (dds_type == types::TK_FLOAT64)
        {
            double val;
            soss::Convert<double>::from_soss_field(it, val);
            ret = output->SetFloat64Value(val, id);
        }
        else if (dds_type == types::TK_FLOAT128)
        {
            long double val;
            soss::Convert<long double>::from_soss_field(it, val);
            ret = output->SetFloat128Value(val, id);
        }
        else if (dds_type == types::TK_CHAR8)
        {
            char val;
            soss::Convert<char>::from_soss_field(it, val);
            ret = output->SetChar8Value(val, id);
        }
        else if (dds_type == types::TK_CHAR16)
        {
            wchar_t val;
            soss::Convert<wchar_t>::from_soss_field(it, val);
            ret = output->SetChar16Value(val, id);
        }
        else if (dds_type == types::TK_STRING8)
        {
            std::string val;
            soss::Convert<std::string>::from_soss_field(it, val);
            ret = output->SetStringValue(val, id);
        }
        else if (dds_type == types::TK_STRUCTURE)
        {
            types::DynamicData* nested_msg_dds = output->LoanValue(id);
            if (nested_msg_dds != nullptr)
            {
                const soss::Message* nested_msg_soss = it->second.cast<soss::Message>();

                if (soss_to_dds(*nested_msg_soss, nested_msg_dds))
                {
                    ret = types::RETCODE_OK;
                }

                output->ReturnLoanedValue(nested_msg_dds);
            }
        }

        if (ret != types::RETCODE_OK)
        {
            std::stringstream ss;
            ss << "Error parsing from soss message '" << output->GetName() << "' in member '" << soss_name << "'. ";
            ss << "Error code: " << ret << ".";
            throw DDSMiddlewareException(ss.str());
        }
    }

    return true;
}

bool Conversion::dds_to_soss(
        const std::string type,
        types::DynamicData* input,
        soss::Message& output)
{
    uint32_t id = 0;
    uint32_t i = 0;
    types::MemberDescriptor descriptor;

    output.type = type;

    while (id != MEMBER_ID_INVALID)
    {
        id = input->GetMemberIdAtIndex(i);
        types::ResponseCode ret = types::RETCODE_ERROR;

        if (id != MEMBER_ID_INVALID)
        {
            soss::Field field;

            DynamicDataSOSS* dd_soss = static_cast<DynamicDataSOSS*>(input);
            ret = dd_soss->GetDescriptorSOSS(descriptor, id);

            if (ret == types::RETCODE_OK)
            {
                types::TypeKind member_type = descriptor.GetKind();

                if (member_type == types::TK_BOOLEAN)
                {
                    bool value;
                    ret = input->GetBoolValue(value, id);
                    field.set<bool>(std::move(value));
                }
                else if (member_type == types::TK_BYTE)
                {
                    u_char value;
                    ret = input->GetByteValue(value, id);
                    field.set<u_char>(std::move(value));
                }
                else if (member_type == types::TK_INT16)
                {
                    int16_t value;
                    ret = input->GetInt16Value(value, id);
                    field.set<int64_t>(std::move(value));
                }
                else if (member_type == types::TK_INT32)
                {
                    int32_t value;
                    ret = input->GetInt32Value(value, id);
                    field.set<int64_t>(std::move(value));
                }
                else if (member_type == types::TK_INT64)
                {
                    int64_t value;
                    ret = input->GetInt64Value(value, id);
                    field.set<int64_t>(std::move(value));
                }
                else if (member_type == types::TK_UINT16)
                {
                    uint16_t value;
                    ret = input->GetUint16Value(value, id);
                    field.set<uint64_t>(std::move(value));
                }
                else if (member_type == types::TK_UINT32)
                {
                    uint32_t value;
                    ret = input->GetUint32Value(value, id);
                    field.set<uint64_t>(std::move(value));
                }
                else if (member_type == types::TK_UINT64)
                {
                    uint64_t value;
                    ret = input->GetUint64Value(value, id);
                    field.set<uint64_t>(std::move(value));
                }
                else if (member_type == types::TK_FLOAT32)
                {
                    float value;
                    ret = input->GetFloat32Value(value, id);
                    field.set<double>(static_cast<double>(std::move(value)));
                }
                else if (member_type == types::TK_FLOAT64)
                {
                    double value;
                    ret = input->GetFloat64Value(value, id);
                    field.set<double>(std::move(value));
                }
                else if (member_type == types::TK_FLOAT128)
                {
                    long double value;
                    ret = input->GetFloat128Value(value, id);
                    field.set<double>(static_cast<double>(std::move(value)));
                }
                else if (member_type == types::TK_CHAR8)
                {
                    char value;
                    ret = input->GetChar8Value(value, id);
                    field.set<char>(std::move(value));
                }
                else if (member_type == types::TK_CHAR16)
                {
                    wchar_t value;
                    ret = input->GetChar16Value(value, id);
                    field.set<wchar_t>(std::move(value));
                }
                else if (member_type == types::TK_STRING8)
                {
                    std::string value;
                    ret = input->GetStringValue(value, id);
                    field.set<std::string>(std::move(value));
                }
                else if (member_type == types::TK_STRING16)
                {
                    std::wstring value;
                    ret = input->GetWstringValue(value, id);
                    field.set<std::wstring>(std::move(value));
                }
                else if (member_type == types::TK_STRUCTURE)
                {
                    types::DynamicData* nested_msg_dds = input->LoanValue(id);
                    soss::Message nested_msg_soss;

                    if (nested_msg_dds != nullptr)
                    {
                        if (dds_to_soss(nested_msg_dds->GetName(), nested_msg_dds, nested_msg_soss))
                        {
                            ret = types::RETCODE_OK;
                        }
                        input->ReturnLoanedValue(nested_msg_dds);
                    }

                    if (ret == types::RETCODE_OK)
                    {
                        output.data[descriptor.GetName()] = soss::make_field<soss::Message>(nested_msg_soss);
                    }
                }
                else
                {
                    ret = types::RETCODE_ERROR;
                }

                if (ret == types::RETCODE_OK &&
                    member_type != types::TK_STRUCTURE)
                {
                    output.data[descriptor.GetName()] = std::move(field);
                }

                i++;
            }

            if (ret != types::RETCODE_OK)
            {
                std::stringstream ss;
                ss << "Error parsing from dynamic type '" << input->GetName();
                ss << "Error code: " << ret << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }
    }

    return true;
}


} // namespace dds
} // namespace soss

