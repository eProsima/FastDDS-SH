/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include "TopicType.hpp"

#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicDataFactory.h>

#include <iostream>

namespace soss {
namespace dds {


TopicType::TopicType(const std::string& name)
{
    eprosima::fastrtps::types::DynamicTypeBuilder_ptr string_builder
        = eprosima::fastrtps::types::DynamicTypeBuilderFactory::GetInstance()->CreateStringBuilder();
    eprosima::fastrtps::types::DynamicTypeBuilder_ptr struct_builder
        = eprosima::fastrtps::types::DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();

    struct_builder->AddMember(0, "message", string_builder.get());
    struct_builder->SetName(name);

    dynamic_type_ = struct_builder->Build();

    std::cout << dynamic_type_->GetName() << std::endl;

    pub_sub_type_.SetDynamicType(dynamic_type_);
}

eprosima::fastrtps::types::DynamicData_ptr TopicType::create_dynamic_data() const
{
    return eprosima::fastrtps::types::DynamicDataFactory::GetInstance()->CreateData(dynamic_type_);
}

} //namespace dds
} //namespace soss

