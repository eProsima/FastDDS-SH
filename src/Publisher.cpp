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

#include "Publisher.hpp"
#include "Participant.hpp"
#include "Conversion.hpp"

#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>
#include <fastrtps/types/DynamicData.h>

#include <iostream>

namespace soss {
namespace dds {


Publisher::Publisher(
        Participant* participant,
        const std::string& topic_name,
        const std::string& message_type)

    : topic_name_{topic_name}
    , message_type_{message_type}
{
    eprosima::fastrtps::PublisherAttributes attributes;
    attributes.topic.topicKind = eprosima::fastrtps::NO_KEY; //Check this
    attributes.topic.topicName = "hello_dds"; /* topic_name_ */;
    attributes.topic.topicDataType = "strings_255" /* message_type_ */;

    dds_publisher_ = eprosima::fastrtps::Domain::createPublisher(participant->get_dds_participant(), attributes, this);

    if (nullptr == dds_publisher_)
    {
        throw DDSMiddlewareException("Error creating a publisher");
    }

    dynamic_data_ = participant->create_dynamic_data(/* message_type_ */);
}

Publisher::~Publisher()
{
    eprosima::fastrtps::Domain::removePublisher(dds_publisher_);
}

bool Publisher::publish(
        const soss::Message& soss_message)
{
    std::cout << "[soss-dds][publisher]: translate message: soss -> dds "
        "(" << topic_name_ << ") " << std::endl;

    //std::string dds_message =
    Conversion::soss_to_dds(soss_message);

    dynamic_data_->SetStringValue("hello_dds");

    dds_publisher_->write(dynamic_data_.get());

    return true;
}


void Publisher::onPublicationMatched(
        eprosima::fastrtps::Publisher* /* publisher */,
        eprosima::fastrtps::rtps::MatchingInfo& /* info */)
{
    std::cout << "Publisher matched!" << std::endl; //TEMP_TRACE
}


} // namespace dds
} // namespace soss
