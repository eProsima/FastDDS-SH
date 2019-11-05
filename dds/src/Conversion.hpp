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

#ifndef SOSS__DDS__INTERNAL__CONVERSION_HPP
#define SOSS__DDS__INTERNAL__CONVERSION_HPP

#include "DDSMiddlewareException.hpp"
#include "DynamicTypeAdapter.hpp"
#include <soss/Message.hpp>
#include <map>

namespace soss {
namespace dds {

struct Conversion {

    static bool soss_to_dds(
            const ::xtypes::DynamicData& input,
            DynamicData* output);

    static bool dds_to_soss(
            const std::string type,
            DynamicData* input,
            ::xtypes::DynamicData& output);

    static ::xtypes::DynamicData dynamic_data(
            const std::string& type_name);

    static void register_type(
            const std::string& type_name,
            DynamicPubSubType* type)
    {
        registered_types_.emplace(type_name, type);
        //convert_type(type->GetDynamicType().get());
    }

    static ::xtypes::DynamicType::Ptr convert_type(
            const DynamicType* type);

private:
    ~Conversion() = default;
    static std::map<std::string, ::xtypes::DynamicType::Ptr> types_;
    static std::map<std::string, DynamicPubSubType*> registered_types_;

    static ::xtypes::DynamicType::Ptr create_type(
            const DynamicType* type);
};


} // namespace dds
} // namespace soss

#endif // SOSS__DDS__INTERNAL__CONVERSION_HPP

