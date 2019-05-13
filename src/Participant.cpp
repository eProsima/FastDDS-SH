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
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/Domain.h>

#include <iostream>

namespace soss {
namespace dds {


Participant::Participant(uint32_t domain)
{
    fastrtps::ParticipantAttributes attributes;
    attributes.rtps.builtin.domainId = domain;
    attributes.rtps.setName("soss-dds-participant");
    dds_participant_ = fastrtps::Domain::createParticipant(attributes, this);

    if (nullptr == dds_participant_)
    {
        throw DDSMiddlewareException("Error creating a participant");
    }
}

Participant::~Participant()
{
    fastrtps::Domain::removeParticipant(dds_participant_);
}

void Participant::register_dynamic_type(
        const std::string& topic_name,
        fastrtps::types::DynamicTypeBuilder* builder)
{
    auto pair = topics_.emplace(topic_name, fastrtps::types::DynamicPubSubType(builder->Build()));
    if (!pair.second)
    {
        throw DDSMiddlewareException("Error creating a dynamic type: already exists");
    }

    if (!fastrtps::Domain::registerDynamicType(dds_participant_, &pair.first->second))
    {
        throw DDSMiddlewareException("Error registering the dynamic type");
    }
}

fastrtps::types::DynamicData_ptr Participant::create_dynamic_data(
        const std::string& topic_name) const
{
    auto it = topics_.find(topic_name);
    if (topics_.end() == it)
    {
        throw DDSMiddlewareException("Error creating a dynamic data: dynamic type not defined");
    }

    const fastrtps::types::DynamicType_ptr& dynamic_type_ = it->second.GetDynamicType();
    return fastrtps::types::DynamicDataFactory::GetInstance()->CreateData(dynamic_type_);
}

void Participant::onParticipantDiscovery(
        fastrtps::Participant* /* participant */,
        fastrtps::rtps::ParticipantDiscoveryInfo&& /* info */)
{
}


} //namespace dds
} //namespace soss

