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


TEST_CASE("Transmit to and receive from dds world", "[dds]")
{
    using namespace std::chrono_literals;

    SECTION("Publish and subscribe by udp")
    {
        std::cout << "[test]: configuring middleware..." << std::endl;
        const YAML::Node config_node = YAML::LoadFile(DDS__UDP_ROUNDTRIP__TEST_CONFIG);
        soss::InstanceHandle soss_handle = soss::run_instance(config_node);
        REQUIRE(soss_handle);

        std::this_thread::sleep_for(2s); // wait publisher and subscriber matching
        std::cout << "[test]: preparing communication..." << std::endl;

        // Generating message
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::stringstream ss;
        ss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
        std::string message_data = "mock test message at " + ss.str();
        std::cout << "[test]: message id: " << ss.str() << std::endl;

        // Message to dds
        soss::Message msg_to_dds;
        msg_to_dds.type = "dds_test_string";
        msg_to_dds.data["data"] = soss::Convert<std::string>::make_soss_field(message_data);
        soss::mock::publish_message("mock_to_dds_topic", msg_to_dds);

        // Message from dds
        std::promise<soss::Message> receive_msg_promise;
        REQUIRE(soss::mock::subscribe(
                "dds_to_mock_topic",
                [&](const soss::Message& msg_from_dds)
        {
            receive_msg_promise.set_value(msg_from_dds);
        }));

        auto receive_msg_future = receive_msg_promise.get_future();
        REQUIRE(std::future_status::ready == receive_msg_future.wait_for(5s));

        const soss::Message msg_from_dds = receive_msg_future.get();
        REQUIRE(message_data == *msg_from_dds.data.at("data").cast<std::string>());
        REQUIRE(msg_to_dds.type == msg_from_dds.type);

        std::cout << "[test]: Finishing commuinication..." << std::endl;
    }
}
