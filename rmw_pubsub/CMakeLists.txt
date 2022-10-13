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
find_package(message REQUIRED)
find_package(layer_pubsub_common REQUIRED)

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
include_directories(include)

add_library(rmw_pubsub src/rmw_pubsub.cpp)
ament_target_dependencies(rmw_pubsub rmw message rcl rmw_implementation layer_pubsub_common)

ament_export_targets(export_rmw_pubsub HAS_LIBRARY_TARGET)
ament_export_dependencies(ament_cmake rmw std_msgs message rcl rmw_implementation)

ament_export_include_directories(include)
ament_export_libraries(rmw_pubsub)

install(
  DIRECTORY include/
  DESTINATION include)


install(
  TARGETS rmw_pubsub
  EXPORT export_rmw_pubsub
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
  )

# set(TALKER "talker_rmw")
# add_executable(${TALKER} src/rmw_minimum_publisher.cpp)
# ament_target_dependencies(${TALKER}
#   "rcl"
#   "std_msgs"
#   "rmw_implementation"
#   "rmw"
#   message)

# set(LISTENER "listener_rmw")
# add_executable(${LISTENER} src/rmw_minimum_subscriber.cpp)
# ament_target_dependencies(${LISTENER}
#   "rcl"
#   "std_msgs"
#   "rmw_implementation"
#   "rmw"
#   message)

# install(TARGETS
#   ${TALKER}
#   ${LISTENER}
#   DESTINATION lib/${PROJECT_NAME})

ament_package()
