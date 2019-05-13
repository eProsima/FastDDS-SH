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
#include "DynamicTypeParser.hpp"

namespace soss {
namespace dds {
namespace dtparser {

p_dynamictype_map_t DynamicTypeParser::data_types_;

static void dimensionsToArrayBounds(const std::string& dimensions, std::vector<uint32_t>& bounds)
{
    std::stringstream ss(dimensions);
    std::string item;

    bounds.clear();

    while (std::getline(ss, item, ','))
    {
        bounds.push_back((uint)std::stoi(item));
    }
}

static p_dynamictypebuilder_t getDiscriminatorTypeBuilder(const std::string &disc)
{
    DynamicTypeBuilderFactory* factory = DynamicTypeBuilderFactory::GetInstance();
    if (disc.compare(BOOLEAN) == 0)
    {
        return factory->CreateBoolBuilder();
    }
    else if (disc.compare(TBYTE) == 0)
    {
        return factory->CreateByteBuilder();
    }
    else if (disc.compare(SHORT) == 0)
    {
        return factory->CreateInt16Builder();
    }
    else if (disc.compare(LONG) == 0)
    {
        return factory->CreateInt32Builder();
    }
    else if (disc.compare(LONGLONG) == 0)
    {
        return factory->CreateInt64Builder();
    }
    else if (disc.compare(USHORT) == 0)
    {
        return factory->CreateUint16Builder();
    }
    else if (disc.compare(ULONG) == 0)
    {
        return factory->CreateUint32Builder();
    }
    else if (disc.compare(ULONGLONG) == 0)
    {
        return factory->CreateUint64Builder();
    }
    else if (disc.compare(FLOAT) == 0)
    {
        return factory->CreateFloat32Builder();
    }
    else if (disc.compare(DOUBLE) == 0)
    {
        return factory->CreateFloat64Builder();
    }
    else if (disc.compare(LONGDOUBLE) == 0)
    {
        return factory->CreateFloat128Builder();
    }
    else if (disc.compare(CHAR) == 0)
    {
        return factory->CreateChar8Builder();
    }
    else if (disc.compare(WCHAR) == 0)
    {
        return factory->CreateChar16Builder();
    }
    else if (disc.compare(STRING) == 0)
    {
        return factory->CreateStringBuilder();
    }
    else if (disc.compare(WSTRING) == 0)
    {
        return factory->CreateWstringBuilder();
    }
    return DynamicTypeParser::get_dt_by_name(disc);
}

PARSER_ret Base::to_dynamic(std::string& name, p_dynamictypebuilder_t& memberBuilder)
{
    PARSER_ret ret = PARSER_ret::PARSER_OK;
    name = this->name();

    bool isArray;
    if (properties() != nullptr &&
        properties()->type().compare(ARRAY_DIMENSIONS) == 0)
    {
        isArray = true;
    }
    else
    {
        isArray = false;
    }

    if (type().compare(TYPEDEF) == 0)
    {
        std::cerr << "Bad use of DynamicTypePrser: typedef should be in 'SimpleValue' class, not in 'Base' class."
                  << std::endl;
        return PARSER_ret::PARSER_ERROR;
    }
    else if (type().compare(STRING) == 0 ||
             type().compare(WSTRING) == 0)
    {
        DynamicTypeBuilderFactory* factory = DynamicTypeBuilderFactory::GetInstance();
        if (properties() == nullptr ||
            properties()->type().compare(STR_MAXLENGTH) != 0)
        {
            if (type().compare(STRING) == 0)
            {
                memberBuilder = factory->CreateStringBuilder();
            }
            else
            {
                memberBuilder = factory->CreateWstringBuilder();
            }
        }
        else
        {
            uint32_t bound = (uint32_t)std::stoi(this->properties()->name());

            if (type().compare(STRING) == 0)
            {
                memberBuilder = factory->CreateStringBuilder(bound);
            }
            else
            {
                memberBuilder = factory->CreateWstringBuilder(bound);
            }
        }
    }
    else if (type().compare(SEQUENCE) == 0)
    {
        // ToDo: Support sequences.
        std::cerr << "Type '" << type() << "' is still not supported." << std::endl;
        return PARSER_ret::PARSER_ERROR;
    }
    else if (type().compare(MAP) == 0)
    {
        // ToDo: Support maps.
        std::cerr << "Type '" << type() << "' is still not supported." << std::endl;
        return PARSER_ret::PARSER_ERROR;
    }

    if (memberBuilder == nullptr)
    {
        memberBuilder = getDiscriminatorTypeBuilder(type());

        if (memberBuilder == nullptr)
        {
            ret = PARSER_ret::PARSER_UNKNOWN_TYPE;
        }
    }
    else if (isArray)
    {
        DynamicTypeBuilderFactory* factory = DynamicTypeBuilderFactory::GetInstance();
        p_dynamictypebuilder_t innerBuilder = std::move(memberBuilder);
        std::vector<uint32_t> bounds;
        dimensionsToArrayBounds(properties()->name(), bounds);
        memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
    }

    return ret;
}

void Structure::add_member(Base *new_member)
{
    members_.push_back(new_member);
}

PARSER_ret Structure::to_dynamic(std::string& name, p_dynamictypebuilder_t& typeBuilder)
{
    name = this->name();

    if (this->type().compare(STRUCT) == 0)
    {
        typeBuilder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();
    }
    else if (this->type().compare(ENUM) == 0)
    {
        typeBuilder = DynamicTypeBuilderFactory::GetInstance()->CreateEnumBuilder();
    }
    else
    {
        std::cerr << "Intermediate structure is not properly set" << std::endl;
        return PARSER_ret::PARSER_ERROR;
    }

    typeBuilder->SetName(this->name());
    uint32_t mId = 0;

    if (this->type().compare(STRUCT) == 0)
    {
        for (auto const& member: members_)
        {
            std::string member_name;
            p_dynamictypebuilder_t mType = nullptr;
            if (member->to_dynamic(member_name, mType) == PARSER_ret::PARSER_UNKNOWN_TYPE)
            {
                //std::cerr << "Error parsing member " << member_name << " of the structure " << name << std::endl;
                return PARSER_ret::PARSER_UNKNOWN_TYPE;
            }
            else
            {
                typeBuilder->AddMember(mId++, member_name, mType);
                // ToDo: Add labels.
            }
        }
    }
    else
    {
        // For enumerations, there is no need to parse the members separatedly, as they can only be literals.
        // ToDo: Enum bitbound to set the internal field
        for (auto const& member: members_)
        {
            if (member->type().compare(LITERAL) != 0)
            {
                std::cerr << "The intermediate class for the enum '" << this->name() << "' is not properly set, "
                << "all its members should be literals." << std::endl;
                return PARSER_ret::PARSER_ERROR;
            }
            std::string member_name = member->name();
            if (member_name == "")
            {
                std::cerr << "Error parsing enum type: Literals must have name." << std::endl;
                return PARSER_ret::PARSER_ERROR;
            }
            std::string value = dynamic_cast<SimpleValue*>(member)->value();
            if (value != "")
            {
                mId = (uint32_t)std::stoi(value);
            }
            typeBuilder->AddEmptyMember(mId++, member_name);
        }
    }

    return PARSER_ret::PARSER_OK;
}

PARSER_ret SimpleValue::to_dynamic(std::string& name, p_dynamictypebuilder_t& typeBuilder)
{
    PARSER_ret ret = PARSER_ret::PARSER_OK;

    name = this->name();

    if (this->type().compare(TYPEDEF) == 0)
    {
        p_dynamictypebuilder_t valueBuilder = nullptr;

        if (dimensions_ == "")
        {
            valueBuilder = getDiscriminatorTypeBuilder(value_);
            if(valueBuilder == nullptr)
            {
                ret = PARSER_ret::PARSER_UNKNOWN_TYPE;
            }
        }
        else
        {
            base_ptr array_instance = new Base(value_, "", new Base("dimensions", dimensions_));
            std::string unnecesary_string;
            ret = array_instance->to_dynamic(unnecesary_string, valueBuilder);
        }

        if (ret == PARSER_ret::PARSER_OK)
        {
            typeBuilder = DynamicTypeBuilderFactory::GetInstance()->CreateAliasBuilder(valueBuilder, name);
            if (typeBuilder == nullptr)
            {
                ret = PARSER_ret::PARSER_ERROR;
            }
        }
    }
    else
    {
        std::cerr << "Error parsing dynamic types: trying to parse a simple value with name '" << name
                  << "which isn't a typedef." << std::endl;
        ret = PARSER_ret::PARSER_ERROR;
    }

    return ret;
}

void DynamicTypeParser::add_type(base_ptr type)
{
    intermediate_types_.push_back(type);
}

PARSER_ret DynamicTypeParser::create_dynamic_types()
{
    std::string name;
    p_dynamictypebuilder_t dynamic_type_builder = nullptr;
    PARSER_ret ret = PARSER_ret::PARSER_OK;

    for (auto const& type : intermediate_types_)
    {
        ret = type->to_dynamic(name, dynamic_type_builder);
        if (ret != PARSER_ret::PARSER_OK)
        {
            if(ret == PARSER_ret::PARSER_ERROR)
            {
                std::cerr << "Error parsing type " << name << std::endl;
            }
            else if (ret == PARSER_ret::PARSER_UNKNOWN_TYPE)
            {
                unknown_types_.push_back(type);
            }
        }
        else
        {
            data_types_[name] = dynamic_type_builder;
        }
    }

    if (unknown_types_.size() != 0)
    {
        // Check again for if the uknown part of the type has been loaded.
        bool new_type_created;
        do
        {
            new_type_created = false;
            for (auto it = unknown_types_.begin(); it != unknown_types_.end(); )
            {
                ret = (*it)->to_dynamic(name, dynamic_type_builder);
                if (ret == PARSER_ret::PARSER_OK)
                {
                    data_types_[name] = dynamic_type_builder;
                    new_type_created = true;
                    unknown_types_.erase(it);

                    if (unknown_types_.size() == 0)
                    {
                        new_type_created = false;
                        break;
                    }
                }
                else
                {
                    it++;
                }
            }
        }while (new_type_created);
    }

    // Check if there is still a type not parsed and cerr it.
    if (unknown_types_.size() != 0)
    {
        ret = PARSER_ret::PARSER_ERROR;
        for (auto it = unknown_types_.begin(); it != unknown_types_.end(); ++it)
        {
            std::cerr << "Error parsing dynamic type '" << (*it)->name()
                      << "'. It contains at least one standard type name that can not be found." << std::endl;
        }
    }

    return ret;
}

p_dynamictypebuilder_t DynamicTypeParser::get_dt_by_name(const std::string& sName)
{
    if (data_types_.find(sName) != data_types_.end())
    {
        return data_types_[sName];
    }
    return nullptr;
}

p_dynamictype_map_t DynamicTypeParser::get_types_map()
{
    return data_types_;
}

eprosima::fastrtps::types::DynamicPubSubType* DynamicTypeParser::CreateDynamicPubSubType(const std::string& typeName)
{
    p_dynamictype_map_t m_dynamictypes = dtparser::DynamicTypeParser::get_types_map();
    if (m_dynamictypes.find(typeName) != m_dynamictypes.end())
    {
        return new eprosima::fastrtps::types::DynamicPubSubType(m_dynamictypes[typeName]->Build());
    }
    return nullptr;
}

void DynamicTypeParser::DeleteInstance()
{
    for (auto pair : data_types_)
    {
        DynamicTypeBuilderFactory::GetInstance()->DeleteBuilder(pair.second);
    }
    data_types_.clear();
}

} /* namespace dtparser */
} /* namespace dds */
} /* namespace soss */
