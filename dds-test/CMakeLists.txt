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
find_package(soss-core REQUIRED)
find_package(xtypes REQUIRED)
find_package(soss-mock REQUIRED)
find_package(fastrtps REQUIRED)

if(WIN32)
    find_file(MOCKDLL NAMES "soss-mock.dll" PATHS "${soss-mock_DIR}" PATH_SUFFIXES "lib" ) #pending to check
    install (FILES ${MOCKDLL} DESTINATION "${CMAKE_INSTALL_PREFIX}/../bin") #pending to check
    find_package(yaml-cpp)
endif()

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

option(CRYSTAL "Test against ROS2 Crystal" OFF)
if(CRYSTAL)
    configure_file(
        ${PROJECT_SOURCE_DIR}/resources/tcp_config_crystal.xml
        ${PROJECT_BINARY_DIR}/resources/tcp_config.xml
        COPYONLY
        )
else()
    configure_file(
        ${PROJECT_SOURCE_DIR}/resources/tcp_config_dashing.xml
        ${PROJECT_BINARY_DIR}/resources/tcp_config.xml
        COPYONLY
        )
endif()

add_executable(${PROJECT_NAME}_conversion
    src/main.cpp
    src/unitary/conversion.cpp
    ../dds/src/Conversion.cpp
)

set_target_properties(${PROJECT_NAME}_conversion
    PROPERTIES
        CXX_STANDARD 14
        POSITION_INDEPENDENT_CODE ON
)

target_compile_options(${PROJECT_NAME}_conversion
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall -Wextra -Wpedantic>
)

target_include_directories(${PROJECT_NAME}_conversion
    PRIVATE
        "${PROJECT_SOURCE_DIR}/thirdparty/catch2/include"
        "${PROJECT_SOURCE_DIR}/../dds/src"
)

target_link_libraries(${PROJECT_NAME}_conversion
    PRIVATE
        fastrtps
        soss::core
)

configure_file(
    ${PROJECT_SOURCE_DIR}/src/unitary/resources/soss_types.idl
    ${PROJECT_BINARY_DIR}/soss_types.idl
    COPYONLY
    )

catch_discover_tests(${PROJECT_NAME})
catch_discover_tests(${PROJECT_NAME}_conversion)
