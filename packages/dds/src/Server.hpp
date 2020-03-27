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

#ifndef SOSS__DDS__INTERNAL__SERVER_HPP
#define SOSS__DDS__INTERNAL__SERVER_HPP

#include "DDSMiddlewareException.hpp"
#include "Participant.hpp"
#include "DynamicTypeAdapter.hpp"

#include <soss/SystemHandle.hpp>

#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>

#include <thread>

namespace soss {
namespace dds {

class Participant;

class Server
    : public virtual ServiceProvider
    , private eprosima::fastrtps::PublisherListener
    , private eprosima::fastrtps::SubscriberListener
{
public:

    Server(
            Participant* participant,
            const std::string& topic_name,
            const xtypes::DynamicType& message_type,
            const YAML::Node& config);

    virtual ~Server();

    Server(
            const Server& rhs) = delete;

    Server& operator = (
            const Server& rhs) = delete;


    Server(
            Server&& rhs) = delete;


    Server& operator = (
            Server&& rhs) = delete;

    void call_service(
            const xtypes::DynamicData& request,
            ServiceClient& client,
            std::shared_ptr<void> call_handle) override;

private:

    void onPublicationMatched(
            eprosima::fastrtps::Publisher* pub,
            eprosima::fastrtps::rtps::MatchingInfo& info) override;

    void onSubscriptionMatched(
            eprosima::fastrtps::Subscriber* sub,
            eprosima::fastrtps::rtps::MatchingInfo& info) override;

    void onNewDataMessage(
            eprosima::fastrtps::Subscriber* sub) override;

    eprosima::fastrtps::Publisher* dds_publisher_;
    eprosima::fastrtps::Subscriber* dds_subscriber_;
    DynamicData_ptr dynamic_data_;

    const std::string topic_name_;
    const xtypes::DynamicType& message_type_;

    std::vector<std::thread> reception_threads_;
    std::map<std::shared_ptr<void>, ServiceClient*> callhandle_client_;
    std::map<eprosima::fastrtps::rtps::SampleIdentity, std::shared_ptr<void>, SampleIdentityComparator>
        sample_callhandle_;
    std::map<std::string, std::string> discriminator_to_type_;
    std::map<std::string, std::string> type_to_discriminator_;
};


} // namespace dds
} // namespace soss

#endif // SOSS__DDS__INTERNAL__SERVER_HPP
