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

#include <soss/mock/api.hpp>
#include <soss/Instance.hpp>

#include <fastrtps/fastrtps_all.h>
#include "../../resources/test_service/test_servicePubSubTypes.h"

#include <catch2/catch.hpp>

#include <iostream>
#include <iomanip>
#include <ctime>

using namespace std::chrono_literals;

class FastDDSTest : public eprosima::fastrtps::PublisherListener, public eprosima::fastrtps::ParticipantListener
{
public:
    FastDDSTest(
            bool server,
            std::mutex& mtx)
        : server_(server)
        , mutex_(mtx)
    {
        using namespace eprosima::fastrtps;
        ParticipantAttributes part_attr;
        if (server)
        {
            part_attr.rtps.setName("FastDDSTestServer");
        }
        else
        {
            part_attr.rtps.setName("FastDDSTestClient");
        }
        part_ = Domain::createParticipant(part_attr);
        Domain::registerType(part_, &psType_req_);
        Domain::registerType(part_, &psType_rep_);

        // Publisher of replies
        PublisherAttributes pub_attr;
        pub_attr.topic.topicKind = rtps::NO_KEY;
        if (server)
        {
            pub_attr.topic.topicDataType = psType_rep_.getName();
            pub_attr.topic.topicName = "TestService_Reply";
        }
        else
        {
            pub_attr.topic.topicDataType = psType_req_.getName();
            pub_attr.topic.topicName = "TestService_Request";
        }
        std::cout << "PUBLISHER: " << pub_attr.topic.topicDataType << " in topic " << pub_attr.topic.topicName << std::endl;
        pub_ = Domain::createPublisher(part_, pub_attr, this);

        request_listener.publisher(this, pub_);
        reply_listener.publisher(this, pub_);

        // Subscriber of requests
        SubscriberAttributes sub_attr;
        sub_attr.topic.topicKind = rtps::NO_KEY;
        if (server)
        {
            sub_attr.topic.topicDataType = psType_req_.getName();
            sub_attr.topic.topicName = "TestService_Request";
            sub_ = Domain::createSubscriber(part_, sub_attr, &request_listener);
        }
        else
        {
            sub_attr.topic.topicDataType = psType_rep_.getName();
            sub_attr.topic.topicName = "TestService_Reply";
            sub_ = Domain::createSubscriber(part_, sub_attr, &reply_listener);
        }
        std::cout << "SUBCRIBER: " << sub_attr.topic.topicDataType << " in topic " << sub_attr.topic.topicName << std::endl;
    }

    ~FastDDSTest()
    {
        using namespace eprosima::fastrtps;
        Domain::removePublisher(pub_);
        Domain::removeSubscriber(sub_);
    }

    std::future<xtypes::DynamicData> request(
            const xtypes::DynamicData& /*data*/)
    {
        TestService_Request request;
        pub_->write(&request);

        return promise_.get_future();
    }

    void onParticipantDiscovery(
            eprosima::fastrtps::Participant* /*participant*/,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& /*info*/) override
    {
        std::cout << "*********** MATCHED" << std::endl;
    }

    void onPublicationMatched(
            eprosima::fastrtps::Publisher*,
            eprosima::fastrtps::rtps::MatchingInfo&) override
    {
        std::unique_lock<std::mutex> lock(matched_mutex_);
        std::cout << "PUBLISHER MATCHED" << std::endl;
        pup_sub_matched_++;;
        if (pup_sub_matched_ == 2)
        {
            mutex_.unlock();
        }
    }

private:
    eprosima::fastrtps::Participant* part_;
    eprosima::fastrtps::Subscriber* sub_;
    eprosima::fastrtps::Publisher* pub_;
    TestService_RequestPubSubType psType_req_;
    TestService_ReplyPubSubType psType_rep_;
    std::promise<xtypes::DynamicData> promise_;
    bool server_;
    std::mutex& mutex_;
    std::mutex matched_mutex_;
    uint32_t pup_sub_matched_ = 0;

    class RequestListener : public eprosima::fastrtps::SubscriberListener
    {
    private:
        eprosima::fastrtps::Publisher* pub_;
        FastDDSTest* test_;

    public:
        void publisher(
                FastDDSTest* test,
                eprosima::fastrtps::Publisher* pub)
        {
            test_ = test;
            pub_ = pub;
        }

        void onSubscriptionMatched(
                eprosima::fastrtps::Subscriber*,
                eprosima::fastrtps::rtps::MatchingInfo&) override
        {
            std::unique_lock<std::mutex> lock(test_->matched_mutex_);
            std::cout << "SUBSCRIBER MATCHED" << std::endl;
            test_->pup_sub_matched_++;;
            if (test_->pup_sub_matched_ == 2)
            {
                test_->mutex_.unlock();
            }
        }

        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub) override
        {
            using namespace eprosima::fastrtps;
            SampleInfo_t info;
            TestService_Request req_msg;
            TestService_Reply rep_msg;
            std::cout << "--------------RECIBO COSAS-----------" << std::endl;

            if (sub->takeNextData(&req_msg, &info))
            {
                const TestService_Call& request = req_msg.data();
                TestService_Return& reply = rep_msg.reply();
                rtps::WriteParams params;
                params.related_sample_identity(info.sample_identity);

                switch(request._d())
                {
                    case 0: // If the data is "TEST", then success will be true.
                    {
                        const Method0_In& input = request.method0();
                        Method0_Result output;
                        output.success(false);
                        if (input.data() == "TEST")
                        {
                            output.success(true);
                        }
                        reply.method0(output);
                        pub_->write(&rep_msg, params);
                        break;
                    }
                    case 1: // a + b = result
                    {
                        const Method1_In& input = request.method1();
                        Method1_Result output;
                        output.result(input.a() + input.b());
                        reply.method1(output);
                        pub_->write(&rep_msg, params);
                        break;
                    }
                    case 2: // Echoes data
                    {
                        const Method2_In& input = request.method2();
                        Method2_Result output;
                        output.data(input.data());
                        reply.method2(output);
                        pub_->write(&rep_msg, params);
                        break;
                    }

                }
            }
        }
    } request_listener;

    class ReplyListener : public eprosima::fastrtps::SubscriberListener
    {
    private:
        eprosima::fastrtps::Publisher* pub_;
        FastDDSTest* test_;

    public:
        void publisher(
                FastDDSTest* test,
                eprosima::fastrtps::Publisher* pub)
        {
            test_ = test;
            pub_ = pub;
        }

        void onSubscriptionMatched(
                eprosima::fastrtps::Subscriber*,
                eprosima::fastrtps::rtps::MatchingInfo&) override
        {
            std::unique_lock<std::mutex> lock(test_->matched_mutex_);
            std::cout << "SUBSCRIBER MATCHED" << std::endl;
            test_->pup_sub_matched_++;;
            if (test_->pup_sub_matched_ == 2)
            {
                test_->mutex_.unlock();
            }
        }

        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub) override
        {
            using namespace eprosima::fastrtps;
            SampleInfo_t info;
            //TestService_Request req_msg;
            TestService_Reply rep_msg;

            if (sub->takeNextData(&rep_msg, &info))
            {
                std::cout << "--------------Holaaaaa-----------" << std::endl;
            }

            test_->promise_.set_value(::xtypes::DynamicData(::xtypes::primitive_type<bool>()));
        }
    } reply_listener;
};

std::string gen_config_yaml(
        const std::string& topic_type,
        const std::string& topic_sent,
        const std::string& topic_recv,
        const std::string& topic_mapped,
        const std::string& dds_config_file_path,
        const std::string& dds_profile_name)
{
    std::string remap = "";
    if (topic_mapped != "") {
        remap = ", remap: {dds: { topic: \"" + topic_mapped + "\" } }";
    }

    std::string s;
    s += "types:\n";
    s += "    idls:\n";
    s += "        - >\n";
    s += "            struct dds_test_string\n";
    s += "            {\n";
    s += "                string data;\n";
    s += "            };\n";

    s += "systems:\n";
    s += "    dds:\n";

    if (dds_config_file_path != "")
    {
        s += "        participant:\n";
        s += "            file_path: " + dds_config_file_path + "\n";
        s += "            profile_name: " + dds_profile_name + "\n";
    }

    s += "    mock: { type: mock, types-from: dds }\n";

    s += "routes:\n";
    s += "    mock_to_dds: { from: mock, to: dds }\n";
    s += "    dds_to_mock: { from: dds, to: mock }\n";

    s += "topics:\n";
    s += "    " + topic_sent + ": { type: \"" + topic_type + "\", route: mock_to_dds" + remap + "}\n";
    s += "    " + topic_recv + ": { type: \"" + topic_type + "\", route: dds_to_mock" + remap + "}\n";
    return s;
}

std::string gen_config_method_yaml(
        const std::string& dds_topic,
        const std::vector<std::string>& request_types,
        const std::vector<std::string>& reply_types,
        const std::vector<std::string>& request_members,
        const std::vector<std::string>& reply_members,
        bool client,
        const std::string& dds_config_file_path,
        const std::string& dds_profile_name)
{
    std::string s;
    s += "types:\n";
    s += "    idls:\n";
    s += "        - >\n";
    s += "            #include \"test_service.idl\"\n";

    s += "systems:\n";
    s += "    dds:\n";

    if (dds_config_file_path != "")
    {
        s += "        participant:\n";
        s += "            file_path: " + dds_config_file_path + "\n";
        s += "            profile_name: " + dds_profile_name + "\n";
    }

    s += "    mock: { type: mock, types-from: dds }\n";

    s += "routes:\n";
    if (client)
    {
        s += "    route: { server: mock, clients: dds }\n";
    }
    else
    {
        s += "    route: { server: dds, clients: mock }\n";
    }

    s += "services:\n";
    for (uint32_t i = 0 ; i < request_types.size(); ++i)
    {
        s += "    " + request_types[i] + "_topic";
        s += ": {";
        s += " request_type: \"" + request_types[i] + "\",";
        s += " reply_type: \"" + reply_types[i] + "\",";
        s += " route: route,";
        s += " remap: { " +
             std::string((client ? "mock" : "dds")) + ": {"
                + " request_type: \"" + request_members[i] + "\", "
                + " reply_type: \"" + reply_members[i] + "\", "
                + " topic: \"" + dds_topic + "\""
                + " }"
             + " }";
        s += " }\n";
    }
    return s;
}

soss::InstanceHandle create_instance(
        const std::string& topic_type,
        const std::string& topic_sent,
        const std::string& topic_recv,
        bool  remap,
        const std::string& config_file,
        const std::string& profile_name)
{
    const std::string topic_name_mapped = (remap) ? topic_sent + topic_recv : "";

    std::string config_yaml = gen_config_yaml(
            topic_type,
            topic_sent,
            topic_recv,
            topic_name_mapped,
            config_file,
            profile_name);

    std::cout << "====================================================================================" << std::endl;
    std::cout << "                                 Current YAML file" << std::endl;
    std::cout << "------------------------------------------------------------------------------------" << std::endl;
    std::cout << config_yaml << std::endl;
    std::cout << "====================================================================================" << std::endl;

    const YAML::Node config_node = YAML::Load(config_yaml);
    soss::InstanceHandle instance = soss::run_instance(config_node);
    REQUIRE(instance);

    std::this_thread::sleep_for(1s); // wait publisher and subscriber matching

    return instance;
}

soss::InstanceHandle create_method_instance(
        const std::string& dds_topic,
        const std::vector<std::string>& request_types,
        const std::vector<std::string>& reply_types,
        const std::vector<std::string>& request_members,
        const std::vector<std::string>& reply_members,
        bool client,
        const std::string& config_file = "",
        const std::string& profile_name = "")
{
    std::string config_yaml = gen_config_method_yaml(
            dds_topic,
            request_types,
            reply_types,
            request_members,
            reply_members,
            client,
            config_file,
            profile_name);

    std::cout << "====================================================================================" << std::endl;
    std::cout << "                                 Current YAML file" << std::endl;
    std::cout << "------------------------------------------------------------------------------------" << std::endl;
    std::cout << config_yaml << std::endl;
    std::cout << "====================================================================================" << std::endl;

    const YAML::Node config_node = YAML::Load(config_yaml);
    soss::InstanceHandle instance = soss::run_instance(config_node);
    REQUIRE(instance);

    std::this_thread::sleep_for(1s); // wait publisher and subscriber matching

    return instance;
}

xtypes::DynamicData roundtrip(
        const std::string& topic_sent,
        const std::string& topic_recv,
        const xtypes::DynamicData& message)
{
    soss::mock::publish_message(topic_sent, message);

    std::promise<xtypes::DynamicData> receive_msg_promise;
    REQUIRE(soss::mock::subscribe(
            topic_recv,
            [&](const xtypes::DynamicData& msg_to_recv)
    {
        receive_msg_promise.set_value(msg_to_recv);
    }));

    auto receive_msg_future = receive_msg_promise.get_future();
    REQUIRE(std::future_status::ready == receive_msg_future.wait_for(5s));

    return receive_msg_future.get();
}

xtypes::DynamicData roundtrip_server(
    const std::string& topic,
    const xtypes::DynamicData& request)
{
    // MOCK->DDS->MOCK
    std::shared_future<xtypes::DynamicData> response_future = soss::mock::request(topic, request);
    REQUIRE(std::future_status::ready == response_future.wait_for(60s));
    xtypes::DynamicData result = std::move(response_future.get());
    {
        using namespace std;
        (&response_future)->~shared_future<xtypes::DynamicData>();
    }
    return result;
}

xtypes::DynamicData roundtrip_client(
        const std::string& topic,
        const xtypes::DynamicData& message,
        FastDDSTest& fastdds)
{
    // DDS->MOCK->DDS
    soss::mock::serve(
            topic,
            [](const xtypes::DynamicData& request)
    {
        return request;
    });

    std::future<xtypes::DynamicData> response_future = fastdds.request(message);
    REQUIRE(std::future_status::ready == response_future.wait_for(5s));
    return response_future.get();
}

/*
TEST_CASE("Transmit to and receive from dds", "[dds]")
{
    SECTION("basic-type")
    {
        const std::string topic_type = "dds_test_string";

        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::stringstream ss;
        ss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
        std::string message_data = "mock test message at " + ss.str();
        std::cout << "[test]: message id: " << ss.str() << std::endl;

        SECTION("udp")
        {
            const std::string topic_sent = "mock_to_dds_topic";
            const std::string topic_recv = "dds_to_mock_topic";
            soss::InstanceHandle instance = create_instance(
                    topic_type,
                    topic_sent,
                    topic_recv,
                    true,
                    "",
                    "");

            const soss::TypeRegistry& mock_types = *instance.type_registry("mock");
            xtypes::DynamicData msg_to_sent(*mock_types.at(topic_type));
            msg_to_sent["data"].value(message_data);

            // Road: [mock -> dds -> dds -> mock]
            xtypes::DynamicData msg_to_recv = roundtrip(topic_sent, topic_recv, msg_to_sent);

            REQUIRE(msg_to_sent == msg_to_recv);
            REQUIRE(0 == instance.quit().wait_for(1s));
        }

        SECTION("tcp tunnel")
        {
            const std::string config_file = "resources/tcp_config.xml";
            const std::string client_to_server_topic = "client_to_server_topic";
            const std::string server_to_client_topic = "server_to_client_topic";

            const std::string profile_name_server = "soss_profile_server";
            soss::InstanceHandle server_instance = create_instance(
                    topic_type,
                    server_to_client_topic,
                    client_to_server_topic,
                    false,
                    config_file,
                    profile_name_server);

            const std::string profile_name_client = "soss_profile_client";
            soss::InstanceHandle client_instance = create_instance(
                    topic_type,
                    client_to_server_topic,
                    server_to_client_topic,
                    false,
                    config_file,
                    profile_name_client);


            const soss::TypeRegistry& mock_types = *server_instance.type_registry("mock");
            client_instance.type_registry("mock");
            xtypes::DynamicData msg_to_sent(*mock_types.at(topic_type));
            msg_to_sent["data"].value(message_data);
            std::this_thread::sleep_for(2s); // wait publisher and subscriber matching

            // Road: [mock -> dds-client] -> [dds-server -> mock]
            xtypes::DynamicData msg_to_recv = roundtrip(client_to_server_topic, client_to_server_topic, msg_to_sent);
            REQUIRE(msg_to_sent == msg_to_recv);

            // Road: [mock <- dds-client] <- [dds-server <- mock]
            msg_to_recv = roundtrip(server_to_client_topic, server_to_client_topic, msg_to_sent);
            REQUIRE(msg_to_sent == msg_to_recv);

            REQUIRE(0 == client_instance.quit().wait_for(1s));
            REQUIRE(0 == server_instance.quit().wait_for(1s));
        }
    }
}
*/

TEST_CASE("Request to and reply from dds", "[dds-server]")
{
    SECTION("config-server")
    {
        soss::InstanceHandle instance = create_method_instance(
                "TestService",
                {"Method0_In","Method1_In","Method2_In"},
                {"Method0_Result","Method1_Result","Method2_Result"},
                {"TestService_Request.data.method0","TestService_Request.data.method1","TestService_Request.data.method2"},
                {"TestService_Reply.reply.method0","TestService_Reply.reply.method1","TestService_Reply.reply.method2"},
                false,
                "",
                "");

        const soss::TypeRegistry& mock_types = *instance.type_registry("mock");

        std::mutex disc_mutex;
        disc_mutex.lock();
        FastDDSTest server(true, disc_mutex);
        SECTION("method0")
        {
            // Road: [mock -> dds -> dds -> mock]
            std::string message_data_fail = "TESTING";
            xtypes::DynamicData msg_to_sent(*mock_types.at("Method0_In"));
            msg_to_sent["data"].value(message_data_fail);
            //disc_mutex.lock();
            std::cout << "[test]: sent message: " << message_data_fail << std::endl;
            xtypes::DynamicData msg_to_recv = roundtrip_server("Method0_In_topic", msg_to_sent);
            //xtypes::DynamicData msg_to_recv = roundtrip_client("Method0_In_topic", msg_to_sent, client);
            REQUIRE(msg_to_recv["success"].value<bool>() == false);

            std::string message_data_ok = "TEST";
            msg_to_sent["data"].value(message_data_ok);
            std::cout << "[test]: sent message: " << message_data_fail << std::endl;
            msg_to_recv = roundtrip_server("Method0_In_topic", msg_to_sent);
            //msg_to_recv = roundtrip_client("Method0_In_topic", msg_to_sent, client);
            REQUIRE(msg_to_recv["success"].value<bool>() == true);


            REQUIRE(0 == instance.quit().wait_for(1s));
        }
    }
}

