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

#ifndef _IS_SH_FASTDDS__INTERNAL__SERVER_HPP_
#define _IS_SH_FASTDDS__INTERNAL__SERVER_HPP_

#include "DDSMiddlewareException.hpp"
#include "Participant.hpp"

#include <is/systemhandle/SystemHandle.hpp>
#include <is/utils/Log.hpp>

#include <fastdds/dds/publisher/Publisher.hpp>
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
 * @class Server
 *        This class represents a DDS Server, built over the publisher/subscriber layer of *Fast DDS*
 *        using the <a href="https://www.omg.org/spec/DDS-RPC/About-DDS-RPC/"> DDS-RPC</a>
 *        paradigm, within the *Integration Service* framework.
 *
 *        It is composed of a *Fast DDS* Publisher, to send the request to the DDS dataspace; plus a
 *        *Fast DDS* Subscriber, to receive replies from the DDS application server and send them
 *        back to the *Integration Service*.
 *
 *        Its topic type definition and data instances for request and reply types are represented by means
 *        of the <a href="https://fast-dds.docs.eprosima.com/en/latest/fastdds/dynamic_types/dynamic_types.html">
 *        Fast DDS Dynamic Types</a> API, which allows to get rid of
 *        TypeSupport for each used type and eases users the task of defining and using their own custom
 *        types on the go, by means of a valid *IDL* definition.
 *
 *        This class inherits from <a href="https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/subscriber/dataReaderListener/dataReaderListener.html">
 *        Fast DDS Data Reader Listener</a> and from <a href="https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/publisher/dataWriterListener/dataWriterListener.html">
 *        Fast DDS Data Writer Listener</a> for reacting to datawriter and datareader events,
 *        such as matching with subscribers and publishers or receiving new data from them.
 *
 *        The request petitions are associated with each received reply by means of the
 *        <a href="https://fast-dds.docs.eprosima.com/en/latest/fastdds/api_reference/dds_pim/subscriber/sampleinfo.html?highlight=rpc#_CPPv4N8eprosima7fastdds3dds10SampleInfo15sample_identityE">
 *        sample identity</a> and the <a href="https://fast-dds.docs.eprosima.com/en/latest/fastdds/api_reference/dds_pim/subscriber/sampleinfo.html?highlight=rpc#_CPPv4N8eprosima7fastdds3dds10SampleInfo23related_sample_identityE">
 *        related sample identity</a> attributes.
 */
class Server
    : public virtual ServiceProvider
    , private ::fastdds::dds::DataWriterListener
    , private ::fastdds::dds::DataReaderListener
{
public:

    /**
     * @brief Construct a new Server object.
     *
     * @param[in] participant The associated *Integration Service* Participant, which holds the
     *            DDS entities that compose this Server.
     *
     * @param[in] service_name The service name. It will produce two topics:
     *            `<service_name>_Request` and `<service_name>_Reply`.
     *
     * @param[in] request_type A dynamic type definition of the request topic's type.
     *
     * @param[in] reply_type A dynamic type definition of the reply topic's type.
     *
     * @param[in] config Additional configuration that might be required to configure this Server.
     *
     * @throws DDSMiddlewareExeption if some error occurs while creating the *Fast DDS* entities.
     */
    Server(
            eprosima::is::sh::fastdds::Participant* participant,
            const std::string& service_name,
            const xtypes::DynamicType& request_type,
            const xtypes::DynamicType& reply_type,
            const YAML::Node& config);

    // TODO(@jamoralp): use XML profile to create request publisher / reply subscriber?

    /**
     * @brief Destroy the Server object.
     */
    virtual ~Server() override;

    /**
     * @brief Server shall not be copy constructible.
     */
    Server(
            const Server& rhs) = delete;

    /**
     * @brief Server shall not be copy assignable.
     */
    Server& operator = (
            const Server& rhs) = delete;

    /**
     * @brief Server shall not be move constructible.
     */
    Server(
            Server&& rhs) = delete;

    /**
     * @brief Server shall not be move assignable.
     */
    Server& operator = (
            Server&& rhs) = delete;

    /**
     * @brief Inherited from ServiceProvider.
     */
    void call_service(
            const xtypes::DynamicData& is_request,
            ServiceClient& client,
            std::shared_ptr<void> call_handle) override;

    /**
     * @brief Handle type remappings for DDS request and reply types.
     *        It allows to resolve complex type remappings, which remap to a specific type member,
     *        for example, an UnionType member, by means of the dot `.` operator.
     *
     * @param[in] configuration The *YAML* configuration containing the remapping to be applied.
     *
     * @returns `true` if success.
     */
    bool add_config(
            const YAML::Node& configuration);

private:

    /**
     * @brief Inherited from *DataWriterListener*.
     */
    void on_publication_matched(
            ::fastdds::dds::DataWriter* /*writer*/,
            const ::fastdds::dds::PublicationMatchedStatus& info) override;

    /**
     * @brief Inherited from *DataReaderListener*.
     */
    void on_data_available(
            ::fastdds::dds::DataReader* /*reader*/) override;

    /**
     * @brief Inherited from *DataReaderListener*.
     */
    void on_subscription_matched(
            ::fastdds::dds::DataReader* /*reader*/,
            const ::fastdds::dds::SubscriptionMatchedStatus& info) override;

    /**
     * @brief Receive a DDS service response, for the provided SampleIdentity.
     *        If the sample_id does not correspond to a previously made petition, it is ignored.
     *
     * @param[in] sample_id The sample identity that identifies the answered request.
     */
    void receive(
            fastrtps::rtps::SampleIdentity sample_id);

    /**
     * @brief Function to look for threads that have already finished and delete them from the
     *        reception threads database.
     */
    void cleaner_function();

    /**
     * Class members.
     */
    Participant* participant_;
    const std::string service_name_;

    struct RequestEntities
    {
        RequestEntities(
                const xtypes::DynamicType& dynamic_type)
            : dds_publisher(nullptr)
            , dds_topic(nullptr)
            , dds_datawriter(nullptr)
            , dynamic_data(nullptr)
            , type(dynamic_type)
            , data_mtx()
        {
        }

        ::fastdds::dds::Publisher* dds_publisher;
        ::fastdds::dds::Topic* dds_topic;
        ::fastdds::dds::DataWriter* dds_datawriter;
        fastrtps::types::DynamicData* dynamic_data;
        const xtypes::DynamicType& type;
        std::mutex data_mtx;
    };
    RequestEntities request_entities_;
    struct ReplyEntities
    {
        ReplyEntities(
                const xtypes::DynamicType& dynamic_type)
            : dds_subscriber(nullptr)
            , dds_topic(nullptr)
            , dds_datareader(nullptr)
            , dynamic_data(nullptr)
            , type(dynamic_type)
            , data_mtx()
        {
        }

        ::fastdds::dds::Subscriber* dds_subscriber;
        ::fastdds::dds::Topic* dds_topic;
        ::fastdds::dds::DataReader* dds_datareader;
        fastrtps::types::DynamicData* dynamic_data;
        const xtypes::DynamicType& type;
        std::mutex data_mtx;
    };
    ReplyEntities reply_entities_;

    struct SampleIdentityComparator
    {
        bool operator () (
                const fastrtps::rtps::SampleIdentity lha,
                const fastrtps::rtps::SampleIdentity rha) const
        {
            if (lha.writer_guid() < rha.writer_guid())
            {
                return true;
            }
            if (rha.writer_guid() < lha.writer_guid()) // operator > doesn't exists for GUID_t
            {
                return false;
            }
            if (lha.sequence_number() < rha.sequence_number())
            {
                return true;
            }
            return false;
        }

    };

    std::map<std::shared_ptr<void>, ServiceClient*> callhandle_client_;
    std::map<fastrtps::rtps::SampleIdentity,
            std::shared_ptr<void>, SampleIdentityComparator> sample_callhandle_;
    std::map<std::string, std::string> type_to_discriminator_;
    std::map<std::string, std::string> request_reply_;
    std::map<fastrtps::rtps::SampleIdentity,
            std::string, SampleIdentityComparator> reply_id_type_;

    std::mutex mtx_;

    std::mutex matched_mtx_;
    uint8_t pub_sub_matched_;

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

#endif //  _IS_SH_FASTDDS__INTERNAL__SERVER_HPP_
