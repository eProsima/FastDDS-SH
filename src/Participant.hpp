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

#ifndef SOSS__DDS__INTERNAL__PARTICIPANT_HPP
#define SOSS__DDS__INTERNAL__PARTICIPANT_HPP

#include "DDSMiddlewareException.hpp"
#include "TopicType.hpp"

#include <soss/SystemHandle.hpp>

#include <fastrtps/participant/ParticipantListener.h>

#include <map>

namespace soss {
namespace dds {

class Participant : private eprosima::fastrtps::ParticipantListener
{
public:
    Participant(uint32_t domain);
    virtual ~Participant();

    eprosima::fastrtps::Participant* get_dds_participant() const { return dds_participant_; }

    const TopicType& create_topic_type(const std::string& name /*, TODO: idl definition */);
    const TopicType& get_topic_type(const std::string& name) const;

private:
    void onParticipantDiscovery(
            eprosima::fastrtps::Participant* participant,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

    eprosima::fastrtps::Participant* dds_participant_;
    std::map<std::string, TopicType> topics_;
};


} // namespace dds
} // namespace soss

#endif // SOSS__DDS__INTERNAL__PARTICIPANT_HPP
