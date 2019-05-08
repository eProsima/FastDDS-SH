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
#include "Participant.hpp"
#include "Conversion.hpp"

#include <soss/Message.hpp>

#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
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
    eprosima::fastrtps::SubscriberAttributes attributes;
    attributes.topic.topicKind = eprosima::fastrtps::NO_KEY;
    attributes.topic.topicName = "hello_dds"; // topic_name;
    attributes.topic.topicDataType = "strings_255"; //message_type

    dds_subscriber_ = eprosima::fastrtps::Domain::createSubscriber(participant->get_dds_participant(), attributes, this);

    if (nullptr == dds_subscriber_)
    {
        throw DDSMiddlewareException("Error creating a subscriber");
    }

    dynamic_data_ = participant->create_dynamic_data(/* message_type_ */);
}

Subscriber::~Subscriber()
{
    eprosima::fastrtps::Domain::removeSubscriber(dds_subscriber_);
}

void Subscriber::receive(const std::string& dds_message)
{
    std::cout << "[soss-dds][subscriber]: translate message: dds -> soss "
        "(" << topic_name_ << ") " << std::endl;

    soss::Message soss_message = Conversion::dds_to_soss(message_type_, dds_message);

    soss_callback_(soss_message);
}

void Subscriber::onSubscriptionMatched(
        eprosima::fastrtps::Subscriber* /* sub */,
        eprosima::fastrtps::rtps::MatchingInfo& /* info */)
{
    std::cout << "Subscriber matched!" << std::endl; //TEMP_TRACE
}

void Subscriber::onNewDataMessage(
        eprosima::fastrtps::Subscriber* /* sub */)
{
    std::cout << "Data message!" << std::endl; //TEMP_TRACE
    /*
    eprosima::fastrtps::SampleInfo_t info;
    if(sub->takeNextData(dynamic_data_, &info))
    {
        if(info.sampleKind == ALIVE)
        {
            std::string message;
            dynamic_data_->GetStringValue(message);

            std::cout << "Data message!" << std::endl; //TEMP_TRACE
        }
    }
    */
}


} // namespace dds
} // namespace soss
