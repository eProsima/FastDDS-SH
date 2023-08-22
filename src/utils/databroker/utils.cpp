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

#include "Participant.hpp"
#include "DDSMiddlewareException.hpp"
#include "Conversion.hpp"
#include "utils/databroker/utils.hpp"

#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>

#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <sstream>

namespace eprosima {
namespace is {
namespace sh {
namespace fastdds {
namespace utils {

eprosima::fastrtps::rtps::GuidPrefix_t guid_server(
        uint8_t id)
{
    eprosima::fastrtps::rtps::GuidPrefix_t guid;
    std::istringstream(SERVER_DEFAULT_GUID) >> guid;
    guid.value[SERVER_DEFAULT_GUID_ID_INDEX] = static_cast<unsigned char>(id);
    return guid;
}

eprosima::fastrtps::rtps::GuidPrefix_t guid_server(
        const YAML::Node& server_id,
        const YAML::Node& server_guid)
{
    if (server_guid)
    {
        // Server GUID set and used
        eprosima::fastrtps::rtps::GuidPrefix_t guid;
        std::istringstream(server_guid.as<std::string>()) >> guid;
        return guid; // There is no easy wat to directly return the guid
    }
    else if (server_id && !server_guid)
    {
        // Server ID set without GUID set
        return guid_server(server_id.as<uint32_t>() % std::numeric_limits<uint8_t>::max());
    }
    else
    {
        // Server GUID by default with ID 0
        return guid_server(0);
    }
}

std::string guid_to_string(
    const eprosima::fastrtps::rtps::GuidPrefix_t& guid)
{
    std::ostringstream guid_ostream;
    guid_ostream << guid;
    return guid_ostream.str();
}

} //  namespace utils
} //  namespace fastdds
} //  namespace sh
} //  namespace is
} //  namespace eprosima
