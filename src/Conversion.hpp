/*
 * Copyright 2019 - present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _IS_SH_FASTDDS__INTERNAL__CONVERSION_HPP_
#define _IS_SH_FASTDDS__INTERNAL__CONVERSION_HPP_

#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicData.h>

#include "DDSMiddlewareException.hpp"

#include <is/core/Message.hpp>
#include <is/utils/Log.hpp>

#include <map>
#include <vector>

namespace fastdds = eprosima::fastdds;
namespace xtypes = eprosima::xtypes;
using namespace eprosima::fastrtps::types;

namespace eprosima {
namespace is {
namespace sh {
namespace fastdds {

/**
 * TODO(@jamoralp): Remove this workaround once get_union_id() is added to DynamicData class.
 * PR: https://github.com/eProsima/Fast-DDS/pull/1903
 */
class UnionDynamicData : public DynamicData
{
public:

    UnionDynamicData() = default;

    MemberId get_union_id() const
    {
        return union_id_;
    }

};

struct NavigationNode
{
    std::string member_name;
    std::string type_name;
    std::map<std::string, std::shared_ptr<NavigationNode> > member_node;
    std::weak_ptr<NavigationNode> parent_node;

    std::string get_path();

    static std::string get_type(
            std::map<std::string, std::shared_ptr<NavigationNode> > map_nodes,
            const std::string& full_path);

    static std::shared_ptr<NavigationNode> fill_root_node(
            std::shared_ptr<NavigationNode> root,
            const ::xtypes::DynamicType& type,
            const std::string& full_path);

    /**
     * @brief get_discriminator Introspects data until it finds an active member of type contained in member_types.
     *        This is useful for discriminate Union types in request/reply types.
     */
    static std::shared_ptr<NavigationNode> get_discriminator(
            const std::map<std::string, std::shared_ptr<NavigationNode> >& member_map,
            const ::xtypes::DynamicData& data,
            const std::vector<std::string>& member_types);

private:

    static std::string get_type(
            std::shared_ptr<NavigationNode> root,
            const std::string& full_path);

    static void fill_member_node(
            std::shared_ptr<NavigationNode> node,
            const ::xtypes::DynamicType& type,
            const std::string& full_path);

    static std::shared_ptr<NavigationNode> get_discriminator(
            std::shared_ptr<NavigationNode> node,
            ::xtypes::ReadableDynamicDataRef data,
            const std::vector<std::string>& member_types);

    static utils::Logger logger_;
};

struct Conversion
{
    static bool xtypes_to_fastdds(
            const ::xtypes::DynamicData& input,
            DynamicData* output);

    static bool fastdds_to_xtypes(
            const DynamicData* input,
            ::xtypes::DynamicData& output);

    static ::xtypes::DynamicData dynamic_data(
            const std::string& type_name);

    static void register_type(
            const std::string& type_name,
            DynamicPubSubType* type)
    {
        registered_types_.emplace(type_name, type);
    }

    static DynamicTypeBuilder* create_builder(
            const xtypes::DynamicType& type);

    // This function patches the problem of dynamic types, which do not admit '/' in their type name.
    static std::string convert_type_name(
            const std::string& message_type);

    static const xtypes::DynamicType& resolve_discriminator_type(
            const ::xtypes::DynamicType& service_type,
            const std::string& discriminator);

    static ::xtypes::WritableDynamicDataRef access_member_data(
            ::xtypes::WritableDynamicDataRef membered_data,
            const std::string& path);

private:

    ~Conversion() = default;
    static std::map<std::string, ::xtypes::DynamicType::Ptr> types_;
    static std::map<std::string, DynamicPubSubType*> registered_types_;
    static std::map<std::string, DynamicTypeBuilder_ptr> builders_;

    static const xtypes::DynamicType& resolve_type(
            const xtypes::DynamicType& type);

    static TypeKind resolve_type(
            const DynamicType_ptr type);

    static DynamicTypeBuilder_ptr get_builder(
            const xtypes::DynamicType& type);

    static void get_array_specs(
            const xtypes::ArrayType& array,
            std::pair<std::vector<uint32_t>, DynamicTypeBuilder_ptr>& result);

    // xtypes Dynamic Data -> FastDDS Dynamic Data
    static void set_primitive_data(
            xtypes::ReadableDynamicDataRef from,
            DynamicData* to,
            eprosima::fastrtps::types::MemberId id);

    // xtypes Dynamic Data -> FastDDS Dynamic Data
    static void set_sequence_data(
            xtypes::ReadableDynamicDataRef from,
            DynamicData* to);

    // xtypes Dynamic Data -> FastDDS Dynamic Data
    static void set_map_data(
            xtypes::ReadableDynamicDataRef from,
            DynamicData* to);

    // xtypes Dynamic Data -> FastDDS Dynamic Data
    static void set_array_data(
            xtypes::ReadableDynamicDataRef from,
            DynamicData* to,
            const std::vector<uint32_t>& indexes);

    // xtypes Dynamic Data -> FastDDS Dynamic Data
    static bool set_struct_data(
            xtypes::ReadableDynamicDataRef input,
            DynamicData* output);

    // xtypes Dynamic Data -> FastDDS Dynamic Data
    static bool set_union_data(
            xtypes::ReadableDynamicDataRef input,
            DynamicData* output);

    // FastDDS Dynamic Data -> xtypes Dynamic Data
    static void set_sequence_data(
            const DynamicData* from,
            xtypes::WritableDynamicDataRef to);

    // FastDDS Dynamic Data -> xtypes Dynamic Data
    static void set_map_data(
            const DynamicData* from,
            xtypes::WritableDynamicDataRef to);

    // FastDDS Dynamic Data -> xtypes Dynamic Data
    static void set_array_data(
            const DynamicData* from,
            xtypes::WritableDynamicDataRef to,
            const std::vector<uint32_t>& indexes);

    // FastDDS Dynamic Data -> xtypes Dynamic Data
    static bool set_struct_data(
            const DynamicData* input,
            ::xtypes::WritableDynamicDataRef output);

    // FastDDS Dynamic Data -> xtypes Dynamic Data
    static bool set_union_data(
            const DynamicData* input,
            ::xtypes::WritableDynamicDataRef output);

    static ::xtypes::WritableDynamicDataRef access_member_data(
            ::xtypes::WritableDynamicDataRef membered_data,
            const std::vector<std::string>& tokens,
            size_t index);

    static utils::Logger logger_;
};

} //  namespace fastdds
} //  namespace sh
} //  namespace is
} //  namespace eprosima

#endif //  _IS_SH_FASTDDS__INTERNAL__CONVERSION_HPP_

