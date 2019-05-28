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

std::string gen_config_yaml(
        const std::string& topic_type,
        const std::string& topic_name,
        const std::string& topic_sent,
        const std::string& topic_recv,
        const std::string& dds_config_file_path = "",
        const std::string& dds_profile_name = "")
{
    std::string s;
    s += "systems:\n";
    s += "    dds:\n";
    s += "        dynamic types:\n";
    s += "            struct dds_test_string:\n";
    s += "                string: \"data\"\n";

    if (dds_config_file_path != "")
    {
        s += "        participant:\n";
        s += "            filep_path: " + dds_config_file_path + "\n";
        s += "            profile_name: " + dds_profile_name + "\n";
    }

    s += "    mock: { type: mock }\n";

    s += "routes:\n";
    s += "    mock_to_dds: { from: mock, to: dds }\n";
    s += "    dds_to_mock: { from: dds, to: mock }\n";

    s += "topics:\n";
    s += "    " + topic_sent + ": { type: \"" + topic_type + "\", route: mock_to_dds, \
                    remap: {dds: \"" + topic_name + "\" } }\n";
    s += "    " + topic_recv + ": { type: \"" + topic_type + "\", route: dds_to_mock, \
                    remap: {dds: \"" + topic_name + "\" } }\n";
    return s;
}

soss::Message roundtrip(
        const std::string& topic_name,
        const std::string& topic_type,
        const soss::Message& message)
{
    using namespace std::chrono_literals;

    const std::string topic_sent = "mock_to_dds_topic";
    const std::string topic_recv = "dds_to_mock_topic";

    std::string config_yaml = gen_config_yaml(topic_type, topic_name, topic_sent, topic_recv);
    const YAML::Node config_node = YAML::Load(config_yaml);
    soss::InstanceHandle soss_handle = soss::run_instance(config_node);
    REQUIRE(soss_handle);

    std::this_thread::sleep_for(1s); // wait publisher and subscriber matching

    soss::mock::publish_message(topic_sent, message);

    std::promise<soss::Message> receive_msg_promise;
    REQUIRE(soss::mock::subscribe(
            topic_recv,
            [&](const soss::Message& msg_from_dds)
    {
        receive_msg_promise.set_value(msg_from_dds);
    }));

    auto receive_msg_future = receive_msg_promise.get_future();
    REQUIRE(std::future_status::ready == receive_msg_future.wait_for(5s));

    return receive_msg_future.get();
}

TEST_CASE("Transmit to and receive from dds", "[dds]")
{
    SECTION("basic-type")
    {
        const std::string topic_name = "dds_mock_test_basic";
        const std::string topic_type = "dds_test_string";

        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::stringstream ss;
        ss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
        std::string message_data = "mock test message at " + ss.str();
        std::cout << "[test]: message id: " << ss.str() << std::endl;

        soss::Message msg_to_dds;
        msg_to_dds.type = topic_type;
        msg_to_dds.data["data"] = soss::Convert<std::string>::make_soss_field(message_data);

        SECTION("udp")
        {
            soss::Message msg_from_dds = roundtrip(
                    topic_name,
                    topic_type,
                    msg_to_dds);

            REQUIRE(message_data == *msg_from_dds.data.at("data").cast<std::string>());
            REQUIRE(msg_to_dds.type == msg_from_dds.type);
        }

        /*SECTION("tcp")//Not implemented yet
        {
            soss::Message msg_from_dds = roundtrip(
                    topic_name,
                    topic_type,
                    msg_to_dds);

            REQUIRE(message_data == *msg_from_dds.data.at("data").cast<std::string>());
            REQUIRE(msg_to_dds.type == msg_from_dds.type);
        }*/
    }
}
