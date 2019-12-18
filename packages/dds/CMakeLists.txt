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

include(ExternalProject)

find_package(soss-core REQUIRED)
find_package(xtypes REQUIRED)
if(WIN32)
    find_package(yaml-cpp)
endif()

########################################################################
# Library preparation
set(SRC
    src/Conversion.cpp
    src/Publisher.cpp
    src/Subscriber.cpp
    src/Participant.cpp
    src/SystemHandle.cpp
    )

add_library(${PROJECT_NAME} SHARED ${SRC})

########################################################################
# Testing preparation
option(SOSS-DDS_BUILD_TESTS "Build unitary tests." OFF)

if (SOSS-DDS_BUILD_TESTS)
    include(CTest)
    include(../dds-test/thirdparty/catch2/cmake/Catch.cmake)

    add_executable(${PROJECT_NAME}_test_conversion
        test/main.cpp
        test/unitary/conversion.cpp
        src/Conversion.cpp
    )

    set_target_properties(${PROJECT_NAME}_test_conversion
        PROPERTIES
            CXX_STANDARD 14
            POSITION_INDEPENDENT_CODE ON
    )

    target_compile_options(${PROJECT_NAME}_test_conversion
        PRIVATE
            $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall -Wextra -Wpedantic>
    )

    target_include_directories(${PROJECT_NAME}_test_conversion
        PRIVATE
            "../dds-test/thirdparty/catch2/include"
            "src"
    )

    configure_file(
        test/unitary/resources/soss_types.idl
        ${PROJECT_BINARY_DIR}/soss_types.idl
        COPYONLY
        )

    catch_discover_tests(${PROJECT_NAME}_test_conversion)
endif()

########################################################################
# Find FastRTPS
find_package(fastrtps QUIET)

if(fastrtps_DIR)
    message(STATUS "Found a fastrtps")
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            soss::core
            fastrtps
            $<$<PLATFORM_ID:Windows>:yaml-cpp>
    )
    if (SOSS-DDS_BUILD_TESTS)
        target_link_libraries(${PROJECT_NAME}_test_conversion
            PRIVATE
                fastrtps
                soss::core
        )
    endif()

else()
    message(STATUS "Using the fastrtps as external project")
    # Fast-RTPS needs foonathan too
    unset(foonathan_memory_DIR CACHE) # Reviewer! Do we need to clear CACHE? Copied from uXRCE-DDS-Agent
    find_package(foonathan_memory QUIET)
    if(NOT foonathan_memory_FOUND)
        ExternalProject_Add(foonathan_memory
            GIT_REPOSITORY
                https://github.com/foonathan/memory.git
            GIT_TAG
                master # ${_foonathan_memory_tag} (set(_foonathan_memory_tag master))
            PREFIX
                ${PROJECT_BINARY_DIR}/foonathan_memory
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install/foonathan_memory
            CMAKE_CACHE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DFOONATHAN_MEMORY_BUILD_EXAMPLES:BOOL=OFF
                -DFOONATHAN_MEMORY_BUILD_TESTS:BOOL=OFF
                -DFOONATHAN_MEMORY_BUILD_TOOLS:BOOL=ON
                -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
            )
    endif()

    # Fast RTPS.
    unset(fastrtps_DIR CACHE) # Reviewer! Do we need to clear CACHE? Copied from uXRCE-DDS-Agent
    # find_package(fastrtps ${_fastrtps_version} EXACT QUIET) #TODO
    if(NOT fastrtps_FOUND)
        ExternalProject_Add(fastrtps
            GIT_REPOSITORY
                https://github.com/eProsima/Fast-RTPS.git
            GIT_TAG
                v1.9.3 #${_fastrtps_tag}
            PREFIX
                ${PROJECT_BINARY_DIR}/fastrtps
            INSTALL_DIR
                ${PROJECT_BINARY_DIR}/temp_install/fastrtps
            CMAKE_CACHE_ARGS
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH};${PROJECT_BINARY_DIR}/temp_install
                -DBUILD_SHARED_LIBS:BOOL=ON
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
                -DTHIRDPARTY:BOOL=ON
                -DSECURITY:BOOL=OFF #${SOSS-DDS_SECURITY_PROFILE}
            DEPENDS
                foonathan_memory
            )

        link_directories(${PROJECT_BINARY_DIR}/temp_install/fastrtps/lib)
        include_directories(${PROJECT_BINARY_DIR}/temp_install/fastrtps/include)
        add_library(libfastrtps IMPORTED SHARED GLOBAL)
        add_library(libfastcdr IMPORTED SHARED GLOBAL)
        add_dependencies(libfastrtps fastrtps)

        set_target_properties(libfastrtps
            PROPERTIES
                "IMPORTED_LOCATION" "${PROJECT_BINARY_DIR}/temp_install/fastrtps/lib/libfastrtps.so"
                "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
            )

        set_target_properties(libfastcdr
            PROPERTIES
                "IMPORTED_LOCATION" "${PROJECT_BINARY_DIR}/temp_install/fastrtps/lib/libfastcdr.so"
                "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
            )

       target_link_libraries(${PROJECT_NAME}
            PRIVATE
                soss::core
                libfastrtps
                $<$<PLATFORM_ID:Windows>:yaml-cpp>
            )

        if (SOSS-DDS_BUILD_TESTS)
            target_link_libraries(${PROJECT_NAME}_test_conversion
                PRIVATE
                    libfastcdr
                    libfastrtps
                    soss::core
            )
        endif()

        install(
            DIRECTORY ${PROJECT_BINARY_DIR}/temp_install/fastrtps/lib/
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT soss-dds
            )

    endif()
endif()

########################################################################
# Library continuation
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
        include
)

########################################################################
# Installation
soss_install_middleware_plugin(
    MIDDLEWARE dds
    TARGET soss-dds
)

install(
    FILES ${PROJECT_SOURCE_DIR}/../../examples/tcp/config.xml
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/../sample/tcp
    COMPONENT soss-dds
    )

