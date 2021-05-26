/*
 * Copyright 2019 - present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "Publisher.hpp"
#include "Conversion.hpp"

#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>

#include <iostream>
#include <sstream>

namespace eprosima {
namespace is {
namespace sh {
namespace fastdds {

Publisher::Publisher(
        Participant* participant,
        const std::string& topic_name,
        const xtypes::DynamicType& message_type,
        const YAML::Node& config)
    : participant_(participant)
    , topic_name_(topic_name)
    , logger_("is::sh::FastDDS::Publisher")
{
    fastrtps::types::DynamicTypeBuilder* builder = Conversion::create_builder(message_type);

    if (builder != nullptr)
    {
        participant->register_dynamic_type(topic_name, message_type.name(), builder);
    }
    else
    {
        throw DDSMiddlewareException(
                  logger_, "Cannot create builder for type " + message_type.name());
    }

    dynamic_data_ = participant->create_dynamic_data(topic_name);

    // Retrieve DDS participant
    ::fastdds::dds::DomainParticipant* dds_participant = participant->get_dds_participant();
    if (!dds_participant)
    {
        throw DDSMiddlewareException(
                  logger_, "Trying to create a publisher without a DDS participant!");
    }

    // Create DDS publisher with default publisher QoS
    dds_publisher_ = dds_participant->create_publisher(::fastdds::dds::PUBLISHER_QOS_DEFAULT);
    if (dds_publisher_)
    {
        logger_ << utils::Logger::Level::DEBUG
                << "Created Fast DDS publisher for topic '" << topic_name << "'" << std::endl;
    }
    else
    {
        std::ostringstream err;
        err << "Fast DDS publisher for topic '" << topic_name << "' was not created";

        throw DDSMiddlewareException(logger_, err.str());
    }

    // Create DDS topic
    auto topic_description = dds_participant->lookup_topicdescription(topic_name);
    if (!topic_description)
    {
        dds_topic_ = dds_participant->create_topic(
            topic_name, message_type.name(), ::fastdds::dds::TOPIC_QOS_DEFAULT);
        if (dds_topic_)
        {
            logger_ << utils::Logger::Level::DEBUG
                    << "Created Fast DDS topic '" << topic_name << "' with type '"
                    << message_type.name() << "'" << std::endl;
        }
        else
        {
            std::ostringstream err;
            err << "Fast DDS topic '" << topic_name << "' with type '"
                << message_type.name() << "' was not created";

            throw DDSMiddlewareException(logger_, err.str());
        }
    }
    else
    {
        dds_topic_ = static_cast<::fastdds::dds::Topic*>(topic_description);
    }

    // Create DDS datawriter
    ::fastdds::dds::DataWriterQos datawriter_qos = ::fastdds::dds::DATAWRITER_QOS_DEFAULT;
    if (config["service_instance_name"])
    {
        fastrtps::rtps::Property instance_property;
        instance_property.name("dds.rpc.service_instance_name");
        instance_property.value(config["service_instance_name"].as<std::string>());
        datawriter_qos.properties().properties().emplace_back(std::move(instance_property));
    }

    dds_datawriter_ = dds_publisher_->create_datawriter(dds_topic_, datawriter_qos, this);
    if (dds_datawriter_)
    {
        logger_ << utils::Logger::Level::DEBUG
                << "Created Fast DDS datawriter for topic '" << topic_name << "'" << std::endl;
        participant_->associate_topic_to_dds_entity(dds_topic_, dds_datawriter_);
    }
    else
    {
        std::ostringstream err;
        err << "Fast DDS datawriter for topic '" << topic_name << "' was not created";

        throw DDSMiddlewareException(logger_, err.str());
    }
}

Publisher::~Publisher()
{
    std::unique_lock<std::mutex> lock(data_mtx_);
    participant_->delete_dynamic_data(dynamic_data_);

    bool delete_topic = participant_->dissociate_topic_from_dds_entity(dds_topic_, dds_datawriter_);

    dds_datawriter_->set_listener(nullptr);
    dds_publisher_->delete_datawriter(dds_datawriter_);
    participant_->get_dds_participant()->delete_publisher(dds_publisher_);

    if (delete_topic)
    {
        participant_->get_dds_participant()->delete_topic(dds_topic_);
    }
}

bool Publisher::publish(
        const ::xtypes::DynamicData& message)
{
    std::unique_lock<std::mutex> lock(data_mtx_);

    logger_ << utils::Logger::Level::INFO
            << "Sending message from Integration Service to DDS for topic '" << topic_name_ << "': "
            << "[[ " << message << " ]]" << std::endl;

    bool success = Conversion::xtypes_to_fastdds(message, dynamic_data_);
    if (success)
    {
        success = dds_datawriter_->write(static_cast<void*>(dynamic_data_));
    }
    else
    {
        logger_ << utils::Logger::Level::ERROR
                << "Failed to convert message from Integration Service to DDS for topic '"
                << topic_name_ << "': [[ " << message << " ]]" << std::endl;
    }

    return success;
}

const fastrtps::rtps::InstanceHandle_t Publisher::get_dds_instance_handle() const
{
    return dds_datawriter_->get_instance_handle();
}

void Publisher::on_publication_matched(
        ::fastdds::dds::DataWriter* /*writer*/,
        const ::fastdds::dds::PublicationMatchedStatus& info)
{
    if (1 == info.current_count_change)
    {
        logger_ << utils::Logger::Level::INFO
                << "Publisher for topic '" << topic_name_ << "' matched" << std::endl;
    }
    else if (-1 == info.current_count_change)
    {
        logger_ << utils::Logger::Level::INFO
                << "Publisher for topic '" << topic_name_ << "' unmatched" << std::endl;
    }
}

} //  namespace fastdds
} //  namespace sh
} //  namespace is
} //  namespace eprosima
