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

#include "Subscriber.hpp"
#include "Conversion.hpp"

#include <soss/Message.hpp>

#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/Domain.h>

#include <functional>
#include <iostream>

using namespace eprosima;

namespace soss {
namespace dds {

Subscriber::Subscriber(
        Participant* participant,
        const std::string& topic_name,
        const ::xtypes::DynamicType& message_type,
        TopicSubscriberSystem::SubscriptionCallback soss_callback)

    : participant_{participant}
    , topic_name_{topic_name}
    , message_type_{message_type}
    , soss_callback_{soss_callback}
    , reception_threads_{}
    , stop_cleaner_{false}
    , cleaner_thread_{&Subscriber::cleaner_function, this}
{
    DynamicTypeBuilder* builder = Conversion::create_builder(message_type);

    if (builder != nullptr)
    {
        participant->register_dynamic_type(topic_name, message_type.name(), builder);
    }
    else
    {
        throw DDSMiddlewareException("Cannot create builder for type " + message_type.name());
    }

    dynamic_data_ = participant->create_dynamic_data(topic_name);

    fastrtps::SubscriberAttributes attributes;
    attributes.topic.topicKind = NO_KEY;
    attributes.topic.topicName = topic_name;
    attributes.topic.topicDataType = message_type.name();

    dds_subscriber_ = fastrtps::Domain::createSubscriber(participant->get_dds_participant(), attributes, this);

    if (nullptr == dds_subscriber_)
    {
        throw DDSMiddlewareException("Error creating a subscriber");
    }
}

Subscriber::~Subscriber()
{
    std::cout << "[soss-dds][subscriber]: waiting current processing messages..." << std::endl;

    {
        std::unique_lock<std::mutex> lock(cleaner_mtx_);
        stop_cleaner_ = true;
        cleaner_cv_.notify_one();
    }

    if (cleaner_thread_.joinable())
    {
        cleaner_thread_.join();
    }

    std::cout << "[soss-dds][subscriber]: wait finished." << std::endl;

    std::unique_lock<std::mutex> lock(data_mtx_);
    participant_->delete_dynamic_data(dynamic_data_);
    dynamic_data_ = nullptr;
    fastrtps::Domain::removeSubscriber(dds_subscriber_);
}

void Subscriber::receive(
        const fastrtps::types::DynamicData* dds_message)
{
    std::cout << "[soss-dds][subscriber]: translate message: dds -> soss "
        "(" << topic_name_ << ") " << std::endl;

    {
        ::xtypes::DynamicData soss_message(message_type_);

        bool success = Conversion::dds_to_soss(dds_message, soss_message);
        data_mtx_.unlock();

        if (success)
        {
            soss_callback_(soss_message);
        }
        else
        {
            std::cerr << "Error converting message from soss message to dynamic types." << std::endl;
        }
    }

    // Notify that we have ended
    std::unique_lock<std::mutex> lock(cleaner_mtx_);
    finished_threads_.push_back(std::this_thread::get_id());
    cleaner_cv_.notify_one();
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
    using namespace std::placeholders;
    fastrtps::SampleInfo_t info;
    std::unique_lock<std::mutex> lock(cleaner_mtx_);
    data_mtx_.lock();
    if (!stop_cleaner_ && dds_subscriber_->takeNextData(dynamic_data_, &info))
    {
        if (ALIVE == info.sampleKind)
        {
            std::thread* thread = new std::thread(&Subscriber::receive, this, dynamic_data_);
            reception_threads_.emplace(thread->get_id(), thread);
        }
        else
        {
            data_mtx_.unlock();
        }
    }
    else
    {
        data_mtx_.unlock();
    }
}

void Subscriber::cleaner_function()
{
    using namespace std::chrono_literals;
    std::unique_lock<std::mutex> lock(cleaner_mtx_);
    std::vector<std::thread::id> stopped;

    while (!stop_cleaner_)
    {
        cleaner_cv_.wait(
            lock,
            [this]()
            {
                return stop_cleaner_ || !finished_threads_.empty();
            });

        for (std::thread::id id : finished_threads_)
        {
            // Some threads may end too quickly, wait until the next iteration
            if (reception_threads_.count(id) > 0)
            {
                std::thread* thread = reception_threads_.at(id);
                reception_threads_.erase(id);

                if (thread->joinable())
                {
                    thread->join();
                }
                delete thread;
                stopped.push_back(id);
            }
        }

        for (std::thread::id id : stopped)
        {
            auto it = std::find(finished_threads_.begin(), finished_threads_.end(), id);
            finished_threads_.erase(it);
        }
        stopped.clear();

        // Free the CPU just a moment
        lock.unlock();
        std::this_thread::sleep_for(10ms);
        lock.lock();
    }

    // Wait for the rest of threads
    for (auto& pair : reception_threads_)
    {
        std::thread* thread = pair.second;
        if (thread->joinable())
        {
            thread->join();
        }
        delete thread;
    }

}

} // namespace dds
} // namespace soss
