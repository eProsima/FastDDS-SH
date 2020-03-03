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
#include <xtypes/xtypes.hpp>
#include <catch2/catch.hpp>

namespace soss {
    using namespace eprosima::xtypes;
    using namespace eprosima::xtypes::idl;
}

namespace dds {
    using DynamicData = ::soss::dds::DynamicData;
    using DynamicData_ptr = ::soss::dds::DynamicData_ptr;
    using DynamicDataFactory = ::soss::dds::DynamicDataFactory;
    using DynamicTypeBuilder = ::soss::dds::DynamicTypeBuilder;
    using DynamicType_ptr = ::soss::dds::DynamicType_ptr;
    using MemberId = ::eprosima::fastrtps::types::MemberId;
}

static const std::string soss_types = "soss_types.idl";

using soss::dds::Conversion;

static void fill_basic_struct(
        soss::WritableDynamicDataRef soss_data)
{
    soss_data["my_bool"] = true;
    soss_data["my_octet"] = static_cast<uint8_t>(55);
    soss_data["my_int16"] = static_cast<int16_t>(-555);
    soss_data["my_int32"] = -555555;
    soss_data["my_int64"] = -55555555555l;
    soss_data["my_uint16"] = static_cast<uint16_t>(555);
    soss_data["my_uint32"] = 555555u;
    soss_data["my_uint64"] = 55555555555ul;
    soss_data["my_float32"] = 55.555e3f;
    soss_data["my_float64"] = 5.8598e40;
    soss_data["my_float128"] = 3.54e2400l;
    soss_data["my_char"] = 'P';
    soss_data["my_wchar"] = L'G';
    soss_data["my_string"] = "Testing a string.";
    soss_data["my_wstring"] = L"Testing a wstring: \u20B1";
    soss_data["my_enum"] = 2u; // C
}

static void check_basic_struct(
        dds::DynamicData* dds_data)
{
    REQUIRE(dds_data->get_bool_value(dds_data->get_member_id_by_name("my_bool")) == true);
    REQUIRE(dds_data->get_byte_value(dds_data->get_member_id_by_name("my_octet")) == 55);
    REQUIRE(dds_data->get_int16_value(dds_data->get_member_id_by_name("my_int16")) == static_cast<int16_t>(-555));
    REQUIRE(dds_data->get_int32_value(dds_data->get_member_id_by_name("my_int32")) == -555555);
    REQUIRE(dds_data->get_int64_value(dds_data->get_member_id_by_name("my_int64")) == -55555555555l);
    REQUIRE(dds_data->get_uint16_value(dds_data->get_member_id_by_name("my_uint16")) == static_cast<uint16_t>(555));
    REQUIRE(dds_data->get_uint32_value(dds_data->get_member_id_by_name("my_uint32")) == 555555u);
    REQUIRE(dds_data->get_uint64_value(dds_data->get_member_id_by_name("my_uint64")) == 55555555555ul);
    REQUIRE(dds_data->get_float32_value(dds_data->get_member_id_by_name("my_float32")) == 55.555e3f);
    REQUIRE(dds_data->get_float64_value(dds_data->get_member_id_by_name("my_float64")) == 5.8598e40);
    REQUIRE(dds_data->get_float128_value(dds_data->get_member_id_by_name("my_float128")) == 3.54e2400l);
    REQUIRE(dds_data->get_char8_value(dds_data->get_member_id_by_name("my_char")) == 'P');
    REQUIRE(dds_data->get_char16_value(dds_data->get_member_id_by_name("my_wchar")) == L'G');
    REQUIRE(dds_data->get_string_value(dds_data->get_member_id_by_name("my_string")) == "Testing a string.");
    REQUIRE(dds_data->get_wstring_value(dds_data->get_member_id_by_name("my_wstring")) == L"Testing a wstring: \u20B1");
    REQUIRE(dds_data->get_enum_value(dds_data->get_member_id_by_name("my_enum")) == "C"); // 2u == "C"
}

static void check_basic_struct(
        soss::ReadableDynamicDataRef soss_data)
{
    REQUIRE(soss_data["my_bool"].value<bool>() == true);
    REQUIRE(soss_data["my_octet"].value<uint8_t>() == 55);
    REQUIRE(soss_data["my_int16"].value<int16_t>() == static_cast<int16_t>(-555));
    REQUIRE(soss_data["my_int32"].value<int32_t>() == -555555);
    REQUIRE(soss_data["my_int64"].value<int64_t>() == -55555555555l);
    REQUIRE(soss_data["my_uint16"].value<uint16_t>() == static_cast<uint16_t>(555));
    REQUIRE(soss_data["my_uint32"].value<uint32_t>() == 555555u);
    REQUIRE(soss_data["my_uint64"].value<uint64_t>() == 55555555555ul);
    REQUIRE(soss_data["my_float32"].value<float>() == 55.555e3f);
    REQUIRE(soss_data["my_float64"].value<double>() == 5.8598e40);
    REQUIRE(soss_data["my_float128"].value<long double>() == 3.54e2400l);
    REQUIRE(soss_data["my_char"].value<char>() == 'P');
    REQUIRE(soss_data["my_wchar"].value<wchar_t>() == L'G');
    REQUIRE(soss_data["my_string"].value<std::string>() == "Testing a string.");
    REQUIRE(soss_data["my_wstring"].value<std::wstring>() == L"Testing a wstring: \u20B1");
    REQUIRE(soss_data["my_enum"].value<uint32_t>() == 2u);
}

static void fill_nested_sequence(
        soss::WritableDynamicDataRef soss_data)
{
    const soss::SequenceType& inner_type = static_cast<const soss::SequenceType&>(soss_data.type());
    for (size_t i = 0; i < 3; ++i)
    {
        soss::DynamicData inner_seq(inner_type.content_type());
        for (size_t j = 0; j < 2; ++j)
        {
            int32_t temp = j + (i * 2);
            inner_seq.push(temp);
        }
        soss_data.push(inner_seq);
    }
}

static void check_nested_sequence(
        dds::DynamicData* dds_data)
{
    for (uint32_t i = 0; i < 3; ++i)
    {
        dds::DynamicData* inner_seq= dds_data->loan_value(i);
        for (uint32_t j = 0; j < 2; ++j)
        {
            int32_t temp = inner_seq->get_int32_value(j);
            REQUIRE(temp == (j + (i * 2)));
        }
        dds_data->return_loaned_value(inner_seq);
    }
}

static void check_nested_sequence(
        soss::ReadableDynamicDataRef soss_data)
{
    for (uint32_t i = 0; i < 3; ++i)
    {
        for (uint32_t j = 0; j < 2; ++j)
        {
            int32_t temp = soss_data[i][j];
            REQUIRE(temp == (j + (i * 2)));
        }
    }
}

static void fill_nested_array(
        soss::WritableDynamicDataRef soss_data)
{
    for (size_t i = 0; i < 4; ++i)
    {
        for (size_t j = 0; j < 5; ++j)
        {
            int32_t temp = j + (i * 5);
            soss_data[i][j] = std::to_string(temp);
        }
    }
}

static void check_nested_array(
        dds::DynamicData* dds_data)
{
    for (uint32_t i = 0; i < 4; ++i)
    {
        for (uint32_t j = 0; j < 5; ++j)
        {
            dds::MemberId idx = dds_data->get_array_index({i, j});
            std::string temp = dds_data->get_string_value(idx);
            REQUIRE(temp == std::to_string(j + (i * 5)));
        }
    }
}

static void check_nested_array(
        soss::ReadableDynamicDataRef soss_data)
{
    for (uint32_t i = 0; i < 4; ++i)
    {
        for (uint32_t j = 0; j < 5; ++j)
        {
            std::string temp = soss_data[i][j];
            REQUIRE(temp == std::to_string(j + (i * 5)));
        }
    }
}

static void fill_mixed_struct(
        soss::DynamicData& soss_data)
{
    using namespace soss;
    const StructType& mixed_type = static_cast<const StructType&>(soss_data.type());
    //sequence<BasicStruct, 3> my_basic_seq;
    const SequenceType& bst = static_cast<const SequenceType&>(mixed_type.member("my_basic_seq").type());
    for (size_t i = 0; i < 3; ++i)
    {
        DynamicData inner(bst.content_type());
        fill_basic_struct(inner);
        soss_data["my_basic_seq"].push(inner);
    }
    //sequence<NestedSequence, 4> my_stseq_seq;
    const SequenceType& sst = static_cast<const SequenceType&>(mixed_type.member("my_stseq_seq").type());
    for (size_t i = 0; i < 4; ++i)
    {
        DynamicData inner(sst.content_type());
        fill_nested_sequence(inner["my_seq_seq"]);
        soss_data["my_stseq_seq"].push(inner);
    }
    //sequence<NestedArray, 5> my_starr_seq;
    const SequenceType& ast = static_cast<const SequenceType&>(mixed_type.member("my_starr_seq").type());
    for (size_t i = 0; i < 5; ++i)
    {
        DynamicData inner(ast.content_type());
        fill_nested_array(inner["my_arr_arr"]);
        soss_data["my_starr_seq"].push(inner);
    }
    //BasicStruct my_basic_arr[3];
    for (size_t i = 0; i < 3; ++i)
    {
        fill_basic_struct(soss_data["my_basic_arr"][i]);
    }
    //NestedSequence my_stseq_arr[4];
    for (size_t i = 0; i < 4; ++i)
    {
        fill_nested_sequence(soss_data["my_stseq_arr"][i]["my_seq_seq"]);
    }
    //NestedArray my_starr_arr[5];
    for (size_t i = 0; i < 5; ++i)
    {
        fill_nested_array(soss_data["my_starr_arr"][i]["my_arr_arr"]);
    }
}

static void check_mixed_struct(
        dds::DynamicData* dds_data)
{
    using namespace dds;
    //sequence<BasicStruct, 3> my_basic_seq;
    {
        DynamicData* member = dds_data->loan_value(dds_data->get_member_id_by_name("my_basic_seq"));
        for (uint32_t i = 0; i < 3; ++i)
        {
            DynamicData* inner_seq = member->loan_value(i);
            check_basic_struct(inner_seq);
            member->return_loaned_value(inner_seq);
        }
        dds_data->return_loaned_value(member);
    }
    //sequence<NestedSequence, 4> my_stseq_seq;
    {
        DynamicData* member = dds_data->loan_value(dds_data->get_member_id_by_name("my_stseq_seq"));
        for (uint32_t i = 0; i < 4; ++i)
        {
            DynamicData* inner_seq = member->loan_value(i);
            DynamicData* inner_member = inner_seq->loan_value(inner_seq->get_member_id_by_name("my_seq_seq"));
            check_nested_sequence(inner_member);
            inner_seq->return_loaned_value(inner_member);
            member->return_loaned_value(inner_seq);
        }
        dds_data->return_loaned_value(member);
    }
    //sequence<NestedArray, 5> my_starr_seq;
    {
        DynamicData* member = dds_data->loan_value(dds_data->get_member_id_by_name("my_starr_seq"));
        for (uint32_t i = 0; i < 5; ++i)
        {
            DynamicData* inner_seq = member->loan_value(i);
            DynamicData* inner_member = inner_seq->loan_value(inner_seq->get_member_id_by_name("my_arr_arr"));
            check_nested_array(inner_member);
            inner_seq->return_loaned_value(inner_member);
            member->return_loaned_value(inner_seq);
        }
        dds_data->return_loaned_value(member);
    }
    //BasicStruct my_basic_arr[3];
    MemberId idx;
    {
        DynamicData* member = dds_data->loan_value(dds_data->get_member_id_by_name("my_basic_arr"));
        for (uint32_t i = 0; i < 3; ++i)
        {
            idx = member->get_array_index({i});
            DynamicData* inner_arr = member->loan_value(idx);
            check_basic_struct(inner_arr);
            member->return_loaned_value(inner_arr);
        }
        dds_data->return_loaned_value(member);
    }
    //NestedSequence my_stseq_arr[4];
    {
        DynamicData* member = dds_data->loan_value(dds_data->get_member_id_by_name("my_stseq_arr"));
        for (uint32_t i = 0; i < 4; ++i)
        {
            idx = member->get_array_index({i});
            DynamicData* inner_arr = member->loan_value(idx);
            DynamicData* inner_member = inner_arr->loan_value(inner_arr->get_member_id_by_name("my_seq_seq"));
            check_nested_sequence(inner_member);
            inner_arr->return_loaned_value(inner_member);
            member->return_loaned_value(inner_arr);
        }
        dds_data->return_loaned_value(member);
    }
    //NestedArray my_starr_arr[5];
    {
        DynamicData* member = dds_data->loan_value(dds_data->get_member_id_by_name("my_starr_arr"));
        for (uint32_t i = 0; i < 5; ++i)
        {
            idx = member->get_array_index({i});
            DynamicData* inner_arr = member->loan_value(idx);
            DynamicData* inner_member = inner_arr->loan_value(inner_arr->get_member_id_by_name("my_arr_arr"));
            check_nested_array(inner_member);
            inner_arr->return_loaned_value(inner_member);
            member->return_loaned_value(inner_arr);
        }
        dds_data->return_loaned_value(member);
    }
}

static void check_mixed_struct(
        soss::ReadableDynamicDataRef soss_data)
{
    //sequence<BasicStruct, 3> my_basic_seq;
    {
        for (uint32_t i = 0; i < 3; ++i)
        {
            check_basic_struct(soss_data["my_basic_seq"][i]);
        }
    }
    //sequence<NestedSequence, 4> my_stseq_seq;
    {
        for (uint32_t i = 0; i < 4; ++i)
        {
            check_nested_sequence(soss_data["my_stseq_seq"][i]["my_seq_seq"]);
        }
    }
    //sequence<NestedArray, 5> my_starr_seq;
    {
        for (uint32_t i = 0; i < 5; ++i)
        {
            check_nested_array(soss_data["my_starr_seq"][i]["my_arr_arr"]);
        }
    }
    //BasicStruct my_basic_arr[3];
    {
        for (uint32_t i = 0; i < 3; ++i)
        {
            check_basic_struct(soss_data["my_basic_arr"][i]);
        }
    }
    //NestedSequence my_stseq_arr[4];
    {
        for (uint32_t i = 0; i < 4; ++i)
        {
            check_nested_sequence(soss_data["my_stseq_arr"][i]["my_seq_seq"]);
        }
    }
    //NestedArray my_starr_arr[5];
    {
        for (uint32_t i = 0; i < 5; ++i)
        {
            check_nested_array(soss_data["my_starr_arr"][i]["my_arr_arr"]);
        }
    }
}

static void fill_union_struct(
        soss::DynamicData& soss_data,
        uint8_t disc)
{
    using namespace soss;
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
    switch(disc)
    {
        case 0:
            soss_data["my_union"]["my_float32"] = 123.456f;
            break;
        case 1:
        case 2:
            soss_data["my_union"]["my_string"] = "Union String";
            break;
        case 3:
            fill_basic_struct(soss_data["my_union"]["abs"]);
            break;
    }
    //map<string, AliasBasicStruct> my_map;
    soss::StringType key_type;
    soss::DynamicData key(key_type);
    key = "Luis";
    fill_basic_struct(soss_data["my_map"][key]);
    key = "Gasco";
    fill_basic_struct(soss_data["my_map"][key]);
    key = "Rulz";
    fill_basic_struct(soss_data["my_map"][key]);
}

static void check_union_struct(
        dds::DynamicData* dds_data)
{
    using namespace dds;
    dds::DynamicData* union_data = dds_data->loan_value(0);
    ::soss::dds::DynamicDataSOSS* dd_soss = static_cast<::soss::dds::DynamicDataSOSS*>(union_data);
    MemberId id = dd_soss->get_union_id();
    uint64_t label = dd_soss->get_union_label();
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
                float v = dd_soss->get_float32_value(id);
                REQUIRE(v == 123.456f);
            }
            break;
        case 1:
        case 2:
            {
                std::string v = dd_soss->get_string_value(id);
                REQUIRE(v == "Union String");
            }
            break;
        case 3:
            {
                dds::DynamicData* v = dd_soss->loan_value(id);
                check_basic_struct(v);
                dd_soss->return_loaned_value(v);
            }
            break;
    }
    dds_data->return_loaned_value(union_data);
    //map<string, AliasBasicStruct> my_map;
    dds::DynamicData* map_data = dds_data->loan_value(1);
    ::soss::dds::DynamicDataSOSS* map_soss = static_cast<::soss::dds::DynamicDataSOSS*>(map_data);
    MemberId keyId = 0;
    MemberId elemId = 1;
    std::string key;
    dds::DynamicData* v;

    uint32_t luis = 0;
    uint32_t gasco = 0;
    uint32_t rulz = 0;
    uint32_t other = 0;

    // map is ordered by hash!
    for (uint32_t i = 0; i < 3; ++i)
    {
        keyId = i * 2;
        elemId = keyId + 1;
        key = map_soss->get_string_value(keyId);
        v = map_soss->loan_value(elemId);

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
        map_soss->return_loaned_value(v);
    }

    REQUIRE(((luis == gasco) && (gasco == rulz) && (rulz == 1u) && (other == 0u)));

    dds_data->return_loaned_value(map_data);
}

static void check_union_struct(
        soss::ReadableDynamicDataRef soss_data)
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
    switch (soss_data["my_union"].d().value<uint8_t>())
    {
        case 0:
            REQUIRE(soss_data["my_union"]["my_float32"].value<float>() == 123.456f);
            break;
        case 1:
        case 2:
            REQUIRE(soss_data["my_union"]["my_string"].value<std::string>() == "Union String");
            break;
        case 3:
            check_basic_struct(soss_data["my_union"]["abs"]);
            break;
    }
    //map<string, AliasBasicStruct> my_map;
    soss::StringType key_type;
    soss::DynamicData key(key_type);
    key = "Luis";
    check_basic_struct(soss_data["my_map"].at(key));
    key = "Gasco";
    check_basic_struct(soss_data["my_map"].at(key));
    key = "Rulz";
    check_basic_struct(soss_data["my_map"].at(key));
}

TEST_CASE("Convert between soss and dds", "[dds]")
{
    static soss::Context context = soss::parse_file(soss_types);
    REQUIRE(context.success);
    std::map<std::string, soss::DynamicType::Ptr> result = context.get_all_types();

    REQUIRE(!result.empty());

    SECTION("basic-type")
    {
        const soss::DynamicType* soss_struct = result["BasicStruct"].get();
        REQUIRE(soss_struct != nullptr);
        // Convert type from soss to dds
        dds::DynamicTypeBuilder* builder = Conversion::create_builder(*soss_struct);
        REQUIRE(builder != nullptr);
        dds::DynamicType_ptr dds_struct = builder->build();
        dds::DynamicData_ptr dds_data_ptr(dds::DynamicDataFactory::get_instance()->create_data(dds_struct));
        dds::DynamicData* dds_data = static_cast<dds::DynamicData*>(dds_data_ptr.get());
        soss::DynamicData soss_data(*soss_struct);
        // Fill soss_data
        fill_basic_struct(soss_data);
        // Convert to dds_data
        Conversion::soss_to_dds(soss_data, dds_data);
        // Check data in dds_data
        check_basic_struct(dds_data);
        // The other way
        soss::DynamicData wayback(*soss_struct);
        Conversion::dds_to_soss(dds_data, wayback);
        check_basic_struct(wayback);
    }

    SECTION("nested-sequences")
    {
        const soss::DynamicType* soss_struct = result["NestedSequence"].get();
        REQUIRE(soss_struct != nullptr);
        // Convert type from soss to dds
        dds::DynamicTypeBuilder* builder = Conversion::create_builder(*soss_struct);
        REQUIRE(builder != nullptr);
        dds::DynamicType_ptr dds_struct = builder->build();
        dds::DynamicData_ptr dds_data_ptr(dds::DynamicDataFactory::get_instance()->create_data(dds_struct));
        dds::DynamicData* dds_data = static_cast<dds::DynamicData*>(dds_data_ptr.get());
        soss::DynamicData soss_data(*soss_struct);
        // Fill soss_data
        fill_nested_sequence(soss_data["my_seq_seq"]);
        // Convert to dds_data
        Conversion::soss_to_dds(soss_data, dds_data);
        // Check data in dds_data
        dds::DynamicData* inner_seq = dds_data->loan_value(dds_data->get_member_id_by_name("my_seq_seq"));
        check_nested_sequence(inner_seq);
        dds_data->return_loaned_value(inner_seq);
        // The other way
        soss::DynamicData wayback(*soss_struct);
        Conversion::dds_to_soss(dds_data, wayback);
        check_nested_sequence(wayback["my_seq_seq"]);
    }

    SECTION("nested-arrays")
    {
        const soss::DynamicType* soss_struct = result["NestedArray"].get();
        REQUIRE(soss_struct != nullptr);
        // Convert type from soss to dds
        dds::DynamicTypeBuilder* builder = Conversion::create_builder(*soss_struct);
        REQUIRE(builder != nullptr);
        dds::DynamicType_ptr dds_struct = builder->build();
        dds::DynamicData_ptr dds_data_ptr(dds::DynamicDataFactory::get_instance()->create_data(dds_struct));
        dds::DynamicData* dds_data = static_cast<dds::DynamicData*>(dds_data_ptr.get());
        soss::DynamicData soss_data(*soss_struct);
        // Fill soss_data
        fill_nested_array(soss_data["my_arr_arr"]);
        // Convert to dds_data
        Conversion::soss_to_dds(soss_data, dds_data);
        // Check data in dds_data
        dds::DynamicData* inner_arr = dds_data->loan_value(dds_data->get_member_id_by_name("my_arr_arr"));
        check_nested_array(inner_arr);
        dds_data->return_loaned_value(inner_arr);
        // The other way
        soss::DynamicData wayback(*soss_struct);
        Conversion::dds_to_soss(dds_data, wayback);
        check_nested_array(wayback["my_arr_arr"]);
    }

    SECTION("mixed")
    {
        const soss::DynamicType* soss_struct = result["MixedStruct"].get();
        REQUIRE(soss_struct != nullptr);
        // Convert type from soss to dds
        dds::DynamicTypeBuilder* builder = Conversion::create_builder(*soss_struct);
        REQUIRE(builder != nullptr);
        dds::DynamicType_ptr dds_struct = builder->build();
        dds::DynamicData_ptr dds_data_ptr(dds::DynamicDataFactory::get_instance()->create_data(dds_struct));
        dds::DynamicData* dds_data = static_cast<dds::DynamicData*>(dds_data_ptr.get());
        soss::DynamicData soss_data(*soss_struct);
        // Fill soss_data
        fill_mixed_struct(soss_data);
        // Convert to dds_data
        Conversion::soss_to_dds(soss_data, dds_data);
        // Check data in dds_data
        check_mixed_struct(dds_data);
        // The other way
        soss::DynamicData wayback(*soss_struct);
        Conversion::dds_to_soss(dds_data, wayback);
        check_mixed_struct(wayback);
    }

    SECTION("union")
    {
        const soss::DynamicType* soss_union = result["MyUnionStruct"].get();
        REQUIRE(soss_union != nullptr);
        // Convert type from soss to dds
        dds::DynamicTypeBuilder* builder = Conversion::create_builder(*soss_union);
        REQUIRE(builder != nullptr);
        dds::DynamicType_ptr dds_struct = builder->build();
        dds::DynamicData_ptr dds_data_ptr(dds::DynamicDataFactory::get_instance()->create_data(dds_struct));
        dds::DynamicData* dds_data = static_cast<dds::DynamicData*>(dds_data_ptr.get());
        soss::DynamicData soss_data(*soss_union);
        {
            // Fill soss_data
            fill_union_struct(soss_data, 0);
            // Convert to dds_data
            Conversion::soss_to_dds(soss_data, dds_data);
            // Check data in dds_data
            check_union_struct(dds_data);
            // The other way
            soss::DynamicData wayback(*soss_union);
            Conversion::dds_to_soss(dds_data, wayback);
            check_union_struct(wayback);
        }
        {
            // Fill soss_data
            fill_union_struct(soss_data, 1);
            // Convert to dds_data
            Conversion::soss_to_dds(soss_data, dds_data);
            // Check data in dds_data
            check_union_struct(dds_data);
            // The other way
            soss::DynamicData wayback(*soss_union);
            Conversion::dds_to_soss(dds_data, wayback);
            check_union_struct(wayback);
        }
        {
            // Fill soss_data
            fill_union_struct(soss_data, 3);
            // Convert to dds_data
            Conversion::soss_to_dds(soss_data, dds_data);
            // Check data in dds_data
            check_union_struct(dds_data);
            // The other way
            soss::DynamicData wayback(*soss_union);
            Conversion::dds_to_soss(dds_data, wayback);
            check_union_struct(wayback);
        }
    }
}
