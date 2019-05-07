#
#  Copyright (C) 2018 Open Source Robotics Foundation
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

cmake_minimum_required(VERSION 3.5.0)

project(soss-dds)

find_package(soss-core REQUIRED)

option (THIRDPARTY "Activate internal build of dependencies" ON)
add_subdirectory(thirdparty/fastrtps)

# Library
set(SRC
    src/Conversion.cpp
    src/Publisher.cpp
    src/Subscriber.cpp
    src/Participant.cpp
    src/SystemHandle.cpp
    )

add_library(${PROJECT_NAME} SHARED ${SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wextra -Wpedantic)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include
    )

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        soss::core
        fastrtps
)

# Install
soss_install_middleware_plugin(
    MIDDLEWARE dds
    TARGET soss-dds
    NO_CONFIG
)

