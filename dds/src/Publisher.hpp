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

#ifndef SOSS__DDS__INTERNAL__PUBLISHER_HPP
#define SOSS__DDS__INTERNAL__PUBLISHER_HPP

#include "DDSMiddlewareException.hpp"
#include "Participant.hpp"
#include "DynamicTypeAdapter.hpp"

#include <soss/Message.hpp>
#include <soss/SystemHandle.hpp>

#include <fastrtps/publisher/PublisherListener.h>

namespace soss {
namespace dds {

class Participant;

class Publisher : public virtual TopicPublisher, private eprosima::fastrtps::PublisherListener
{
public:

    Publisher(
            Participant* participant,
            const std::string& topic_name,
            const std::string& message_type);

    virtual ~Publisher() override;

    Publisher(
            const Publisher& rhs) = delete;

    Publisher& operator = (
            const Publisher& rhs) = delete;

    Publisher(
            Publisher&& rhs) = delete;

    Publisher& operator = (
            Publisher&& rhs) = delete;

    bool publish(
            const soss::Message& message) override;

private:

    void onPublicationMatched(
            eprosima::fastrtps::Publisher* pub,
            eprosima::fastrtps::rtps::MatchingInfo& info) override;

    eprosima::fastrtps::Publisher* dds_publisher_;
    DynamicData_ptr dynamic_data_;

    const std::string topic_name_;
    const std::string message_type_;
};


} //namespace dds
} //namespace soss

#endif // SOSS__DDS__INTERNAL__PUBLISHER_HPP
