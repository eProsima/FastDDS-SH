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

#include <Conversion.hpp>

#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataFactory.h>

#include <xtypes/xtypes.hpp>

#include <gtest/gtest.h>

namespace fastdds = eprosima::fastdds;

namespace eprosima {
namespace is {
namespace sh {
namespace fastdds {
namespace test {

static const std::string fastdds_sh_unit_test_types = "fastdds_sh_unit_test_types.idl";

static void fill_basic_struct(
        xtypes::WritableDynamicDataRef xtypes_data)
{
    xtypes_data["my_bool"] = true;
    xtypes_data["my_octet"] = static_cast<uint8_t>(55);
    xtypes_data["my_int16"] = static_cast<int16_t>(-555);
    xtypes_data["my_int32"] = -555555;
    xtypes_data["my_int64"] = -55555555555l;
    xtypes_data["my_uint16"] = static_cast<uint16_t>(555);
    xtypes_data["my_uint32"] = 555555u;
    xtypes_data["my_uint64"] = 55555555555ul;
    xtypes_data["my_float32"] = 55.555e3f;
    xtypes_data["my_float64"] = 5.8598e40;
    xtypes_data["my_float128"] = 3.54e2400l;
    xtypes_data["my_char"] = 'P';
    xtypes_data["my_wchar"] = L'G';
    xtypes_data["my_string"] = "Testing a string.";
    xtypes_data["my_wstring"] = L"Testing a wstring: \u20B1";
    xtypes_data["my_enum"] = 2u; // C
}

static void check_basic_struct(
        fastrtps::types::DynamicData* dds_data)
{
    ASSERT_TRUE(dds_data->get_bool_value(dds_data->get_member_id_by_name("my_bool")));
    ASSERT_EQ(dds_data->get_byte_value(dds_data->get_member_id_by_name("my_octet")),
            static_cast<fastrtps::rtps::octet>(55));
    ASSERT_EQ(dds_data->get_int16_value(dds_data->get_member_id_by_name("my_int16")),
            static_cast<int16_t>(-555));
    ASSERT_EQ(dds_data->get_int32_value(dds_data->get_member_id_by_name("my_int32")),
            static_cast<int32_t>(-555555));
    ASSERT_EQ(dds_data->get_int64_value(dds_data->get_member_id_by_name("my_int64")),
            static_cast<int64_t>(-55555555555l));
    ASSERT_EQ(dds_data->get_uint16_value(dds_data->get_member_id_by_name("my_uint16")),
            static_cast<uint16_t>(555));
    ASSERT_EQ(dds_data->get_uint32_value(dds_data->get_member_id_by_name("my_uint32")),
            static_cast<uint32_t>(555555u));
    ASSERT_EQ(dds_data->get_uint64_value(dds_data->get_member_id_by_name("my_uint64")),
            static_cast<uint64_t>(55555555555ul));
    ASSERT_EQ(dds_data->get_float32_value(dds_data->get_member_id_by_name("my_float32")),
            static_cast<float>(55.555e3f));
    ASSERT_EQ(dds_data->get_float64_value(dds_data->get_member_id_by_name("my_float64")),
            static_cast<double>(5.8598e40));
    ASSERT_EQ(dds_data->get_float128_value(dds_data->get_member_id_by_name("my_float128")),
            static_cast<long double>(3.54e2400l));
    ASSERT_EQ(dds_data->get_char8_value(dds_data->get_member_id_by_name("my_char")),
            'P');
    ASSERT_EQ(dds_data->get_char16_value(dds_data->get_member_id_by_name("my_wchar")),
            L'G');
    ASSERT_EQ(dds_data->get_string_value(dds_data->get_member_id_by_name("my_string")),
            "Testing a string.");
    ASSERT_EQ(dds_data->get_wstring_value(dds_data->get_member_id_by_name("my_wstring")),
            L"Testing a wstring: \u20B1");
    ASSERT_EQ(dds_data->get_enum_value(dds_data->get_member_id_by_name("my_enum")),
            "C"); // 2u == "C"
}

static void check_basic_struct(
        xtypes::ReadableDynamicDataRef xtypes_data)
{
    ASSERT_TRUE(xtypes_data["my_bool"].value<bool>());
    ASSERT_EQ(xtypes_data["my_octet"].value<uint8_t>(), static_cast<uint8_t>(55));
    ASSERT_EQ(xtypes_data["my_int16"].value<int16_t>(), static_cast<int16_t>(-555));
    ASSERT_EQ(xtypes_data["my_int32"].value<int32_t>(), static_cast<int32_t>(-555555));
    ASSERT_EQ(xtypes_data["my_int64"].value<int64_t>(), static_cast<int64_t>(-55555555555l));
    ASSERT_EQ(xtypes_data["my_uint16"].value<uint16_t>(), static_cast<uint16_t>(555));
    ASSERT_EQ(xtypes_data["my_uint32"].value<uint32_t>(), static_cast<uint32_t>(555555u));
    ASSERT_EQ(xtypes_data["my_uint64"].value<uint64_t>(), static_cast<uint64_t>(55555555555ul));
    ASSERT_EQ(xtypes_data["my_float32"].value<float>(), static_cast<float>(55.555e3f));
    ASSERT_EQ(xtypes_data["my_float64"].value<double>(), static_cast<double>(5.8598e40));
    ASSERT_EQ(xtypes_data["my_float128"].value<long double>(), static_cast<long double>(3.54e2400l));
    ASSERT_EQ(xtypes_data["my_char"].value<char>(), 'P');
    ASSERT_EQ(xtypes_data["my_wchar"].value<wchar_t>(), L'G');
    ASSERT_EQ(xtypes_data["my_string"].value<std::string>(), "Testing a string.");
    ASSERT_EQ(xtypes_data["my_wstring"].value<std::wstring>(), L"Testing a wstring: \u20B1");
    ASSERT_EQ(xtypes_data["my_enum"].value<uint32_t>(), static_cast<uint32_t>(2u));
}

static void fill_nested_sequence(
        xtypes::WritableDynamicDataRef xtypes_data)
{
    const xtypes::SequenceType& inner_type =
            static_cast<const xtypes::SequenceType&>(xtypes_data.type());

    for (size_t i = 0; i < 3; ++i)
    {
        xtypes::DynamicData inner_seq(inner_type.content_type());

        for (size_t j = 0; j < 2; ++j)
        {
            int32_t temp = j + (i * 2);
            inner_seq.push(temp);
        }

        xtypes_data.push(inner_seq);
    }
}

static void check_nested_sequence(
        fastrtps::types::DynamicData* dds_data)
{
    for (uint32_t i = 0; i < 3; ++i)
    {
        fastrtps::types::DynamicData* inner_seq = dds_data->loan_value(i);
        for (uint32_t j = 0; j < 2; ++j)
        {
            int32_t temp = inner_seq->get_int32_value(j);
            ASSERT_EQ(temp, (j + (i * 2)));
        }
        dds_data->return_loaned_value(inner_seq);
    }
}

static void check_nested_sequence(
        xtypes::ReadableDynamicDataRef xtypes_data)
{
    for (uint32_t i = 0; i < 3; ++i)
    {
        for (uint32_t j = 0; j < 2; ++j)
        {
            int32_t temp = xtypes_data[i][j];
            ASSERT_EQ(temp, (j + (i * 2)));
        }
    }
}

static void fill_nested_array(
        xtypes::WritableDynamicDataRef xtypes_data)
{
    for (size_t i = 0; i < 4; ++i)
    {
        for (size_t j = 0; j < 5; ++j)
        {
            int32_t temp = j + (i * 5);
            xtypes_data[i][j] = std::to_string(temp);
        }
    }
}

static void check_nested_array(
        fastrtps::types::DynamicData* dds_data)
{
    for (uint32_t i = 0; i < 4; ++i)
    {
        for (uint32_t j = 0; j < 5; ++j)
        {
            fastrtps::types::MemberId idx = dds_data->get_array_index({i, j});
            std::string temp = dds_data->get_string_value(idx);
            ASSERT_EQ(temp, std::to_string(j + (i * 5)));
        }
    }
}

static void check_nested_array(
        xtypes::ReadableDynamicDataRef xtypes_data)
{
    for (uint32_t i = 0; i < 4; ++i)
    {
        for (uint32_t j = 0; j < 5; ++j)
        {
            std::string temp = xtypes_data[i][j];
            ASSERT_EQ(temp, std::to_string(j + (i * 5)));
        }
    }
}

static void fill_mixed_struct(
        xtypes::DynamicData& xtypes_data)
{
    const xtypes::StructType& mixed_type = static_cast<const xtypes::StructType&>(xtypes_data.type());
    //sequence<BasicStruct, 3> my_basic_seq;
    const xtypes::SequenceType& bst =
            static_cast<const xtypes::SequenceType&>(mixed_type.member("my_basic_seq").type());
    for (size_t i = 0; i < 3; ++i)
    {
        xtypes::DynamicData inner(bst.content_type());
        fill_basic_struct(inner);
        xtypes_data["my_basic_seq"].push(inner);
    }
    //sequence<NestedSequence, 4> my_stseq_seq;
    const xtypes::SequenceType& sst =
            static_cast<const xtypes::SequenceType&>(mixed_type.member("my_stseq_seq").type());
    for (size_t i = 0; i < 4; ++i)
    {
        xtypes::DynamicData inner(sst.content_type());
        fill_nested_sequence(inner["my_seq_seq"]);
        xtypes_data["my_stseq_seq"].push(inner);
    }
    //sequence<NestedArray, 5> my_starr_seq;
    const xtypes::SequenceType& ast =
            static_cast<const xtypes::SequenceType&>(mixed_type.member("my_starr_seq").type());
    for (size_t i = 0; i < 5; ++i)
    {
        xtypes::DynamicData inner(ast.content_type());
        fill_nested_array(inner["my_arr_arr"]);
        xtypes_data["my_starr_seq"].push(inner);
    }
    //BasicStruct my_basic_arr[3];
    for (size_t i = 0; i < 3; ++i)
    {
        fill_basic_struct(xtypes_data["my_basic_arr"][i]);
    }
    //NestedSequence my_stseq_arr[4];
    for (size_t i = 0; i < 4; ++i)
    {
        fill_nested_sequence(xtypes_data["my_stseq_arr"][i]["my_seq_seq"]);
    }
    //NestedArray my_starr_arr[5];
    for (size_t i = 0; i < 5; ++i)
    {
        fill_nested_array(xtypes_data["my_starr_arr"][i]["my_arr_arr"]);
    }
}

static void check_mixed_struct(
        fastrtps::types::DynamicData* dds_data)
{
    //sequence<BasicStruct, 3> my_basic_seq;
    {
        fastrtps::types::DynamicData* member =
                dds_data->loan_value(dds_data->get_member_id_by_name("my_basic_seq"));
        for (uint32_t i = 0; i < 3; ++i)
        {
            fastrtps::types::DynamicData* inner_seq = member->loan_value(i);
            check_basic_struct(inner_seq);
            member->return_loaned_value(inner_seq);
        }
        dds_data->return_loaned_value(member);
    }
    //sequence<NestedSequence, 4> my_stseq_seq;
    {
        fastrtps::types::DynamicData* member =
                dds_data->loan_value(dds_data->get_member_id_by_name("my_stseq_seq"));
        for (uint32_t i = 0; i < 4; ++i)
        {
            fastrtps::types::DynamicData* inner_seq = member->loan_value(i);
            fastrtps::types::DynamicData* inner_member =
                    inner_seq->loan_value(inner_seq->get_member_id_by_name("my_seq_seq"));
            check_nested_sequence(inner_member);
            inner_seq->return_loaned_value(inner_member);
            member->return_loaned_value(inner_seq);
        }
        dds_data->return_loaned_value(member);
    }
    //sequence<NestedArray, 5> my_starr_seq;
    {
        fastrtps::types::DynamicData* member =
                dds_data->loan_value(dds_data->get_member_id_by_name("my_starr_seq"));
        for (uint32_t i = 0; i < 5; ++i)
        {
            fastrtps::types::DynamicData* inner_seq = member->loan_value(i);
            fastrtps::types::DynamicData* inner_member =
                    inner_seq->loan_value(inner_seq->get_member_id_by_name("my_arr_arr"));
            check_nested_array(inner_member);
            inner_seq->return_loaned_value(inner_member);
            member->return_loaned_value(inner_seq);
        }
        dds_data->return_loaned_value(member);
    }
    //BasicStruct my_basic_arr[3];
    MemberId idx;
    {
        fastrtps::types::DynamicData* member =
                dds_data->loan_value(dds_data->get_member_id_by_name("my_basic_arr"));
        for (uint32_t i = 0; i < 3; ++i)
        {
            idx = member->get_array_index({i});
            fastrtps::types::DynamicData* inner_arr = member->loan_value(idx);
            check_basic_struct(inner_arr);
            member->return_loaned_value(inner_arr);
        }
        dds_data->return_loaned_value(member);
    }
    //NestedSequence my_stseq_arr[4];
    {
        fastrtps::types::DynamicData* member =
                dds_data->loan_value(dds_data->get_member_id_by_name("my_stseq_arr"));
        for (uint32_t i = 0; i < 4; ++i)
        {
            idx = member->get_array_index({i});
            fastrtps::types::DynamicData* inner_arr = member->loan_value(idx);
            fastrtps::types::DynamicData* inner_member =
                    inner_arr->loan_value(inner_arr->get_member_id_by_name("my_seq_seq"));
            check_nested_sequence(inner_member);
            inner_arr->return_loaned_value(inner_member);
            member->return_loaned_value(inner_arr);
        }
        dds_data->return_loaned_value(member);
    }
    //NestedArray my_starr_arr[5];
    {
        fastrtps::types::DynamicData* member =
                dds_data->loan_value(dds_data->get_member_id_by_name("my_starr_arr"));
        for (uint32_t i = 0; i < 5; ++i)
        {
            idx = member->get_array_index({i});
            fastrtps::types::DynamicData* inner_arr = member->loan_value(idx);
            fastrtps::types::DynamicData* inner_member =
                    inner_arr->loan_value(inner_arr->get_member_id_by_name("my_arr_arr"));
            check_nested_array(inner_member);
            inner_arr->return_loaned_value(inner_member);
            member->return_loaned_value(inner_arr);
        }
        dds_data->return_loaned_value(member);
    }
}

static void check_mixed_struct(
        xtypes::ReadableDynamicDataRef xtypes_data)
{
    //sequence<BasicStruct, 3> my_basic_seq;
    {
        for (uint32_t i = 0; i < 3; ++i)
        {
            check_basic_struct(xtypes_data["my_basic_seq"][i]);
        }
    }
    //sequence<NestedSequence, 4> my_stseq_seq;
    {
        for (uint32_t i = 0; i < 4; ++i)
        {
            check_nested_sequence(xtypes_data["my_stseq_seq"][i]["my_seq_seq"]);
        }
    }
    //sequence<NestedArray, 5> my_starr_seq;
    {
        for (uint32_t i = 0; i < 5; ++i)
        {
            check_nested_array(xtypes_data["my_starr_seq"][i]["my_arr_arr"]);
        }
    }
    //BasicStruct my_basic_arr[3];
    {
        for (uint32_t i = 0; i < 3; ++i)
        {
            check_basic_struct(xtypes_data["my_basic_arr"][i]);
        }
    }
    //NestedSequence my_stseq_arr[4];
    {
        for (uint32_t i = 0; i < 4; ++i)
        {
            check_nested_sequence(xtypes_data["my_stseq_arr"][i]["my_seq_seq"]);
        }
    }
    //NestedArray my_starr_arr[5];
    {
        for (uint32_t i = 0; i < 5; ++i)
        {
            check_nested_array(xtypes_data["my_starr_arr"][i]["my_arr_arr"]);
        }
    }
}

static void fill_union_struct(
        xtypes::DynamicData& xtypes_data,
        uint8_t disc)
{
    //MyUnion my_union;
    /*
       union MyUnion switch (uint8)
       {
        case 0: float my_float32;
        case 1:
        case 2: string my_string;
        case 3: AliasBasicStruct abs;
       };
     */
    switch (disc)
    {
        case 0:
            xtypes_data["my_union"]["my_float32"] = 123.456f;
            break;
        case 1:
        case 2:
            xtypes_data["my_union"]["my_string"] = "Union String";
            break;
        case 3:
            fill_basic_struct(xtypes_data["my_union"]["abs"]);
            break;
    }
    //map<string, AliasBasicStruct> my_map;
    xtypes::StringType key_type;
    xtypes::DynamicData key(key_type);
    key = "Luis";
    fill_basic_struct(xtypes_data["my_map"][key]);
    key = "Gasco";
    fill_basic_struct(xtypes_data["my_map"][key]);
    key = "Rulz";
    fill_basic_struct(xtypes_data["my_map"][key]);
}

static void check_union_struct(
        fastrtps::types::DynamicData* dds_data)
{
    fastrtps::types::DynamicData* union_data = dds_data->loan_value(0);
    UnionDynamicData* dd_union = static_cast<UnionDynamicData*>(union_data);
    MemberId id = dd_union->get_union_id();
    uint64_t label = dd_union->get_union_label();
    //MyUnion my_union;
    /*
       union MyUnion switch (uint8)
       {
        case 0: float my_float32;
        case 1:
        case 2: string my_string;
        case 3: AliasBasicStruct abs;
       };
     */
    switch (label)
    {
        case 0:
        {
            float v = dd_union->get_float32_value(id);
            ASSERT_EQ(v, 123.456f);
        }
        break;
        case 1:
        case 2:
        {
            std::string v = dd_union->get_string_value(id);
            ASSERT_EQ(v, "Union String");
        }
        break;
        case 3:
        {
            fastrtps::types::DynamicData* v = dd_union->loan_value(id);
            check_basic_struct(v);
            dd_union->return_loaned_value(v);
        }
        break;
    }
    dds_data->return_loaned_value(union_data);
    //map<string, AliasBasicStruct> my_map;
    fastrtps::types::DynamicData* map_data = dds_data->loan_value(1);
    MemberId keyId = 0;
    MemberId elemId = 1;
    std::string key;
    fastrtps::types::DynamicData* v;

    uint32_t luis = 0;
    uint32_t gasco = 0;
    uint32_t rulz = 0;
    uint32_t other = 0;

    // map is ordered by hash!
    for (uint32_t i = 0; i < 3; ++i)
    {
        keyId = i * 2;
        elemId = keyId + 1;
        key = map_data->get_string_value(keyId);
        v = map_data->loan_value(elemId);

        if (key == "Luis")
        {
            ++luis;
        }
        else if (key == "Gasco")
        {
            ++gasco;
        }
        else if (key == "Rulz")
        {
            ++rulz;
        }
        else
        {
            ++other;
        }


        check_basic_struct(v);
        map_data->return_loaned_value(v);
    }

    ASSERT_TRUE(((luis == gasco) && (gasco == rulz) && (rulz == 1u) && (other == 0u)));

    dds_data->return_loaned_value(map_data);
}

static void check_union_struct(
        xtypes::ReadableDynamicDataRef xtypes_data)
{
    //MyUnion my_union;
    /*
       union MyUnion switch (uint8)
       {
        case 0: float my_float32;
        case 1:
        case 2: string my_string;
        case 3: AliasBasicStruct abs;
       };
     */
    switch (xtypes_data["my_union"].d().value<uint8_t>())
    {
        case 0:
            ASSERT_EQ(xtypes_data["my_union"]["my_float32"].value<float>(), 123.456f);
            break;
        case 1:
        case 2:
            ASSERT_EQ(xtypes_data["my_union"]["my_string"].value<std::string>(), "Union String");
            break;
        case 3:
            check_basic_struct(xtypes_data["my_union"]["abs"]);
            break;
    }
    //map<string, AliasBasicStruct> my_map;
    xtypes::StringType key_type;
    xtypes::DynamicData key(key_type);
    key = "Luis";
    check_basic_struct(xtypes_data["my_map"].at(key));
    key = "Gasco";
    check_basic_struct(xtypes_data["my_map"].at(key));
    key = "Rulz";
    check_basic_struct(xtypes_data["my_map"].at(key));
}

TEST(FastDDSUnitary, Convert_between_Integration_Service_and_DDS__basic_type)
{
    xtypes::idl::Context context = xtypes::idl::parse_file(fastdds_sh_unit_test_types);
    ASSERT_TRUE(context.success);

    auto result = context.get_all_scoped_types();
    ASSERT_FALSE(result.empty());

    const xtypes::DynamicType* basic_struct = result["BasicStruct"].get();
    ASSERT_NE(basic_struct, nullptr);
    // Convert type from Integration Service to dds
    fastrtps::types::DynamicTypeBuilder* builder = Conversion::create_builder(*basic_struct);
    ASSERT_NE(builder, nullptr);
    fastrtps::types::DynamicType_ptr dds_struct = builder->build();
    fastrtps::types::DynamicData_ptr dds_data_ptr(
        fastrtps::types::DynamicDataFactory::get_instance()->create_data(dds_struct));
    fastrtps::types::DynamicData* dds_data =
            static_cast<fastrtps::types::DynamicData*>(dds_data_ptr.get());
    xtypes::DynamicData xtypes_data(*basic_struct);
    // Fill xtypes_data
    fill_basic_struct(xtypes_data);
    // Convert to dds_data
    Conversion::xtypes_to_fastdds(xtypes_data, dds_data);
    // Check data in dds_data
    check_basic_struct(dds_data);
    // The other way
    xtypes::DynamicData wayback(*basic_struct);
    Conversion::fastdds_to_xtypes(dds_data, wayback);
    check_basic_struct(wayback);
}

TEST(FastDDSUnitary, Convert_between_Integration_Service_and_DDS__nested_sequence)
{
    xtypes::idl::Context context = xtypes::idl::parse_file(fastdds_sh_unit_test_types);
    ASSERT_TRUE(context.success);

    auto result = context.get_all_scoped_types();
    ASSERT_FALSE(result.empty());

    const xtypes::DynamicType* nested_sequence = result["NestedSequence"].get();
    ASSERT_NE(nested_sequence, nullptr);
    // Convert type from Integration Service to dds
    fastrtps::types::DynamicTypeBuilder* builder = Conversion::create_builder(*nested_sequence);
    ASSERT_NE(builder, nullptr);
    fastrtps::types::DynamicType_ptr dds_sequence = builder->build();
    fastrtps::types::DynamicData_ptr dds_data_ptr(
        fastrtps::types::DynamicDataFactory::get_instance()->create_data(dds_sequence));
    fastrtps::types::DynamicData* dds_data =
            static_cast<fastrtps::types::DynamicData*>(dds_data_ptr.get());
    xtypes::DynamicData xtypes_data(*nested_sequence);
    // Fill xtypes_data
    fill_nested_sequence(xtypes_data["my_seq_seq"]);
    // Convert to dds_data
    Conversion::xtypes_to_fastdds(xtypes_data, dds_data);
    // Check data in dds_data
    fastrtps::types::DynamicData* inner_seq =
            dds_data->loan_value(dds_data->get_member_id_by_name("my_seq_seq"));
    check_nested_sequence(inner_seq);
    dds_data->return_loaned_value(inner_seq);
    // The other way
    xtypes::DynamicData wayback(*nested_sequence);
    Conversion::fastdds_to_xtypes(dds_data, wayback);
    check_nested_sequence(wayback["my_seq_seq"]);
}

TEST(FastDDSUnitary, Convert_between_Integration_Service_and_DDS__nested_array)
{
    xtypes::idl::Context context = xtypes::idl::parse_file(fastdds_sh_unit_test_types);
    ASSERT_TRUE(context.success);

    auto result = context.get_all_scoped_types();
    ASSERT_FALSE(result.empty());

    const xtypes::DynamicType* nested_array = result["NestedArray"].get();
    ASSERT_NE(nested_array, nullptr);
    // Convert type from Integration Service to dds
    fastrtps::types::DynamicTypeBuilder* builder = Conversion::create_builder(*nested_array);
    ASSERT_NE(builder, nullptr);
    fastrtps::types::DynamicType_ptr dds_array = builder->build();
    fastrtps::types::DynamicData_ptr dds_data_ptr(
        fastrtps::types::DynamicDataFactory::get_instance()->create_data(dds_array));
    fastrtps::types::DynamicData* dds_data =
            static_cast<fastrtps::types::DynamicData*>(dds_data_ptr.get());
    xtypes::DynamicData xtypes_data(*nested_array);
    // Fill xtypes_data
    fill_nested_array(xtypes_data["my_arr_arr"]);
    // Convert to dds_data
    Conversion::xtypes_to_fastdds(xtypes_data, dds_data);
    // Check data in dds_data
    fastrtps::types::DynamicData* inner_arr =
            dds_data->loan_value(dds_data->get_member_id_by_name("my_arr_arr"));
    check_nested_array(inner_arr);
    dds_data->return_loaned_value(inner_arr);
    // The other way
    xtypes::DynamicData wayback(*nested_array);
    Conversion::fastdds_to_xtypes(dds_data, wayback);
    check_nested_array(wayback["my_arr_arr"]);
}

TEST(FastDDSUnitary, Convert_between_Integration_Service_and_DDS__mixed_struct)
{
    xtypes::idl::Context context = xtypes::idl::parse_file(fastdds_sh_unit_test_types);
    ASSERT_TRUE(context.success);

    auto result = context.get_all_scoped_types();
    ASSERT_FALSE(result.empty());

    const xtypes::DynamicType* mixed_struct = result["MixedStruct"].get();
    ASSERT_NE(mixed_struct, nullptr);
    // Convert type from Integration Service to dds
    fastrtps::types::DynamicTypeBuilder* builder = Conversion::create_builder(*mixed_struct);
    ASSERT_NE(builder, nullptr);
    fastrtps::types::DynamicType_ptr dds_struct = builder->build();
    fastrtps::types::DynamicData_ptr dds_data_ptr(
        fastrtps::types::DynamicDataFactory::get_instance()->create_data(dds_struct));
    fastrtps::types::DynamicData* dds_data =
            static_cast<fastrtps::types::DynamicData*>(dds_data_ptr.get());
    xtypes::DynamicData xtypes_data(*mixed_struct);
    // Fill xtypes_data
    fill_mixed_struct(xtypes_data);
    // Convert to dds_data
    Conversion::xtypes_to_fastdds(xtypes_data, dds_data);
    // Check data in dds_data
    check_mixed_struct(dds_data);
    // The other way
    xtypes::DynamicData wayback(*mixed_struct);
    Conversion::fastdds_to_xtypes(dds_data, wayback);
    check_mixed_struct(wayback);
}

TEST(FastDDSUnitary, Convert_between_Integration_Service_and_DDS__union_struct)
{
    xtypes::idl::Context context = xtypes::idl::parse_file(fastdds_sh_unit_test_types);
    ASSERT_TRUE(context.success);

    auto result = context.get_all_scoped_types();
    ASSERT_FALSE(result.empty());

    const xtypes::DynamicType* union_struct = result["MyUnionStruct"].get();
    ASSERT_NE(union_struct, nullptr);
    // Convert type from Integration Service to dds
    fastrtps::types::DynamicTypeBuilder* builder = Conversion::create_builder(*union_struct);
    ASSERT_NE(builder, nullptr);
    fastrtps::types::DynamicType_ptr dds_struct = builder->build();
    fastrtps::types::DynamicData_ptr dds_data_ptr(
        fastrtps::types::DynamicDataFactory::get_instance()->create_data(dds_struct));
    fastrtps::types::DynamicData* dds_data =
            static_cast<fastrtps::types::DynamicData*>(dds_data_ptr.get());
    xtypes::DynamicData xtypes_data(*union_struct);
    {
        // Fill xtypes_data
        fill_union_struct(xtypes_data, 0);
        // Convert to dds_data
        Conversion::xtypes_to_fastdds(xtypes_data, dds_data);
        // Check data in dds_data
        check_union_struct(dds_data);
        // The other way
        xtypes::DynamicData wayback(*union_struct);
        Conversion::fastdds_to_xtypes(dds_data, wayback);
        check_union_struct(wayback);
    }
    {
        // Fill xtypes_data
        fill_union_struct(xtypes_data, 1);
        // Convert to dds_data
        Conversion::xtypes_to_fastdds(xtypes_data, dds_data);
        // Check data in dds_data
        check_union_struct(dds_data);
        // The other way
        xtypes::DynamicData wayback(*union_struct);
        Conversion::fastdds_to_xtypes(dds_data, wayback);
        check_union_struct(wayback);
    }
    {
        // Fill xtypes_data
        fill_union_struct(xtypes_data, 3);
        // Convert to dds_data
        Conversion::xtypes_to_fastdds(xtypes_data, dds_data);
        // Check data in dds_data
        check_union_struct(dds_data);
        // The other way
        xtypes::DynamicData wayback(*union_struct);
        Conversion::fastdds_to_xtypes(dds_data, wayback);
        check_union_struct(wayback);
    }
}

TEST(FastDDSUnitary, Convert_between_Integration_Service_and_DDS__namespaced_type)
{
    xtypes::idl::Context context = xtypes::idl::parse_file(fastdds_sh_unit_test_types);
    ASSERT_TRUE(context.success);

    auto result = context.get_all_scoped_types();
    ASSERT_FALSE(result.empty());

    const xtypes::DynamicType* namespaced_type =
            result["fastdds_sh::unit_test::types::NamespacedType"].get();
    ASSERT_NE(namespaced_type, nullptr);
    // Convert type from Integration Service to dds
    fastrtps::types::DynamicTypeBuilder* builder = Conversion::create_builder(*namespaced_type);
    ASSERT_NE(builder, nullptr);
    fastrtps::types::DynamicType_ptr dds_namespaced_type = builder->build();
    fastrtps::types::DynamicData_ptr dds_data_ptr(
        fastrtps::types::DynamicDataFactory::get_instance()->create_data(dds_namespaced_type));
    fastrtps::types::DynamicData* dds_data =
            static_cast<fastrtps::types::DynamicData*>(dds_data_ptr.get());
    xtypes::DynamicData xtypes_data(*namespaced_type);
    // Fill xtypes_data
    fill_basic_struct(xtypes_data["basic"]);
    // Convert to dds_data
    Conversion::xtypes_to_fastdds(xtypes_data, dds_data);
    // Check data in dds_data
    fastrtps::types::DynamicData* dds_basic_data =
            dds_data->loan_value(dds_data->get_member_id_by_name("basic"));
    check_basic_struct(dds_basic_data);
    dds_data->return_loaned_value(dds_basic_data);
    // The other way
    xtypes::DynamicData wayback(*namespaced_type);
    Conversion::fastdds_to_xtypes(dds_data, wayback);
    check_basic_struct(wayback["basic"]);
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
