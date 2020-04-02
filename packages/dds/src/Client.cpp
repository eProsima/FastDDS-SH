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

#include "Client.hpp"
#include "Conversion.hpp"

#include "Participant.hpp"
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/Domain.h>

#include <iostream>

using namespace eprosima;

namespace soss {
namespace dds {

Client::Client(
        Participant* participant,
        const std::string& service_name,
        const ::xtypes::DynamicType& request_type,
        const ::xtypes::DynamicType& reply_type,
        ServiceClientSystem::RequestCallback callback,
        const YAML::Node& config)
    : request_type_{request_type}
    , reply_type_{reply_type}
    , callback_{callback}
    , service_name_{service_name}
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
        attributes.topic.topicName = service_name_ + "_Request";
        attributes.topic.topicDataType = request_type.name();

        dds_subscriber_ = fastrtps::Domain::createSubscriber(participant->get_dds_participant(), attributes, this);

        if (nullptr == dds_subscriber_)
        {
            throw DDSMiddlewareException("Error creating a subscriber");
        }
        else
        {
            std::cout << "[soss-dds][client][sub]: "
                      << attributes.topic.topicDataType << " in topic "
                      << attributes.topic.topicName << std::endl;
        }
    }

    // Create publisher
    {
        fastrtps::PublisherAttributes attributes;
        attributes.topic.topicKind = NO_KEY; //Check this
        attributes.topic.topicName = service_name + "_Reply";
        attributes.topic.topicDataType = reply_type.name();

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
            std::cout << "[soss-dds][client][pub]: "
                      << attributes.topic.topicDataType << " in topic "
                      << attributes.topic.topicName << std::endl;
        }
    }
}

Client::~Client()
{
    request_dynamic_data_ = nullptr;
    reply_dynamic_data_ = nullptr;

    fastrtps::Domain::removePublisher(dds_publisher_);
    fastrtps::Domain::removeSubscriber(dds_subscriber_);

    for (std::thread& thread: reception_threads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

void Client::add_member(
        const ::xtypes::DynamicType& type,
        const std::string& path)
{
    std::shared_ptr<NavigationNode> root;
    if (member_tree_.count(type.name()) > 0)
    {
        root = member_tree_[type.name()];
    }
    else
    {
        root = std::make_shared<NavigationNode>();
        member_tree_[type.name()] = root;
    }
    NavigationNode::fill_root_node(root, type, path);
}

bool Client::add_config(
        const YAML::Node& config)
{
    // Map discriminator to type from config
    if (config["remap"])
    {
        if (config["remap"]["dds"]) // Or name...
        {
            if (config["remap"]["dds"]["type"])
            {
                std::string disc = config["remap"]["dds"]["type"].as<std::string>();
                const ::xtypes::DynamicType& member_type = Conversion::resolve_discriminator_type(request_type_, disc);
                type_to_discriminator_[member_type.name()] = disc;
                member_types_.push_back(member_type.name());
                std::cout << "[soss-dds] client: member \"" << disc << "\" has type \""
                          << member_type.name() << "\"." << std::endl;

                add_member(request_type_, disc);
            }
            else
            {
                std::string req;
                if (config["remap"]["dds"]["request_type"])
                {
                    std::string disc = config["remap"]["dds"]["request_type"].as<std::string>();
                    const ::xtypes::DynamicType& member_type =
                        Conversion::resolve_discriminator_type(request_type_, disc);
                    type_to_discriminator_[member_type.name()] = disc;
                    member_types_.push_back(member_type.name());
                    std::cout << "[soss-dds] client: member \"" << disc << "\" has request type \""
                              << member_type.name() << "\"." << std::endl;
                    req = member_type.name();
                    add_member(request_type_, disc);
                }
                if (config["remap"]["dds"]["reply_type"])
                {
                    std::string disc = config["remap"]["dds"]["reply_type"].as<std::string>();
                    const ::xtypes::DynamicType& member_type =
                        Conversion::resolve_discriminator_type(reply_type_, disc);
                    type_to_discriminator_[member_type.name()] = disc;
                    member_types_.push_back(member_type.name());
                    std::cout << "[soss-dds] client: member \"" << disc << "\" has reply type \""
                              << member_type.name() << "\"." << std::endl;
                    request_reply_[req] = member_type.name();
                    add_member(reply_type_, disc);
                }
            }
        }
    }
    return true;
}

void Client::receive_response(
        std::shared_ptr<void> call_handle,
        const ::xtypes::DynamicData& response)
{
    fastrtps::rtps::WriteParams params;
    fastrtps::rtps::SampleIdentity sample_id = *static_cast<fastrtps::rtps::SampleIdentity*>(call_handle.get());
    params.related_sample_identity(sample_id);

    std::string path = reply_type_.name();
    if (reply_id_type_.count(sample_id) > 0)
    {
        path = type_to_discriminator_[reply_id_type_[sample_id]];
        reply_id_type_.erase(sample_id);
    }

    ::xtypes::DynamicData reply(reply_type_);

    Conversion::access_member_data(reply, path) = response;

    std::cout << "[soss-dds][client]: translate reply: soss -> dds "
        "(" << service_name_ << ") " << std::endl;

    bool success = Conversion::soss_to_dds(reply, static_cast<DynamicData*>(reply_dynamic_data_.get()));

    if (success)
    {
        success = dds_publisher_->write(reply_dynamic_data_.get(), params);
    }
    else
    {
        std::cerr << "Error converting message from soss message to dynamic types." << std::endl;
    }
}

void Client::onPublicationMatched(
        fastrtps::Publisher* /* publisher */,
        fastrtps::rtps::MatchingInfo& info)
{
    std::string matching = fastrtps::rtps::MatchingStatus::MATCHED_MATCHING == info.status ? "matched" : "unmatched";
    std::cout << "[soss-dds][client]: " << matching <<
        " (" << service_name_ << ") " << std::endl;
}


void Client::onSubscriptionMatched(
        fastrtps::Subscriber* /* sub */,
        fastrtps::rtps::MatchingInfo& info)
{
    std::string matching = fastrtps::rtps::MatchingStatus::MATCHED_MATCHING == info.status ? "matched" : "unmatched";
    std::cout << "[soss-dds][client]: " << matching <<
        " (" << service_name_ << ") " << std::endl;
}

void Client::onNewDataMessage(
        fastrtps::Subscriber* /* sub */)
{
    using namespace std::placeholders;
    fastrtps::SampleInfo_t info;
    // TODO Protect request_dynamic_data or create a local variable (copying it to the thread)
    if (dds_subscriber_->takeNextData(request_dynamic_data_.get(), &info))
    {
        if (ALIVE == info.sampleKind)
        {
            reception_threads_.emplace_back(std::thread(&Client::receive, this, info.sample_identity));
        }
    }
}

void Client::receive(
        eprosima::fastrtps::rtps::SampleIdentity sample_id)
{
    ::xtypes::DynamicData received(request_type_);
    bool success = Conversion::dds_to_soss(static_cast<DynamicData*>(request_dynamic_data_.get()), received);

    if (success)
    {
        std::shared_ptr<NavigationNode> member =
            NavigationNode::get_discriminator(member_tree_, received, member_types_);

        if (request_reply_.count(member->type_name) > 0)
        {
            reply_id_type_[sample_id] = request_reply_[member->type_name];
        }

        ::xtypes::WritableDynamicDataRef ref =
            Conversion::access_member_data(received, member->get_path());
        ::xtypes::DynamicData message(ref, ref.type());
        callback_(
            message,
            *this, std::make_shared<fastrtps::rtps::SampleIdentity>(sample_id));
    }
    else
    {
        std::cerr << "Error converting message from dynamic types to soss message." << std::endl;
    }
}

} // namespace dds
} // namespace soss
