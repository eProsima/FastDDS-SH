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

#include <is/sh/mock/api.hpp>
#include <is/core/Instance.hpp>

#include "../../src/DDSMiddlewareException.hpp"

#include <fastrtps/fastrtps_all.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include "resources/test_service/test_servicePubSubTypes.h"

#include <gtest/gtest.h>

#include <is/utils/Log.hpp>

#include <iostream>
#include <iomanip>
#include <ctime>

using namespace std::chrono_literals;
static eprosima::is::utils::Logger logger("is::sh::FastDDS::test");

namespace fastdds = eprosima::fastdds;

namespace eprosima {
namespace is {
namespace sh {
namespace fastdds {
namespace test {

class FastDDSTest
    : public ::fastdds::dds::DataWriterListener
{
public:

    FastDDSTest(
            bool server,
            std::mutex& mtx)
        : ::fastdds::dds::DataWriterListener()
        , participant_factory_(::fastdds::dds::DomainParticipantFactory::get_instance())
        , tsType_req_(new TestService_RequestPubSubType())
        , tsType_rep_(new TestService_ReplyPubSubType())
        , server_(server)
        , mutex_(mtx)
    {
        // Create participant
        ::fastdds::dds::DomainId_t default_domain_id(0);
        ::fastdds::dds::DomainParticipantQos participant_qos = ::fastdds::dds::PARTICIPANT_QOS_DEFAULT;

        participant_qos.name(server ? "FastDDSTestServer" : "FastDDSTestClient");

        participant_ = participant_factory_->create_participant(
            default_domain_id, participant_qos);

        if (!participant_)
        {
            std::ostringstream err;
            err << "Error while creating participant '" << participant_qos.name()
                << "' with default QoS attributes and Domain ID: " << default_domain_id;

            throw DDSMiddlewareException(logger, err.str());
        }

        // Register types
        participant_->register_type(tsType_req_);
        participant_->register_type(tsType_rep_);

        // Create subscriber entities
        subscriber_ = participant_->create_subscriber(::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);

        if (!subscriber_)
        {
            throw DDSMiddlewareException(logger, "Subscriber was not created");
        }

        sub_topic_ = participant_->create_topic(
            server ? "TestService_Request" : "TestService_Reply",
            server ? tsType_req_.get_type_name() : tsType_rep_.get_type_name(),
            ::fastdds::dds::TOPIC_QOS_DEFAULT);

        if (!sub_topic_)
        {
            throw DDSMiddlewareException(logger, "Creating subscriber topic failed");
        }

        ::fastdds::dds::DataReaderQos datareader_qos;
        datareader_ = subscriber_->create_datareader(
            sub_topic_, ::fastdds::dds::DATAREADER_QOS_DEFAULT,
            server ? static_cast<::fastdds::dds::DataReaderListener*>(&request_listener)
            : static_cast<::fastdds::dds::DataReaderListener*>(&reply_listener));

        if (!datareader_)
        {
            std::ostringstream err;
            err << "Creating datareader for topic '" << sub_topic_->get_name() << "' failed";

            throw DDSMiddlewareException(logger, err.str());
        }

        // Create publisher entities
        publisher_ = participant_->create_publisher(::fastdds::dds::PUBLISHER_QOS_DEFAULT);

        if (!publisher_)
        {
            throw DDSMiddlewareException(logger, "Publisher was not created");
        }

        pub_topic_ = participant_->create_topic(
            server ? "TestServiceReply" : "TestServiceRequest",
            server ? tsType_rep_.get_type_name() : tsType_req_.get_type_name(),
            ::fastdds::dds::TOPIC_QOS_DEFAULT);

        if (!pub_topic_)
        {
            throw DDSMiddlewareException(logger, "Error creating publisher topic");
        }

        datawriter_ = publisher_->create_datawriter(
            pub_topic_, ::fastdds::dds::DATAWRITER_QOS_DEFAULT, this);

        if (!datawriter_)
        {
            std::ostringstream err;
            err << "Creating datawriter for topic '" << pub_topic_->get_name() << "' failed";

            throw DDSMiddlewareException(logger, err.str());
        }

        request_listener.publisher(this, datawriter_);
        reply_listener.publisher(this);
    }

    ~FastDDSTest()
    {
        std::cout << "FastDDSTest::~FastDDSTest: start" << std::endl;
        // Unregister types
        participant_->unregister_type(tsType_req_.get_type_name());
        participant_->unregister_type(tsType_rep_.get_type_name());
        std::cout << "FastDDSTest::~FastDDSTest: unregistered all types" << std::endl;

        // Delete subscriber entities
        subscriber_->delete_datareader(datareader_);
        participant_->delete_subscriber(subscriber_);
        participant_->delete_topic(sub_topic_);

        // Delete publisher entities
        publisher_->delete_datawriter(datawriter_);
        participant_->delete_publisher(publisher_);
        participant_->delete_topic(pub_topic_);

        // Delete participant
        participant_factory_->delete_participant(participant_);
    }

    std::future<eprosima::xtypes::DynamicData> request(
            const eprosima::xtypes::DynamicData& data)
    {
        TestService_Request request;

        if (data.type().name() == "Method0_In")
        {
            Method0_In in;
            in.data(data["data"]);
            request.data().method0(in);
        }
        else if (data.type().name() == "Method1_In")
        {
            Method1_In in;
            in.a(data["a"]);
            in.b(data["b"]);
            request.data().method1(in);
        }
        else if (data.type().name() == "Method2_In")
        {
            Method2_In in;
            in.data(data["data"]);
            request.data().method2(in);
        }

        datawriter_->write(static_cast<void*>(&request));
        delete promise_;
        promise_ = new std::promise<eprosima::xtypes::DynamicData>();
        return promise_->get_future();
    }

    void on_publication_matched(
            ::fastdds::dds::DataWriter* /*writer*/,
            const ::fastdds::dds::PublicationMatchedStatus& /*info*/) override
    {
        std::unique_lock<std::mutex> lock(matched_mutex_);
        pub_sub_matched_++;
        if (2 == pub_sub_matched_)
        {
            mutex_.unlock();
        }
    }

private:

    ::fastdds::dds::DomainParticipantFactory* participant_factory_;
    ::fastdds::dds::DomainParticipant* participant_;

    ::fastdds::dds::Subscriber* subscriber_;
    ::fastdds::dds::Topic* sub_topic_;
    ::fastdds::dds::DataReader* datareader_;

    ::fastdds::dds::Publisher* publisher_;
    ::fastdds::dds::Topic* pub_topic_;
    ::fastdds::dds::DataWriter* datawriter_;

    ::fastdds::dds::TypeSupport tsType_req_;
    ::fastdds::dds::TypeSupport tsType_rep_;

    std::promise<eprosima::xtypes::DynamicData>* promise_ = nullptr;
    bool server_;
    std::mutex& mutex_;
    std::mutex matched_mutex_;
    uint32_t pub_sub_matched_ = 0;

    class RequestListener : public ::fastdds::dds::DataReaderListener
    {
    private:

        ::fastdds::dds::DataWriter* datawriter_;
        FastDDSTest* test_;

    public:

        void publisher(
                FastDDSTest* test,
                ::fastdds::dds::DataWriter* datawriter)
        {
            test_ = test;
            datawriter_ = datawriter;
        }

        void on_subscription_matched(
                ::fastdds::dds::DataReader* /*datareader*/,
                const ::fastdds::dds::SubscriptionMatchedStatus& /*info*/) override
        {
            std::unique_lock<std::mutex> lock(test_->matched_mutex_);
            test_->pub_sub_matched_++;
            if (test_->pub_sub_matched_ == 2)
            {
                test_->mutex_.unlock();
            }
        }

        void on_data_available(
                ::fastdds::dds::DataReader* reader) override
        {
            ::fastdds::dds::SampleInfo info;
            TestService_Request req_msg;
            TestService_Reply rep_msg;

            if (fastrtps::types::ReturnCode_t::RETCODE_OK ==
                    reader->take_next_sample(&req_msg, &info))
            {
#if FASTRTPS_VERSION_MINOR < 2
                if (::fastdds::dds::InstanceStateKind::ALIVE == info.instance_state)
#else
                if (::fastdds::dds::InstanceStateKind::ALIVE_INSTANCE_STATE == info.instance_state)
#endif //  if FASTRTPS_VERSION_MINOR < 2
                {
                    const TestService_Call& request = req_msg.data();
                    TestService_Return& reply = rep_msg.reply();
                    fastrtps::rtps::WriteParams params;
                    params.related_sample_identity(info.sample_identity);

                    switch (request._d())
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
                            datawriter_->write(&rep_msg, params);
                            break;
                        }
                        case 1: // a + b = result
                        {
                            const Method1_In& input = request.method1();
                            Method1_Result output;
                            output.result(input.a() + input.b());
                            reply.method1(output);
                            datawriter_->write(&rep_msg, params);
                            break;
                        }
                        case 2: // Echoes data
                        {
                            const Method2_In& input = request.method2();
                            Method2_Result output;
                            output.data(input.data());
                            reply.method2(output);
                            datawriter_->write(&rep_msg, params);
                            break;
                        }
                    }
                }
            }
        }

    }
    request_listener;

    class ReplyListener : public ::fastdds::dds::DataReaderListener
    {
    private:

        FastDDSTest* test_;
        eprosima::xtypes::DynamicType::Ptr method0_reply_type;
        eprosima::xtypes::DynamicType::Ptr method1_reply_type;
        eprosima::xtypes::DynamicType::Ptr method2_reply_type;

    public:

        void publisher(
                FastDDSTest* test)
        {
            test_ = test;

            {
                eprosima::xtypes::StructType reply_struct("Method0_Result");
                reply_struct.add_member(eprosima::xtypes::Member("success", eprosima::xtypes::primitive_type<bool>()));
                method0_reply_type = std::move(reply_struct);
            }

            {
                eprosima::xtypes::StructType reply_struct("Method1_Result");
                reply_struct.add_member(eprosima::xtypes::Member("result",
                        eprosima::xtypes::primitive_type<int32_t>()));
                method1_reply_type = std::move(reply_struct);
            }

            {
                eprosima::xtypes::StructType reply_struct("Method2_Result");
                reply_struct.add_member(eprosima::xtypes::Member("data", eprosima::xtypes::primitive_type<float>()));
                method2_reply_type = std::move(reply_struct);
            }
        }

        void on_subscription_matched(
                ::fastdds::dds::DataReader* /*datareader*/,
                const ::fastdds::dds::SubscriptionMatchedStatus& /*info*/) override
        {
            std::unique_lock<std::mutex> lock(test_->matched_mutex_);
            test_->pub_sub_matched_++;
            if (test_->pub_sub_matched_ == 2)
            {
                test_->mutex_.unlock();
            }
        }

        void on_data_available(
                ::fastdds::dds::DataReader* reader) override
        {
            ::fastdds::dds::SampleInfo info;
            TestService_Reply rep_msg;

            if (fastrtps::types::ReturnCode_t::RETCODE_OK ==
                    reader->take_next_sample(&rep_msg, &info))
            {
#if FASTRTPS_VERSION_MINOR < 2
                if (::fastdds::dds::InstanceStateKind::ALIVE == info.instance_state)
#else
                if (::fastdds::dds::InstanceStateKind::ALIVE_INSTANCE_STATE == info.instance_state)
#endif //  if FASTRTPS_VERSION_MINOR < 2
                {
                    switch (rep_msg.reply()._d())
                    {
                        case 0:
                        {
                            eprosima::xtypes::DynamicData reply(*method0_reply_type);
                            reply["success"] = rep_msg.reply().method0().success();
                            test_->promise_->set_value(std::move(reply));
                            return;
                        }
                        case 1:
                        {
                            eprosima::xtypes::DynamicData reply(*method1_reply_type);
                            reply["result"] = rep_msg.reply().method1().result();
                            test_->promise_->set_value(std::move(reply));
                            return;
                        }
                        case 2:
                        {
                            eprosima::xtypes::DynamicData reply(*method2_reply_type);
                            reply["data"] = rep_msg.reply().method2().data();
                            test_->promise_->set_value(std::move(reply));
                            return;
                        }
                    }
                }
            }

            test_->promise_->set_value(
                ::eprosima::xtypes::DynamicData(::eprosima::xtypes::primitive_type<bool>()));
        }

    }
    reply_listener;
};

// class FastDDSTest : public eprosima::fastrtps::PublisherListener, public eprosima::fastrtps::ParticipantListener
// {
// public:

//     FastDDSTest(
//             bool server,
//             std::mutex& mtx)
//         : server_(server)
//         , mutex_(mtx)
//     {
//         using namespace eprosima::fastrtps;
//         ParticipantAttributes part_attr;
//         if (server)
//         {
//             part_attr.rtps.setName("FastDDSTestServer");
//         }
//         else
//         {
//             part_attr.rtps.setName("FastDDSTestClient");
//         }
//         part_ = Domain::createParticipant(part_attr);

//         Domain::registerType(part_, &psType_req_);
//         Domain::registerType(part_, &psType_rep_);

//         // Publisher of replies
//         PublisherAttributes pub_attr;
//         pub_attr.topic.topicKind = rtps::NO_KEY;
//         if (server)
//         {
//             pub_attr.topic.topicDataType = psType_rep_.getName();
//             pub_attr.topic.topicName = "TestService_Reply";
//         }
//         else
//         {
//             pub_attr.topic.topicDataType = psType_req_.getName();
//             pub_attr.topic.topicName = "TestService_Request";
//         }
//         pub_ = Domain::createPublisher(part_, pub_attr, this);

//         request_listener.publisher(this, pub_);
//         reply_listener.publisher(this, pub_);

//         // Subscriber of requests
//         SubscriberAttributes sub_attr;
//         sub_attr.topic.topicKind = rtps::NO_KEY;
//         if (server)
//         {
//             sub_attr.topic.topicDataType = psType_req_.getName();
//             sub_attr.topic.topicName = "TestService_Request";
//             sub_ = Domain::createSubscriber(part_, sub_attr, &request_listener);
//         }
//         else
//         {
//             sub_attr.topic.topicDataType = psType_rep_.getName();
//             sub_attr.topic.topicName = "TestService_Reply";
//             sub_ = Domain::createSubscriber(part_, sub_attr, &reply_listener);
//         }
//     }

//     ~FastDDSTest()
//     {
//         using namespace eprosima::fastrtps;
//         Domain::removePublisher(pub_);
//         Domain::removeSubscriber(sub_);
//         Domain::removeParticipant(part_);
//         delete promise_;
//     }

//     std::future<eprosima::xtypes::DynamicData> request(
//             const eprosima::xtypes::DynamicData& data)
//     {
//         TestService_Request request;

//         if (data.type().name() == "Method0_In")
//         {
//             Method0_In in;
//             in.data(data["data"]);
//             request.data().method0(in);
//         }
//         else if (data.type().name() == "Method1_In")
//         {
//             Method1_In in;
//             in.a(data["a"]);
//             in.b(data["b"]);
//             request.data().method1(in);
//         }
//         else if (data.type().name() == "Method2_In")
//         {
//             Method2_In in;
//             in.data(data["data"]);
//             request.data().method2(in);
//         }

//         pub_->write(&request);
//         delete promise_;
//         promise_ = new std::promise<eprosima::xtypes::DynamicData>();
//         return promise_->get_future();
//     }

//     void onParticipantDiscovery(
//             eprosima::fastrtps::Participant* /*participant*/,
//             eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& /*info*/) override
//     {
//     }

//     void onPublicationMatched(
//             eprosima::fastrtps::Publisher*,
//             eprosima::fastrtps::rtps::MatchingInfo&) override
//     {
//         std::unique_lock<std::mutex> lock(matched_mutex_);
//         pup_sub_matched_++;
//         if (pup_sub_matched_ == 2)
//         {
//             mutex_.unlock();
//         }
//     }

// private:

//     eprosima::fastrtps::Participant* part_;
//     eprosima::fastrtps::Subscriber* sub_;
//     eprosima::fastrtps::Publisher* pub_;
//     TestService_RequestPubSubType psType_req_;
//     TestService_ReplyPubSubType psType_rep_;
//     std::promise<eprosima::xtypes::DynamicData>* promise_ = nullptr;
//     bool server_;
//     std::mutex& mutex_;
//     std::mutex matched_mutex_;
//     uint32_t pup_sub_matched_ = 0;

//     class RequestListener : public eprosima::fastrtps::SubscriberListener
//     {
//     private:

//         eprosima::fastrtps::Publisher* pub_;
//         FastDDSTest* test_;

//     public:

//         void publisher(
//                 FastDDSTest* test,
//                 eprosima::fastrtps::Publisher* pub)
//         {
//             test_ = test;
//             pub_ = pub;
//         }

//         void onSubscriptionMatched(
//                 eprosima::fastrtps::Subscriber*,
//                 eprosima::fastrtps::rtps::MatchingInfo&) override
//         {
//             std::unique_lock<std::mutex> lock(test_->matched_mutex_);
//             test_->pup_sub_matched_++;
//             if (test_->pup_sub_matched_ == 2)
//             {
//                 test_->mutex_.unlock();
//             }
//         }

//         void onNewDataMessage(
//                 eprosima::fastrtps::Subscriber* sub) override
//         {
//             using namespace eprosima::fastrtps;
//             SampleInfo_t info;
//             TestService_Request req_msg;
//             TestService_Reply rep_msg;

//             if (sub->takeNextData(&req_msg, &info))
//             {
//                 const TestService_Call& request = req_msg.data();
//                 TestService_Return& reply = rep_msg.reply();
//                 rtps::WriteParams params;
//                 params.related_sample_identity(info.sample_identity);

//                 switch (request._d())
//                 {
//                     case 0: // If the data is "TEST", then success will be true.
//                     {
//                         const Method0_In& input = request.method0();
//                         Method0_Result output;
//                         output.success(false);
//                         if (input.data() == "TEST")
//                         {
//                             output.success(true);
//                         }
//                         reply.method0(output);
//                         pub_->write(&rep_msg, params);
//                         break;
//                     }
//                     case 1: // a + b = result
//                     {
//                         const Method1_In& input = request.method1();
//                         Method1_Result output;
//                         output.result(input.a() + input.b());
//                         reply.method1(output);
//                         pub_->write(&rep_msg, params);
//                         break;
//                     }
//                     case 2: // Echoes data
//                     {
//                         const Method2_In& input = request.method2();
//                         Method2_Result output;
//                         output.data(input.data());
//                         reply.method2(output);
//                         pub_->write(&rep_msg, params);
//                         break;
//                     }

//                 }
//             }
//         }

//     }
//     request_listener;

//     class ReplyListener : public eprosima::fastrtps::SubscriberListener
//     {
//     private:

//         eprosima::fastrtps::Publisher* pub_;
//         FastDDSTest* test_;
//         eprosima::xtypes::DynamicType::Ptr method0_reply_type;
//         eprosima::xtypes::DynamicType::Ptr method1_reply_type;
//         eprosima::xtypes::DynamicType::Ptr method2_reply_type;

//     public:

//         void publisher(
//                 FastDDSTest* test,
//                 eprosima::fastrtps::Publisher* pub)
//         {
//             test_ = test;
//             pub_ = pub;

//             {
//                 eprosima::xtypes::StructType reply_struct("Method0_Result");
//                 reply_struct.add_member(eprosima::xtypes::Member("success", eprosima::xtypes::primitive_type<bool>()));
//                 method0_reply_type = std::move(reply_struct);
//             }

//             {
//                 eprosima::xtypes::StructType reply_struct("Method1_Result");
//                 reply_struct.add_member(eprosima::xtypes::Member("result",
//                         eprosima::xtypes::primitive_type<int32_t>()));
//                 method1_reply_type = std::move(reply_struct);
//             }

//             {
//                 eprosima::xtypes::StructType reply_struct("Method2_Result");
//                 reply_struct.add_member(eprosima::xtypes::Member("data", eprosima::xtypes::primitive_type<float>()));
//                 method2_reply_type = std::move(reply_struct);
//             }
//         }

//         void onSubscriptionMatched(
//                 eprosima::fastrtps::Subscriber*,
//                 eprosima::fastrtps::rtps::MatchingInfo&) override
//         {
//             std::unique_lock<std::mutex> lock(test_->matched_mutex_);
//             test_->pup_sub_matched_++;
//             if (test_->pup_sub_matched_ == 2)
//             {
//                 test_->mutex_.unlock();
//             }
//         }

//         void onNewDataMessage(
//                 eprosima::fastrtps::Subscriber* sub) override
//         {
//             using namespace eprosima::fastrtps;
//             SampleInfo_t info;
//             TestService_Reply rep_msg;

//             if (sub->takeNextData(&rep_msg, &info))
//             {
//                 switch (rep_msg.reply()._d())
//                 {
//                     case 0:
//                     {
//                         eprosima::xtypes::DynamicData reply(*method0_reply_type);
//                         reply["success"] = rep_msg.reply().method0().success();
//                         test_->promise_->set_value(std::move(reply));
//                         return;
//                     }
//                     case 1:
//                     {
//                         eprosima::xtypes::DynamicData reply(*method1_reply_type);
//                         reply["result"] = rep_msg.reply().method1().result();
//                         test_->promise_->set_value(std::move(reply));
//                         return;
//                     }
//                     case 2:
//                     {
//                         eprosima::xtypes::DynamicData reply(*method2_reply_type);
//                         reply["data"] = rep_msg.reply().method2().data();
//                         test_->promise_->set_value(std::move(reply));
//                         return;
//                     }
//                 }
//             }

//             test_->promise_->set_value(::eprosima::xtypes::DynamicData(::eprosima::xtypes::primitive_type<bool>()));
//         }

//     }
//     reply_listener;
// };

std::string gen_config_yaml(
        const std::string& topic_type,
        const std::string& topic_sent,
        const std::string& topic_recv,
        const std::string& topic_mapped,
        const std::string& dds_config_file_path,
        const std::string& dds_profile_name)
{
    std::string remap = "";
    if (topic_mapped != "")
    {
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
    s += "        type: fastdds\n";

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
        const std::vector<std::string>& dds_topics,
        const std::vector<std::string>& topics,
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
    s += "        type: fastdds\n";

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
    for (uint32_t i = 0; i < request_types.size(); ++i)
    {
        s += "    " + topics[i];
        s += ": {";
        s += " request_type: \"" + request_types[i] + "\",";
        s += " reply_type: \"" + reply_types[i] + "\",";
        s += " route: route,";
        // Use dds_service ONLY if remap can cause collision, for example, same dds_service to connect with multiple
        // soss services, like in the "client" tests.
        //s += " dds_service: \"" + dds_topics[i] + "\",";
        s += " remap: { " +
                std::string("dds: {")
                + " request_type: \"" + request_members[i] + "\", "
                + " reply_type: \"" + reply_members[i] + "\", "
                + (topics[i] == dds_topics[i] ? "" : " topic: \"" + dds_topics[i] + "\"")
                + " },"; // dds
        if (client)
        {
            s += std::string("mock: {")
                    + " topic: \"" + dds_topics[i] + "\""
                    + " },"; // mock
        }
        s += " }"; // remap
        s += " }\n";
    }
    return s;
}

is::core::InstanceHandle create_instance(
        const std::string& topic_type,
        const std::string& topic_sent,
        const std::string& topic_recv,
        bool remap,
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

    logger << utils::Logger::Level::INFO << "Current YAML file:" << std::endl;
    std::cout << "====================================================================================" << std::endl;
    std::cout << "------------------------------------------------------------------------------------" << std::endl;
    std::cout << config_yaml << std::endl;
    std::cout << "====================================================================================" << std::endl;

    const YAML::Node config_node = YAML::Load(config_yaml);
    is::core::InstanceHandle instance = is::run_instance(config_node);

    std::this_thread::sleep_for(1s); // wait publisher and subscriber matching

    return instance;
}

is::core::InstanceHandle create_method_instance(
        const std::vector<std::string>& dds_topics,
        const std::vector<std::string>& topics,
        const std::vector<std::string>& request_types,
        const std::vector<std::string>& reply_types,
        const std::vector<std::string>& request_members,
        const std::vector<std::string>& reply_members,
        bool client,
        const std::string& config_file = "",
        const std::string& profile_name = "")
{
    std::string config_yaml = gen_config_method_yaml(
        dds_topics,
        topics,
        request_types,
        reply_types,
        request_members,
        reply_members,
        client,
        config_file,
        profile_name);

    logger << utils::Logger::Level::INFO <<  "Current YAML file" << std::endl;
    std::cout << "====================================================================================" << std::endl;
    std::cout << "------------------------------------------------------------------------------------" << std::endl;
    std::cout << config_yaml << std::endl;
    std::cout << "====================================================================================" << std::endl;

    const YAML::Node config_node = YAML::Load(config_yaml);
    is::core::InstanceHandle instance = is::run_instance(config_node);

    std::this_thread::sleep_for(1s); // wait publisher and subscriber matching

    return instance;
}

void roundtrip(
        const std::string& topic_sent,
        const std::string& topic_recv,
        const eprosima::xtypes::DynamicData& message,
        eprosima::xtypes::DynamicData& result)
{
    std::promise<eprosima::xtypes::DynamicData> receive_msg_promise;
    ASSERT_TRUE(is::sh::mock::subscribe(
                topic_recv,
                [&](const eprosima::xtypes::DynamicData& msg_to_recv)
                {
                    receive_msg_promise.set_value(msg_to_recv);
                }));

    is::sh::mock::publish_message(topic_sent, message);

    auto receive_msg_future = receive_msg_promise.get_future();
    ASSERT_EQ(std::future_status::ready, receive_msg_future.wait_for(5s));

    result = receive_msg_future.get();
}

void roundtrip_server(
        const std::string& topic,
        const eprosima::xtypes::DynamicData& request,
        eprosima::xtypes::DynamicData& response)
{
    // MOCK->DDS->MOCK
    std::shared_future<eprosima::xtypes::DynamicData> response_future =
            is::sh::mock::request(topic, request);
    ASSERT_EQ(std::future_status::ready, response_future.wait_for(5s));
    eprosima::xtypes::DynamicData result = /*std::move*/ (response_future.get());
    {
        using namespace std;
        (&response_future)->~shared_future<eprosima::xtypes::DynamicData>();
    }
    response = result;
}

void roundtrip_client(
        const eprosima::xtypes::DynamicData& message,
        std::shared_ptr<FastDDSTest> fastdds,
        eprosima::xtypes::DynamicData& response)
{
    // DDS->MOCK->DDS
    std::future<eprosima::xtypes::DynamicData> response_future = fastdds->request(message);
    ASSERT_EQ(std::future_status::ready, response_future.wait_for(5s));

    response = response_future.get();
}

TEST(FastDDS, Transmit_to_and_receive_from_dds__basic_type_udp)
{
    const std::string topic_type = "dds_test_string";

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::stringstream ss;
    ss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    std::string message_data = "mock test message at " + ss.str();

    logger << utils::Logger::Level::INFO
           << "Message id: " << ss.str() << std::endl;

    const std::string topic_sent = "mock_to_dds_topic";
    const std::string topic_recv = "dds_to_mock_topic";
    is::core::InstanceHandle instance = create_instance(
        topic_type,
        topic_sent,
        topic_recv,
        true,
        "",
        "");

    ASSERT_TRUE(instance);

    const is::TypeRegistry& mock_types = *instance.type_registry("mock");
    eprosima::xtypes::DynamicData msg_to_sent(*mock_types.at(topic_type));
    msg_to_sent["data"].value(message_data);

    // Road: [mock -> dds -> dds -> mock]
    const is::TypeRegistry& dds_types = *instance.type_registry("dds");
    eprosima::xtypes::DynamicData msg_to_recv(*dds_types.at(topic_type));
    roundtrip(topic_sent, topic_recv, msg_to_sent, msg_to_recv);

    ASSERT_EQ(msg_to_sent, msg_to_recv);
    ASSERT_EQ(0, instance.quit().wait_for(1s));
    std::cout << "exiting first test..." << std::endl;
}

TEST(FastDDS, Transmit_to_and_receive_from_dds__basic_type_tcp_tunnel)
{
    const std::string topic_type = "dds_test_string";

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::stringstream ss;
    ss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    std::string message_data = "mock test message at " + ss.str();

    logger << utils::Logger::Level::INFO
           << "Message id: " << ss.str() << std::endl;

    const std::string config_file = "resources/tcp_config.xml";
    const std::string client_to_server_topic = "client_to_server_topic";
    const std::string server_to_client_topic = "server_to_client_topic";

    const std::string profile_name_server = "soss_profile_server";
    is::core::InstanceHandle server_instance = create_instance(
        topic_type,
        server_to_client_topic,
        client_to_server_topic,
        false,
        config_file,
        profile_name_server);

    ASSERT_TRUE(server_instance);

    const std::string profile_name_client = "soss_profile_client";
    is::core::InstanceHandle client_instance = create_instance(
        topic_type,
        client_to_server_topic,
        server_to_client_topic,
        false,
        config_file,
        profile_name_client);

    ASSERT_TRUE(client_instance);

    const is::TypeRegistry& mock_types = *server_instance.type_registry("mock");
    client_instance.type_registry("mock");
    eprosima::xtypes::DynamicData msg_to_sent(*mock_types.at(topic_type));
    msg_to_sent["data"].value(message_data);
    std::this_thread::sleep_for(2s);         // wait publisher and subscriber matching

    // Road: [mock -> dds-client] -> [dds-server -> mock]
    const is::TypeRegistry& dds_types = *server_instance.type_registry("dds");
    eprosima::xtypes::DynamicData msg_to_recv(*dds_types.at(topic_type));
    roundtrip(client_to_server_topic, client_to_server_topic, msg_to_sent, msg_to_recv);
    ASSERT_EQ(msg_to_sent, msg_to_recv);

    // Road: [mock <- dds-client] <- [dds-server <- mock]
    roundtrip(server_to_client_topic, server_to_client_topic, msg_to_sent, msg_to_recv);
    ASSERT_EQ(msg_to_sent, msg_to_recv);

    ASSERT_EQ(0, client_instance.quit().wait_for(1s));
    ASSERT_EQ(0, server_instance.quit().wait_for(1s));
}

TEST(FastDDS, Request_to_and_reply_from_dds_server_to_mock_client)
{
    // Road: [mock -> dds -> dds -> mock]
    is::core::InstanceHandle instance = create_method_instance(
        {"TestService", "TestService", "TestService"},
        {"TestService_0", "TestService_1", "TestService_2"},
        {"Method0_In", "Method1_In", "Method2_In"},
        {"Method0_Result", "Method1_Result", "Method2_Result"},
        {"TestService_Request.data.method0", "TestService_Request.data.method1",
         "TestService_Request.data.method2"},
        {"TestService_Reply.reply.method0", "TestService_Reply.reply.method1", "TestService_Reply.reply.method2"},
        false,
        "",
        "");

    ASSERT_TRUE(instance);

    const is::TypeRegistry& mock_types = *instance.type_registry("mock");
    const is::TypeRegistry& dds_types = *instance.type_registry("dds");

    std::mutex disc_mutex;
    disc_mutex.lock();

    std::shared_ptr<FastDDSTest> server = nullptr;
    ASSERT_NO_THROW(server.reset(new FastDDSTest(true, disc_mutex)));

    // method0
    {
        std::string message_data_fail = "TESTING";
        eprosima::xtypes::DynamicData msg_to_sent(*mock_types.at("Method0_In"));
        msg_to_sent["data"].value(message_data_fail);
        disc_mutex.lock();

        logger << utils::Logger::Level::INFO
               << "sent message: " << message_data_fail << std::endl;

        eprosima::xtypes::DynamicData msg_to_recv(*dds_types.at("Method0_Result"));
        roundtrip_server("TestService_0", msg_to_sent, msg_to_recv);
        ASSERT_FALSE(msg_to_recv["success"].value<bool>());

        std::string message_data_ok = "TEST";
        msg_to_sent["data"].value(message_data_ok);

        logger << utils::Logger::Level::INFO
               << "Sent message: " << message_data_ok << std::endl;

        roundtrip_server("TestService_0", msg_to_sent, msg_to_recv);
        ASSERT_TRUE(msg_to_recv["success"].value<bool>());
    }

    // method1
    {
        int32_t a = 23;
        int32_t b = 42;
        eprosima::xtypes::DynamicData msg_to_sent(*mock_types.at("Method1_In"));
        msg_to_sent["a"] = a;
        msg_to_sent["b"] = b;
        //disc_mutex.lock();
        logger << utils::Logger::Level::INFO
               << "Sent message: " << a << " + " << b << std::endl;

        eprosima::xtypes::DynamicData msg_to_recv(*dds_types.at("Method1_Result"));
        roundtrip_server("TestService_1", msg_to_sent, msg_to_recv);
        ASSERT_EQ(msg_to_recv["result"].value<int32_t>(), (a + b));
    }

    // method2
    {
        float data = 23.42;
        eprosima::xtypes::DynamicData msg_to_sent(*mock_types.at("Method2_In"));
        msg_to_sent["data"] = data;
        //disc_mutex.lock();

        logger << utils::Logger::Level::INFO
               << "Sent message: " << data << std::endl;

        eprosima::xtypes::DynamicData msg_to_recv(*dds_types.at("Method2_Result"));
        roundtrip_server("TestService_2", msg_to_sent, msg_to_recv);
        ASSERT_EQ(msg_to_recv["data"].value<float>(), data);
    }

    ASSERT_EQ(0, instance.quit().wait_for(1s));
}

TEST(FastDDS, Request_to_and_reply_from_mock_server_to_dds_client)
{
    eprosima::xtypes::StructType reply_struct0("Method0_Result");
    reply_struct0.add_member(eprosima::xtypes::Member("success", eprosima::xtypes::primitive_type<bool>()));

    eprosima::xtypes::StructType reply_struct1("Method1_Result");
    reply_struct1.add_member(eprosima::xtypes::Member("result", eprosima::xtypes::primitive_type<int32_t>()));

    eprosima::xtypes::StructType reply_struct2("Method2_Result");
    reply_struct2.add_member(eprosima::xtypes::Member("data", eprosima::xtypes::primitive_type<float>()));

    is::core::InstanceHandle instance = create_method_instance(
        {"TestService", "TestService", "TestService"},
        {"TestService_0", "TestService_1", "TestService_2"},
        {"Method0_In", "Method1_In", "Method2_In"},
        {"Method0_Result", "Method1_Result", "Method2_Result"},
        {"TestService_Request.data.method0", "TestService_Request.data.method1",
         "TestService_Request.data.method2"},
        {"TestService_Reply.reply.method0", "TestService_Reply.reply.method1", "TestService_Reply.reply.method2"},
        true,
        "",
        "");

    ASSERT_TRUE(instance);

    const is::TypeRegistry& mock_types = *instance.type_registry("mock");
    const is::TypeRegistry& dds_types = *instance.type_registry("dds");

    std::mutex disc_mutex;
    disc_mutex.lock();

    std::shared_ptr<FastDDSTest> client = nullptr;
    ASSERT_NO_THROW(client.reset(new FastDDSTest(false, disc_mutex)));

    // Serve using mock
    is::sh::mock::serve(
        "TestService",
        [&](const eprosima::xtypes::DynamicData& request)
        {
            eprosima::xtypes::DynamicData reply(reply_struct0);
            reply["success"] = (request["data"].value<std::string>() == "TEST");
            return reply;
        }, "Method0_In");

    is::sh::mock::serve(
        "TestService",
        [&](const eprosima::xtypes::DynamicData& request)
        {
            eprosima::xtypes::DynamicData reply(reply_struct1);
            reply["result"] = request["a"].value<int32_t>() + request["b"].value<int32_t>();
            return reply;
        }, "Method1_In");

    is::sh::mock::serve(
        "TestService",
        [&](const eprosima::xtypes::DynamicData& request)
        {
            eprosima::xtypes::DynamicData reply(reply_struct2);
            reply["data"] = request["data"].value<float>();
            return reply;
        }, "Method2_In");

    // method0
    {
        std::string message_data_fail = "TESTING";
        eprosima::xtypes::DynamicData msg_to_sent(*mock_types.at("Method0_In"));
        msg_to_sent["data"].value(message_data_fail);
        disc_mutex.lock();

        logger << utils::Logger::Level::INFO
               << "Sent message: " << message_data_fail << std::endl;

        eprosima::xtypes::DynamicData msg_to_recv(*dds_types.at("Method0_Result"));
        roundtrip_client(msg_to_sent, client, msg_to_recv);
        ASSERT_FALSE(msg_to_recv["success"].value<bool>());

        std::string message_data_ok = "TEST";
        msg_to_sent["data"].value(message_data_ok);

        logger << utils::Logger::Level::INFO
               << "Sent message: " << message_data_ok << std::endl;

        roundtrip_client(msg_to_sent, client, msg_to_recv);

        ASSERT_TRUE(msg_to_recv["success"].value<bool>());
    }

    // method1
    {
        int32_t a = 23;
        int32_t b = 42;
        eprosima::xtypes::DynamicData msg_to_sent(*mock_types.at("Method1_In"));
        msg_to_sent["a"] = a;
        msg_to_sent["b"] = b;
        //disc_mutex.lock();

        logger << utils::Logger::Level::INFO
               << "Sent message: " << a << " + " << b << std::endl;

        eprosima::xtypes::DynamicData msg_to_recv(*dds_types.at("Method1_Result"));
        roundtrip_client(msg_to_sent, client, msg_to_recv);

        ASSERT_EQ(msg_to_recv["result"].value<int32_t>(), (a + b));
    }

    // method2
    {
        float data = 23.42;
        eprosima::xtypes::DynamicData msg_to_sent(*mock_types.at("Method2_In"));
        msg_to_sent["data"] = data;
        //disc_mutex.lock();

        logger << utils::Logger::Level::INFO
               << "Sent message: " << data << std::endl;

        eprosima::xtypes::DynamicData msg_to_recv(*dds_types.at("Method2_Result"));
        roundtrip_client(msg_to_sent, client, msg_to_recv);

        ASSERT_EQ(msg_to_recv["data"].value<float>(), data);
    }

    ASSERT_EQ(0, instance.quit().wait_for(1s));
}

} //  namespace test
} //  namespace fastdds
} //  namespace sh
} //  namespace is
} //  namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
