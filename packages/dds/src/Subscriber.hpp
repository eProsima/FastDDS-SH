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

#ifndef SOSS__DDS__INTERNAL__SUBSCRIBER_HPP
#define SOSS__DDS__INTERNAL__SUBSCRIBER_HPP

#include "DDSMiddlewareException.hpp"
#include "Participant.hpp"
#include "DynamicTypeAdapter.hpp"

#include <soss/SystemHandle.hpp>

#include <fastrtps/subscriber/SubscriberListener.h>

#include <thread>
#include <condition_variable>

namespace soss {
namespace dds {

class Participant;

class Subscriber : private eprosima::fastrtps::SubscriberListener
{
public:

    Subscriber(
            Participant* participant,
            const std::string& topic_name,
            const xtypes::DynamicType& message_type,
            TopicSubscriberSystem::SubscriptionCallback soss_callback);

    virtual ~Subscriber();

    Subscriber(
            const Subscriber& rhs) = delete;

    Subscriber& operator = (
            const Subscriber& rhs) = delete;


    Subscriber(
            Subscriber&& rhs) = delete;


    Subscriber& operator = (
            Subscriber&& rhs) = delete;

    void receive(
            const DynamicData* dds_message);

private:

    void onSubscriptionMatched(
            eprosima::fastrtps::Subscriber* sub,
            eprosima::fastrtps::rtps::MatchingInfo& info) override;

    void onNewDataMessage(
            eprosima::fastrtps::Subscriber* sub) override;

    Participant* participant_;
    eprosima::fastrtps::Subscriber* dds_subscriber_;
    DynamicData* dynamic_data_;
    std::mutex data_mtx_;

    const std::string topic_name_;
    const xtypes::DynamicType& message_type_;

    TopicSubscriberSystem::SubscriptionCallback soss_callback_;
    std::map<std::thread::id, std::thread*> reception_threads_;
    bool stop_cleaner_;
    std::thread cleaner_thread_;
    std::vector<std::thread::id> finished_threads_;
    std::mutex cleaner_mtx_;
    std::condition_variable cleaner_cv_;

    void cleaner_function();
};


} // namespace dds
} // namespace soss

#endif // SOSS__DDS__INTERNAL__SUBSCRIBER_HPP
