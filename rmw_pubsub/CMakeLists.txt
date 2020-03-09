cmake_minimum_required(VERSION 3.5)
project(rmw_pubsub)

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
find_package(rcl REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rmw REQUIRED)
find_package(rmw_implementation REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

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

set(TALKER "talker_rmw")
add_executable(${TALKER} src/rmw_minimum_publisher.cpp)
ament_target_dependencies(${TALKER}
  "rcl"
  "std_msgs"
  "rmw_implementation"
  "rmw")

set(LISTENER "listener_rmw")
add_executable(${LISTENER} src/rmw_minimum_subscriber.cpp)
ament_target_dependencies(${LISTENER}
  "rcl"
  "std_msgs"
  "rmw_implementation"
  "rmw")

install(TARGETS
  ${TALKER}
  ${LISTENER}
  DESTINATION lib/${PROJECT_NAME})

ament_package()