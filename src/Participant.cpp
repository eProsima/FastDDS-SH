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
#include "DDSMiddlewareException.hpp"
#include "Conversion.hpp"

#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>

#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <sstream>

namespace eprosima {
namespace is {
namespace sh {
namespace fastdds {

Participant::Participant()
    : dds_participant_(nullptr)
    , logger_("is::sh::FastDDS::Participant")
{
    build_participant();
}

Participant::Participant(
        const YAML::Node& config)
    : dds_participant_(nullptr)
    , logger_("is::sh::FastDDS::Participant")
{
    using fastrtps::xmlparser::XMLP_ret;
    using fastrtps::xmlparser::XMLProfileManager;

    if (!config.IsMap() || !config["file_path"] || !config["profile_name"])
    {
        if (config["domain_id"])
        {
            const ::fastdds::dds::DomainId_t domain_id = config["domain_id"].as<uint32_t>();
            build_participant(domain_id);
        }
        else
        {
            std::ostringstream err;
            err << "The node 'participant' in the YAML configuration of the 'fastdds' system "
                << "must be a map containing two keys: 'file_path' and 'profile_name'";

            throw DDSMiddlewareException(logger_, err.str());
        }

    }
    else
    {
        const std::string file_path = config["file_path"].as<std::string>();
        if (XMLP_ret::XML_OK != XMLProfileManager::loadXMLFile(file_path))
        {
            throw DDSMiddlewareException(
                      logger_, "Loading provided XML file in 'file_path' field was not successful");
        }

        const std::string profile_name = config["profile_name"].as<std::string>();

        dds_participant_ = this->create_participant_with_profile(profile_name);

        if (dds_participant_)
        {
            logger_ << utils::Logger::Level::INFO
                    << "Created Fast DDS participant '" << dds_participant_->get_qos().name()
                    << "' from profile configuration '" << profile_name << "'" << std::endl;
        }
        else
        {
            std::ostringstream err;
            err << "Fast DDS participant '" << dds_participant_->get_qos().name()
                << "' from profile configuration '" << profile_name << "' creation failed";

            throw DDSMiddlewareException(logger_, err.str());
        }
    }
}

Participant::~Participant()
{
    if (!dds_participant_->has_active_entities())
    {
        dds_participant_->set_listener(nullptr);

        if (fastrtps::types::ReturnCode_t::RETCODE_OK !=
                ::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(dds_participant_))
        {
            logger_ << utils::Logger::Level::ERROR
                    << "Cannot delete Fast DDS participant yet: it has active entities" << std::endl;
        }
    }
}

void Participant::build_participant(
        const ::fastdds::dds::DomainId_t& domain_id)
{
    ::fastdds::dds::DomainParticipantQos participant_qos = ::fastdds::dds::PARTICIPANT_QOS_DEFAULT;
    participant_qos.name("default_IS-FastDDS-SH_participant");

    // By default use UDPv4 due to communication failures between dockers sharing the network with the host
    // When it is solved in Fast-DDS delete the following lines and use the default builtin transport.
    participant_qos.transport().use_builtin_transports = false;
    auto udp_transport = std::make_shared<::fastdds::rtps::UDPv4TransportDescriptor>();
    participant_qos.transport().user_transports.push_back(udp_transport);

    dds_participant_ = ::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        domain_id, participant_qos);

    if (dds_participant_)
    {
        logger_ << utils::Logger::Level::INFO
                << "Created Fast DDS participant '" << participant_qos.name()
                << "' with default QoS attributes and Domain ID: "
                << domain_id << std::endl;
    }
    else
    {
        std::ostringstream err;
        err << "Error while creating Fast DDS participant '" << participant_qos.name()
            << "' with default QoS attributes and Domain ID: " << domain_id;

        throw DDSMiddlewareException(logger_, err.str());
    }
}

::fastdds::dds::DomainParticipant* Participant::get_dds_participant() const
{
    return dds_participant_;
}

void Participant::register_dynamic_type(
        const std::string& topic_name,
        const std::string& type_name,
        fastrtps::types::DynamicTypeBuilder* builder)
{
    auto topic_to_type_it = topic_to_type_.find(topic_name);
    if (topic_to_type_it != topic_to_type_.end())
    {
        return; // Already registered.
    }

    auto types_it = types_.find(type_name);
    if (types_.end() != types_it)
    {
        // Type known, add the entry in the map topic->type
        topic_to_type_.emplace(topic_name, type_name);

        logger_ << utils::Logger::Level::DEBUG
                << "Adding type '" << type_name << "' to topic '"
                << topic_name << "'" << std::endl;

        return;
    }

    fastrtps::types::DynamicType_ptr dtptr = builder->build();

    if (dtptr != nullptr)
    {
        auto pair = types_.emplace(type_name, fastrtps::types::DynamicPubSubType(dtptr));
        fastrtps::types::DynamicPubSubType& dynamic_type_support = pair.first->second;

        topic_to_type_.emplace(topic_name, type_name);

        // Check if already registered
        ::fastdds::dds::TypeSupport p_type = dds_participant_->find_type(type_name);

        if (nullptr == p_type)
        {
            dynamic_type_support.setName(type_name.c_str());

            /**
             * The following lines are added here so that a bug with UnionType in
             * Fast DDS Dynamic Types is bypassed. This is a workaround and SHOULD
             * be removed once this bug is solved.
             * Until that moment, the Fast DDS SystemHandle will not be compatible with
             * Fast DDS Dynamic Type Discovery mechanism.
             *
             * More information here: https://eprosima.easyredmine.com/issues/11349
             */
            // WORKAROUND START
            dynamic_type_support.auto_fill_type_information(false);
            dynamic_type_support.auto_fill_type_object(false);
            // WORKAROUND END

            // Register it within the DomainParticipant
            if (pair.second && !dds_participant_->register_type(dynamic_type_support))
            {
                std::ostringstream err;
                err << "Dynamic type '" << type_name << "' registration failed";

                throw DDSMiddlewareException(logger_, err.str());
            }
        }

        if (pair.second)
        {
            logger_ << utils::Logger::Level::DEBUG
                    << "Registered type '" << type_name << "' in topic '"
                    << topic_name << "'" << std::endl;

            Conversion::register_type(topic_name, &dynamic_type_support);
        }
        else
        {
            logger_ << utils::Logger::Level::WARN
                    << "Failed registering type '" << type_name << "' in topic '"
                    << topic_name << "'" << std::endl;
        }
    }
    else
    {
        std::ostringstream err;
        err << "Dynamic type '" << type_name << "' for topic '"
            << topic_name << "' was not correctly built";

        throw DDSMiddlewareException(logger_, err.str());
    }
}

fastrtps::types::DynamicData* Participant::create_dynamic_data(
        const std::string& topic_name) const
{
    auto topic_to_type_it = topic_to_type_.find(topic_name);
    if (topic_to_type_.end() == topic_to_type_it)
    {
        std::ostringstream err;
        err << "Creating dynamic data for topic '" << topic_name
            << "' failed because the topic was not registered";

        throw DDSMiddlewareException(logger_, err.str());
    }

    auto types_it = types_.find(topic_to_type_it->second);
    if (types_.end() == types_it)
    {
        std::ostringstream err;
        err << "Creating dynamic data: dynamic type '" << types_it->first << "' not defined";

        throw DDSMiddlewareException(logger_, err.str());
    }

    const fastrtps::types::DynamicType_ptr& dynamic_type_ = types_it->second.GetDynamicType();
    return fastrtps::types::DynamicDataFactory::get_instance()->create_data(dynamic_type_);
}

void Participant::delete_dynamic_data(
        fastrtps::types::DynamicData* data) const
{
    DynamicDataFactory::get_instance()->delete_data(data);
}

const fastrtps::types::DynamicType* Participant::get_dynamic_type(
        const std::string& name) const
{
    auto it = types_.find(name);
    if (it == types_.end())
    {
        return nullptr;
    }

    return static_cast<const fastrtps::types::DynamicType*>(it->second.GetDynamicType().get());
}

const std::string& Participant::get_topic_type(
        const std::string& topic) const
{
    return topic_to_type_.at(topic);
}

void Participant::associate_topic_to_dds_entity(
        ::fastdds::dds::Topic* topic,
        ::fastdds::dds::DomainEntity* entity)
{
    std::unique_lock<std::mutex> lock(topic_to_entities_mtx_);

    if (topic_to_entities_.end() == topic_to_entities_.find(topic))
    {
        std::set<::fastdds::dds::DomainEntity*> entities;
        entities.emplace(entity);

        topic_to_entities_[topic] = std::move(entities);
    }
    else
    {
        topic_to_entities_.at(topic).emplace(entity);
    }
}

bool Participant::dissociate_topic_from_dds_entity(
        ::fastdds::dds::Topic* topic,
        ::fastdds::dds::DomainEntity* entity)
{
    // std::unique_lock<std::mutex> lock(topic_to_entities_mtx_);

    if (1 == topic_to_entities_.at(topic).size())
    {
        // Only one entity remains in the map
        topic_to_entities_.erase(topic);
        return true;
    }
    else // Size should not be 0 at this point
    {
        topic_to_entities_.at(topic).erase(entity);
        return false;
    }
}

static void set_qos_from_attributes(
        ::fastdds::dds::DomainParticipantQos& qos,
        const eprosima::fastrtps::rtps::RTPSParticipantAttributes& attr)
{
    qos.user_data().setValue(attr.userData);
    qos.allocation() = attr.allocation;
    qos.properties() = attr.properties;
    qos.wire_protocol().prefix = attr.prefix;
    qos.wire_protocol().participant_id = attr.participantID;
    qos.wire_protocol().builtin = attr.builtin;
    qos.wire_protocol().port = attr.port;
    qos.wire_protocol().throughput_controller = attr.throughputController;
    qos.wire_protocol().default_unicast_locator_list = attr.defaultUnicastLocatorList;
    qos.wire_protocol().default_multicast_locator_list = attr.defaultMulticastLocatorList;

    if (attr.useBuiltinTransports)
    {
        // By default use UDPv4 due to communication failures between dockers sharing the network with the host
        // When it is solved in Fast-DDS delete the following lines and use the default builtin transport.
        qos.transport().use_builtin_transports = false;
        auto udp_transport = std::make_shared<::fastdds::rtps::UDPv4TransportDescriptor>();
        qos.transport().user_transports.push_back(udp_transport);
    }
    else
    {
        qos.transport().user_transports = attr.userTransports;
        qos.transport().use_builtin_transports = attr.useBuiltinTransports;
    }
    
    qos.transport().send_socket_buffer_size = attr.sendSocketBufferSize;
    qos.transport().listen_socket_buffer_size = attr.listenSocketBufferSize;
    qos.name() = attr.getName();
}

::fastdds::dds::DomainParticipant* Participant::create_participant_with_profile(
    const std::string& profile_name)
{
    using namespace fastrtps::xmlparser;

    fastrtps::ParticipantAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillParticipantAttributes(profile_name, attr))
    {
        ::fastdds::dds::DomainParticipantQos qos = ::fastdds::dds::PARTICIPANT_QOS_DEFAULT;
        set_qos_from_attributes(qos, attr.rtps);

        return ::fastdds::dds::DomainParticipantFactory::get_instance()->
               create_participant(attr.domainId, qos);
    }
    else
    {
        std::ostringstream err;
        err << "Failed to fetch Fast DDS participant attributes from XML "
            << "for profile named '" << profile_name << "'";

        throw DDSMiddlewareException(logger_, err.str());
    }
}

} //  namespace fastdds
} //  namespace sh
} //  namespace is
} //  namespace eprosima
