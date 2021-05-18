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

#ifndef _IS_SH_FASTDDS__INTERNAL__DDSMIDDLEWAREEXCEPTION_HPP_
#define _IS_SH_FASTDDS__INTERNAL__DDSMIDDLEWAREEXCEPTION_HPP_

#include <stdexcept>

#include <is/utils/Log.hpp>

namespace eprosima {
namespace is {
namespace sh {
namespace fastdds {

/**
 * @class DDSMiddlewareException
 *        Launches a runtime error every time an unexpected behaviour occurs
 *        related to *Fast DDS* middleware, when configuring or using this is::SystemHandle.
 */
class DDSMiddlewareException : public std::runtime_error
{
public:

    /**
     * @brief Construct a new DDSMiddlewareException object.
     *
     * @param[in] logger The logging tool.
     *
     * @param[in] message The message to throw the runtime error with.
     */
    DDSMiddlewareException(
            const utils::Logger& logger,
            const std::string& message)
        : std::runtime_error(message)
        , from_logger(logger)
    {
    }

    utils::Logger from_logger;
};

} //  namespace fastdds
} //  namespace sh
} //  namespace is
} //  namespace eprosima

#endif //  _IS_SH_FASTDDS__INTERNAL__DDSMIDDLEWAREEXCEPTION_HPP_
