# Copyright 2019 Open Source Robotics Foundation, Inc.
# Copyright (C) 2020 - present Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# is-fastdds SystemHandle library unitary tests project

#########################################################################################
# CMake build rules for the Integration Service Fast DDS SystemHandle  tests library
#########################################################################################
include(CTest)
include(${IS_GTEST_CMAKE_MODULE_DIR}/gtest.cmake)
enable_testing()

#########################################################################################
# Unitary tests
#########################################################################################
add_executable(${PROJECT_NAME}-unit-test
    unitary/conversion.cpp
)

set_target_properties(${PROJECT_NAME}-unit-test PROPERTIES
    VERSION
        ${PROJECT_VERSION}
    SOVERSION
        ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    CXX_STANDARD
        17
    CXX_STANDARD_REQUIRED
        YES
    )


target_link_libraries(${PROJECT_NAME}-unit-test
    PRIVATE
        $<IF:$<BOOL:${IS_FASTDDS_SH_USING_FASTDDS_EXTERNALPROJECT}>,libfastrtps,fastrtps>
        is-fastdds
        yaml-cpp
    PUBLIC
        $<IF:$<BOOL:${IS_GTEST_EXTERNAL_PROJECT}>,libgtest,gtest>
    )

target_compile_options(${PROJECT_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall -Wextra -Wpedantic>
    )

target_include_directories(${PROJECT_NAME}-unit-test
    PRIVATE
        $<TARGET_PROPERTY:${PROJECT_NAME},INTERFACE_INCLUDE_DIRECTORIES>
    )

configure_file(
    unitary/resources/fastdds_sh_unit_test_types.idl
    ${PROJECT_BINARY_DIR}/test/fastdds_sh_unit_test_types.idl
    )

add_gtest(${PROJECT_NAME}-unit-test SOURCES unitary/conversion.cpp)

#########################################################################################
# Integration tests
#########################################################################################
find_package(is-mock REQUIRED)

add_executable(${PROJECT_NAME}-integration-test
    integration/roundtrip.cpp
    integration/resources/test_service/test_service.cxx
    integration/resources/test_service/test_servicePubSubTypes.cxx
    )

set_target_properties(${PROJECT_NAME}-integration-test PROPERTIES
    VERSION
        ${PROJECT_VERSION}
    SOVERSION
        ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    CXX_STANDARD
        17
    CXX_STANDARD_REQUIRED
        YES
    )

target_compile_options(${PROJECT_NAME}-integration-test
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall -Wextra -Wpedantic>
    )

target_include_directories(${PROJECT_NAME}-integration-test
    PRIVATE
        $<TARGET_PROPERTY:${PROJECT_NAME},INTERFACE_INCLUDE_DIRECTORIES>
    )

target_link_libraries(${PROJECT_NAME}-integration-test
    PRIVATE
        is::mock
        $<IF:$<BOOL:${IS_FASTDDS_SH_USING_FASTDDS_EXTERNALPROJECT}>,libfastrtps,fastrtps>
        $<IF:$<BOOL:${IS_FASTDDS_SH_USING_FASTDDS_EXTERNALPROJECT}>,libfastcdr,fastcdr>
        $<$<PLATFORM_ID:Windows>:yaml-cpp>
    PUBLIC
        $<IF:$<BOOL:${IS_GTEST_EXTERNAL_PROJECT}>,libgtest,gtest>
    )


option(CRYSTAL "Test against ROS2 Crystal" OFF)
if(CRYSTAL)
    configure_file(
        integration/resources/tcp_config_crystal.xml
        ${PROJECT_BINARY_DIR}/test/resources/tcp_config.xml
        COPYONLY
        )
else()
    configure_file(
        integration/resources/tcp_config_dashing.xml
        ${PROJECT_BINARY_DIR}/test/resources/tcp_config.xml
        COPYONLY
        )
endif()

configure_file(
    integration/resources/test_service.idl
    ${PROJECT_BINARY_DIR}/test/test_service.idl
    COPYONLY
    )

add_gtest(${PROJECT_NAME}-integration-test
    SOURCES
        integration/roundtrip.cpp
    )