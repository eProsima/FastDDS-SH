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

#include "Participant.hpp"
#include "Conversion.hpp"

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/Domain.h>
#include "fastrtps/xmlparser/XMLProfileManager.h"
#include <fastrtps/transport/UDPv4TransportDescriptor.h>

#include <iostream>

using namespace eprosima;
using eprosima::fastrtps::types::DynamicType_ptr;

namespace soss {
namespace dds {

Participant::Participant()
{
    fastrtps::ParticipantAttributes attributes;
    attributes.rtps.setName("DefaultSOSSDDSParticipant");
    dds_participant_ = fastrtps::Domain::createParticipant(attributes);
}

Participant::Participant(
        const YAML::Node& config)
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
    for (auto topic : topics_)
    {
        fastrtps::Domain::unregisterType(dds_participant_, topic.first.c_str());
    }
    topics_.clear();
    fastrtps::Domain::removeParticipant(dds_participant_);
}

void Participant::register_dynamic_type(
        const std::string& topic_name,
        const std::string& type_name,
        DynamicTypeBuilder* builder)
{
    auto type_it = topic_to_type_.find(topic_name);
    if (type_it != topic_to_type_.end())
    {
        return; // Already registered.
    }

    auto it = topics_.find(type_name);
    if (topics_.end() != it)
    {
        // Type known, add the entry in the map topic->type
        topic_to_type_.emplace(topic_name, type_name);
        std::cout << "[soss-dds][participant]: Adding type '" << type_name << "' to topic '"
                  << topic_name << "'." << std::endl;
        return;
    }

    DynamicType_ptr dtptr = builder->build();

    if (dtptr != nullptr)
    {
        auto pair = topics_.emplace(type_name, fastrtps::types::DynamicPubSubType(dtptr));
        topic_to_type_.emplace(topic_name, type_name);

        // Check if already registered
        eprosima::fastrtps::TopicDataType* p_type = nullptr;
        if (!fastrtps::Domain::getRegisteredType(dds_participant_, type_name.c_str(), &p_type))
        {
            pair.first->second.setName(type_name.c_str());
            // Register it in fastrtps
            if (pair.second && !fastrtps::Domain::registerType(dds_participant_, &pair.first->second))
            {
                std::stringstream ss;
                ss << "Error registering dynamic type '" << pair.first->second.getName() << "'.";
                throw DDSMiddlewareException(ss.str());
            }
        }

        if (pair.second)
        {
            std::cout << "[soss-dds][participant]: Registered type '" << type_name << "' in topic '"
                      << topic_name << "'." << std::endl;
            Conversion::register_type(topic_name, &pair.first->second);
        }
        else
        {
            std::cout << "[soss-dds][participant]: Failed registering type '" << type_name << "' in topic '"
                      << topic_name << "'." << std::endl;
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
    auto type_it = topic_to_type_.find(topic_name);
    if (topic_to_type_.end() == type_it)
    {
        std::stringstream ss;
        ss << "Error creating dynamic data: Topic type not found '" << topic_name << "'.";
        throw DDSMiddlewareException(ss.str());
    }

    auto it = topics_.find(type_it->second);
    if (topics_.end() == it)
    {
        std::stringstream ss;
        ss << "Error creating dynamic data: dynamic type " << type_it->second << " not defined";
        throw DDSMiddlewareException(ss.str());
    }

    const DynamicType_ptr& dynamic_type_ = it->second.GetDynamicType();
    return fastrtps::types::DynamicData_ptr(DynamicDataFactory::get_instance()->create_data(dynamic_type_));
}

void Participant::onParticipantDiscovery(
        fastrtps::Participant* /* participant */,
        fastrtps::rtps::ParticipantDiscoveryInfo&& /* info */)
{
}

} //namespace dds
} //namespace soss
