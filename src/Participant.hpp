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

#include <soss/SystemHandle.hpp>

#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicPubSubType.h>

#include <memory>

namespace soss {
namespace dds {

class Participant
{
public:
    Participant(uint32_t domain);
    virtual ~Participant();

    eprosima::fastrtps::Participant* get_dds_participant() const { return dds_participant_; }
    eprosima::fastrtps::types::DynamicData_ptr create_dynamic_data();

private:
    class Listener : public eprosima::fastrtps::ParticipantListener
    {
        void onParticipantDiscovery(
                eprosima::fastrtps::Participant* participant,
                eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

    } listener_;

    eprosima::fastrtps::Participant* dds_participant_;
    eprosima::fastrtps::types::DynamicType_ptr dynamic_type_;
    eprosima::fastrtps::types::DynamicPubSubType pub_sub_type_;
};


} // namespace dds
} // namespace soss

#endif // SOSS__DDS__INTERNAL__PARTICIPANT_HPP
