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

#include "SystemHandle.hpp"

#include <soss/Mix.hpp>
#include <soss/Search.hpp>

#include <iostream>
#include <thread>

namespace soss {
namespace dds{

bool SystemHandle::configure(
    const RequiredTypes& /* types */,
    const YAML::Node& /* configuration */)
{
    std::cout << "[soss-dds]: configured!" << std::endl;
    return true;
}

bool SystemHandle::okay() const
{
    return true;
}

bool SystemHandle::spin_once()
{
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    return okay();
}

bool SystemHandle::subscribe(
    const std::string& topic_name,
    const std::string& message_type,
    SubscriptionCallback /* callback */,
    const YAML::Node& /* configuration */)
{
    std::cout << "[soss-dds]: subscriber created. "
        "topic: " << topic_name << ". "
        "type: " << message_type << ". "
        << std::endl;

    return true;
}

std::shared_ptr<TopicPublisher> SystemHandle::advertise(
    const std::string& topic_name,
    const std::string& message_type,
    const YAML::Node& /* configuration */)
{
    std::cout << "[soss-dds]: publisher created. "
        "topic: " << topic_name << ". "
        "type: " << message_type << ". "
        << std::endl;

    return std::shared_ptr<TopicPublisher>();
}


} // namespace dds
} // namespace soss

SOSS_REGISTER_SYSTEM("dds", soss::dds::SystemHandle)
