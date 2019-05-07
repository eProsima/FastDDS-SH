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

#include "Participant.hpp"
#include "Publisher.hpp"
#include "Subscriber.hpp"

#include <iostream>
#include <thread>

namespace soss {
namespace dds{


SystemHandle::~SystemHandle() = default;

bool SystemHandle::configure(
    const RequiredTypes& /* types */,
    const YAML::Node& /* configuration */)
{
    try
    {
        participant_ = std::make_unique<Participant>();
    }
    catch(DDSMiddlewareException& e)
    {
        std::cerr << "[soss-dds]: " << e.what() << std::endl;
        return false;
    }
    std::cout << "[soss-dds]: configured!" << std::endl;

    return true;
}

bool SystemHandle::okay() const
{
    //TODO
    return true;
}

bool SystemHandle::spin_once()
{
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    //check TODO
    return okay();
}

bool SystemHandle::subscribe(
    const std::string& topic_name,
    const std::string& message_type,
    SubscriptionCallback callback,
    const YAML::Node& /* configuration */)
{
    try
    {
        auto subscriber = std::make_shared<Subscriber>(participant_.get(), topic_name, message_type, callback);
        subscribers_.emplace_back(std::move(subscriber));

        std::cout << "[soss-dds]: subscriber created. "
            "topic: " << topic_name << ", "
            "type: " << message_type << std::endl;

        return true;
    }
    catch(DDSMiddlewareException& e)
    {
        std::cerr << "[soss-dds]: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<TopicPublisher> SystemHandle::advertise(
    const std::string& topic_name,
    const std::string& message_type,
    const YAML::Node& /* configuration */)
{
    try
    {
        auto publisher = std::make_shared<Publisher>(participant_.get(), topic_name, message_type);
        publishers_.emplace_back(std::move(publisher));

        std::cout << "[soss-dds]: publisher created. "
            "topic: " << topic_name << ", "
            "type: " << message_type << std::endl;

        return publishers_.back();
    }
    catch(DDSMiddlewareException& e)
    {
        std::cerr << "[soss-dds]: " << e.what() << std::endl;
        return std::shared_ptr<TopicPublisher>();
    }
}


} // namespace dds
} // namespace soss

SOSS_REGISTER_SYSTEM("dds", soss::dds::SystemHandle)
