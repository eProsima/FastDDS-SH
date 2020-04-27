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

#include "Server.hpp"
#include "Conversion.hpp"

#include <soss/Message.hpp>

#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/Domain.h>

#include <functional>
#include <iostream>

using namespace eprosima;

namespace soss {
namespace dds {

Server::Server(
        Participant* participant,
        const std::string& service_name,
        const ::xtypes::DynamicType& request_type,
        const ::xtypes::DynamicType& reply_type,
        const YAML::Node& config)
    : service_name_{service_name}
    , request_type_{request_type}
    , reply_type_{reply_type}
{
    add_config(config);

    // Create DynamicData
    DynamicTypeBuilder* builder_request = Conversion::create_builder(request_type);
    DynamicTypeBuilder* builder_reply = Conversion::create_builder(reply_type);

    if (builder_request != nullptr)
    {
        participant->register_dynamic_type(service_name + "_Request", request_type.name(), builder_request);
    }
    else
    {
        throw DDSMiddlewareException("Cannot create builder for type " + request_type.name());
    }

    if (builder_reply != nullptr)
    {
        participant->register_dynamic_type(service_name + "_Reply", reply_type.name(), builder_reply);
    }
    else
    {
        throw DDSMiddlewareException("Cannot create builder for type " + reply_type.name());
    }

    request_dynamic_data_ = participant->create_dynamic_data(service_name + "_Request");
    reply_dynamic_data_ = participant->create_dynamic_data(service_name + "_Reply");

    // Create Subscriber
    {
        fastrtps::SubscriberAttributes attributes;
        attributes.topic.topicKind = NO_KEY;
        attributes.topic.topicName = service_name_ + "_Reply";
        attributes.topic.topicDataType = reply_type.name();

        if (config["service_instance_name"])
        {
            fastrtps::rtps::Property instance_property;
            instance_property.name("dds.rpc.service_instance_name");
            instance_property.value(config["service_instance_name"].as<std::string>());
            attributes.properties.properties().push_back(instance_property);
        }

        dds_subscriber_ = fastrtps::Domain::createSubscriber(participant->get_dds_participant(), attributes, this);

        if (nullptr == dds_subscriber_)
        {
            throw DDSMiddlewareException("Error creating a subscriber");
        }
        else
        {
            std::cout << "[soss-dds][server][sub]: "
                      << attributes.topic.topicDataType << " in topic "
                      << attributes.topic.topicName << std::endl;
        }
    }

    // Create Publisher
    {
        fastrtps::PublisherAttributes attributes;
        attributes.topic.topicKind = NO_KEY; //Check this
        attributes.topic.topicName = service_name + "_Request";
        attributes.topic.topicDataType = request_type.name();

        if (config["service_instance_name"])
        {
            eprosima::fastrtps::rtps::Property instance_property;
            instance_property.name("dds.rpc.service_instance_name");
            instance_property.value(config["service_instance_name"].as<std::string>());
            attributes.properties.properties().push_back(instance_property);
        }

        dds_publisher_ = fastrtps::Domain::createPublisher(participant->get_dds_participant(), attributes, this);

        if (nullptr == dds_publisher_)
        {
            throw DDSMiddlewareException("Error creating a publisher");
        }
        else
        {
            std::cout << "[soss-dds][server][pub]: "
                      << attributes.topic.topicDataType << " in topic "
                      << attributes.topic.topicName << std::endl;
        }
    }
}

Server::~Server()
{
    std::cout << "[soss-dds][server]: waiting current processing messages..." << std::endl;

    std::cout << "[soss-dds][server]: wait finished." << std::endl;

    request_dynamic_data_ = nullptr;
    reply_dynamic_data_ = nullptr;

    //fastrtps::Domain::removeSubscriber(dds_subscriber_);
    //fastrtps::Domain::removePublisher(dds_publisher_);

    std::unique_lock<std::mutex> lock(thread_mtx_);
    stop_ = true;
    for (std::thread& thread : reception_threads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

bool Server::add_config(
        const YAML::Node& config)
{
    // Map discriminator to type from config
    if (config["remap"])
    {
        if (config["remap"]["dds"]) // Or name...
        {
            if (config["remap"]["dds"]["type"])
            {
                std::string req;
                std::string disc = config["remap"]["dds"]["type"].as<std::string>();
                const ::xtypes::DynamicType& member_type = Conversion::resolve_discriminator_type(request_type_, disc);
                if (member_type.name().find("::") == 0)
                {
                    req = member_type.name().substr(2);
                }
                else
                {
                    req = member_type.name();
                }
                type_to_discriminator_[req] = disc;
                if (config["type"])
                {
                    if (config["type"].as<std::string>() != req)
                    {
                        // Add alias from other types
                        // TODO - Solve path?
                        type_to_discriminator_[config["type"].as<std::string>()] = disc;
                    }
                }
                std::cout << "[soss-dds] server: member \"" << disc << "\" has type \""
                          << member_type.name() << "\"." << std::endl;
            }
            else
            {
                std::string req;
                std::string req_alias;
                if (config["remap"]["dds"]["request_type"])
                {
                    std::string disc = config["remap"]["dds"]["request_type"].as<std::string>();
                    const ::xtypes::DynamicType& member_type = Conversion::resolve_discriminator_type(request_type_, disc);
                    if (member_type.name().find("::") == 0)
                    {
                        req = member_type.name().substr(2);
                    }
                    else
                    {
                        req = member_type.name();
                    }
                    type_to_discriminator_[req] = disc;
                    if (config["request_type"])
                    {
                        if (config["request_type"].as<std::string>() != req)
                        {
                            // Add alias from other types
                            // TODO - Solve path?
                            req_alias = config["request_type"].as<std::string>();
                            type_to_discriminator_[req_alias] = disc;
                        }
                    }
                    std::cout << "[soss-dds] server: member \"" << disc << "\" has request type \""
                              << member_type.name() << "\"." << std::endl;
                }
                if (config["remap"]["dds"]["reply_type"])
                {
                    std::string rep;
                    std::string disc = config["remap"]["dds"]["reply_type"].as<std::string>();
                    const ::xtypes::DynamicType& member_type = Conversion::resolve_discriminator_type(reply_type_, disc);
                    if (member_type.name().find("::") == 0)
                    {
                        rep = member_type.name().substr(2);
                    }
                    else
                    {
                        rep = member_type.name();
                    }
                    request_reply_[req] = rep;
                    type_to_discriminator_[rep] = disc;
                    if (config["reply_type"])
                    {
                        if (config["reply_type"].as<std::string>() != req)
                        {
                            // Add alias from other types
                            // TODO - Solve path?
                            type_to_discriminator_[config["reply_type"].as<std::string>()] = disc;
                            request_reply_[req_alias] = rep;
                        }
                    }
                    std::cout << "[soss-dds] server: member \"" << disc << "\" has reply type \""
                              << member_type.name() << "\"." << std::endl;
                }
            }
        }
    }
    return true;
}

void Server::call_service(
        const ::xtypes::DynamicData& soss_request,
        ServiceClient& client,
        std::shared_ptr<void> call_handle)
{
    bool success = false;
    ::xtypes::DynamicData request(request_type_);

    if (soss_request.type().name().find("::") == 0)
    {
        Conversion::access_member_data(request, type_to_discriminator_[soss_request.type().name().substr(2)]) = soss_request;
    }
    else
    {
        Conversion::access_member_data(request, type_to_discriminator_[soss_request.type().name()]) = soss_request;
    }

    std::cout << "[soss-dds][server]: translate request: soss -> dds "
        "(" << service_name_ << ") " << std::endl;

    success = Conversion::soss_to_dds(request, request_dynamic_data_);

    if (success)
    {
        callhandle_client_[call_handle] = &client;
        fastrtps::rtps::WriteParams params;
        std::unique_lock<std::mutex> lock(mtx_);
        success = dds_publisher_->write(request_dynamic_data_, params);
        if (!success)
        {
            std::cout << "Failed to publish message into DDS world." << std::endl;
        }
        else
        {
            fastrtps::rtps::SampleIdentity sample = params.sample_identity();
            sample_callhandle_[sample] = call_handle;
            if (request_reply_.count(soss_request.type().name()) > 0)
            {
                reply_id_type_[sample] = request_reply_[soss_request.type().name()];
            }
        }
    }
    else
    {
        std::cerr << "Error converting message from soss message to dynamic types." << std::endl;
    }
}

void Server::onPublicationMatched(
        fastrtps::Publisher* /* publisher */,
        fastrtps::rtps::MatchingInfo& info)
{
    std::string matching = fastrtps::rtps::MatchingStatus::MATCHED_MATCHING == info.status ? "matched" : "unmatched";
    std::cout << "[soss-dds][server]: " << matching <<
        " (" << service_name_ << ") " << std::endl;
}

void Server::onSubscriptionMatched(
        fastrtps::Subscriber* /* sub */,
        fastrtps::rtps::MatchingInfo& info)
{
    std::string matching = fastrtps::rtps::MatchingStatus::MATCHED_MATCHING == info.status ? "matched" : "unmatched";
    std::cout << "[soss-dds][server]: " << matching <<
        " (" << service_name_ << ") " << std::endl;
}

void Server::onNewDataMessage(
        fastrtps::Subscriber* /* sub */)
{
    using namespace std::placeholders;
    fastrtps::SampleInfo_t info;
    // TODO Protect reply_dynamic_data or create a local variable (copying it to the thread)
    if (dds_subscriber_->takeNextData(reply_dynamic_data_, &info))
    {
        if (ALIVE == info.sampleKind)
        {
            std::unique_lock<std::mutex> lock(thread_mtx_);
            if (!stop_)
            {
                reception_threads_.emplace_back(&Server::receive, this, info.related_sample_identity);
            }
        }
    }
}

void Server::receive(
        fastrtps::rtps::SampleIdentity sample_id)
{
    std::shared_ptr<void> call_handle;

    {
        std::unique_lock<std::mutex> lock(mtx_);
        call_handle = sample_callhandle_[sample_id];
    }

    ::xtypes::DynamicData received(reply_type_);
    bool success = Conversion::dds_to_soss(reply_dynamic_data_, received);

    if (success)
    {
        std::string path = reply_type_.name();
        if (reply_id_type_.count(sample_id) > 0)
        {
            path = type_to_discriminator_[reply_id_type_[sample_id]];
        }

        ::xtypes::WritableDynamicDataRef ref = Conversion::access_member_data(received, path);
        ::xtypes::DynamicData message(ref, ref.type());
        callhandle_client_[call_handle]->receive_response(
            call_handle,
            message);

        callhandle_client_.erase(call_handle);
        sample_callhandle_.erase(sample_id);
        reply_id_type_.erase(sample_id);
    }
    else
    {
        std::cerr << "Error converting message from dynamic types to soss message." << std::endl;
    }
}

} // namespace dds
} // namespace soss
