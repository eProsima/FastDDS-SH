cmake_minimum_required(VERSION 3.5.0)

project(soss_dds_example_types)

# Get SOSS dependencies
find_package(soss-core REQUIRED)
find_package(soss-rosidl REQUIRED)
find_package(soss-ros2 REQUIRED)

# Get message dependencies
find_package(std_msgs REQUIRED)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14)
endif()

soss_rosidl_mix(
    PACKAGES std_msgs
    MIDDLEWARES ros2
    )
