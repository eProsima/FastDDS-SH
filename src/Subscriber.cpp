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

#include "Subscriber.hpp"
#include "Conversion.hpp"

#include <soss/Message.hpp>

#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/Domain.h>
#include <fastrtps/types/DynamicData.h>

#include <functional>
#include <iostream>

namespace soss {
namespace dds {

Subscriber::Subscriber(
        Participant* participant,
        const std::string& topic_name,
        const std::string& message_type,
        TopicSubscriberSystem::SubscriptionCallback soss_callback)

    : topic_name_{topic_name}
    , message_type_{message_type}
    , soss_callback_{soss_callback}
{
    dynamic_data_ = participant->create_dynamic_data(message_type);

    fastrtps::SubscriberAttributes attributes;
    attributes.topic.topicKind = fastrtps::NO_KEY;
    attributes.topic.topicName = "hello_dds"; // topic_name;
    attributes.topic.topicDataType = message_type;

    dds_subscriber_ = fastrtps::Domain::createSubscriber(participant->get_dds_participant(), attributes, this);

    if (nullptr == dds_subscriber_)
    {
        throw DDSMiddlewareException("Error creating a subscriber");
    }
}

Subscriber::~Subscriber()
{
    fastrtps::Domain::removeSubscriber(dds_subscriber_);
}

void Subscriber::receive(const fastrtps::types::DynamicData_ptr dds_message)
{
    std::cout << "[soss-dds][subscriber]: translate message: dds -> soss "
        "(" << topic_name_ << ") " << std::endl;

    soss::Message soss_message;

    bool success = Conversion::dds_to_soss(message_type_, dds_message, soss_message);

    if (success)
    {
        soss_callback_(soss_message);
    }
    else
    {
        std::cerr << "Error converting message from soss message to dynamic types." << std::endl;
    }
}

void Subscriber::onSubscriptionMatched(
        fastrtps::Subscriber* /* sub */,
        fastrtps::rtps::MatchingInfo& info)
{
    std::string matching = fastrtps::rtps::MatchingStatus::MATCHED_MATCHING == info.status ? "matched" : "unmatched";
    std::cout << "[soss-dds][subscriber]: " << matching <<
        " (" << topic_name_ << ") " << std::endl;
}

void Subscriber::onNewDataMessage(
        fastrtps::Subscriber* /* sub */)
{
    fastrtps::SampleInfo_t info;
    if(dds_subscriber_->takeNextData(dynamic_data_.get(), &info))
    {
        if(fastrtps::ALIVE == info.sampleKind)
        {
            receive(dynamic_data_);
        }
    }
}


} // namespace dds
} // namespace soss
