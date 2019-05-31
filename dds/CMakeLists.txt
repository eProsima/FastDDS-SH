#
#  Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

########################################################################
# Find Asio
find_package(fastrtps QUIET)

if(fastrtps_DIR)
    message(STATUS "Found a fastrtps")
else()
    message(STATUS "Using the fastrtps thirdparty")
    option (THIRDPARTY "Activate internal build of dependencies" ON)
    add_subdirectory(thirdparty/fastrtps)
endif()

########################################################################
# Library
set(SRC_DTPARSER
    src/dtparser/DynamicTypeParser.cpp
    src/dtparser/DynamicTypeParserCommon.cpp
    src/dtparser/YAMLParser.cpp
    )

set(SRC
    src/Conversion.cpp
    src/Publisher.cpp
    src/Subscriber.cpp
    src/Participant.cpp
    src/SystemHandle.cpp
    )

add_library(${PROJECT_NAME} SHARED ${SRC_DTPARSER} ${SRC})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 14
        POSITION_INDEPENDENT_CODE ON
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Werror -Wall -Wextra -Wpedantic
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        soss::core
        fastrtps
)

########################################################################
# Installation
soss_install_middleware_plugin(
    MIDDLEWARE dds
    TARGET soss-dds
)

install(
    FILES ${PROJECT_SOURCE_DIR}/sample/tcp/config.xml
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/../sample/tcp
    COMPONENT soss-dds
    )

