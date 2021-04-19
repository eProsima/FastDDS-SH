/*
 * Copyright 2019 - present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _IS_SH_FASTDDS__INTERNAL__SUBSCRIBER_HPP_
#define _IS_SH_FASTDDS__INTERNAL__SUBSCRIBER_HPP_

#include "DDSMiddlewareException.hpp"
#include "Participant.hpp"

#include <is/systemhandle/SystemHandle.hpp>
#include <is/utils/Log.hpp>

#include <fastdds/dds/subscriber/Subscriber.hpp>

#include <thread>
#include <condition_variable>

namespace fastdds = eprosima::fastdds;

namespace eprosima {
namespace is {
namespace sh {
namespace fastdds {

/**
 * @brief Forward declaration.
 */
class Participant;

/**
 * @class Subscriber
 *        This class represents a <a href="https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/subscriber/subscriber.html"
 *        Fast DDS Subscriber</a> within the *Integration Service* framework.
 *
 *        Its topic type definition and data instances are represented by means
 *        of the <a href="https://fast-dds.docs.eprosima.com/en/latest/fastdds/dynamic_types/dynamic_types.html">
 *        Fast DDS Dynamic Types</a> API, which allows to get rid of
 *        typesupport for each used type and eases users the task of defining and using their own custom
 *        types on the go, by means of a valid IDL definition.
 *
 *        This class inherits from <a href="https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/subscriber/dataReaderListener/dataReaderListener.html">
 *        Fast DDS Data Reader Listener</a> for reacting to datareader events,
 *        such as matching with publishers or receiving new data from them.
 */
class Subscriber : private ::fastdds::dds::DataReaderListener
{
public:

    /**
     * @brief Construct a new Subscriber object.
     *
     * @param[in] participant The associated *Integration Service* Participant,
     *            which holds this Subscriber.
     *
     * @param[in] topic_name The topic that this DDS subscriber will attach to.
     *
     * @param[in] message_type A dynamic type definition of the topic's type.
     *
     * @param[in] is_callback Callback function signature defined by the *Integration Service*,
     *            triggered each time a new data arrives to the DDS Subscriber.
     *
     * @throws DDSMiddlewareException if some error occurs while creating the Fast DDS subscriber.
     */
    Subscriber(
            Participant* participant,
            const std::string& topic_name,
            const xtypes::DynamicType& message_type,
            TopicSubscriberSystem::SubscriptionCallback is_callback);

    // TODO(@jamoralp): Create subscriber based on XML profiles?

    /**
     * @brief Destroy the Subscriber object.
     */
    virtual ~Subscriber();

    /**
     * @brief Subscriber shall not be copy constructible.
     */
    Subscriber(
            const Subscriber& /*rhs*/) = delete;

    /**
     * @brief Subscriber shall not be copy assignable.
     */
    Subscriber& operator = (
            const Subscriber& /*rhs*/) = delete;

    /**
     * @brief Subscriber shall not be move constructible.
     */
    Subscriber(
            Subscriber&& /*rhs*/) = delete;

    /**
     * @brief Subscriber shall not be move assignable.
     */
    Subscriber& operator = (
            Subscriber&& /*rhs*/) = delete;

    /**
     * @brief Handle the receiving of a new message from the DDS dataspace.
     *
     * @param[in] dds_message The incomming message.
     */
    void receive(
            const fastrtps::types::DynamicData* dds_message);

private:

    /**
     * @brief Inherited from *DataReaderListener*.
     */
    void on_data_available(
            ::fastdds::dds::DataReader* /*reader*/) override;

    /**
     * @brief Inherited from *DataReaderListener*.
     */
    void on_subscription_matched(
            ::fastdds::dds::DataReader* reader,
            const ::fastdds::dds::SubscriptionMatchedStatus& info) override;

    /**
     * @brief Function to look for threads that have already finished and delete them from the
     *        reception threads database.
     */
    void cleaner_function();

    /**
     * Class members.
     */
    Participant* participant_;
    ::fastdds::dds::Subscriber* dds_subscriber_;
    ::fastdds::dds::Topic* dds_topic_;
    ::fastdds::dds::DataReader* dds_datareader_;

    fastrtps::types::DynamicData* dynamic_data_;
    std::mutex data_mtx_;

    const std::string topic_name_;
    const xtypes::DynamicType& message_type_;

    TopicSubscriberSystem::SubscriptionCallback is_callback_;

    std::map<std::thread::id, std::thread*> reception_threads_;
    bool stop_cleaner_;
    std::vector<std::thread::id> finished_threads_;
    std::mutex cleaner_mtx_;
    std::condition_variable cleaner_cv_;
    std::thread cleaner_thread_;

    utils::Logger logger_;
};

} //  namespace fastdds
} //  namespace sh
} //  namespace is
} //  namespace eprosima

#endif // SOSS__DDS__INTERNAL__SUBSCRIBER_HPP
