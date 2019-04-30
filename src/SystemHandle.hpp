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

#ifndef SOSS__FASTRTPS__INTERNAL__SYSTEMHANDLE_HPP
#define SOSS__FASTRTPS__INTERNAL__SYSTEMHANDLE_HPP

//#include "Subscriber.hpp"
//#include "Publisher.hpp"

#include <soss/SystemHandle.hpp>

#include <vector>
#include <memory>

namespace soss {
namespace dds {


class SystemHandle : public virtual TopicSystem
{
public:
    SystemHandle() = default;
    virtual ~SystemHandle() override = default;

    bool configure(
        const RequiredTypes& types,
        const YAML::Node& configuration) override;

    bool okay() const override;

    bool spin_once() override;

    bool subscribe(
        const std::string& topic_name,
        const std::string& message_type,
        SubscriptionCallback callback,
        const YAML::Node& configuration) override;

    std::shared_ptr<TopicPublisher> advertise(
        const std::string& topic_name,
        const std::string& message_type,
        const YAML::Node& configuration) override;

private:
    //std::vector<std::shared_ptr<Publisher>> publishers_;
    //std::vector<std::shared_ptr<Subscriber>> subscribers_;
};


} // namespace dds
} // namespace soss


#endif // SOSS__FASTRTPS__INTERNAL__SYSTEMHANDLE_HPP
