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

#ifndef SOSS__DDS__INTERNAL__PARTICIPANT_HPP
#define SOSS__DDS__INTERNAL__PARTICIPANT_HPP

#include "DDSMiddlewareException.hpp"
#include "DynamicTypeAdapter.hpp"

#include <fastrtps/participant/ParticipantListener.h>

#include <yaml-cpp/yaml.h>

#include <map>

namespace soss {
namespace dds {

class Participant : private eprosima::fastrtps::ParticipantListener
{
public:

    Participant(); // Constructor for creating a participant with default values (UDP).

    Participant(
            const YAML::Node& config);

    virtual ~Participant();

    eprosima::fastrtps::Participant* get_dds_participant() const
    {
        return dds_participant_;
    }

    void register_dynamic_type(
            const std::string& topic_name,
            const std::string& type_name,
            DynamicTypeBuilder* builder);

    DynamicData* create_dynamic_data(
            const std::string& name) const;

    const DynamicType* get_dynamic_type(
            const std::string& name) const
    {
        auto it = topics_.find(name);
        if (it == topics_.end())
        {
            return nullptr;
        }
        return static_cast<const DynamicType*>(it->second.GetDynamicType().get());
    }

    const std::string& get_topic_type(
            const std::string& topic) const
    {
        return topic_to_type_.at(topic);
    }

private:

    void onParticipantDiscovery(
            eprosima::fastrtps::Participant* participant,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

    eprosima::fastrtps::Participant* dds_participant_;
    std::map<std::string, DynamicPubSubType> topics_;
    std::map<std::string, std::string> topic_to_type_;
};


} // namespace dds
} // namespace soss

#endif // SOSS__DDS__INTERNAL__PARTICIPANT_HPP
