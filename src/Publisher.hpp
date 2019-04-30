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

#ifndef SOSS__FASTRTPS__INTERNAL__PUBLISHER_HPP
#define SOSS__FIWARE__INTERNAL__PUBLISHER_HPP

#include <soss/Message.hpp>
#include <soss/SystemHandle.hpp>

namespace soss {
namespace dds {


class Publisher : public virtual TopicPublisher
{
public:
    Publisher(
            /* Participant* participan, */
            const std::string& topic_name,
            const std::string& message_type);

    ~Publisher() = default;

    Publisher(const Publisher& rhs) = delete;
    Publisher& operator = (const Publisher& rhs) = delete;
    Publisher(Publisher&& rhs) = delete;
    Publisher& operator = (Publisher&& rhs) = delete;

    bool publish(
            const soss::Message& message) override;

private:
    const std::string topic_name_;
    const std::string message_type_;
};


} //namespace dds
} //namespace soss

#endif // SOSS__FASTRTPS__INTERNAL__PUBLISHER_HPP
