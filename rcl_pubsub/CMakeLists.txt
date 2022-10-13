cmake_minimum_required(VERSION 3.5)
project(rcl_pubsub)

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
find_package(rosidl_typesupport_cpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rmw REQUIRED)
find_package(message REQUIRED)
find_package(layer_pubsub_common REQUIRED)

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

add_library(rcl_pubsub src/rcl_pubsub.cpp)
ament_target_dependencies(rcl_pubsub rcl message layer_pubsub_common)


ament_export_targets(export_rcl_pubsub HAS_LIBRARY_TARGET)
ament_export_dependencies(ament_cmake std_msgs message)

ament_export_include_directories(include)
ament_export_libraries(rcl_pubsub)

install(
  DIRECTORY include/
  DESTINATION include
  )

install(
  TARGETS rcl_pubsub
  EXPORT export_rcl_pubsub
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
  )

# install(TARGETS
#   ${TALKER}
#   ${LISTENER}
#   DESTINATION lib/${PROJECT_NAME})

ament_package()
