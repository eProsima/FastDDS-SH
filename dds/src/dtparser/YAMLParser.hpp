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

#ifndef SOSS__DDS__DTPARSER__INTERNAL__YAMLPARSER_HPP
#define SOSS__DDS__DTPARSER__INTERNAL__YAMLPARSER_HPP

#include "DynamicTypeParserCommon.hpp"
#include "DynamicTypeParser.hpp"

#include <fastrtps/types/DynamicPubSubType.h>

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <string>
#include <sstream>

namespace soss {
namespace dds {
namespace dtparser {

/**
 * Enum class YAMLP_ret, used to provide a strongly typed result from the operations within this module.
 * @ingroup YAMLPARSER_MODULE
 */
enum class YAMLP_ret
{
    YAMLP_ERROR,
    YAMLP_OK,
    YAMLP_NOK
};

class YAMLParser
{
    static base_ptr parse_node(const YAML::const_iterator& it);
    static YAMLP_ret parse_config(const YAML::Node& node);
    static YAMLP_ret create_dynamic_types();
public:
    YAMLParser(const std::string& config_file);
    YAMLParser(const YAML::Node& config_node);
    static p_dynamictypebuilder_t get_type_by_name(const std::string&);
    static p_dynamictype_map_t get_types_map();
    static YAMLP_ret loadYAMLFile(const std::string&);
    static YAMLP_ret parseYAMLNode(const YAML::Node&);
    static eprosima::fastrtps::types::DynamicPubSubType* CreateDynamicPubSubType(const std::string&);
    static void DeleteInstance();
private:
    static dtparser::DynamicTypeParser dtparse_;
};

} // namespace dtparser
} // namespace dds
} // namespace soss

#endif //SOSS__DDS__DTPARSER__INTERNAL__YAMLPARSER_HPP
