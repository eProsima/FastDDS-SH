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

#include "SystemHandle.hpp"

#include "Participant.hpp"
#include "Publisher.hpp"
#include "Subscriber.hpp"
#include "Conversion.hpp"

#include "dtparser/YAMLParser.hpp"

#include <fastrtps/Domain.h>

#include <iostream>
#include <thread>

namespace soss {
namespace dds{

namespace {

// This function patches the problem of dynamic types, which do not admit '/' in their type name.
/*
std::string transform_type(const std::string& message_type)
{
    std::string type = message_type;

    for (size_t i = type.find('/'); i != std::string::npos; i = type.find('/', i))
    {
        type.replace(i, 1, "__");
    }

    return type;
}
*/

}

SystemHandle::~SystemHandle() = default;

bool SystemHandle::configure(
    const RequiredTypes& /* types */,
    const YAML::Node& configuration,
    TypeRegistry& type_registry)
{
    if (!configuration["dynamic types"])
    {
        std::cerr << "[soss-dds]: Configuration must have a 'dynamic types' field." << std::endl;
        return false;
    }


    try
    {
        if (configuration["participant"])
        {
            participant_ = std::make_unique<Participant>(configuration["participant"]);
        }
        else
        {
            std::cout << "[soss-dds]: participant not provided in configuration file. " <<
                "UDP default participant will be created." << std::endl;

            participant_ = std::make_unique<Participant>();
        }

        dtparser::RegisterCallback callback =
            [=, &type_registry](std::string name, p_dynamictypebuilder_t builder)
        {
            participant_->register_dynamic_type(name, builder);
            type_registry.emplace(name, Conversion::convert_type(participant_->get_dynamic_type(name)));
        };

        dtparser::YAMLParser::set_callback(callback);

        if (dtparser::YAMLP_ret::YAMLP_OK != dtparser::YAMLParser::parseYAMLNode(configuration))
        {
            std::cerr << "[soss-dds]: error parsing the dynamic types" << std::endl;
        }
    }
    catch(DDSMiddlewareException& e)
    {
        std::cerr << "[soss-dds]: " << e.what() << std::endl;
        return false;
    }

    dtparser::YAMLParser::DeleteInstance(); // TODO: an exception error will not delete the instances
                                            // (Use RAII in YamlParser could fix it cleanly)

    std::cout << "[soss-dds]: configured!" << std::endl;
    return true;
}

bool SystemHandle::okay() const
{
    return true;
}

bool SystemHandle::spin_once()
{
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    return okay();
}

bool SystemHandle::subscribe(
    const std::string& topic_name,
    const xtypes::DynamicType& message_type,
    SubscriptionCallback callback,
    const YAML::Node& /* configuration */)
{
    try
    {
        auto subscriber = std::make_shared<Subscriber>(
            participant_.get(), topic_name, message_type.name(), callback);

        subscribers_.emplace_back(std::move(subscriber));

        std::cout << "[soss-dds]: subscriber created. "
            "topic: " << topic_name << ", "
            "type: " << message_type.name() << std::endl;

        return true;
    }
    catch(DDSMiddlewareException& e)
    {
        std::cerr << "[soss-dds]: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<TopicPublisher> SystemHandle::advertise(
    const std::string& topic_name,
    const xtypes::DynamicType& message_type,
    const YAML::Node& /* configuration */)
{
    try
    {
        auto publisher = std::make_shared<Publisher>(participant_.get(), topic_name, message_type.name());
        publishers_.emplace_back(std::move(publisher));

        std::cout << "[soss-dds]: publisher created. "
            "topic: " << topic_name << ", "
            "type: " << message_type.name() << std::endl;

        return publishers_.back();
    }
    catch(DDSMiddlewareException& e)
    {
        std::cerr << "[soss-dds]: " << e.what() << std::endl;
        return std::shared_ptr<TopicPublisher>();
    }
}


} // namespace dds
} // namespace soss

SOSS_REGISTER_SYSTEM("dds", soss::dds::SystemHandle)
