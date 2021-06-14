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
#include "../../src/Conversion.hpp"

#include <fastrtps/fastrtps_all.h>

#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/MemberDescriptor.h>

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

// Method0: If the data is "TEST", then success will be true.
// Method1: a + b = result
// Method2: Echoes data
static const std::string service_request_idl =
        R"(            struct Method0_In
            {
                string data;
            };

            struct Method1_In
            {
                int32 a;
                int32 b;
            };

            struct Method2_In
            {
                float data;
            };

            union TestService_Call switch (uint32)
            {
                case 0: Method0_In method0;
                case 1: Method1_In method1;
                case 2: Method2_In method2;
            };

            struct TestService_Request
            {
                TestService_Call data;
            };)";

// Method0: If the data is "TEST", then success will be true.
// Method1: a + b = result
// Method2: Echoes data
static const std::string service_reply_idl =
        R"(            struct Method0_Result
            {
                boolean success;
            };
            struct Method1_Result
            {
                int32 result;
            };
            struct Method2_Result
            {
                float data;
            };
            union TestService_Return switch (int32)
            {
                case 0: Method0_Result method0;
                case 1: Method1_Result method1;
                case 2: Method2_Result method2;
            };

            struct TestService_Reply
            {
                TestService_Return reply;
            };)";

class FastDDSServicesTest
    : public ::fastdds::dds::DataWriterListener
{
public:

    FastDDSServicesTest(
            bool server,
            std::mutex& mtx)
        : ::fastdds::dds::DataWriterListener()
        , participant_factory_(::fastdds::dds::DomainParticipantFactory::get_instance())
        , server_(server)
        , mutex_(mtx)
        , matched_mutex_()
    {
        auto create_typesupport =
                [&](
            const std::string& type_name,
            const std::string& idl,
            fastrtps::types::DynamicPubSubType& dynamic_typesupport)
                {
                    xtypes::idl::Context context = xtypes::idl::parse(idl);
                    if (!context.success)
                    {
                        throw DDSMiddlewareException(
                                  logger, "IDL definition for type " + type_name + " is incorrect");
                    }

                    xtypes::DynamicType::Ptr type = context.module().type(type_name);
                    if (nullptr == type.get())
                    {
                        throw DDSMiddlewareException(logger, "Could not retrieve service type '"
                                      + type_name + "' from the IDL definition");
                    }

                    fastrtps::types::DynamicTypeBuilder* builder =
                            Conversion::create_builder(*type);
                    if (!builder)
                    {
                        throw DDSMiddlewareException(
                                  logger, "Cannot create builder for type " + type_name);
                    }

                    fastrtps::types::DynamicType_ptr dtptr = builder->build();
                    if (!dtptr)
                    {
                        throw DDSMiddlewareException(
                                  logger, "Could not build DynamicType_ptr for type " + type_name);
                    }

                    dynamic_typesupport = fastrtps::types::DynamicPubSubType(dtptr);
                    dynamic_typesupport.setName(type_name.c_str());
                    /**
                     * The following lines are added here so that a bug with UnionType in
                     * Fast DDS Dynamic Types is bypassed. This is a workaround and SHOULD
                     * be removed once this bug is solved.
                     * Until that moment, the Fast DDS SystemHandle will not be compatible with
                     * Fast DDS Dynamic Type Discovery mechanism.
                     *
                     * More information here: https://eprosima.easyredmine.com/issues/11349
                     */
                    // WORKAROUND START
                    dynamic_typesupport.auto_fill_type_information(false);
                    dynamic_typesupport.auto_fill_type_object(false);
                    // WORKAROUND END
                };

        // Parse IDL and generate dynamic typesupport for service request type
        create_typesupport("TestService_Request", service_request_idl, tsType_req_);

        // Parse IDL and generate dynamic typesupport for service reply type
        create_typesupport("TestService_Reply", service_reply_idl, tsType_rep_);

        // Create participant
        ::fastdds::dds::DomainId_t default_domain_id(3);
        ::fastdds::dds::DomainParticipantQos participant_qos = ::fastdds::dds::PARTICIPANT_QOS_DEFAULT;

        participant_qos.name(server ? "FastDDSServicesTestServer" : "FastDDSServicesTestClient");

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
        // Create publisher entities
        publisher_ = participant_->create_publisher(::fastdds::dds::PUBLISHER_QOS_DEFAULT);

        if (!publisher_)
        {
            throw DDSMiddlewareException(logger, "Publisher was not created");
        }

        pub_topic_ = participant_->create_topic(
            server ? "TestService_Reply" : "TestService_Request",
            server ? tsType_rep_.getName() : tsType_req_.getName(),
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

        if (server)
        {
            request_listener.publisher(this, datawriter_);
        }
        else
        {
            reply_listener.publisher(this);
        }

        // Create subscriber entities
        subscriber_ = participant_->create_subscriber(::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);

        if (!subscriber_)
        {
            throw DDSMiddlewareException(logger, "Subscriber was not created");
        }

        sub_topic_ = participant_->create_topic(
            server ? "TestService_Request" : "TestService_Reply",
            server ? tsType_req_.getName() : tsType_rep_.getName(),
            ::fastdds::dds::TOPIC_QOS_DEFAULT);

        if (!sub_topic_)
        {
            throw DDSMiddlewareException(logger, "Creating subscriber topic failed");
        }

        ::fastdds::dds::DataReaderQos datareader_qos = ::fastdds::dds::DATAREADER_QOS_DEFAULT;
        ::fastdds::dds::ReliabilityQosPolicy rel_policy;
        rel_policy.kind = ::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        datareader_qos.reliability(rel_policy);

        datareader_ = subscriber_->create_datareader(
            sub_topic_, datareader_qos,
            server ? static_cast<::fastdds::dds::DataReaderListener*>(&request_listener)
            : static_cast<::fastdds::dds::DataReaderListener*>(&reply_listener));

        if (!datareader_)
        {
            std::ostringstream err;
            err << "Creating datareader for topic '" << sub_topic_->get_name() << "' failed";

            throw DDSMiddlewareException(logger, err.str());
        }
    }

    ~FastDDSServicesTest()
    {
        // Unregister types
        participant_->unregister_type(tsType_req_.getName());
        participant_->unregister_type(tsType_rep_.getName());

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
        const fastrtps::types::DynamicType_ptr& req_type = tsType_req_.GetDynamicType();
        fastrtps::types::DynamicData* req_msg =
                fastrtps::types::DynamicDataFactory::get_instance()->create_data(req_type);
        fastrtps::types::MemberId req_id = req_msg->get_member_id_by_name("data");
        fastrtps::types::DynamicData* request = req_msg->loan_value(req_id);

        if ("Method0_In" == data.type().name())
        {
            fastrtps::types::MemberId u_req_id = request->get_member_id_by_name("method0");
            fastrtps::types::DynamicData* method0_in = request->loan_value(u_req_id);

            if (!Conversion::xtypes_to_fastdds(data, method0_in))
            {
                throw DDSMiddlewareException(
                          logger, "Error converting from xTypes to FastDDS DynamicData for type"
                          + data.type().name());
            }
            request->return_loaned_value(method0_in);
        }
        else if ("Method1_In" == data.type().name())
        {
            fastrtps::types::MemberId u_req_id = request->get_member_id_by_name("method1");
            fastrtps::types::DynamicData* method1_in = request->loan_value(u_req_id);

            if (!Conversion::xtypes_to_fastdds(data, method1_in))
            {
                throw DDSMiddlewareException(
                          logger, "Error converting from xTypes to FastDDS DynamicData for type"
                          + data.type().name());
            }
            request->return_loaned_value(method1_in);
        }
        else if ("Method2_In" == data.type().name())
        {
            fastrtps::types::MemberId u_req_id = request->get_member_id_by_name("method2");
            fastrtps::types::DynamicData* method2_in = request->loan_value(u_req_id);

            if (!Conversion::xtypes_to_fastdds(data, method2_in))
            {
                throw DDSMiddlewareException(
                          logger, "Error converting from xTypes to FastDDS DynamicData for type"
                          + data.type().name());
            }
            request->return_loaned_value(method2_in);
        }
        else
        {
            throw DDSMiddlewareException(
                      logger, "DDS Client: invalid request type: " + data.type().name());
        }
        req_msg->return_loaned_value(request);

        datawriter_->write(static_cast<void*>(request));
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

    fastrtps::types::DynamicPubSubType tsType_req_;
    fastrtps::types::DynamicPubSubType tsType_rep_;

    std::promise<eprosima::xtypes::DynamicData>* promise_ = nullptr;
    bool server_;
    std::mutex& mutex_;
    std::mutex matched_mutex_;
    uint32_t pub_sub_matched_ = 0;

    class RequestListener : public ::fastdds::dds::DataReaderListener
    {
    private:

        ::fastdds::dds::DataWriter* datawriter_;
        FastDDSServicesTest* test_;

    public:

        void publisher(
                FastDDSServicesTest* test,
                ::fastdds::dds::DataWriter* datawriter)
        {
            test_ = test;
            datawriter_ = datawriter;
        }

        void on_subscription_matched(
                ::fastdds::dds::DataReader* /*datareader*/,
                const ::fastdds::dds::SubscriptionMatchedStatus& info) override
        {
            if (1 == info.current_count_change)
            {
                logger << utils::Logger::Level::INFO
                       << "Subscriber for Request matched" << std::endl;
            }
            else if (-1 == info.current_count_change)
            {
                logger << utils::Logger::Level::INFO
                       << "Subscriber for Request unmatched" << std::endl;
            }

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

            const fastrtps::types::DynamicType_ptr& req_type = test_->tsType_req_.GetDynamicType();
            fastrtps::types::DynamicData* req_msg =
                    fastrtps::types::DynamicDataFactory::get_instance()->create_data(req_type);

            const fastrtps::types::DynamicType_ptr& rep_type = test_->tsType_rep_.GetDynamicType();
            fastrtps::types::DynamicData* rep_msg =
                    fastrtps::types::DynamicDataFactory::get_instance()->create_data(rep_type);

            if (fastrtps::types::ReturnCode_t::RETCODE_OK ==
                    reader->take_next_sample(req_msg, &info))
            {
#if FASTRTPS_VERSION_MINOR < 2
                if (::fastdds::dds::InstanceStateKind::ALIVE == info.instance_state)
#else
                if (::fastdds::dds::InstanceStateKind::ALIVE_INSTANCE_STATE == info.instance_state)
#endif //  if FASTRTPS_VERSION_MINOR < 2
                {
                    fastrtps::types::MemberId req_id = req_msg->get_member_id_by_name("data");
                    fastrtps::types::DynamicData* request = req_msg->loan_value(req_id);

                    fastrtps::types::MemberId rep_id = rep_msg->get_member_id_by_name("reply");
                    fastrtps::types::DynamicData* reply = rep_msg->loan_value(rep_id);

                    fastrtps::rtps::WriteParams params;
                    params.related_sample_identity(info.sample_identity);

                    // Get active member
                    UnionDynamicData* u_request = static_cast<UnionDynamicData*>(request);
                    fastrtps::types::MemberId u_req_id = u_request->get_union_id();
                    fastrtps::types::MemberDescriptor u_req_descriptor;
                    request->get_descriptor(u_req_descriptor, u_req_id);

                    if ("method0" == u_req_descriptor.get_name())
                    {
                        fastrtps::types::DynamicData* method0_in = request->loan_value(u_req_id);
                        fastrtps::types::MemberId u_rep_id = reply->get_member_id_by_name("method0");
                        fastrtps::types::DynamicData* method0_result = reply->loan_value(u_rep_id);

                        std::string data;
                        fastrtps::types::MemberId method0_in_data_id =
                                method0_in->get_member_id_by_name("data");
                        method0_in->get_string_value(data, method0_in_data_id);
                        request->return_loaned_value(method0_in);

                        fastrtps::types::MemberId method0_result_success_id =
                                method0_result->get_member_id_by_name("success");
                        if ("TEST" == data)
                        {
                            method0_result->set_bool_value(true, method0_result_success_id);
                        }
                        else
                        {
                            method0_result->set_bool_value(false, method0_result_success_id);
                        }
                        reply->return_loaned_value(method0_result);
                    }
                    else if ("method1" == u_req_descriptor.get_name())
                    {
                        fastrtps::types::DynamicData* method1_in = request->loan_value(u_req_id);
                        fastrtps::types::MemberId u_rep_id = reply->get_member_id_by_name("method1");
                        fastrtps::types::DynamicData* method1_result = reply->loan_value(u_rep_id);

                        int32_t a;
                        int32_t b;
                        fastrtps::types::MemberId method1_in_a_id =
                                method1_in->get_member_id_by_name("a");
                        fastrtps::types::MemberId method1_in_b_id =
                                method1_in->get_member_id_by_name("b");
                        method1_in->get_int32_value(a, method1_in_a_id);
                        method1_in->get_int32_value(b, method1_in_b_id);
                        request->return_loaned_value(method1_in);

                        fastrtps::types::MemberId method1_result_result_id =
                                method1_result->get_member_id_by_name("result");
                        method1_result->set_int32_value(a + b, method1_result_result_id);
                        reply->return_loaned_value(method1_result);
                    }
                    else if ("method2" == u_req_descriptor.get_name())
                    {
                        fastrtps::types::DynamicData* method2_in = request->loan_value(u_req_id);
                        fastrtps::types::MemberId u_rep_id = reply->get_member_id_by_name("method2");
                        fastrtps::types::DynamicData* method2_result = reply->loan_value(u_rep_id);

                        float data;
                        fastrtps::types::MemberId method2_in_data_id =
                                method2_in->get_member_id_by_name("data");
                        method2_in->get_float32_value(data, method2_in_data_id);
                        request->return_loaned_value(method2_in);

                        fastrtps::types::MemberId method2_result_data_id =
                                method2_result->get_member_id_by_name("data");
                        method2_result->set_float32_value(data, method2_result_data_id);
                        reply->return_loaned_value(method2_result);
                    }
                    else
                    {
                        throw DDSMiddlewareException(logger,
                                      "DDS Server: received request with invalid member name" +
                                      u_req_descriptor.get_name());
                    }

                    req_msg->return_loaned_value(request);
                    rep_msg->return_loaned_value(reply);

                    datawriter_->write(rep_msg, params);
                }
            }
        }

    };

    RequestListener request_listener;

    class ReplyListener : public ::fastdds::dds::DataReaderListener
    {
    private:

        FastDDSServicesTest* test_;
        eprosima::xtypes::DynamicType::Ptr method0_reply_type;
        eprosima::xtypes::DynamicType::Ptr method1_reply_type;
        eprosima::xtypes::DynamicType::Ptr method2_reply_type;

    public:

        void publisher(
                FastDDSServicesTest* test)
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
            const fastrtps::types::DynamicType_ptr& rep_type = test_->tsType_rep_.GetDynamicType();
            fastrtps::types::DynamicData* rep_msg =
                    fastrtps::types::DynamicDataFactory::get_instance()->create_data(rep_type);

            if (fastrtps::types::ReturnCode_t::RETCODE_OK ==
                    reader->take_next_sample(rep_msg, &info))
            {
#if FASTRTPS_VERSION_MINOR < 2
                if (::fastdds::dds::InstanceStateKind::ALIVE == info.instance_state)
#else
                if (::fastdds::dds::InstanceStateKind::ALIVE_INSTANCE_STATE == info.instance_state)
#endif //  if FASTRTPS_VERSION_MINOR < 2
                {
                    fastrtps::types::MemberId rep_id = rep_msg->get_member_id_by_name("reply");
                    fastrtps::types::DynamicData* reply = rep_msg->loan_value(rep_id);

                    // Get active member
                    UnionDynamicData* u_reply = static_cast<UnionDynamicData*>(reply);
                    fastrtps::types::MemberId u_rep_id = u_reply->get_union_id();
                    fastrtps::types::MemberDescriptor u_rep_descriptor;
                    reply->get_descriptor(u_rep_descriptor, u_rep_id);

                    if ("method0" == u_rep_descriptor.get_name())
                    {
                        fastrtps::types::MemberId u_rep_id = reply->get_member_id_by_name("method0");
                        fastrtps::types::DynamicData* method0_result = reply->loan_value(u_rep_id);
                        fastrtps::types::MemberId method0_result_success_id =
                                method0_result->get_member_id_by_name("success");

                        xtypes::DynamicData xtypes_reply(*method0_reply_type);
                        bool success;
                        method0_result->get_bool_value(success, method0_result_success_id);
                        reply->return_loaned_value(method0_result);
                        xtypes_reply["success"] = success;
                        test_->promise_->set_value(std::move(xtypes_reply));
                    }
                    else if ("method1" == u_rep_descriptor.get_name())
                    {
                        fastrtps::types::MemberId u_rep_id = reply->get_member_id_by_name("method1");
                        fastrtps::types::DynamicData* method1_result = reply->loan_value(u_rep_id);
                        fastrtps::types::MemberId method1_result_result_id =
                                method1_result->get_member_id_by_name("result");

                        xtypes::DynamicData xtypes_reply(*method1_reply_type);
                        int32_t result;
                        method1_result->get_int32_value(result, method1_result_result_id);
                        reply->return_loaned_value(method1_result);
                        xtypes_reply["result"] = result;
                        test_->promise_->set_value(std::move(xtypes_reply));
                    }
                    else if ("method2" == u_rep_descriptor.get_name())
                    {
                        fastrtps::types::MemberId u_rep_id = reply->get_member_id_by_name("method2");
                        fastrtps::types::DynamicData* method2_result = reply->loan_value(u_rep_id);
                        fastrtps::types::MemberId method2_result_data_id =
                                method2_result->get_member_id_by_name("data");

                        xtypes::DynamicData xtypes_reply(*method2_reply_type);
                        float data;
                        method2_result->get_float32_value(data, method2_result_data_id);
                        reply->return_loaned_value(method2_result);
                        xtypes_reply["data"] = data;
                        test_->promise_->set_value(std::move(xtypes_reply));
                    }
                    else
                    {
                        throw DDSMiddlewareException(logger,
                                      "DDS Client: received reply with invalid member name" +
                                      u_rep_descriptor.get_name());
                    }

                    rep_msg->return_loaned_value(reply);
                    return;
                }

            }

            test_->promise_->set_value(
                ::eprosima::xtypes::DynamicData(::eprosima::xtypes::primitive_type<bool>()));
        }

    };

    ReplyListener reply_listener;
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

    s += "    mock: { type: mock }\n";

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
    s += service_request_idl + "\n\n";
    s += "        - >\n";
    s += service_reply_idl + "\n\n";

    s += "systems:\n";
    s += "    dds:\n";
    s += "        type: fastdds\n";
    s += "        participant: { domain_id: 3 }\n";

    if (dds_config_file_path != "")
    {
        s += "        participant:\n";
        s += "            file_path: " + dds_config_file_path + "\n";
        s += "            profile_name: " + dds_profile_name + "\n";
    }

    s += "    mock: { type: mock }\n";

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
        // Integration Service services, like in the "client" tests.
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
        std::shared_ptr<FastDDSServicesTest> fastdds,
        eprosima::xtypes::DynamicData& response)
{
    // DDS->MOCK->DDS
    std::future<eprosima::xtypes::DynamicData> response_future = fastdds->request(message);
    ASSERT_EQ(std::future_status::ready, response_future.wait_for(5s));

    response = response_future.get();
}

TEST(FastDDS, Transmit_to_and_receive_from_dds__basic_type_default_transport)
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
    ASSERT_NO_THROW(roundtrip(topic_sent, topic_recv, msg_to_sent, msg_to_recv));

    ASSERT_EQ(msg_to_sent, msg_to_recv);
    ASSERT_EQ(0, instance.quit().wait_for(1s));
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

    const std::string profile_name_server = "is_profile_server";
    is::core::InstanceHandle server_instance = create_instance(
        topic_type,
        server_to_client_topic,
        client_to_server_topic,
        false,
        config_file,
        profile_name_server);

    ASSERT_TRUE(server_instance);

    const std::string profile_name_client = "is_profile_client";
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
    ASSERT_NO_THROW(roundtrip(client_to_server_topic, client_to_server_topic, msg_to_sent, msg_to_recv));
    ASSERT_EQ(msg_to_sent, msg_to_recv);

    // Road: [mock <- dds-client] <- [dds-server <- mock]
    ASSERT_NO_THROW(roundtrip(server_to_client_topic, server_to_client_topic, msg_to_sent, msg_to_recv));
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

    const is::TypeRegistry* mock_types = instance.type_registry("mock");

    std::mutex disc_mutex;
    disc_mutex.lock();

    std::shared_ptr<FastDDSServicesTest> server = nullptr;
    ASSERT_NO_THROW(server.reset(new FastDDSServicesTest(true, disc_mutex)));

    // method0
    {
        std::string message_data_fail = "TESTING";
        eprosima::xtypes::DynamicData msg_to_sent(*mock_types->at("Method0_In"));
        msg_to_sent["data"].value(message_data_fail);
        disc_mutex.lock();

        logger << utils::Logger::Level::INFO
               << "Sent message: " << message_data_fail << std::endl;

        eprosima::xtypes::DynamicData msg_to_recv(*mock_types->at("Method0_Result"));
        ASSERT_NO_THROW(roundtrip_server("TestService_0", msg_to_sent, msg_to_recv));
        ASSERT_FALSE(msg_to_recv["success"].value<bool>());

        std::string message_data_ok = "TEST";
        msg_to_sent["data"].value(message_data_ok);

        logger << utils::Logger::Level::INFO
               << "Sent message: " << message_data_ok << std::endl;

        ASSERT_NO_THROW(roundtrip_server("TestService_0", msg_to_sent, msg_to_recv));
        ASSERT_TRUE(msg_to_recv["success"].value<bool>());
    }

    // method1
    {
        int32_t a = 23;
        int32_t b = 42;
        eprosima::xtypes::DynamicData msg_to_sent(*mock_types->at("Method1_In"));
        msg_to_sent["a"] = a;
        msg_to_sent["b"] = b;
        logger << utils::Logger::Level::INFO
               << "Sent message: " << a << " + " << b << std::endl;

        eprosima::xtypes::DynamicData msg_to_recv(*mock_types->at("Method1_Result"));
        ASSERT_NO_THROW(roundtrip_server("TestService_1", msg_to_sent, msg_to_recv));
        ASSERT_EQ(msg_to_recv["result"].value<int32_t>(), (a + b));
    }

    // method2
    {
        float data = 23.42;
        eprosima::xtypes::DynamicData msg_to_sent(*mock_types->at("Method2_In"));
        msg_to_sent["data"] = data;

        logger << utils::Logger::Level::INFO
               << "Sent message: " << data << std::endl;

        eprosima::xtypes::DynamicData msg_to_recv(*mock_types->at("Method2_Result"));
        ASSERT_NO_THROW(roundtrip_server("TestService_2", msg_to_sent, msg_to_recv));
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

    const is::TypeRegistry* mock_types = instance.type_registry("mock");
    ASSERT_NE(mock_types, nullptr);

    std::mutex disc_mutex;
    disc_mutex.lock();

    std::shared_ptr<FastDDSServicesTest> client = nullptr;
    ASSERT_NO_THROW(client.reset(new FastDDSServicesTest(false, disc_mutex)));

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

        eprosima::xtypes::DynamicData msg_to_sent(*mock_types->at("Method0_In"));
        msg_to_sent["data"].value(message_data_fail);
        disc_mutex.lock();

        logger << utils::Logger::Level::INFO
               << "Sent message: " << message_data_fail << std::endl;

        eprosima::xtypes::DynamicData msg_to_recv(*mock_types->at("Method0_Result"));
        ASSERT_NO_THROW(roundtrip_client(msg_to_sent, client, msg_to_recv));
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
        eprosima::xtypes::DynamicData msg_to_sent(*mock_types->at("Method1_In"));
        msg_to_sent["a"] = a;
        msg_to_sent["b"] = b;

        logger << utils::Logger::Level::INFO
               << "Sent message: " << a << " + " << b << std::endl;

        eprosima::xtypes::DynamicData msg_to_recv(*mock_types->at("Method1_Result"));
        ASSERT_NO_THROW(roundtrip_client(msg_to_sent, client, msg_to_recv));

        ASSERT_EQ(msg_to_recv["result"].value<int32_t>(), (a + b));
    }

    // method2
    {
        float data = 23.42;
        eprosima::xtypes::DynamicData msg_to_sent(*mock_types->at("Method2_In"));
        msg_to_sent["data"] = data;

        logger << utils::Logger::Level::INFO
               << "Sent message: " << data << std::endl;

        eprosima::xtypes::DynamicData msg_to_recv(*mock_types->at("Method2_Result"));
        ASSERT_NO_THROW(roundtrip_client(msg_to_sent, client, msg_to_recv));

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
