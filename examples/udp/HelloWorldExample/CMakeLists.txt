cmake_minimum_required(VERSION 3.5.0)

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    cmake_policy(SET CMP0048 NEW)
endif()

project(HelloWorldExample)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT foonathan_memory_FOUND)
    find_package(foonathan_memory REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

# Generate mix
find_package(soss-core REQUIRED)
find_package(soss-mock REQUIRED)
find_package(soss-ros2 REQUIRED)

find_package(soss-rosidl REQUIRED)
find_package(std_msgs REQUIRED)

# Use C++14 standard. Required for soss_rosidl_mix
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14)
endif()

soss_rosidl_mix(
    PACKAGES std_msgs
    MIDDLEWARES ros2
)

message(STATUS "Configuring HelloWorld example...")
file(GLOB HELLOWORLD_EXAMPLE_SOURCES_CXX "*.cxx")
file(GLOB HELLOWORLD_EXAMPLE_SOURCES_CPP "*.cpp")

add_executable(HelloWorldExample ${HELLOWORLD_EXAMPLE_SOURCES_CXX} ${HELLOWORLD_EXAMPLE_SOURCES_CPP})
target_link_libraries(HelloWorldExample fastrtps fastcdr foonathan_memory
    ${std_msgs_LIBRARIES})
install(TARGETS HelloWorldExample
    RUNTIME DESTINATION examples/C++/HelloWorldExample/${BIN_INSTALL_DIR})
