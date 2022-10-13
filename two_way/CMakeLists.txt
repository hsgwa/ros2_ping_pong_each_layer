cmake_minimum_required(VERSION 3.5)
project(two_way)

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
find_package(ament_cmake REQUIRED)
find_package(message REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rttest REQUIRED)
find_package(rcl_pubsub REQUIRED)
find_package(rmw_pubsub REQUIRED)
find_package(fastrtps_pubsub REQUIRED)
find_package(rclcpp_pubsub REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


set(CMAKE_VERBOSE_MAKEFILE ON)
add_compile_options(-O0 -rdynamic)
set (CMAKE_CXX_FLAGS "-rdynamic")

# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

find_package(osrf_testing_tools_cpp REQUIRED)
osrf_testing_tools_cpp_require_googletest(VERSION_GTE 1.8)

add_executable(two_way src/two_way.cpp)
target_include_directories(two_way PUBLIC
  include
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  "${rclcpp_pubsub_INCLUDE_DIRS}"
  "${rcl_pubsub_INCLUDE_DIRS}"
  "${rmw_pubsub_INCLUDE_DIRS}"
  "${fastrtps_pubsub_INCLUDE_DIRS}"
  )
ament_target_dependencies(two_way
  message
  rclcpp_pubsub rcl_pubsub rttest rmw_pubsub fastrtps_pubsub
  )
target_link_libraries(two_way
  ${rcl_pubsub_LIBRARIES}
  ${rmw_pubsub_LIBRARIES}
  ${fastrtps_pubsub_LIBRARIES}
  Threads::Threads
  osrf_testing_tools_cpp::memory_tools
  )
# target_link_directories(two_way PUBLIC
#   lib/rclcpp_pubsub
#   )
# target_link_libraries(two_way rclcpp_pubsub)

# install(TARGETS two_way
#   EXPORT export_${PROJECT_NAME}
#   DESTINATION lib/${PROJECT_NAME})

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

ament_package()
