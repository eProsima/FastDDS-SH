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

#include <functional>
#include <iostream>

namespace soss {
namespace dds {

Subscriber::Subscriber(
        /* dds::Subscriber* subscriber */
        const std::string& topic_name,
        const std::string& message_type,
        TopicSubscriberSystem::SubscriptionCallback soss_callback)

    : topic_name_{topic_name}
    , message_type_{message_type}
    , soss_callback_{soss_callback}
{
    //TODO
}

bool Subscriber::subscribe()
{
    //TODO: its really necessary this function?
    return true;
}

void Subscriber::receive(const std::string& dds_message)
{
    std::cout << "[soss-dds][subscriber]: translate message: dds -> soss "
        "(" << topic_name_ << ") " << std::endl;

    soss::Message soss_message = Conversion::dds_to_soss(message_type_, dds_message);

    soss_callback_(soss_message);
}


} // namespace dds
} // namespace soss
