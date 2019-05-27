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
#include "fastrtps/xmlparser/XMLProfileManager.h"
#include <fastrtps/transport/UDPv4TransportDescriptor.h>

#include <iostream>

namespace soss {
namespace dds {

Participant::Participant()
{
    std::cout << "[soss-dds]: Warning. Participant not provided in configuration file. " <<
                 "A UDP default participant will be created." << std::endl;

    dds_participant_ = fastrtps::Domain::createParticipant(fastrtps::ParticipantAttributes());
}

Participant::Participant(const YAML::Node& config)
{
    using eprosima::fastrtps::xmlparser::XMLP_ret;
    using eprosima::fastrtps::xmlparser::XMLProfileManager;

    if (!config.IsMap() || !config["file_path"] || !config["profile_name"])
    {
        std::string err_msg = "The node 'Participant' in the configuration must be a map containing two keys: "
                              "'file_path' and 'profile_name'. ";
        throw DDSMiddlewareException(err_msg);
    }

    std::string file_path = config["file_path"].as<std::string>();
    if (XMLP_ret::XML_OK != XMLProfileManager::loadXMLFile(file_path))
    {
        throw DDSMiddlewareException("Error loading xml file");
    }

    std::string profile_name = config["profile_name"].as<std::string>();
    dds_participant_ = fastrtps::Domain::createParticipant(profile_name);

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
    fastrtps::types::DynamicType_ptr dtptr = builder->Build();
    if(dtptr != nullptr)
    {
        auto pair = topics_.emplace(topic_name, fastrtps::types::DynamicPubSubType(dtptr));
        if (!pair.second)
        {
            throw DDSMiddlewareException("Error creating a dynamic type: already exists");
        }

        if (!fastrtps::Domain::registerDynamicType(dds_participant_, &pair.first->second))
        {
            throw DDSMiddlewareException("Error registering the dynamic type");
        }
    }
    else
    {
        throw DDSMiddlewareException("Error Building the dynamic type.");
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

