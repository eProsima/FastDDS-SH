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

project(soss-dds-test)

find_package(soss-dds REQUIRED)
find_package(soss-mock REQUIRED)
find_package(yaml-cpp)

include(CTest)
include(${PROJECT_SOURCE_DIR}/thirdparty/catch2/cmake/Catch.cmake)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/integration/roundtrip.cpp
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 14
        POSITION_INDEPENDENT_CODE ON
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall -Wextra -Wpedantic>
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        "${PROJECT_SOURCE_DIR}/thirdparty/catch2/include"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        soss::mock
        $<$<PLATFORM_ID:Windows>:yaml-cpp>
)


file(COPY ${PROJECT_SOURCE_DIR}/resources/tcp_config.xml
    DESTINATION ${PROJECT_BINARY_DIR}/resources
    )

catch_discover_tests(${PROJECT_NAME})
