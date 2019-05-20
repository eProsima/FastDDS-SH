/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
#include "fastrtps/types/DynamicTypeBuilderFactory.h"

namespace soss {
namespace dds {


bool Conversion::soss_to_dds(
        const soss::Message& input,
        types::DynamicData_ptr output)
{
    for(auto it : input.data)
    {
        std::string name = it.first;
        uint32_t id = output->GetMemberIdByName(name);
        std::string type = it.second.type();

        if (type == typeid(bool).name())
        {
            output->SetBoolValue(*it.second.cast<bool>(), id);
        }
        else if (type == typeid(u_char).name())
        {
            output->SetByteValue(*it.second.cast<u_char>(), id);
        }
        else if (type == typeid(uint16_t).name())
        {
            output->SetUint16Value(*it.second.cast<uint16_t>(), id);
        }
        else if (type == typeid(uint32_t).name())
        {
            output->SetUint32Value(*it.second.cast<uint32_t>(), id);
        }
        else if (type == typeid(uint64_t).name())
        {
            output->SetUint64Value(*it.second.cast<uint64_t>(), id);
        }
        else if (type == typeid(int16_t).name())
        {
            output->SetInt16Value(*it.second.cast<int16_t>(), id);
        }
        else if (type == typeid(int32_t).name())
        {
            output->SetInt32Value(*it.second.cast<int32_t>(), id);
        }
        else if (type == typeid(int64_t).name())
        {
            output->SetInt64Value(*it.second.cast<int64_t>(), id);
        }
        else if (type == typeid(float).name())
        {
            output->SetFloat32Value(*it.second.cast<float>(), id);
        }
        else if (type == typeid(double).name())
        {
            output->SetFloat64Value(*it.second.cast<double>(), id);
        }
        else if (type == typeid(long double).name())
        {
            output->SetFloat128Value(*it.second.cast<long double>(), id);
        }
        else if (type == typeid(char).name())
        {
            output->SetChar8Value(*it.second.cast<char>(), id);
        }
        else if (type == typeid(wchar_t).name())
        {
            output->SetChar16Value(*it.second.cast<wchar_t>(), id);
        }
        else if (type == typeid(std::string).name())
        {
            output->SetStringValue(*it.second.cast<std::string>(), id);
        }
        else if (type == typeid(std::wstring).name())
        {
            output->SetWstringValue(*it.second.cast<std::wstring>(), id);
        }
        else
        {
            throw DDSMiddlewareException("Error parsing from soss message.");
        }
    }

    return true;
}

bool Conversion::dds_to_soss(
        const std::string type,
        const types::DynamicData_ptr input,
        soss::Message& output)
{
    uint32_t id = 0;
    uint32_t i = 0;
    types::MemberDescriptor descriptor;

    output.type = type;

    while (true)
    {
        id = input->GetMemberIdAtIndex(i);

        if (id != MEMBER_ID_INVALID)
        {
            soss::Field field;

            input->GetDescriptor(descriptor, id);

            types::TypeKind type = descriptor.GetKind();

            if (type == types::TK_BOOLEAN)
            {
                bool value = input->GetBoolValue(id);
                field.set<bool>(std::move(value));
            }
            else if (type == types::TK_BYTE)
            {
                u_char value = input->GetByteValue(id);
                field.set<u_char>(std::move(value));
            }
            else if (type == types::TK_INT16)
            {
                int16_t value = input->GetInt16Value(id);
                field.set<int16_t>(std::move(value));
            }
            else if (type == types::TK_INT32)
            {
                int32_t value = input->GetInt32Value(id);
                field.set<int32_t>(std::move(value));
            }
            else if (type == types::TK_INT64)
            {
                int64_t value = input->GetInt64Value(id);
                field.set<int64_t>(std::move(value));
            }
            else if (type == types::TK_UINT16)
            {
                uint16_t value = input->GetUint16Value(id);
                field.set<uint16_t>(std::move(value));
            }
            else if (type == types::TK_UINT32)
            {
                uint32_t value = input->GetUint32Value(id);
                field.set<uint32_t>(std::move(value));
            }
            else if (type == types::TK_UINT64)
            {
                uint64_t value = input->GetUint64Value(id);
                field.set<uint64_t>(std::move(value));
            }
            else if (type == types::TK_FLOAT32)
            {
                float value = input->GetFloat32Value(id);
                field.set<float>(std::move(value));
            }
            else if (type == types::TK_FLOAT64)
            {
                double value = input->GetFloat64Value(id);
                field.set<double>(std::move(value));
            }
            else if (type == types::TK_FLOAT128)
            {
                long double value = input->GetFloat128Value(id);
                field.set<long double>(std::move(value));
            }
            else if (type == types::TK_CHAR8)
            {
                char value = input->GetChar8Value(id);
                field.set<char>(std::move(value));
            }
            else if (type == types::TK_CHAR16)
            {
                wchar_t value = input->GetChar16Value(id);
                field.set<wchar_t>(std::move(value));
            }
            else if (type == types::TK_STRING8)
            {
                std::string value = input->GetStringValue(id);
                field.set<std::string>(std::move(value));
            }
            else if (type == types::TK_STRING16)
            {
                std::wstring value = input->GetWstringValue(id);
                field.set<std::wstring>(std::move(value));
            }
            else
            {
                throw DDSMiddlewareException("Error parsing from dynamic data.");
            }

            output.data[descriptor.GetName()] = std::move(field);
            i++;
        }
        else
        {
            break;
        }
    }

    return true;
}


} // namespace dds
} // namespace soss

