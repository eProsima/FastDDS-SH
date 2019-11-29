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
#include <vector>

namespace soss {
namespace dds {

struct Conversion {

    static bool soss_to_dds(
            const ::xtypes::DynamicData& input,
            DynamicData* output);

    static bool dds_to_soss(
            const DynamicData* input,
            ::xtypes::DynamicData& output);

    static ::xtypes::DynamicData dynamic_data(
            const std::string& type_name);

    static void register_type(
            const std::string& type_name,
            DynamicPubSubType* type)
    {
        registered_types_.emplace(type_name, type);
    }

    static DynamicTypeBuilder* create_builder(
            const xtypes::DynamicType& type);
private:
    ~Conversion() = default;
    static std::map<std::string, ::xtypes::DynamicType::Ptr> types_;
    static std::map<std::string, DynamicPubSubType*> registered_types_;
    static std::map<std::string, DynamicTypeBuilder_ptr> builders_;

    static DynamicTypeBuilder_ptr get_builder(
        const xtypes::DynamicType& type);

    static void get_array_specs(
        const xtypes::ArrayType& array,
        std::pair<std::vector<uint32_t>, DynamicTypeBuilder_ptr>& result);

    // soss -> dds
    static void set_primitive_data(
        xtypes::ReadableDynamicDataRef from,
        DynamicData* to,
        eprosima::fastrtps::types::MemberId id);

    // soss -> dds
    static void set_sequence_data(
        xtypes::ReadableDynamicDataRef from,
        DynamicData* to);

    // soss -> dds
    static void set_array_data(
        xtypes::ReadableDynamicDataRef from,
        DynamicData* to,
        const std::vector<uint32_t>& indexes);

    // soss -> dds
    static bool set_struct_data(
        xtypes::ReadableDynamicDataRef input,
        DynamicData* output);

    // dds -> soss
    static void set_sequence_data(
        const DynamicData* from,
        xtypes::WritableDynamicDataRef to);

    // dds -> soss
    static void set_array_data(
        const DynamicData* from,
        xtypes::WritableDynamicDataRef to,
        const std::vector<uint32_t>& indexes);

    // dds -> soss
    static bool set_struct_data(
            const DynamicData* input,
            ::xtypes::WritableDynamicDataRef output);

};


} // namespace dds
} // namespace soss

#endif // SOSS__DDS__INTERNAL__CONVERSION_HPP

