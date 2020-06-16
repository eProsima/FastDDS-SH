cmake_minimum_required(VERSION 3.5)
project(hello_type)

set(hello_type_MAJOR_VERSION 0)
set(hello_type_MINOR_VERSION 0)
set(hello_type_PATCH_VERSION 0)
set(hello_type_VERSION
  ${hello_type_MAJOR_VERSION}.${hello_type_MINOR_VERSION}.${hello_type_PATCH_VERSION})
# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

#find_package(ament_cmake REQUIRED)
#find_package(rosidl_default_generators REQUIRED)

#rosidl_generate_interfaces(${PROJECT_NAME}
#  "msg/String.msg"
#  )

#ament_package()

find_package(soss-rosidl REQUIRED)
find_package(std_msgs REQUIRED)

soss_rosidl_mix(
    PACKAGES std_msgs
    MIDDLEWARES ros2
)
