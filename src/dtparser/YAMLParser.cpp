// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "YAMLParser.hpp"

namespace soss {
namespace dds {
namespace dtparser {

dtparser::DynamicTypeParser YAMLParser::dtparse_;

YAMLParser::YAMLParser(const std::string& config_file)
{
    YAML::Node config = YAML::LoadFile(config_file);
    parse_config(config["dynamic types"]);
}

YAMLParser::YAMLParser(const YAML::Node& config)
{
    parse_config(config);
}

YAMLP_ret YAMLParser::loadYAMLFile(const std::string& config_file)
{
    YAMLP_ret ret;

    YAML::Node config = YAML::LoadFile(config_file);

    ret = parseYAMLNode(config);

    return ret;
}

YAMLP_ret YAMLParser::parseYAMLNode(const YAML::Node& config)
{
    YAMLP_ret ret;

    if(!config.IsMap())
    {
        std::cerr << "YAML parser error: Input file must be a map!" << std::endl;
        ret = YAMLP_ret::YAMLP_ERROR;
    }
    else
    {
        ret = parse_config(config["dynamic types"]);
    }

    return ret;
}

YAMLP_ret YAMLParser::parse_config(const YAML::Node& node)
{
    YAMLP_ret ret = YAMLP_ret::YAMLP_OK;

    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
    {
        base_ptr dyntype = parse_node(it);
        if (dyntype == nullptr)
        {
            std::cerr << "YAMLParser: Error parsing node " << it->first.as<std::string>() << std::endl;
            ret = YAMLP_ret::YAMLP_ERROR;
            break;
        }

        dtparse_.add_type(dyntype);
    }

    if (ret == YAMLP_ret::YAMLP_OK)
    {
        ret = create_dynamic_types();
    }

    return ret;
}

base_ptr YAMLParser::parse_node(const YAML::const_iterator& it)
{
    std::string type_name = it->first.as<std::string>();
    std::istringstream iss(type_name);
    std::string type;
    std::string name;
    std::string empty_part;
    iss >> type;
    iss >> name;

    if (name == "")
    {
        if (!it->second.IsMap())
        {
            // If the second part is a value, this is a simple type or a property of a type.
            name = it->second.as<std::string>();
            if (name != "")
            {
                // return dyn type
                return new dtparser::Base(type, name);
            }
        }

        // The user forgot to put a name on the type.
        std::cerr << "The dynamic types are not defined properly in the YAML configuration file." << std::endl
                  << "There is a variable of type '" << type << "' that has no name specified." << std::endl;
        return nullptr;
    }

    iss >> empty_part;
    if (empty_part != "")
    {
        std::cerr << "The dynamic type '" << type_name << "' defined in the YAML configuration file is not "
        "properly named.\nIn its name, it should have at most two words, one for the type (if necessary) and other"
        " for the dynamic type name (e.g. struct my_struct)." << std::endl;
        return nullptr;
    }

    if (type.compare(dtparser::STRUCT) == 0)
    {
        if (!it->second.IsMap())
        {
            std::cerr << "The dynamic types are not defined properly in the YAML configuration file." << std::endl
                      << "There is a variable of type struct that has no members." << std::endl;
            return nullptr;
        }

        dtparser::Structure* structure = new dtparser::Structure(type, name);

        YAML::Node node = it->second;
        for (YAML::const_iterator sub_it = node.begin(); sub_it != node.end(); ++sub_it)
        {
            structure->add_member(parse_node(sub_it));
        }

        return structure;
    }
    else if (type.compare(dtparser::TYPEDEF) == 0)
    {
        if (it->second.IsMap())
        {
            // If the second is a map, then this node should have a type and may have a "dimensions" property.
            YAML::Node node = it->second;
            std::string first;
            std::string td_type = "";
            std::string td_dimensions = "";
            for (YAML::const_iterator sub_it = node.begin(); sub_it != node.end(); ++sub_it)
            {
                first = sub_it->first.as<std::string>();
                if (first == dtparser::TYPE && !sub_it->second.IsMap())
                {
                    td_type = sub_it->second.as<std::string>();
                }
                else if (first == dtparser::ARRAY_DIMENSIONS && !sub_it->second.IsMap())
                {
                    td_dimensions = sub_it->second.as<std::string>();
                }
                else
                {
                    std::cerr << "Property " << sub_it->second.as<std::string>() << " of " << type_name
                              << " not recognized." << std::endl;
                    return nullptr;
                }
            }

            if (td_type == "")
            {
                std::cerr << type_name << " is not properly formed. Property for the type not found." << std::endl;
                return nullptr;
            }
            else
            {
                return new dtparser::SimpleValue(type, name, td_type, td_dimensions);
            }

        }
        else
        {
            std::string td_type = it->second.as<std::string>();
            return new dtparser::SimpleValue(type, name, td_type);
        }
    }
    else if (type.compare(dtparser::ENUM) == 0)
    {
        dtparser::Structure* structure = new dtparser::Structure(type, name);
        YAML::Node node = it->second;
        dtparser::SimpleValue* literal_member;

        for (YAML::const_iterator sub_it = node.begin(); sub_it != node.end(); ++sub_it)
        {
            std::string m_type_name = sub_it->first.as<std::string>();
            std::istringstream m_iss(m_type_name);
            std::string m_type;
            std::string m_name;

            m_iss >> m_type;
            m_iss >> m_name;

            std::string m_value = sub_it->second.as<std::string>();

            literal_member = new dtparser::SimpleValue(m_type, m_name, m_value);
            structure->add_member(literal_member);
        }

        return structure;
    }
    else if (type.compare(dtparser::UNION) == 0)
    {
        std::cerr << "Type '" << type << "' still not suported." << std:: endl;
        return nullptr;
    }
    else
    {
        // At this point, it must be a simple type with properties.
        // That property may be in the second part as a map or as a simple value.
        if (it->second.IsMap())
        {
            // Add properties.
            YAML::Node node = it->second;
            YAML::const_iterator sub_it = node.begin();
            return new dtparser::Base(type, name, parse_node(sub_it));
        }
        else
        {
            std::string property_name = "unknownProperty";
            if (type.compare(dtparser::STRING) == 0 ||
                type.compare(dtparser::WSTRING) == 0)
            {
                property_name = dtparser::STR_MAXLENGTH;
            }

            return new dtparser::Base(type, name, new dtparser::Base(property_name,it->second.as<std::string>()));
        }
    }
}

YAMLP_ret YAMLParser::create_dynamic_types()
{
    if (dtparse_.create_dynamic_types() == dtparser::PARSER_ret::PARSER_OK)
    {
        return YAMLP_ret::YAMLP_OK;
    }
    else
    {
        return YAMLP_ret::YAMLP_ERROR;
    }
}

p_dynamictypebuilder_t YAMLParser::get_type_by_name(const std::string& t_name)
{
    return dtparse_.get_dt_by_name(t_name);
}

p_dynamictype_map_t YAMLParser::get_types_map()
{
    return dtparse_.get_types_map();
}

eprosima::fastrtps::types::DynamicPubSubType* YAMLParser::CreateDynamicPubSubType(const std::string& typeName)
{
    return dtparser::DynamicTypeParser::CreateDynamicPubSubType(typeName);
}

void YAMLParser::DeleteInstance()
{
    dtparser::DynamicTypeParser::DeleteInstance();
}

} // namespace dtparser
} // namespace dds
} // namespace soss
