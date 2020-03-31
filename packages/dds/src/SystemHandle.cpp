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
#include "Server.hpp"
#include "Client.hpp"
#include "Conversion.hpp"

#include <fastrtps/Domain.h>

#include <iostream>
#include <thread>

namespace soss {
namespace dds {

class SystemHandle : public virtual FullSystem
{
public:

    bool configure(
            const RequiredTypes& types,
            const YAML::Node& configuration,
            TypeRegistry& /*type_registry*/) override
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

        for (const std::string& type : types.services)
        {
            std::cout << "******** " << type << " *********" << std::endl;
        }

        return true;
    }

    bool okay() const override
    {
        return true;
    }

    bool spin_once() override
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
        return okay();
    }

    bool subscribe(
            const std::string& topic_name,
            const xtypes::DynamicType& message_type,
            SubscriptionCallback callback,
            const YAML::Node& /* configuration */) override
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
            const YAML::Node& configuration) override
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

    bool create_client_proxy(
            const std::string& service_name,
            const xtypes::DynamicType& request_type,
            const xtypes::DynamicType& reply_type,
            RequestCallback callback,
            const YAML::Node& configuration) override
    {
        if (clients_.count(service_name) == 0)
        {
            try
            {
                auto client = std::make_shared<Client>(
                    participant_.get(),
                    service_name,
                    request_type,
                    reply_type,
                    callback,
                    configuration);

                clients_[service_name] = std::move(client);

                std::cout << "[soss-dds]: client created. "
                    << "service: " << service_name
                    << ", request_type: " << request_type.name()
                    << ", reply_type: " << reply_type.name() << std::endl;

                return true;

            }
            catch (DDSMiddlewareException& e)
            {
                std::cerr << "[soss-dds]: " << e.what() << std::endl;
                return false;
            }
        }

        return clients_[service_name]->add_config(configuration);
    }

    std::shared_ptr<ServiceProvider> create_service_proxy(
            const std::string& service_name,
            const xtypes::DynamicType& request_type,
            const xtypes::DynamicType& reply_type,
            const YAML::Node& configuration) override
    {
        if (servers_.count(service_name) == 0)
        {
            try
            {
                auto server = std::make_shared<Server>(
                    participant_.get(),
                    service_name,
                    request_type,
                    reply_type,
                    configuration);

                servers_[service_name] = std::move(server);

                std::cout << "[soss-dds]: server created. "
                    << "service: " << service_name
                    << ", request_type: " << request_type.name()
                    << ", reply_type: " << reply_type.name() << std::endl;

                return servers_[service_name];
            }
            catch (DDSMiddlewareException& e)
            {
                std::cerr << "[soss-dds]: " << e.what() << std::endl;
                return nullptr;
            }
        }

        if (servers_[service_name]->add_config(configuration))
        {
            return servers_[service_name];
        }
        else
        {
            return nullptr;
        }
    }

private:

    std::unique_ptr<Participant> participant_;
    std::vector<std::shared_ptr<Publisher> > publishers_;
    std::vector<std::shared_ptr<Subscriber> > subscribers_;
    std::map<std::string, std::shared_ptr<Client> > clients_;
    std::map<std::string, std::shared_ptr<Server> > servers_;
};




} // namespace dds
} // namespace soss

SOSS_REGISTER_SYSTEM("dds", soss::dds::SystemHandle)
