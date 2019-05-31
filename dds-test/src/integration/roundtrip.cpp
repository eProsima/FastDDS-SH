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
#include <soss/utilities.hpp>

#include <catch2/catch.hpp>

#include <iostream>
#include <iomanip>
#include <ctime>

using namespace std::chrono_literals;

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
        remap = ", remap: {dds: \"" + topic_mapped + "\" }";
    }

    std::string s;
    s += "systems:\n";
    s += "    dds:\n";
    s += "        dynamic types:\n";
    s += "            struct dds_test_string:\n";
    s += "                string: \"data\"\n";

    if (dds_config_file_path != "")
    {
        s += "        participant:\n";
        s += "            file_path: " + dds_config_file_path + "\n";
        s += "            profile_name: " + dds_profile_name + "\n";
    }

    s += "    mock: { type: mock }\n";

    s += "routes:\n";
    s += "    mock_to_dds: { from: mock, to: dds }\n";
    s += "    dds_to_mock: { from: dds, to: mock }\n";

    s += "topics:\n";
    s += "    " + topic_sent + ": { type: \"" + topic_type + "\", route: mock_to_dds" + remap + "}\n";
    s += "    " + topic_recv + ": { type: \"" + topic_type + "\", route: dds_to_mock" + remap + "}\n";
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

soss::Message roundtrip(
        const std::string& topic_sent,
        const std::string& topic_recv,
        const soss::Message& message)
{
    soss::mock::publish_message(topic_sent, message);

    std::promise<soss::Message> receive_msg_promise;
    REQUIRE(soss::mock::subscribe(
            topic_recv,
            [&](const soss::Message& msg_to_recv)
    {
        receive_msg_promise.set_value(msg_to_recv);
    }));

    auto receive_msg_future = receive_msg_promise.get_future();
    REQUIRE(std::future_status::ready == receive_msg_future.wait_for(5s));

    return receive_msg_future.get();
}

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

        soss::Message msg_to_sent;
        msg_to_sent.type = topic_type;
        msg_to_sent.data["data"] = soss::Convert<std::string>::make_soss_field(message_data);

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

            // Road: [mock -> dds -> dds -> mock]
            soss::Message msg_to_recv = roundtrip(topic_sent, topic_recv, msg_to_sent);

            REQUIRE(*msg_to_sent.data.at("data").cast<std::string>() == *msg_to_recv.data.at("data").cast<std::string>());
            REQUIRE(msg_to_sent.type == msg_to_recv.type);
            REQUIRE(0 == instance.quit().wait_for(1s));
        }

        SECTION("tcp tunnel")
        {
            const std::string config_file = "../../src/eprosima/soss/dds/dds-test/resources/tcp_config.xml";
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

            std::this_thread::sleep_for(2s); // wait publisher and subscriber matching
            soss::Message msg_to_recv;

            // Road: [mock -> dds-client] -> [dds-server -> mock]
            msg_to_recv = roundtrip(client_to_server_topic, client_to_server_topic, msg_to_sent);
            REQUIRE(*msg_to_sent.data.at("data").cast<std::string>() == *msg_to_recv.data.at("data").cast<std::string>());
            REQUIRE(msg_to_sent.type == msg_to_recv.type);

            // Road: [mock <- dds-client] <- [dds-server <- mock]
            msg_to_recv = roundtrip(server_to_client_topic, server_to_client_topic, msg_to_sent);
            REQUIRE(*msg_to_sent.data.at("data").cast<std::string>() == *msg_to_recv.data.at("data").cast<std::string>());
            REQUIRE(msg_to_sent.type == msg_to_recv.type);

            REQUIRE(0 == client_instance.quit().wait_for(1s));
            REQUIRE(0 == server_instance.quit().wait_for(1s));
        }
    }
}

