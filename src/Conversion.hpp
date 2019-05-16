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

#ifndef SOSS__DDS__INTERNAL__CONVERSION_HPP
#define SOSS__DDS__INTERNAL__CONVERSION_HPP

#include "DDSMiddlewareException.hpp"
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <soss/Message.hpp>

namespace soss {
namespace dds {

namespace types = eprosima::fastrtps::types;

struct Conversion {


    static bool soss_to_dds(
            const soss::Message& input,
            types::DynamicData_ptr output);
    static bool dds_to_soss(
            const std::string type,
            const types::DynamicData_ptr input,
            soss::Message& output);

private:
    ~Conversion() = default;
};


} // namespace dds
} // namespace soss

#endif // SOSS__DDS__INTERNAL__CONVERSION_HPP

