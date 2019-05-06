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

#include "Participant.hpp"
#include "Publisher.hpp"
#include "Conversion.hpp"

#include <iostream>

namespace eprosima {
namespace fastrtps {
    class Subscriber;
}
}


namespace soss {
namespace dds {


Publisher::Publisher(
        Participant* /* participant */,
        const std::string& topic_name,
        const std::string& message_type)

    : topic_name_{topic_name}
    , message_type_{message_type}
{
    //TODO
}

bool Publisher::publish(
        const soss::Message& soss_message)
{
    std::cout << "[soss-dds][publisher]: translate message: soss -> dds "
        "(" << topic_name_ << ") " << std::endl;

    //soss::Message dds_message =
    Conversion::soss_to_dds(soss_message);

    //TODO: send by dds

    return true;
}


} // namespace dds
} // namespace soss
