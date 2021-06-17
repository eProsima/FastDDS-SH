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

#ifndef _IS_SH_FASTDDS__INTERNAL__PUBLISHER_HPP_
#define _IS_SH_FASTDDS__INTERNAL__PUBLISHER_HPP_

#include "DDSMiddlewareException.hpp"
#include "Participant.hpp"

#include <is/core/Message.hpp>
#include <is/systemhandle/SystemHandle.hpp>
#include <is/utils/Log.hpp>

#include <fastdds/dds/publisher/Publisher.hpp>

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
 * @class Publisher
 *        This class represents a <a href="https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/publisher/publisher.html">
 *        Fast DDS Publisher</a> within the *Integration Service* framework.
 *
 *        Its topic type definition and data instances are represented by means
 *        of the <a href="https://fast-dds.docs.eprosima.com/en/latest/fastdds/dynamic_types/dynamic_types.html">
 *        Fast DDS Dynamic Types</a> API, which allows to get rid of
 *        TypeSupport for each used type and eases users the task of defining and using their own custom
 *        types on the go, by means of a valid *IDL* definition.
 *
 *        This class inherits from <a href="https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/publisher/dataWriterListener/dataWriterListener.html">
 *        Fast DDS Data Writer Listener</a> for reacting to datawriter events,
 *        such as matching with subscribers.
 */
class Publisher
    : public virtual is::TopicPublisher
    , private ::fastdds::dds::DataWriterListener
{
public:

    /**
     * @brief Construct a new Publisher object.
     *
     * @param[in] participant The associated *Integration Service* Participant,
     *            that holds this Publisher.
     *
     * @param[in] topic_name The topic that this DDS publisher will send data to.
     *
     * @param[in] message_type A dynamic type definition of the topic's type.
     *
     * @param[in] config Specific configuration regarding this publisher, in *YAML* format.
     *            Allowed fields are:
     *            - `service_instance_name`: Specify the DDS RPC service instance name property.
     *
     * @throws DDSMiddlewareException if some error occurs while creating the *Fast DDS* publisher.
     */
    Publisher(
            Participant* participant,
            const std::string& topic_name,
            const xtypes::DynamicType& message_type,
            const YAML::Node& config);

    // TODO(@jamoralp): Create publisher based on XML profiles?

    /**
     * @brief Destroy the Publisher object.
     */
    virtual ~Publisher() override;

    /**
     * @brief Publisher shall not be copy constructible.
     */
    Publisher(
            const Publisher& /*rhs*/) = delete;

    /**
     * @brief Publisher shall not be copy assignable.
     */
    Publisher& operator = (
            const Publisher& /*rhs*/) = delete;

    /**
     * @brief Publisher shall not be move constructible.
     */
    Publisher(
            Publisher&& /*rhs*/) = delete;

    /**
     * @brief Publisher shall not be move assignable.
     */
    Publisher& operator = (
            Publisher&& /*rhs*/) = delete;

    /**
     * @brief Inherited from TopicPublisher.
     */
    bool publish(
            const xtypes::DynamicData& message) override;


    /**
     * @brief Get the topic name where this publisher sends data to.
     *
     * @returns The topic name.
     */
    const std::string& topic_name() const;

    /**
     * @brief Get the DDS instance handle object for the associated datawriter.
     *
     * @returns The datawriter instance handle.
     */
    const fastrtps::rtps::InstanceHandle_t get_dds_instance_handle() const;

private:

    /**
     * @brief Inherited from *DataWriterListener*.
     */
    void on_publication_matched(
            ::fastdds::dds::DataWriter* /*writer*/,
            const ::fastdds::dds::PublicationMatchedStatus& info) override;

    /**
     * Class members.
     */
    Participant* participant_;
    ::fastdds::dds::Publisher* dds_publisher_;
    ::fastdds::dds::Topic* dds_topic_;
    ::fastdds::dds::DataWriter* dds_datawriter_;

    fastrtps::types::DynamicData* dynamic_data_;
    std::mutex data_mtx_;

    const std::string topic_name_;

    utils::Logger logger_;
};

} //  namespace fastdds
} //  namespace sh
} //  namespace is
} //  namespace eprosima

#endif //  _IS_SH_FASTDDS__INTERNAL__PARTICIPANT_HPP_
