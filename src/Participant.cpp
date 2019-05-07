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

#include "Participant.hpp"

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/Domain.h>

#include <iostream>

namespace soss {
namespace dds {


Participant::Participant()
{
    eprosima::fastrtps::ParticipantAttributes attributes;
    attributes.rtps.builtin.domainId = 0u;
    attributes.rtps.setName("soss-dds-participant");
    dds_participant_ = eprosima::fastrtps::Domain::createParticipant(attributes, &listener_);

    if (nullptr == dds_participant_)
    {
        throw DDSMiddlewareException("Error creating a participant");
    }

    dynamic_type_ = eprosima::fastrtps::types::DynamicTypeBuilderFactory::GetInstance()->CreateStringType();

    pub_sub_type_.SetDynamicType(dynamic_type_);
    eprosima::fastrtps::Domain::registerDynamicType(dds_participant_, &pub_sub_type_);
}

Participant::~Participant()
{
    eprosima::fastrtps::Domain::removeParticipant(dds_participant_);
}

eprosima::fastrtps::types::DynamicData_ptr Participant::create_dynamic_data()
{
    return eprosima::fastrtps::types::DynamicDataFactory::GetInstance()->CreateData(dynamic_type_);
}

void Participant::Listener::onParticipantDiscovery(
        eprosima::fastrtps::Participant* /* participant */,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& /* info */)
{
    std::cout << "Participant discovered!" << std::endl; //TEMP_TRACE
}


} //namespace dds
} //namespace soss

