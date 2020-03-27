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
        const ::xtypes::DynamicType& service_type,
        ServiceClientSystem::RequestCallback callback,
        const YAML::Node& config)
    : message_type_{service_type}
    , callback_{callback}
    , service_name_{service_name}
{
    DynamicTypeBuilder* builder = Conversion::create_builder(service_type);

    if (builder != nullptr)
    {
        participant->register_dynamic_type(service_name, service_type.name(), builder);
    }
    else
    {
        throw DDSMiddlewareException("Cannot create builder for type " + service_type.name());
    }

    dynamic_data_ = participant->create_dynamic_data(service_name);

    fastrtps::PublisherAttributes attributes;
    attributes.topic.topicKind = NO_KEY; //Check this
    attributes.topic.topicName = service_name_;
    attributes.topic.topicDataType = service_type.name();

    if (config["service_instance_name"])
    {
        eprosima::fastrtps::rtps::Property instance_property;
        instance_property.name("dds.rpc.service_instance_name");
        instance_property.value(config["service_instance_name"].as<std::string>());
        attributes.properties.properties().push_back(instance_property);
    }

    // TODO Map discriminator to type from config

    dds_publisher_ = fastrtps::Domain::createPublisher(participant->get_dds_participant(), attributes, this);

    if (nullptr == dds_publisher_)
    {
        throw DDSMiddlewareException("Error creating a publisher");
    }
}

Client::~Client()
{
    fastrtps::Domain::removePublisher(dds_publisher_);
    fastrtps::Domain::removeSubscriber(dds_subscriber_);
}

void Client::receive_response(
        std::shared_ptr<void> call_handle,
        const ::xtypes::DynamicData& response)
{
    fastrtps::rtps::WriteParams params;
    params.related_sample_identity(*static_cast<fastrtps::rtps::SampleIdentity*>(call_handle.get()));

    ::xtypes::DynamicData request(message_type_);

    request[type_to_discriminator_[response.type().name()]] = response;

    std::cout << "[soss-dds][client]: translate request: soss -> dds "
        "(" << service_name_ << ") " << std::endl;

    bool success = Conversion::soss_to_dds(request, static_cast<DynamicData*>(dynamic_data_.get()));

    if (success)
    {
        success = dds_publisher_->write(dynamic_data_.get(), params);
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
    if (dds_subscriber_->takeNextData(dynamic_data_.get(), &info))
    {
        if (ALIVE == info.sampleKind)
        {
            fastrtps::rtps::SampleIdentity sample_id = info.sample_identity; // TODO Verify (related_sample_identity?)
            ::xtypes::DynamicData received(message_type_);
            bool success = Conversion::dds_to_soss(static_cast<DynamicData*>(dynamic_data_.get()), received);

            if (success)
            {
                callback_(
                    received[type_to_discriminator_[dynamic_data_->get_name()]],
                    *this, std::make_shared<fastrtps::rtps::SampleIdentity>(sample_id));
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
