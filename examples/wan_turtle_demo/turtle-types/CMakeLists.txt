cmake_minimum_required(VERSION 3.5.0)

project(soss_turtle_types)

find_package(ament_cmake REQUIRED)
find_package(soss-genmsg REQUIRED)
find_package(soss-rosidl REQUIRED)

if(NOT CMAKE_CXX_STANDARD)
  # TODO(MXG): Remove this block and use target_compile_features(~)
  # instead when we no longer need to support Ubuntu 16.04.
  set(CMAKE_CXX_STANDARD 14)
endif()

find_package(geometry_msgs REQUIRED)

soss_rosidl_mix(
    PACKAGES geometry_msgs
    MIDDLEWARES ros2
)

ament_package()
