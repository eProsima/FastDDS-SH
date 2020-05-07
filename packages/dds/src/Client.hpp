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

#ifndef SOSS__DDS__INTERNAL__CLIENT_HPP
#define SOSS__DDS__INTERNAL__CLIENT_HPP

#include "DDSMiddlewareException.hpp"
#include "DynamicTypeAdapter.hpp"

#include <soss/Message.hpp>
#include <soss/SystemHandle.hpp>

#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>

#include <map>
#include <thread>
#include <condition_variable>

namespace soss {
namespace dds {

struct NavigationNode;
class Participant;

class Client
    : public virtual ServiceClient
    , private eprosima::fastrtps::PublisherListener
    , private eprosima::fastrtps::SubscriberListener
{
public:

    Client(
            Participant* participant,
            const std::string& service_name,
            const xtypes::DynamicType& request_type,
            const xtypes::DynamicType& reply_type,
            ServiceClientSystem::RequestCallback callback,
            const YAML::Node& config);

    virtual ~Client() override;

    Client(
            const Client& rhs) = delete;

    Client& operator = (
            const Client& rhs) = delete;

    Client(
            Client&& rhs) = delete;

    Client& operator = (
            Client&& rhs) = delete;

    void receive_response(
            std::shared_ptr<void> call_handle,
            const xtypes::DynamicData& response) override;

    bool add_config(
            const YAML::Node& configuration,
            ServiceClientSystem::RequestCallback callback);

private:

    void onPublicationMatched(
            eprosima::fastrtps::Publisher* pub,
            eprosima::fastrtps::rtps::MatchingInfo& info) override;

    void onSubscriptionMatched(
            eprosima::fastrtps::Subscriber* sub,
            eprosima::fastrtps::rtps::MatchingInfo& info) override;

    void onNewDataMessage(
            eprosima::fastrtps::Subscriber* sub) override;

    void receive(
            eprosima::fastrtps::rtps::SampleIdentity sample_id);

    void add_member(
            const xtypes::DynamicType& type,
            const std::string& path);

    Participant* participant_;
    eprosima::fastrtps::Publisher* dds_publisher_;
    eprosima::fastrtps::Subscriber* dds_subscriber_;
    DynamicData* request_dynamic_data_;
    DynamicData* reply_dynamic_data_;
    const xtypes::DynamicType& request_type_;
    const xtypes::DynamicType& reply_type_;
    std::map<std::string, ServiceClientSystem::RequestCallback> callbacks_;

    const std::string service_name_;
    std::map<std::string, std::shared_ptr<NavigationNode> > member_tree_;
    std::map<std::string, std::string> type_to_discriminator_;
    std::map<std::string, std::string> request_reply_;
    std::vector<std::string> member_types_;
    std::map<eprosima::fastrtps::rtps::SampleIdentity, std::string, SampleIdentityComparator> reply_id_type_;
    std::mutex mtx_;

    std::mutex request_data_mtx_;
    std::mutex reply_data_mtx_;

    std::map<std::thread::id, std::thread*> reception_threads_;
    bool stop_cleaner_;
    std::vector<std::thread::id> finished_threads_;
    std::mutex cleaner_mtx_;
    std::condition_variable cleaner_cv_;
    std::thread cleaner_thread_;

    void cleaner_function();
};


} //namespace dds
} //namespace soss

#endif // SOSS__DDS__INTERNAL__CLIENT_HPP
