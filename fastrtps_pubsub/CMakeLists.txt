cmake_minimum_required(VERSION 3.5)
project(fastrtps_pubsub)

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

# find dependencies
# see src/ros2/rmw_fastrtps/rmw_fastrtps_cpp/CMakeLists.txt
find_package(ament_cmake REQUIRED)
find_package(std_msgs REQUIRED)
find_package(fastrtps_cmake_module REQUIRED)
find_package(rosidl_generator_c REQUIRED)  # for get_message_typesupport_handle() etc
find_package(fastcdr REQUIRED CONFIG)
find_package(fastrtps REQUIRED CONFIG)
find_package(FastRTPS REQUIRED MODULE)
find_package(rosidl_typesupport_fastrtps_cpp REQUIRED)  # for message_type_support_callbacks_t etc
find_package(rosidl_typesupport_fastrtps_c REQUIRED)  # for message_type_support_callbacks_t etc
find_package(rmw_fastrtps_cpp REQUIRED)  # for MessageTypeSupport
find_package(rmw_fastrtps_shared_cpp REQUIRED)  # for MessageTypeSupport

# FastRTPS is needed as in src/ros2/rmw_fastrtps/rmw_fastrtps_cpp/rmw_fastrtps_shared_cpp-extras.cmake.
# fastcdr, rmw_fastrtps_cpp are added in try-and-error.
include_directories(${FastRTPS_INCLUDE_DIR} ${fastcdr_INCLUDE_DIR} ${rmw_fastrtps_cpp_INCLUDE_DIR})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # uncomment the line when a copyright and license is not present in all source files
  #set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # uncomment the line when this package is not in a git repo
  #set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

set(TALKER "talker_fastrtps")
add_executable(${TALKER}
  src/fastrtps_minimum_publisher.cpp
  src/my_message_type_support.cpp)
ament_target_dependencies(${TALKER}
  "std_msgs"
  "fastrtps"
  "fastcdr"
  "FastRTPS"
  "rmw_fastrtps_cpp"
  "rmw_fastrtps_shared_cpp"
  "rosidl_typesupport_fastrtps_c"
  "rosidl_typesupport_fastrtps_cpp"
  "rosidl_generator_c"
  )

ament_package()
