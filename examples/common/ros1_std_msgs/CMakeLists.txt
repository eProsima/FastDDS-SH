cmake_minimum_required(VERSION 3.5.0)

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    cmake_policy(SET CMP0048 NEW)
endif()

project(std_msgs_ros1)

# Find requirements

# Generate mix
find_package(soss-core REQUIRED)
find_package(soss-genmsg REQUIRED)
find_package(soss-ros1 REQUIRED)

find_package(soss-rosidl REQUIRED)
find_package(std_msgs REQUIRED)

# Use C++14 standard. Required for soss_rosidl_mix
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14)
endif()

soss_genmsg_mix(
    PACKAGES std_msgs
    MIDDLEWARES ros1
)

