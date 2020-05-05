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

#include <soss/SystemHandle.hpp>

#include "Participant.hpp"
#include "Publisher.hpp"
#include "Subscriber.hpp"
#include "Conversion.hpp"

#include <fastrtps/Domain.h>

#include <iostream>
#include <thread>

namespace soss {
namespace dds {

class SystemHandle : public virtual TopicSystem
{
public:

    bool configure(
            const RequiredTypes& /* types */,
            const YAML::Node& configuration,
            TypeRegistry& /*type_registry*/)
    {
        /*
         * SOSS-DDS doesn't define new types. Needed types will be defined in the 'types' section
         * of the YAML file, and hence, already registered in the 'TypeRegistry' by soss-core.
         */
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
        }
        catch (DDSMiddlewareException& e)
        {
            std::cerr << "[soss-dds]: " << e.what() << std::endl;
            return false;
        }

        std::cout << "[soss-dds]: configured!" << std::endl;
        return true;
    }

    bool okay() const
    {
        return true;
    }

    bool spin_once()
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
        return okay();
    }

    bool subscribe(
            const std::string& topic_name,
            const xtypes::DynamicType& message_type,
            SubscriptionCallback callback,
            const YAML::Node& /* configuration */)
    {
        try
        {
            auto subscriber = std::make_shared<Subscriber>(
                participant_.get(), topic_name, message_type, callback);

            subscribers_.emplace_back(std::move(subscriber));

            std::cout << "[soss-dds]: subscriber created. "
                "topic: " << topic_name << ", "
                "type: " << message_type.name() << std::endl;

            return true;
        }
        catch (DDSMiddlewareException& e)
        {
            std::cerr << "[soss-dds]: " << e.what() << std::endl;
            return false;
        }
    }

    std::shared_ptr<TopicPublisher> advertise(
            const std::string& topic_name,
            const xtypes::DynamicType& message_type,
            const YAML::Node& configuration)
    {
        try
        {
            auto publisher = std::make_shared<Publisher>(participant_.get(), topic_name, message_type, configuration);
            publishers_.emplace_back(std::move(publisher));

            std::cout << "[soss-dds]: publisher created. "
                "topic: " << topic_name << ", "
                "type: " << message_type.name() << std::endl;

            return publishers_.back();
        }
        catch (DDSMiddlewareException& e)
        {
            std::cerr << "[soss-dds]: " << e.what() << std::endl;
            return std::shared_ptr<TopicPublisher>();
        }
    }

private:

    std::unique_ptr<Participant> participant_;
    std::vector<std::shared_ptr<Publisher> > publishers_;
    std::vector<std::shared_ptr<Subscriber> > subscribers_;
};




} // namespace dds
} // namespace soss

SOSS_REGISTER_SYSTEM("dds", soss::dds::SystemHandle)
