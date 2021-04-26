cmake_minimum_required(VERSION 3.5.0)

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    cmake_policy(SET CMP0048 NEW)
endif()

project(std_msgs_ros2)

# Generate mix
find_package(is-core REQUIRED)
find_package(is-ros2 REQUIRED)
find_package(is-ros2-rosidl REQUIRED)
find_package(std_msgs REQUIRED)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

is_ros2_rosidl_mix(
    PACKAGES std_msgs
    MIDDLEWARES ros2
)

