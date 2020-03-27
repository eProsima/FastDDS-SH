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
        const std::string& topic_name,
        const ::xtypes::DynamicType& message_type,
        const YAML::Node& config)

    : topic_name_{topic_name}
    , message_type_{message_type}
    , reception_threads_{}
{
    DynamicTypeBuilder* builder = Conversion::create_builder(message_type);

    if (builder != nullptr)
    {
        participant->register_dynamic_type(topic_name, message_type.name(), builder);
    }
    else
    {
        throw DDSMiddlewareException("Cannot create builder for type " + message_type.name());
    }

    dynamic_data_ = participant->create_dynamic_data(topic_name);

    // TODO Map discriminator to type from config
    (void)config;

    fastrtps::SubscriberAttributes attributes;
    attributes.topic.topicKind = NO_KEY;
    attributes.topic.topicName = topic_name;
    attributes.topic.topicDataType = message_type.name();

    dds_subscriber_ = fastrtps::Domain::createSubscriber(participant->get_dds_participant(), attributes, this);

    if (nullptr == dds_subscriber_)
    {
        throw DDSMiddlewareException("Error creating a subscriber");
    }
}

Server::~Server()
{
    std::cout << "[soss-dds][server]: waiting current processing messages..." << std::endl;
    for (std::thread& thread: reception_threads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    std::cout << "[soss-dds][server]: wait finished." << std::endl;

    fastrtps::Domain::removeSubscriber(dds_subscriber_);
    fastrtps::Domain::removePublisher(dds_publisher_);
}

void Server::call_service(
        const ::xtypes::DynamicData& soss_request,
        ServiceClient& client,
        std::shared_ptr<void> call_handle)
{
    bool success = false;
    ::xtypes::DynamicData request(message_type_);

    request[type_to_discriminator_[soss_request.type().name()]] = soss_request;

    std::cout << "[soss-dds][server]: translate request: soss -> dds "
        "(" << topic_name_ << ") " << std::endl;

    success = Conversion::soss_to_dds(request, static_cast<DynamicData*>(dynamic_data_.get()));

    if (success)
    {
        callhandle_client_[call_handle] = &client;
        fastrtps::rtps::WriteParams params;
        success = dds_publisher_->write(dynamic_data_.get(), params);
        // TODO Retrieve sample_id from the publisher
        fastrtps::rtps::SampleIdentity sample = params.sample_identity();
        sample_callhandle_[sample] = call_handle;
        //sample_callhandle_.emplace(std::make_pair(sample, call_handle));
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
        " (" << topic_name_ << ") " << std::endl;
}

void Server::onSubscriptionMatched(
        fastrtps::Subscriber* /* sub */,
        fastrtps::rtps::MatchingInfo& info)
{
    std::string matching = fastrtps::rtps::MatchingStatus::MATCHED_MATCHING == info.status ? "matched" : "unmatched";
    std::cout << "[soss-dds][server]: " << matching <<
        " (" << topic_name_ << ") " << std::endl;
}

void Server::onNewDataMessage(
        fastrtps::Subscriber* /* sub */)
{
    using namespace std::placeholders;
    fastrtps::SampleInfo_t info;
    if (dds_subscriber_->takeNextData(dynamic_data_.get(), &info))
    {
        if (ALIVE == info.sampleKind)
        {
            //auto sample_id = info.sample_identity;
            fastrtps::rtps::SampleIdentity sample_id = info.related_sample_identity; // TODO Verify
            std::shared_ptr<void> call_handle = sample_callhandle_[sample_id];

            ::xtypes::DynamicData received(message_type_);
            bool success = Conversion::dds_to_soss(static_cast<DynamicData*>(dynamic_data_.get()), received);

            if (success)
            {
                callhandle_client_[call_handle]->receive_response(
                    call_handle,
                    received[type_to_discriminator_[dynamic_data_->get_name()]]);

                callhandle_client_.erase(call_handle);
                sample_callhandle_.erase(sample_id);
            }
            else
            {
                std::cerr << "Error converting message from dynamic types to soss message." << std::endl;
            }
        }
    }
}

} // namespace dds
} // namespace soss
