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

#include "Conversion.hpp"
#include <fastrtps/types/TypeDescriptor.h>
#include <sstream>
#include <stack>

using eprosima::fastrtps::types::MemberId;

namespace soss {
namespace dds {

using namespace eprosima::fastrtps;

std::map<std::string, ::xtypes::DynamicType::Ptr> Conversion::types_;
std::map<std::string, DynamicPubSubType*> Conversion::registered_types_;
std::map<std::string, DynamicTypeBuilder_ptr> Conversion::builders_;

// This function patches the problem of dynamic types, which do not admit '/' in their type name.
std::string Conversion::convert_type_name(
        const std::string& message_type)
{
    std::string type = message_type;

    //TODO(Ricardo) Remove when old dyntypes support namespaces.
    size_t npos = type.rfind(":");
    if (std::string::npos != npos)
    {
        type = type.substr(npos + 1);
    }

    for (size_t i = type.find('/'); i != std::string::npos; i = type.find('/', i))
    {
        type.replace(i, 1, "__");
    }

    return type;
}

const xtypes::DynamicType& Conversion::resolve_type(
        const xtypes::DynamicType& type)
{
    if (type.kind() == ::xtypes::TypeKind::ALIAS_TYPE)
    {
        return static_cast<const ::xtypes::AliasType&>(type).rget();
    }
    return type;
}

void Conversion::set_primitive_data(
        ::xtypes::ReadableDynamicDataRef from,
        DynamicData* to,
        MemberId id)
{
    switch (resolve_type(from.type()).kind())
    {
        case ::xtypes::TypeKind::BOOLEAN_TYPE:
            to->set_bool_value(from.value<bool>(), id);
            break;
        case ::xtypes::TypeKind::CHAR_8_TYPE:
            to->set_char8_value(from.value<char>(), id);
            break;
        case ::xtypes::TypeKind::CHAR_16_TYPE:
            to->set_char16_value(from.value<wchar_t>(), id);
            break;
        case ::xtypes::TypeKind::UINT_8_TYPE:
            to->set_uint8_value(from.value<uint8_t>(), id);
            break;
        case ::xtypes::TypeKind::INT_8_TYPE:
            to->set_int8_value(from.value<int8_t>(), id);
            break;
        case ::xtypes::TypeKind::INT_16_TYPE:
            to->set_int16_value(from.value<int16_t>(), id);
            break;
        case ::xtypes::TypeKind::UINT_16_TYPE:
            to->set_uint16_value(from.value<uint16_t>(), id);
            break;
        case ::xtypes::TypeKind::INT_32_TYPE:
            to->set_int32_value(from.value<int32_t>(), id);
            break;
        case ::xtypes::TypeKind::UINT_32_TYPE:
            to->set_uint32_value(from.value<uint32_t>(), id);
            break;
        case ::xtypes::TypeKind::INT_64_TYPE:
            to->set_int64_value(from.value<int64_t>(), id);
            break;
        case ::xtypes::TypeKind::UINT_64_TYPE:
            to->set_uint64_value(from.value<uint64_t>(), id);
            break;
        case ::xtypes::TypeKind::FLOAT_32_TYPE:
            to->set_float32_value(from.value<float>(), id);
            break;
        case ::xtypes::TypeKind::FLOAT_64_TYPE:
            to->set_float64_value(from.value<double>(), id);
            break;
        case ::xtypes::TypeKind::FLOAT_128_TYPE:
            to->set_float128_value(from.value<long double>(), id);
            break;
        case ::xtypes::TypeKind::STRING_TYPE:
            to->set_string_value(from.value<std::string>(), id);
            break;
        case ::xtypes::TypeKind::WSTRING_TYPE:
            to->set_wstring_value(from.value<std::wstring>(), id);
            break;
        case ::xtypes::TypeKind::ENUMERATION_TYPE:
            to->set_enum_value(from.value<uint32_t>(), id);
            break;
        default:
        {
            std::stringstream ss;
            ss << "Expected primitive data, but found " << from.type().name();
            throw DDSMiddlewareException(ss.str());
        }
    }
}

void Conversion::set_array_data(
        ::xtypes::ReadableDynamicDataRef from,
        DynamicData* to,
        const std::vector<uint32_t>& indexes)
{
    const ::xtypes::ArrayType& type = static_cast<const ::xtypes::ArrayType&>(from.type());
    const ::xtypes::DynamicType& inner_type = resolve_type(type.content_type());
    DynamicDataFactory* factory = DynamicDataFactory::get_instance();
    MemberId id;

    for (uint32_t idx = 0; idx < from.size(); ++idx)
    {
        std::vector<uint32_t> new_indexes = indexes;
        new_indexes.push_back(idx);
        switch (inner_type.kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_bool_value(from[idx].value<bool>(), id);
                break;
            case ::xtypes::TypeKind::CHAR_8_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_char8_value(from[idx].value<char>(), id);
                break;
            case ::xtypes::TypeKind::CHAR_16_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_char16_value(static_cast<wchar_t>(from[idx].value<char32_t>()), id);
                break;
            case ::xtypes::TypeKind::UINT_8_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_uint8_value(from[idx].value<uint8_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_8_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_int8_value(from[idx].value<int8_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_16_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_int16_value(from[idx].value<int16_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_16_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_uint16_value(from[idx].value<uint16_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_32_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_int32_value(from[idx].value<int32_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_32_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_uint32_value(from[idx].value<uint32_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_64_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_int64_value(from[idx].value<int64_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_64_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_uint64_value(from[idx].value<uint64_t>(), id);
                break;
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_float32_value(from[idx].value<float>(), id);
                break;
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_float64_value(from[idx].value<double>(), id);
                break;
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_float128_value(from[idx].value<long double>(), id);
                break;
            case ::xtypes::TypeKind::STRING_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_string_value(from[idx].value<std::string>(), id);
                break;
            case ::xtypes::TypeKind::WSTRING_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_wstring_value(from[idx].value<std::wstring>(), id);
                break;
            case ::xtypes::TypeKind::ENUMERATION_TYPE:
                id = to->get_array_index(new_indexes);
                to->set_enum_value(from[idx].value<uint32_t>(), id);
                break;
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                set_array_data(from[idx], to, new_indexes);
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                id = to->get_array_index(new_indexes);
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner sequence builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* seq_data = factory->create_data(builder_ptr->build());
                set_sequence_data(from[idx], seq_data);
                to->set_complex_value(seq_data, id);
                break;
            }
            case ::xtypes::TypeKind::MAP_TYPE:
            {
                id = to->get_array_index(new_indexes);
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner map builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* seq_data = factory->create_data(builder_ptr->build());
                set_map_data(from[idx], seq_data);
                to->set_complex_value(seq_data, id);
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                id = to->get_array_index(new_indexes);
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner struct builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* st_data = factory->create_data(builder_ptr->build());
                set_struct_data(from[idx], st_data);
                to->set_complex_value(st_data, id);
                break;
            }
            case ::xtypes::TypeKind::UNION_TYPE:
            {
                id = to->get_array_index(new_indexes);
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner struct builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* st_data = factory->create_data(builder_ptr->build());
                set_union_data(from[idx], st_data);
                to->set_complex_value(st_data, id);
                break;
            }
            default:
            {
                std::stringstream ss;
                ss << "Unexpected data type: " << from.type().name() << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }
    }
}

void Conversion::set_sequence_data(
        ::xtypes::ReadableDynamicDataRef from,
        DynamicData* to)
{
    const ::xtypes::SequenceType& type = static_cast<const ::xtypes::SequenceType&>(from.type());
    MemberId id;
    DynamicDataFactory* factory = DynamicDataFactory::get_instance();
    to->clear_all_values();
    for (uint32_t idx = 0; idx < from.size(); ++idx)
    {
        to->insert_sequence_data(id);
        switch (resolve_type(type.content_type()).kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
                to->set_bool_value(from[idx].value<bool>(), id);
                break;
            case ::xtypes::TypeKind::CHAR_8_TYPE:
                to->set_char8_value(from[idx].value<char>(), id);
                break;
            case ::xtypes::TypeKind::CHAR_16_TYPE:
                to->set_char16_value(static_cast<wchar_t>(from[idx].value<char32_t>()), id);
                break;
            case ::xtypes::TypeKind::UINT_8_TYPE:
                //to->set_uint8_value(from[idx].value<uint8_t>(), id);
                to->set_byte_value(from[idx].value<uint8_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_8_TYPE:
                //to->set_int8_value(from[idx].value<int8_t>(), id);
                to->set_byte_value(static_cast<rtps::octet>(from[idx].value<int8_t>()), id);
                break;
            case ::xtypes::TypeKind::INT_16_TYPE:
                to->set_int16_value(from[idx].value<int16_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_16_TYPE:
                to->set_uint16_value(from[idx].value<uint16_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_32_TYPE:
                to->set_int32_value(from[idx].value<int32_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_32_TYPE:
                to->set_uint32_value(from[idx].value<uint32_t>(), id);
                break;
            case ::xtypes::TypeKind::INT_64_TYPE:
                to->set_int64_value(from[idx].value<int64_t>(), id);
                break;
            case ::xtypes::TypeKind::UINT_64_TYPE:
                to->set_uint64_value(from[idx].value<uint64_t>(), id);
                break;
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
                to->set_float32_value(from[idx].value<float>(), id);
                break;
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
                to->set_float64_value(from[idx].value<double>(), id);
                break;
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
                to->set_float128_value(from[idx].value<long double>(), id);
                break;
            case ::xtypes::TypeKind::STRING_TYPE:
                to->set_string_value(from[idx].value<std::string>(), id);
                break;
            case ::xtypes::TypeKind::WSTRING_TYPE:
                to->set_wstring_value(from[idx].value<std::wstring>(), id);
                break;
            case ::xtypes::TypeKind::ENUMERATION_TYPE:
                to->set_enum_value(from[idx].value<uint32_t>(), id);
                break;
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner array builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* array_data = factory->create_data(builder_ptr->build());
                set_array_data(from[idx], array_data, std::vector<uint32_t>());
                to->set_complex_value(array_data, id);
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner sequence builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* seq_data = factory->create_data(builder_ptr->build());
                set_sequence_data(from[idx], seq_data);
                to->set_complex_value(seq_data, id);
                break;
            }
            case ::xtypes::TypeKind::MAP_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner map builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* seq_data = factory->create_data(builder_ptr->build());
                set_map_data(from[idx], seq_data);
                to->set_complex_value(seq_data, id);
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner struct builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* st_data = factory->create_data(builder_ptr->build());
                set_struct_data(from[idx], st_data);
                to->set_complex_value(st_data, id);
                break;
            }
            case ::xtypes::TypeKind::UNION_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(from[idx].type()); // The inner union builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* st_data = factory->create_data(builder_ptr->build());
                set_union_data(from[idx], st_data);
                to->set_complex_value(st_data, id);
                break;
            }
            default:
            {
                std::stringstream ss;
                ss << "Unexpected data type: " << from.type().name() << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }
    }
}

void Conversion::set_map_data(
        ::xtypes::ReadableDynamicDataRef from,
        DynamicData* to)
{
    const ::xtypes::MapType& type = static_cast<const ::xtypes::MapType&>(from.type());
    const ::xtypes::PairType& pair_type = static_cast<const ::xtypes::PairType&>(type.content_type());
    MemberId id_key;
    MemberId id_value;
    DynamicDataFactory* factory = DynamicDataFactory::get_instance();
    to->clear_all_values();

    DynamicTypeBuilder_ptr key_builder = get_builder(pair_type.first());
    DynamicTypeBuilder_ptr value_builder = get_builder(pair_type.second());

    for (::xtypes::ReadableDynamicDataRef pair : from)
    {
        // Convert key
        DynamicData* key_data = factory->create_data(key_builder->build());
        ::xtypes::ReadableDynamicDataRef key = pair[0];
        MemberId id = MEMBER_ID_INVALID;

        switch (resolve_type(pair_type.first()).kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
                key_data->set_bool_value(key, id);
                break;
            case ::xtypes::TypeKind::CHAR_8_TYPE:
                key_data->set_char8_value(key, id);
                break;
            case ::xtypes::TypeKind::CHAR_16_TYPE:
                key_data->set_char16_value(key, id);
                break;
            case ::xtypes::TypeKind::UINT_8_TYPE:
                //key_data->set_uint8_value(from[key].value<uint8_t>(), id);
                key_data->set_byte_value(key, id);
                break;
            case ::xtypes::TypeKind::INT_8_TYPE:
                //key_data->set_int8_value(from[key].value<int8_t>(), id);
                key_data->set_byte_value(key, id);
                break;
            case ::xtypes::TypeKind::INT_16_TYPE:
                key_data->set_int16_value(key, id);
                break;
            case ::xtypes::TypeKind::UINT_16_TYPE:
                key_data->set_uint16_value(key, id);
                break;
            case ::xtypes::TypeKind::INT_32_TYPE:
                key_data->set_int32_value(key, id);
                break;
            case ::xtypes::TypeKind::UINT_32_TYPE:
                key_data->set_uint32_value(key, id);
                break;
            case ::xtypes::TypeKind::INT_64_TYPE:
                key_data->set_int64_value(key, id);
                break;
            case ::xtypes::TypeKind::UINT_64_TYPE:
                key_data->set_uint64_value(key, id);
                break;
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
                key_data->set_float32_value(key, id);
                break;
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
                key_data->set_float64_value(key, id);
                break;
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
                key_data->set_float128_value(key, id);
                break;
            case ::xtypes::TypeKind::STRING_TYPE:
                key_data->set_string_value(key, id);
                break;
            case ::xtypes::TypeKind::WSTRING_TYPE:
                key_data->set_wstring_value(key, id);
                break;
            case ::xtypes::TypeKind::ENUMERATION_TYPE:
                key_data->set_enum_value(key.value<uint32_t>(), id);
                break;
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(key.type()); // The inner array builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* array_data = factory->create_data(builder_ptr->build());
                set_array_data(key, array_data, std::vector<uint32_t>());
                key_data->set_complex_value(array_data, id);
                break;
            }
            case ::xtypes::TypeKind::MAP_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(key.type()); // The inner map builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* seq_data = factory->create_data(builder_ptr->build());
                set_map_data(key, seq_data);
                key_data->set_complex_value(seq_data, id);
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(key.type()); // The inner sequence builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* seq_data = factory->create_data(builder_ptr->build());
                set_sequence_data(key, seq_data);
                key_data->set_complex_value(seq_data, id);
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(key.type()); // The inner struct builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* st_data = factory->create_data(builder_ptr->build());
                set_struct_data(key, st_data);
                key_data->set_complex_value(st_data, id);
                break;
            }
            case ::xtypes::TypeKind::UNION_TYPE:
            {
                DynamicTypeBuilder_ptr builder = get_builder(key.type()); // The inner struct builder
                DynamicTypeBuilder* builder_ptr = static_cast<DynamicTypeBuilder*>(builder.get());
                DynamicData* st_data = factory->create_data(builder_ptr->build());
                set_union_data(key, st_data);
                key_data->set_complex_value(st_data, id);
                break;
            }
            default:
            {
                std::stringstream ss;
                ss << "Unexpected data type: " << from.type().name() << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }

        // Convert data
        id = MEMBER_ID_INVALID;
        DynamicData* value_data = factory->create_data(value_builder->build());
        ::xtypes::ReadableDynamicDataRef value = pair[1];

        switch (resolve_type(pair_type.second()).kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
                value_data->set_bool_value(value, id);
                break;
            case ::xtypes::TypeKind::CHAR_8_TYPE:
                value_data->set_char8_value(value, id);
                break;
            case ::xtypes::TypeKind::CHAR_16_TYPE:
                value_data->set_char16_value(value, id);
                break;
            case ::xtypes::TypeKind::UINT_8_TYPE:
                //value_data->set_uint8_value(value, id);
                value_data->set_byte_value(value, id);
                break;
            case ::xtypes::TypeKind::INT_8_TYPE:
                //value_data->set_int8_value(value, id);
                value_data->set_byte_value(value, id);
                break;
            case ::xtypes::TypeKind::INT_16_TYPE:
                value_data->set_int16_value(value, id);
                break;
            case ::xtypes::TypeKind::UINT_16_TYPE:
                value_data->set_uint16_value(value, id);
                break;
            case ::xtypes::TypeKind::INT_32_TYPE:
                value_data->set_int32_value(value, id);
                break;
            case ::xtypes::TypeKind::UINT_32_TYPE:
                value_data->set_uint32_value(value, id);
                break;
            case ::xtypes::TypeKind::INT_64_TYPE:
                value_data->set_int64_value(value, id);
                break;
            case ::xtypes::TypeKind::UINT_64_TYPE:
                value_data->set_uint64_value(value, id);
                break;
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
                value_data->set_float32_value(value, id);
                break;
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
                value_data->set_float64_value(value, id);
                break;
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
                value_data->set_float128_value(value, id);
                break;
            case ::xtypes::TypeKind::STRING_TYPE:
                value_data->set_string_value(value, id);
                break;
            case ::xtypes::TypeKind::WSTRING_TYPE:
                value_data->set_wstring_value(value, id);
                break;
            case ::xtypes::TypeKind::ENUMERATION_TYPE:
                value_data->set_enum_value(value.value<uint32_t>(), id);
                break;
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                set_array_data(value, value_data, std::vector<uint32_t>());
                break;
            }
            case ::xtypes::TypeKind::MAP_TYPE:
            {
                set_map_data(value, value_data);
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                set_sequence_data(value, value_data);
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                set_struct_data(value, value_data);
                break;
            }
            case ::xtypes::TypeKind::UNION_TYPE:
            {
                set_union_data(value, value_data);
                break;
            }
            default:
            {
                std::stringstream ss;
                ss << "Unexpected data type: " << from.type().name() << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }

        // Add key value
        to->insert_map_data(key_data, value_data, id_key, id_value);
    }
}

bool Conversion::soss_to_dds(
        const ::xtypes::DynamicData& input,
        DynamicData* output)
{
    return set_struct_data(input, output);
}

bool Conversion::set_struct_data(
        ::xtypes::ReadableDynamicDataRef input,
        DynamicData* output)
{
    std::stringstream ss;

    const ::xtypes::StructType& type = static_cast<const ::xtypes::StructType&>(input.type());

    for (const ::xtypes::Member& member : type.members())
    {
        MemberId id = output->get_member_id_by_name(member.name());
        switch (resolve_type(member.type()).kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
            case ::xtypes::TypeKind::CHAR_8_TYPE:
            case ::xtypes::TypeKind::CHAR_16_TYPE:
            case ::xtypes::TypeKind::UINT_8_TYPE:
            case ::xtypes::TypeKind::INT_8_TYPE:
            case ::xtypes::TypeKind::INT_16_TYPE:
            case ::xtypes::TypeKind::UINT_16_TYPE:
            case ::xtypes::TypeKind::INT_32_TYPE:
            case ::xtypes::TypeKind::UINT_32_TYPE:
            case ::xtypes::TypeKind::INT_64_TYPE:
            case ::xtypes::TypeKind::UINT_64_TYPE:
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
            case ::xtypes::TypeKind::STRING_TYPE:
            case ::xtypes::TypeKind::WSTRING_TYPE:
            case ::xtypes::TypeKind::ENUMERATION_TYPE:
            {
                set_primitive_data(input[member.name()], output, id);
                break;
            }
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                DynamicData* array_data = output->loan_value(id);
                set_array_data(input[member.name()], array_data, std::vector<uint32_t>());
                output->return_loaned_value(array_data);
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                DynamicData* seq_data = output->loan_value(id);
                set_sequence_data(input[member.name()], seq_data);
                output->return_loaned_value(seq_data);
                break;
            }
            case ::xtypes::TypeKind::MAP_TYPE:
            {
                DynamicData* seq_data = output->loan_value(id);
                set_map_data(input[member.name()], seq_data);
                output->return_loaned_value(seq_data);
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                DynamicData* st_data = output->loan_value(id);
                set_struct_data(input[member.name()], st_data);
                output->return_loaned_value(st_data);
                break;
            }
            case ::xtypes::TypeKind::UNION_TYPE:
            {
                DynamicData* st_data = output->loan_value(id);
                set_union_data(input[member.name()], st_data);
                output->return_loaned_value(st_data);
                break;
            }
            default:
                ss << "Unsupported type: " << member.type().name();
                throw DDSMiddlewareException(ss.str());

        }
    }
    return true;
}

bool Conversion::set_union_data(
        ::xtypes::ReadableDynamicDataRef input,
        DynamicData* output)
{
    std::stringstream ss;

    // Discriminator
    switch (resolve_type(input.d().type()).kind())
    {
        case ::xtypes::TypeKind::BOOLEAN_TYPE:
            output->set_discriminator_value(input.d().value<bool>());
            break;
        case ::xtypes::TypeKind::CHAR_8_TYPE:
            output->set_discriminator_value(static_cast<uint64_t>(input.d().value<char>()));
            break;
        case ::xtypes::TypeKind::CHAR_16_TYPE:
            output->set_discriminator_value(static_cast<uint64_t>(input.d().value<char16_t>()));
            break;
        case ::xtypes::TypeKind::UINT_8_TYPE:
            output->set_discriminator_value(input.d().value<uint8_t>());
            break;
        case ::xtypes::TypeKind::INT_8_TYPE:
            output->set_discriminator_value(static_cast<uint64_t>(input.d().value<int8_t>()));
            break;
        case ::xtypes::TypeKind::INT_16_TYPE:
            output->set_discriminator_value(static_cast<uint64_t>(input.d().value<int16_t>()));
            break;
        case ::xtypes::TypeKind::UINT_16_TYPE:
            output->set_discriminator_value(input.d().value<uint16_t>());
            break;
        case ::xtypes::TypeKind::INT_32_TYPE:
            output->set_discriminator_value(static_cast<uint64_t>(input.d().value<int32_t>()));
            break;
        case ::xtypes::TypeKind::UINT_32_TYPE:
            output->set_discriminator_value(input.d().value<uint32_t>());
            break;
        case ::xtypes::TypeKind::INT_64_TYPE:
            output->set_discriminator_value(static_cast<uint64_t>(input.d().value<int64_t>()));
            break;
        case ::xtypes::TypeKind::UINT_64_TYPE:
            output->set_discriminator_value(input.d().value<uint64_t>());
            break;
        case ::xtypes::TypeKind::ENUMERATION_TYPE:
            output->set_discriminator_value(input.d().value<uint32_t>());
            break;
        default:
            break;
    }

    // Active member
    const ::xtypes::Member& member = input.current_case();
    MemberId id = output->get_member_id_by_name(member.name());
    switch (resolve_type(member.type()).kind())
    {
        case ::xtypes::TypeKind::BOOLEAN_TYPE:
        case ::xtypes::TypeKind::CHAR_8_TYPE:
        case ::xtypes::TypeKind::CHAR_16_TYPE:
        case ::xtypes::TypeKind::UINT_8_TYPE:
        case ::xtypes::TypeKind::INT_8_TYPE:
        case ::xtypes::TypeKind::INT_16_TYPE:
        case ::xtypes::TypeKind::UINT_16_TYPE:
        case ::xtypes::TypeKind::INT_32_TYPE:
        case ::xtypes::TypeKind::UINT_32_TYPE:
        case ::xtypes::TypeKind::INT_64_TYPE:
        case ::xtypes::TypeKind::UINT_64_TYPE:
        case ::xtypes::TypeKind::FLOAT_32_TYPE:
        case ::xtypes::TypeKind::FLOAT_64_TYPE:
        case ::xtypes::TypeKind::FLOAT_128_TYPE:
        case ::xtypes::TypeKind::STRING_TYPE:
        case ::xtypes::TypeKind::WSTRING_TYPE:
        case ::xtypes::TypeKind::ENUMERATION_TYPE:
        {
            set_primitive_data(input[member.name()], output, id);
            break;
        }
        case ::xtypes::TypeKind::ARRAY_TYPE:
        {
            DynamicData* array_data = output->loan_value(id);
            set_array_data(input[member.name()], array_data, std::vector<uint32_t>());
            output->return_loaned_value(array_data);
            break;
        }
        case ::xtypes::TypeKind::SEQUENCE_TYPE:
        {
            DynamicData* seq_data = output->loan_value(id);
            set_sequence_data(input[member.name()], seq_data);
            output->return_loaned_value(seq_data);
            break;
        }
        case ::xtypes::TypeKind::STRUCTURE_TYPE:
        {
            DynamicData* st_data = output->loan_value(id);
            set_struct_data(input[member.name()], st_data);
            output->return_loaned_value(st_data);
            break;
        }
        case ::xtypes::TypeKind::MAP_TYPE:
        {
            DynamicData* st_data = output->loan_value(id);
            set_map_data(input[member.name()], st_data);
            output->return_loaned_value(st_data);
            break;
        }
        case ::xtypes::TypeKind::UNION_TYPE:
        {
            DynamicData* st_data = output->loan_value(id);
            set_union_data(input[member.name()], st_data);
            output->return_loaned_value(st_data);
            break;
        }
        default:
            ss << "Unsupported type: " << member.type().name();
            throw DDSMiddlewareException(ss.str());

    }
    return true;
}

void Conversion::set_sequence_data(
        const DynamicData* c_from,
        ::xtypes::WritableDynamicDataRef to)
{
    const ::xtypes::SequenceType& type = static_cast<const ::xtypes::SequenceType&>(to.type());
    DynamicData* from = const_cast<DynamicData*>(c_from);

    for (uint32_t idx = 0; idx < c_from->get_item_count(); ++idx)
    {
        MemberId id = idx;
        ResponseCode ret = ResponseCode::RETCODE_ERROR;

        switch (resolve_type(type.content_type()).kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
            {
                bool value;
                ret = from->get_bool_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::CHAR_8_TYPE:
            {
                char value;
                ret = from->get_char8_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::CHAR_16_TYPE:
            {
                wchar_t value;
                ret = from->get_char16_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_8_TYPE:
            {
                uint8_t value;
                ret = from->get_uint8_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::INT_8_TYPE:
            {
                int8_t value;
                ret = from->get_int8_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::INT_16_TYPE:
            {
                int16_t value;
                ret = from->get_int16_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_16_TYPE:
            {
                uint16_t value;
                ret = from->get_uint16_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::INT_32_TYPE:
            {
                int32_t value;
                ret = from->get_int32_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_32_TYPE:
            {
                uint32_t value;
                ret = from->get_uint32_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::INT_64_TYPE:
            {
                int64_t value;
                ret = from->get_int64_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_64_TYPE:
            {
                uint64_t value;
                ret = from->get_uint64_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
            {
                float value;
                ret = from->get_float32_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
            {
                double value;
                ret = from->get_float64_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
            {
                long double value;
                ret = from->get_float128_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::STRING_TYPE:
            {
                std::string value;
                ret = from->get_string_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::WSTRING_TYPE:
            {
                std::wstring value;
                ret = from->get_wstring_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::ENUMERATION_TYPE:
            {
                uint32_t value;
                ret = from->get_enum_value(value, id);
                to.push(value);
            }
            break;
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                DynamicData* array = from->loan_value(id);
                ::xtypes::DynamicData soss_array(type.content_type());
                set_array_data(array, soss_array.ref(), std::vector<uint32_t>());
                from->return_loaned_value(array);
                to.push(soss_array);
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                DynamicData* seq = from->loan_value(id);
                ::xtypes::DynamicData soss_seq(type.content_type());
                set_sequence_data(seq, soss_seq.ref());
                from->return_loaned_value(seq);
                to.push(soss_seq);
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::MAP_TYPE:
            {
                DynamicData* seq = from->loan_value(id);
                ::xtypes::DynamicData soss_seq(type.content_type());
                set_map_data(seq, soss_seq.ref());
                from->return_loaned_value(seq);
                to.push(soss_seq);
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                DynamicData* st = from->loan_value(id);
                ::xtypes::DynamicData soss_st(type.content_type());
                set_struct_data(st, soss_st.ref());
                from->return_loaned_value(st);
                to.push(soss_st);
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::UNION_TYPE:
            {
                DynamicData* st = from->loan_value(id);
                ::xtypes::DynamicData soss_st(type.content_type());
                set_union_data(st, soss_st.ref());
                from->return_loaned_value(st);
                to.push(soss_st);
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            default:
            {
                std::stringstream ss;
                ss << "Unexpected data type: " << to.type().name() << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }

        if (ret != ResponseCode::RETCODE_OK)
        {
            std::stringstream ss;
            ss << "Error parsing from dynamic type '" << to.type().name();
            throw DDSMiddlewareException(ss.str());
        }
    }
}

void Conversion::set_map_data(
        const DynamicData* c_from,
        ::xtypes::WritableDynamicDataRef to)
{
    const ::xtypes::MapType& map_type = static_cast<const ::xtypes::MapType&>(to.type());
    const ::xtypes::PairType& pair_type = static_cast<const ::xtypes::PairType&>(map_type.content_type());
    const ::xtypes::DynamicType& key_type = pair_type.first();
    const ::xtypes::DynamicType& value_type = pair_type.second();
    DynamicData* from = const_cast<DynamicData*>(c_from);

    for (uint32_t idx = 0; idx < c_from->get_item_count(); ++idx)
    {
        MemberId key_id = idx * 2;
        MemberId value_id = key_id + 1;
        ResponseCode ret = ResponseCode::RETCODE_ERROR;

        ::xtypes::DynamicData key_data(key_type);
        ::xtypes::DynamicData value_data(value_type);

        // Key
        switch (resolve_type(key_type).kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
            {
                bool value;
                ret = from->get_bool_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::CHAR_8_TYPE:
            {
                char value;
                ret = from->get_char8_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::CHAR_16_TYPE:
            {
                wchar_t value;
                ret = from->get_char16_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::UINT_8_TYPE:
            {
                uint8_t value;
                ret = from->get_uint8_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::INT_8_TYPE:
            {
                int8_t value;
                ret = from->get_int8_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::INT_16_TYPE:
            {
                int16_t value;
                ret = from->get_int16_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::UINT_16_TYPE:
            {
                uint16_t value;
                ret = from->get_uint16_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::INT_32_TYPE:
            {
                int32_t value;
                ret = from->get_int32_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::UINT_32_TYPE:
            {
                uint32_t value;
                ret = from->get_uint32_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::INT_64_TYPE:
            {
                int64_t value;
                ret = from->get_int64_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::UINT_64_TYPE:
            {
                uint64_t value;
                ret = from->get_uint64_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
            {
                float value;
                ret = from->get_float32_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
            {
                double value;
                ret = from->get_float64_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
            {
                long double value;
                ret = from->get_float128_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::STRING_TYPE:
            {
                std::string value;
                ret = from->get_string_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::WSTRING_TYPE:
            {
                std::wstring value;
                ret = from->get_wstring_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::ENUMERATION_TYPE:
            {
                uint32_t value;
                ret = from->get_enum_value(value, key_id);
                key_data = value;
            }
            break;
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                DynamicData* array = from->loan_value(key_id);
                ::xtypes::DynamicData soss_array(key_type);
                set_array_data(array, soss_array.ref(), std::vector<uint32_t>());
                from->return_loaned_value(array);
                key_data = soss_array;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                DynamicData* seq = from->loan_value(key_id);
                ::xtypes::DynamicData soss_seq(key_type);
                set_sequence_data(seq, soss_seq.ref());
                from->return_loaned_value(seq);
                key_data = soss_seq;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::MAP_TYPE:
            {
                DynamicData* seq = from->loan_value(key_id);
                ::xtypes::DynamicData soss_seq(key_type);
                set_map_data(seq, soss_seq.ref());
                from->return_loaned_value(seq);
                key_data = soss_seq;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                DynamicData* st = from->loan_value(key_id);
                ::xtypes::DynamicData soss_st(key_type);
                set_struct_data(st, soss_st.ref());
                from->return_loaned_value(st);
                key_data = soss_st;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::UNION_TYPE:
            {
                DynamicData* st = from->loan_value(key_id);
                ::xtypes::DynamicData soss_st(key_type);
                set_union_data(st, soss_st.ref());
                from->return_loaned_value(st);
                key_data = soss_st;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            default:
            {
                std::stringstream ss;
                ss << "Unexpected data type: " << key_type.name() << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }

        // Value
        switch (resolve_type(value_type).kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
            {
                bool value;
                ret = from->get_bool_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::CHAR_8_TYPE:
            {
                char value;
                ret = from->get_char8_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::CHAR_16_TYPE:
            {
                wchar_t value;
                ret = from->get_char16_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::UINT_8_TYPE:
            {
                uint8_t value;
                ret = from->get_uint8_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::INT_8_TYPE:
            {
                int8_t value;
                ret = from->get_int8_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::INT_16_TYPE:
            {
                int16_t value;
                ret = from->get_int16_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::UINT_16_TYPE:
            {
                uint16_t value;
                ret = from->get_uint16_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::INT_32_TYPE:
            {
                int32_t value;
                ret = from->get_int32_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::UINT_32_TYPE:
            {
                uint32_t value;
                ret = from->get_uint32_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::INT_64_TYPE:
            {
                int64_t value;
                ret = from->get_int64_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::UINT_64_TYPE:
            {
                uint64_t value;
                ret = from->get_uint64_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
            {
                float value;
                ret = from->get_float32_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
            {
                double value;
                ret = from->get_float64_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
            {
                long double value;
                ret = from->get_float128_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::STRING_TYPE:
            {
                std::string value;
                ret = from->get_string_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::WSTRING_TYPE:
            {
                std::wstring value;
                ret = from->get_wstring_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::ENUMERATION_TYPE:
            {
                uint32_t value;
                ret = from->get_enum_value(value, value_id);
                value_data = value;
            }
            break;
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                DynamicData* array = from->loan_value(value_id);
                ::xtypes::DynamicData soss_array(value_type);
                set_array_data(array, soss_array.ref(), std::vector<uint32_t>());
                from->return_loaned_value(array);
                value_data = soss_array;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                DynamicData* seq = from->loan_value(value_id);
                ::xtypes::DynamicData soss_seq(value_type);
                set_sequence_data(seq, soss_seq.ref());
                from->return_loaned_value(seq);
                value_data = soss_seq;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::MAP_TYPE:
            {
                DynamicData* seq = from->loan_value(value_id);
                ::xtypes::DynamicData soss_seq(value_type);
                set_map_data(seq, soss_seq.ref());
                from->return_loaned_value(seq);
                value_data = soss_seq;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                DynamicData* st = from->loan_value(value_id);
                ::xtypes::DynamicData soss_st(value_type);
                set_struct_data(st, soss_st.ref());
                from->return_loaned_value(st);
                value_data = soss_st;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::UNION_TYPE:
            {
                DynamicData* st = from->loan_value(value_id);
                ::xtypes::DynamicData soss_st(value_type);
                set_union_data(st, soss_st.ref());
                from->return_loaned_value(st);
                value_data = soss_st;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            default:
            {
                std::stringstream ss;
                ss << "Unexpected data type: " << value_type.name() << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }

        if (ret != ResponseCode::RETCODE_OK)
        {
            std::stringstream ss;
            ss << "Error parsing from dynamic type '" << to.type().name();
            throw DDSMiddlewareException(ss.str());
        }

        // Add entry
        to[key_data] = value_data;
    }
}

void Conversion::set_array_data(
        const DynamicData* c_from,
        ::xtypes::WritableDynamicDataRef to,
        const std::vector<uint32_t>& indexes)
{
    const ::xtypes::ArrayType& type = static_cast<const ::xtypes::ArrayType&>(to.type());
    const ::xtypes::DynamicType& inner_type = type.content_type();
    DynamicData* from = const_cast<DynamicData*>(c_from);
    MemberId id;

    for (uint32_t idx = 0; idx < type.dimension(); ++idx)
    {
        std::vector<uint32_t> new_indexes = indexes;
        new_indexes.push_back(idx);
        ResponseCode ret = ResponseCode::RETCODE_ERROR;
        switch (resolve_type(inner_type).kind())
        {
            case ::xtypes::TypeKind::BOOLEAN_TYPE:
            {
                id = from->get_array_index(new_indexes);
                bool value;
                ret = from->get_bool_value(value, id);
                to[idx].value<bool>(value);
            }
            break;
            case ::xtypes::TypeKind::CHAR_8_TYPE:
            {
                id = from->get_array_index(new_indexes);
                char value;
                ret = from->get_char8_value(value, id);
                to[idx].value<char>(value);
            }
            break;
            case ::xtypes::TypeKind::CHAR_16_TYPE:
            {
                id = from->get_array_index(new_indexes);
                wchar_t value;
                ret = from->get_char16_value(value, id);
                to[idx].value<wchar_t>(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_8_TYPE:
            {
                id = from->get_array_index(new_indexes);
                uint8_t value;
                ret = from->get_uint8_value(value, id);
                to[idx].value<uint8_t>(value);
            }
            break;
            case ::xtypes::TypeKind::INT_8_TYPE:
            {
                id = from->get_array_index(new_indexes);
                int8_t value;
                ret = from->get_int8_value(value, id);
                to[idx].value<int8_t>(value);
            }
            break;
            case ::xtypes::TypeKind::INT_16_TYPE:
            {
                id = from->get_array_index(new_indexes);
                int16_t value;
                ret = from->get_int16_value(value, id);
                to[idx].value<int16_t>(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_16_TYPE:
            {
                id = from->get_array_index(new_indexes);
                uint16_t value;
                ret = from->get_uint16_value(value, id);
                to[idx].value<uint16_t>(value);
            }
            break;
            case ::xtypes::TypeKind::INT_32_TYPE:
            {
                id = from->get_array_index(new_indexes);
                int32_t value;
                ret = from->get_int32_value(value, id);
                to[idx].value<int32_t>(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_32_TYPE:
            {
                id = from->get_array_index(new_indexes);
                uint32_t value;
                ret = from->get_uint32_value(value, id);
                to[idx].value<uint32_t>(value);
            }
            break;
            case ::xtypes::TypeKind::INT_64_TYPE:
            {
                id = from->get_array_index(new_indexes);
                int64_t value;
                ret = from->get_int64_value(value, id);
                to[idx].value<int64_t>(value);
            }
            break;
            case ::xtypes::TypeKind::UINT_64_TYPE:
            {
                id = from->get_array_index(new_indexes);
                uint64_t value;
                ret = from->get_uint64_value(value, id);
                to[idx].value<uint64_t>(value);
            }
            break;
            case ::xtypes::TypeKind::FLOAT_32_TYPE:
            {
                id = from->get_array_index(new_indexes);
                float value;
                ret = from->get_float32_value(value, id);
                to[idx].value<float>(value);
            }
            break;
            case ::xtypes::TypeKind::FLOAT_64_TYPE:
            {
                id = from->get_array_index(new_indexes);
                double value;
                ret = from->get_float64_value(value, id);
                to[idx].value<double>(value);
            }
            break;
            case ::xtypes::TypeKind::FLOAT_128_TYPE:
            {
                id = from->get_array_index(new_indexes);
                long double value;
                ret = from->get_float128_value(value, id);
                to[idx].value<long double>(value);
            }
            break;
            case ::xtypes::TypeKind::STRING_TYPE:
            {
                id = from->get_array_index(new_indexes);
                std::string value;
                ret = from->get_string_value(value, id);
                to[idx].value<std::string>(value);
            }
            break;
            case ::xtypes::TypeKind::WSTRING_TYPE:
            {
                id = from->get_array_index(new_indexes);
                std::wstring value;
                ret = from->get_wstring_value(value, id);
                to[idx].value<std::wstring>(value);
            }
            break;
            case ::xtypes::TypeKind::ENUMERATION_TYPE:
            {
                id = from->get_array_index(new_indexes);
                uint32_t value;
                ret = from->get_enum_value(value, id);
                to[idx].value<uint32_t>(value);
            }
            break;
            case ::xtypes::TypeKind::ARRAY_TYPE:
            {
                ::xtypes::DynamicData soss_array(type.content_type());
                set_array_data(from, soss_array.ref(), new_indexes);
                to[idx] = soss_array;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::SEQUENCE_TYPE:
            {
                id = from->get_array_index(new_indexes);
                DynamicData* seq = from->loan_value(id);
                ::xtypes::DynamicData soss_seq(type.content_type());
                set_sequence_data(seq, soss_seq.ref());
                from->return_loaned_value(seq);
                to[idx] = soss_seq;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::MAP_TYPE:
            {
                id = from->get_array_index(new_indexes);
                DynamicData* seq = from->loan_value(id);
                ::xtypes::DynamicData soss_seq(type.content_type());
                set_map_data(seq, soss_seq.ref());
                from->return_loaned_value(seq);
                to[idx] = soss_seq;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::STRUCTURE_TYPE:
            {
                id = from->get_array_index(new_indexes);
                DynamicData* st = from->loan_value(id);
                ::xtypes::DynamicData soss_st(type.content_type());
                set_struct_data(st, soss_st.ref());
                from->return_loaned_value(st);
                to[idx] = soss_st;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            case ::xtypes::TypeKind::UNION_TYPE:
            {
                id = from->get_array_index(new_indexes);
                DynamicData* st = from->loan_value(id);
                ::xtypes::DynamicData soss_st(type.content_type());
                set_union_data(st, soss_st.ref());
                from->return_loaned_value(st);
                to[idx] = soss_st;
                ret = ResponseCode::RETCODE_OK;
                break;
            }
            default:
            {
                std::stringstream ss;
                ss << "Unexpected data type: " << to.type().name() << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }

        if (ret != ResponseCode::RETCODE_OK)
        {
            std::stringstream ss;
            ss << "Error parsing from dynamic type '" << to.type().name();
            throw DDSMiddlewareException(ss.str());
        }
    }

}

// TODO: Can we receive a type without members as root?
bool Conversion::dds_to_soss(
        const DynamicData* c_input,
        ::xtypes::DynamicData& output)
{
    return set_struct_data(c_input, output.ref());
}

TypeKind Conversion::resolve_type(
        const DynamicType_ptr type)
{
    TypeDescriptor* desc_type = type->get_descriptor();
    while (desc_type->get_kind() == types::TK_ALIAS)
    {
        desc_type = desc_type->get_base_type()->get_descriptor();
    }
    return desc_type->get_kind();
}

bool Conversion::set_struct_data(
        const DynamicData* c_input,
        ::xtypes::WritableDynamicDataRef output)
{
    std::stringstream ss;
    uint32_t id = 0;
    uint32_t i = 0;
    MemberDescriptor descriptor;

    // We promise to not modify it, but we need it non-const, so we can call loan_value freely.
    DynamicData* input = const_cast<DynamicData*>(c_input);

    while (id != MEMBER_ID_INVALID)
    {
        id = input->get_member_id_at_index(i);
        ResponseCode ret = ResponseCode::RETCODE_ERROR;

        if (id != MEMBER_ID_INVALID)
        {
            DynamicDataSOSS* dd_soss = static_cast<DynamicDataSOSS*>(input);
            ret = dd_soss->GetDescriptorSOSS(descriptor, id);

            if (ret == ResponseCode::RETCODE_OK)
            {
                switch (resolve_type(descriptor.get_type()))
                {
                    case types::TK_BOOLEAN:
                    {
                        bool value;
                        ret = input->get_bool_value(value, id);
                        output[descriptor.get_name()].value<bool>(value);
                        break;
                    }
                    case types::TK_BYTE:
                    {
                        uint8_t value;
                        ret = input->get_byte_value(value, id);
                        output[descriptor.get_name()].value<uint8_t>(value);
                        break;
                    }
                    case types::TK_INT16:
                    {
                        int16_t value;
                        ret = input->get_int16_value(value, id);
                        output[descriptor.get_name()].value<int16_t>(value);
                        break;
                    }
                    case types::TK_INT32:
                    {
                        int32_t value;
                        ret = input->get_int32_value(value, id);
                        output[descriptor.get_name()].value<int32_t>(value);
                        break;
                    }
                    case types::TK_INT64:
                    {
                        int64_t value;
                        ret = input->get_int64_value(value, id);
                        output[descriptor.get_name()].value<int64_t>(value);
                        break;
                    }
                    case types::TK_UINT16:
                    {
                        uint16_t value;
                        ret = input->get_uint16_value(value, id);
                        output[descriptor.get_name()].value<uint16_t>(value);
                        break;
                    }
                    case types::TK_UINT32:
                    {
                        uint32_t value;
                        ret = input->get_uint32_value(value, id);
                        output[descriptor.get_name()].value<uint32_t>(value);
                        break;
                    }
                    case types::TK_UINT64:
                    {
                        uint64_t value;
                        ret = input->get_uint64_value(value, id);
                        output[descriptor.get_name()].value<uint64_t>(value);
                        break;
                    }
                    case types::TK_FLOAT32:
                    {
                        float value;
                        ret = input->get_float32_value(value, id);
                        output[descriptor.get_name()].value<float>(value);
                        break;
                    }
                    case types::TK_FLOAT64:
                    {
                        double value;
                        ret = input->get_float64_value(value, id);
                        output[descriptor.get_name()].value<double>(value);
                        break;
                    }
                    case types::TK_FLOAT128:
                    {
                        long double value;
                        ret = input->get_float128_value(value, id);
                        output[descriptor.get_name()].value<long double>(value);
                        break;
                    }
                    case types::TK_CHAR8:
                    {
                        char value;
                        ret = input->get_char8_value(value, id);
                        output[descriptor.get_name()].value<char>(value);
                        break;
                    }
                    case types::TK_CHAR16:
                    {
                        wchar_t value;
                        ret = input->get_char16_value(value, id);
                        output[descriptor.get_name()].value<wchar_t>(value);
                        break;
                    }
                    case types::TK_STRING8:
                    {
                        std::string value;
                        ret = input->get_string_value(value, id);
                        output[descriptor.get_name()].value<std::string>(value);
                        break;
                    }
                    case types::TK_STRING16:
                    {
                        std::wstring value;
                        ret = input->get_wstring_value(value, id);
                        output[descriptor.get_name()].value<std::wstring>(value);
                        break;
                    }
                    case types::TK_ENUM:
                    {
                        uint32_t value;
                        ret = input->get_enum_value(value, id);
                        output[descriptor.get_name()].value<uint32_t>(value);
                        break;
                    }
                    case types::TK_ARRAY:
                    {
                        DynamicData* array = input->loan_value(id);
                        set_array_data(array, output[descriptor.get_name()], std::vector<uint32_t>());
                        input->return_loaned_value(array);
                        break;
                    }
                    case types::TK_SEQUENCE:
                    {
                        DynamicData* seq = input->loan_value(id);
                        set_sequence_data(seq, output[descriptor.get_name()]);
                        input->return_loaned_value(seq);
                        break;
                    }
                    case types::TK_MAP:
                    {
                        DynamicData* seq = input->loan_value(id);
                        set_map_data(seq, output[descriptor.get_name()]);
                        input->return_loaned_value(seq);
                        break;
                    }
                    case types::TK_STRUCTURE:
                    {
                        DynamicData* nested_msg_dds = input->loan_value(id);
                        //::xtypes::DynamicData nested_msg_soss = output[descriptor.get_name()][id];

                        if (nested_msg_dds != nullptr)
                        {
                            //if (set_struct_data(nested_msg_dds, nested_msg_soss.ref()))
                            if (set_struct_data(nested_msg_dds, output[descriptor.get_name()]))
                            {
                                ret = ResponseCode::RETCODE_OK;
                            }
                            input->return_loaned_value(nested_msg_dds);
                        }
                        break;
                    }
                    case types::TK_UNION:
                    {
                        DynamicData* nested_msg_dds = input->loan_value(id);

                        if (nested_msg_dds != nullptr)
                        {
                            if (set_union_data(nested_msg_dds, output[descriptor.get_name()]))
                            {
                                ret = ResponseCode::RETCODE_OK;
                            }
                            input->return_loaned_value(nested_msg_dds);
                        }
                        break;
                    }
                    default:
                    {
                        ret = ResponseCode::RETCODE_ERROR;
                        break;
                    }
                }

                i++;
            }

            if (ret != ResponseCode::RETCODE_OK)
            {
                std::stringstream ss;
                ss << "Error parsing from dynamic type '" << input->get_name();
                //ss << "Error code: " << ret << ".";
                throw DDSMiddlewareException(ss.str());
            }
        }
    }

    return true;
}

bool Conversion::set_union_data(
        const DynamicData* c_input,
        ::xtypes::WritableDynamicDataRef output)
{
    std::stringstream ss;
    MemberDescriptor descriptor;

    // We promise to not modify it, but we need it non-const, so we can call loan_value freely.
    DynamicData* input = const_cast<DynamicData*>(c_input);

    // Discriminator is set automatically when the operator[] is used.

    // Active member
    DynamicDataSOSS* dd_soss = static_cast<DynamicDataSOSS*>(input);
    MemberId id = dd_soss->get_union_id();
    ResponseCode ret = ResponseCode::RETCODE_ERROR;

    ret = dd_soss->GetDescriptorSOSS(descriptor, id);

    if (ret == ResponseCode::RETCODE_OK)
    {
        switch (resolve_type(descriptor.get_type()))
        {
            case types::TK_BOOLEAN:
            {
                bool value;
                ret = input->get_bool_value(value, id);
                output[descriptor.get_name()].value<bool>(value);
                break;
            }
            case types::TK_BYTE:
            {
                uint8_t value;
                ret = input->get_byte_value(value, id);
                output[descriptor.get_name()].value<uint8_t>(value);
                break;
            }
            case types::TK_INT16:
            {
                int16_t value;
                ret = input->get_int16_value(value, id);
                output[descriptor.get_name()].value<int16_t>(value);
                break;
            }
            case types::TK_INT32:
            {
                int32_t value;
                ret = input->get_int32_value(value, id);
                output[descriptor.get_name()].value<int32_t>(value);
                break;
            }
            case types::TK_INT64:
            {
                int64_t value;
                ret = input->get_int64_value(value, id);
                output[descriptor.get_name()].value<int64_t>(value);
                break;
            }
            case types::TK_UINT16:
            {
                uint16_t value;
                ret = input->get_uint16_value(value, id);
                output[descriptor.get_name()].value<uint16_t>(value);
                break;
            }
            case types::TK_UINT32:
            {
                uint32_t value;
                ret = input->get_uint32_value(value, id);
                output[descriptor.get_name()].value<uint32_t>(value);
                break;
            }
            case types::TK_UINT64:
            {
                uint64_t value;
                ret = input->get_uint64_value(value, id);
                output[descriptor.get_name()].value<uint64_t>(value);
                break;
            }
            case types::TK_FLOAT32:
            {
                float value;
                ret = input->get_float32_value(value, id);
                output[descriptor.get_name()].value<float>(value);
                break;
            }
            case types::TK_FLOAT64:
            {
                double value;
                ret = input->get_float64_value(value, id);
                output[descriptor.get_name()].value<double>(value);
                break;
            }
            case types::TK_FLOAT128:
            {
                long double value;
                ret = input->get_float128_value(value, id);
                output[descriptor.get_name()].value<long double>(value);
                break;
            }
            case types::TK_CHAR8:
            {
                char value;
                ret = input->get_char8_value(value, id);
                output[descriptor.get_name()].value<char>(value);
                break;
            }
            case types::TK_CHAR16:
            {
                wchar_t value;
                ret = input->get_char16_value(value, id);
                output[descriptor.get_name()].value<wchar_t>(value);
                break;
            }
            case types::TK_STRING8:
            {
                std::string value;
                ret = input->get_string_value(value, id);
                output[descriptor.get_name()].value<std::string>(value);
                break;
            }
            case types::TK_STRING16:
            {
                std::wstring value;
                ret = input->get_wstring_value(value, id);
                output[descriptor.get_name()].value<std::wstring>(value);
                break;
            }
            case types::TK_ENUM:
            {
                uint32_t value;
                ret = input->get_enum_value(value, id);
                output[descriptor.get_name()].value<uint32_t>(value);
                break;
            }
            case types::TK_ARRAY:
            {
                DynamicData* array = input->loan_value(id);
                set_array_data(array, output[descriptor.get_name()], std::vector<uint32_t>());
                input->return_loaned_value(array);
                break;
            }
            case types::TK_SEQUENCE:
            {
                DynamicData* seq = input->loan_value(id);
                set_sequence_data(seq, output[descriptor.get_name()]);
                input->return_loaned_value(seq);
                break;
            }
            case types::TK_MAP:
            {
                DynamicData* seq = input->loan_value(id);
                set_map_data(seq, output[descriptor.get_name()]);
                input->return_loaned_value(seq);
                break;
            }
            case types::TK_STRUCTURE:
            {
                DynamicData* nested_msg_dds = input->loan_value(id);
                //::xtypes::DynamicData nested_msg_soss = output[descriptor.get_name()][id];

                if (nested_msg_dds != nullptr)
                {
                    //if (set_struct_data(nested_msg_dds, nested_msg_soss.ref()))
                    if (set_struct_data(nested_msg_dds, output[descriptor.get_name()]))
                    {
                        ret = ResponseCode::RETCODE_OK;
                    }
                    input->return_loaned_value(nested_msg_dds);
                }
                break;
            }
            case types::TK_UNION:
            {
                DynamicData* nested_msg_dds = input->loan_value(id);

                if (nested_msg_dds != nullptr)
                {
                    if (set_union_data(nested_msg_dds, output[descriptor.get_name()]))
                    {
                        ret = ResponseCode::RETCODE_OK;
                    }
                    input->return_loaned_value(nested_msg_dds);
                }
                break;
            }
            default:
            {
                ret = ResponseCode::RETCODE_ERROR;
                break;
            }
        }
    }

    if (ret != ResponseCode::RETCODE_OK)
    {
        std::stringstream ss;
        ss << "Error parsing from dynamic type '" << input->get_name();
        //ss << "Error code: " << ret << ".";
        throw DDSMiddlewareException(ss.str());
    }

    return true;
}

::xtypes::DynamicData Conversion::dynamic_data(
        const std::string& type_name)
{
    auto it = types_.find(type_name);

    if (it == types_.end())
    {
        std::stringstream ss;
        ss << "Error getting data from dynamic type '" << type_name << " (not registered).";
        throw DDSMiddlewareException(ss.str());
    }

    return ::xtypes::DynamicData(*it->second.get());
}

DynamicTypeBuilder* Conversion::create_builder(
        const ::xtypes::DynamicType& type)
{
    if (builders_.count(type.name()) > 0)
    {
        return static_cast<DynamicTypeBuilder*>(builders_[type.name()].get());
    }

    DynamicTypeBuilder_ptr builder = get_builder(type);
    if (builder == nullptr)
    {
        return nullptr;
    }
    DynamicTypeBuilder* result = static_cast<DynamicTypeBuilder*>(builder.get());
    result->set_name(convert_type_name(type.name()));
    builders_.emplace(type.name(), std::move(builder));
    return result;
}

DynamicTypeBuilder_ptr Conversion::get_builder(
        const ::xtypes::DynamicType& type)
{
    DynamicTypeBuilderFactory* factory = DynamicTypeBuilderFactory::get_instance();
    switch (type.kind())
    {
        case ::xtypes::TypeKind::BOOLEAN_TYPE:
        {
            return factory->create_bool_builder();
        }
        case ::xtypes::TypeKind::INT_8_TYPE:
        case ::xtypes::TypeKind::UINT_8_TYPE:
        {
            return factory->create_byte_builder();
        }
        case ::xtypes::TypeKind::INT_16_TYPE:
        {
            return factory->create_int16_builder();
        }
        case ::xtypes::TypeKind::UINT_16_TYPE:
        {
            return factory->create_uint16_builder();
        }
        case ::xtypes::TypeKind::INT_32_TYPE:
        {
            return factory->create_int32_builder();
        }
        case ::xtypes::TypeKind::UINT_32_TYPE:
        {
            return factory->create_uint32_builder();
        }
        case ::xtypes::TypeKind::INT_64_TYPE:
        {
            return factory->create_int64_builder();
        }
        case ::xtypes::TypeKind::UINT_64_TYPE:
        {
            return factory->create_uint64_builder();
        }
        case ::xtypes::TypeKind::FLOAT_32_TYPE:
        {
            return factory->create_float32_builder();
        }
        case ::xtypes::TypeKind::FLOAT_64_TYPE:
        {
            return factory->create_float64_builder();
        }
        case ::xtypes::TypeKind::FLOAT_128_TYPE:
        {
            return factory->create_float128_builder();
        }
        case ::xtypes::TypeKind::CHAR_8_TYPE:
        {
            return factory->create_char8_builder();
        }
        case ::xtypes::TypeKind::CHAR_16_TYPE:
        {
            return factory->create_char16_builder();
        }
        case ::xtypes::TypeKind::ENUMERATION_TYPE:
        {
            DynamicTypeBuilder* builder = factory->create_enum_builder();
            builder->set_name(convert_type_name(type.name()));
            const ::xtypes::EnumerationType<uint32_t>& enum_type =
                    static_cast<const ::xtypes::EnumerationType<uint32_t>&>(type);
            const std::map<std::string, uint32_t>& enumerators = enum_type.enumerators();
            for (auto pair : enumerators)
            {
                builder->add_empty_member(pair.second, pair.first);
            }
            return builder;
        }
        case ::xtypes::TypeKind::BITSET_TYPE:
        {
            // TODO
            break;
        }
        case ::xtypes::TypeKind::ALIAS_TYPE:
        {
            /* Real alias... doesn't returns a builder, but a type
               DynamicTypeBuilder_ptr content = get_builder(static_cast<const ::xtypes::AliasType&>(type).get());
               DynamicTypeBuilder* ptr = content.get();
               return factory->create_alias_type(ptr, type.name());
             */
            return get_builder(static_cast<const ::xtypes::AliasType&>(type).rget()); // Resolve alias
        }
        /*
           case ::xtypes::TypeKind::BITMASK_TYPE:
           {
            // TODO
           }
         */
        case ::xtypes::TypeKind::ARRAY_TYPE:
        {
            const ::xtypes::ArrayType& c_type = static_cast<const ::xtypes::ArrayType&>(type);
            std::pair<std::vector<uint32_t>, DynamicTypeBuilder_ptr> pair;
            get_array_specs(c_type, pair);
            DynamicTypeBuilder* builder = static_cast<DynamicTypeBuilder*>(pair.second.get());
            DynamicTypeBuilder_ptr result = factory->create_array_builder(builder, pair.first);
            return result;
        }
        case ::xtypes::TypeKind::SEQUENCE_TYPE:
        {
            const ::xtypes::SequenceType& c_type = static_cast<const ::xtypes::SequenceType&>(type);
            DynamicTypeBuilder_ptr content = get_builder(c_type.content_type());
            DynamicTypeBuilder* builder = static_cast<DynamicTypeBuilder*>(content.get());
            DynamicTypeBuilder_ptr result = factory->create_sequence_builder(builder, c_type.bounds());
            return result;
        }
        case ::xtypes::TypeKind::STRING_TYPE:
        {
            const ::xtypes::StringType& c_type = static_cast<const ::xtypes::StringType&>(type);
            return factory->create_string_builder(c_type.bounds());
        }
        case ::xtypes::TypeKind::WSTRING_TYPE:
        {
            const ::xtypes::WStringType& c_type = static_cast<const ::xtypes::WStringType&>(type);
            return factory->create_wstring_builder(c_type.bounds());
        }
        case ::xtypes::TypeKind::MAP_TYPE:
        {
            const ::xtypes::MapType& c_type = static_cast<const ::xtypes::MapType&>(type);
            const ::xtypes::PairType& content_type = static_cast<const ::xtypes::PairType&>(c_type.content_type());
            DynamicTypeBuilder_ptr key = get_builder(content_type.first());
            DynamicTypeBuilder_ptr value = get_builder(content_type.second());
            DynamicTypeBuilder_ptr result = factory->create_map_builder(key.get(), value.get(), c_type.bounds());
            return result;
        }
        case ::xtypes::TypeKind::UNION_TYPE:
        {
            const ::xtypes::UnionType& c_type = static_cast<const ::xtypes::UnionType&>(type);
            DynamicTypeBuilder_ptr disc_type = get_builder(c_type.discriminator());
            DynamicTypeBuilder_ptr result = factory->create_union_builder(disc_type.get());

            MemberId idx = 0;
            for (const std::string& member_name : c_type.get_case_members())
            {
                bool is_default = c_type.is_default(member_name);
                const ::xtypes::Member& member = c_type.member(member_name);
                std::vector<int64_t> labels_original = c_type.get_labels(member_name);
                std::vector<uint64_t> labels;
                for (int64_t label : labels_original)
                {
                    labels.push_back(static_cast<uint64_t>(label));
                }
                DynamicTypeBuilder_ptr member_builder = get_builder(member.type());
                result->add_member(idx++, member_name, member_builder.get(), "", labels, is_default);
            }
            return result;
        }
        case ::xtypes::TypeKind::STRUCTURE_TYPE:
        {
            DynamicTypeBuilder_ptr result = factory->create_struct_builder();
            const ::xtypes::StructType& from = static_cast<const ::xtypes::StructType&>(type);

            for (size_t idx = 0; idx < from.members().size(); ++idx)
            {
                const ::xtypes::Member& member = from.member(idx);
                DynamicTypeBuilder_ptr member_builder = get_builder(member.type());
                DynamicTypeBuilder* builder = static_cast<DynamicTypeBuilder*>(member_builder.get());
                DynamicTypeBuilder* result_ptr = static_cast<DynamicTypeBuilder*>(result.get());
                result_ptr->add_member(static_cast<MemberId>(idx), member.name(), builder);
            }
            return result;
        }
        default:
            break;
    }
    return nullptr;
}

void Conversion::get_array_specs(
        const ::xtypes::ArrayType& array,
        std::pair<std::vector<uint32_t>, DynamicTypeBuilder_ptr>& result)
{
    result.first.push_back(array.dimension());
    if (array.content_type().kind() == ::xtypes::TypeKind::ARRAY_TYPE)
    {
        get_array_specs(static_cast<const ::xtypes::ArrayType&>(array.content_type()), result);
    }
    else
    {
        result.second = get_builder(array.content_type());
    }
}

} // namespace dds
} // namespace soss
