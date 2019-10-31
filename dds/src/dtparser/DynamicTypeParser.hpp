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

#ifndef SOSS__DDS__DTPARSER__INTERNAL__DYNAMICTYPEPARSER_HPP
#define SOSS__DDS__DTPARSER__INTERNAL__DYNAMICTYPEPARSER_HPP

#define base_ptr dtparser::Base*

#include "DynamicTypeParserCommon.hpp"
#include "../DynamicTypeAdapter.hpp"

#include <fastrtps/TopicDataType.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <map>

namespace soss {
namespace dds {

typedef DynamicTypeBuilder*          p_dynamictypebuilder_t;
typedef std::map<std::string, p_dynamictypebuilder_t> p_dynamictype_map_t;

namespace dtparser{
typedef std::function<void(std::string, p_dynamictypebuilder_t)> RegisterCallback;

/**
 * Enum class PARSER_ret, used to provide a strongly typed result from the operations within this module.
 * @ingroup PARSER_MODULE
 */
enum class PARSER_ret
{
    PARSER_ERROR,   // Error no related to type names.
    PARSER_OK,      // There is no error.
    PARSER_UNKNOWN_TYPE   // A type name has not been found.
};

// Base class with just a type and a name.
class Base
{
public:
    Base(
            std::string type,
            std::string name,
            base_ptr properties = nullptr)
        : type_(type)
        , name_(name)
        , properties_(properties)
    {
    }

    virtual ~Base(){}
    virtual PARSER_ret to_dynamic(std::string&, p_dynamictypebuilder_t&);

    std::string type()
    {
        return type_;
    }

    std::string name()
    {
        return name_;
    }

    base_ptr properties()
    {
        return properties_;
    }

private:
    std::string type_;
    std::string name_;
    base_ptr properties_;
};

// Class for structures and enumerations, to add members to the base class.
class Structure : public Base
{
public:
    Structure(
            std::string type,
            std::string name)
        : Base(type, name)
    {
    }

    void add_member(base_ptr new_member);
    PARSER_ret to_dynamic(std::string&, p_dynamictypebuilder_t &) override;
private:
    std::vector<base_ptr> members_;
};

// Class for types that need values, such as literals in enumerations.
class SimpleValue : public Base
{
public:
    SimpleValue(
            std::string type,
            std::string name,
            std::string value,
            std::string dimensions = "")
        : Base(type, name)
        , value_(value)
        , dimensions_(dimensions)
    {
    }

    std::string value()
    {
        return value_;
    }
    PARSER_ret to_dynamic(std::string&, p_dynamictypebuilder_t&) override;
private:
    std::string value_;
    std::string dimensions_;
};

// Class to manage the dynamic types.
class DynamicTypeParser
{
public:
    DynamicTypeParser(){}
    void add_type(base_ptr);
    PARSER_ret create_dynamic_types();
    p_dynamictypebuilder_t get_discriminator_type_builder(const std::string&);
    static p_dynamictypebuilder_t get_dt_by_name(const std::string& sName);
    static p_dynamictype_map_t get_types_map();
    static eprosima::fastrtps::types::DynamicPubSubType* CreateDynamicPubSubType(const std::string& typeName);
    static void set_callback(RegisterCallback);
    static void DeleteInstance();
private:
    std::vector<base_ptr> intermediate_types_;
    std::vector<base_ptr> unknown_types_;
    static p_dynamictype_map_t data_types_;
    static std::vector<RegisterCallback> participant_callbacks_;
};

} // namespace dtparser
} // namespace dds
} // namespace soss

#endif //SOSS__DDS__DTPARSER__INTERNAL__DYNAMICTYPEPARSER_HPP
