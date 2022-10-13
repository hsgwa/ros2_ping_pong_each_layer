cmake_minimum_required(VERSION 3.5)
project(rclcpp_pubsub)

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
find_package(rclcpp REQUIRED)
find_package(tlsf_cpp REQUIRED)
find_package(message REQUIRED)
find_package(rttest REQUIRED)
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

add_library(rclcpp_pubsub src/rclcpp_pubsub.cpp)
# target_include_directories(rclcpp_pubsub PUBLIC include)
ament_target_dependencies(rclcpp_pubsub rclcpp message tlsf_cpp rttest layer_pubsub_common)

# set(LISTENER listener_rclcpp)
# add_executable(${LISTENER} src/rclcpp_minimum_subscriber.cpp)
# ament_target_dependencies(${LISTENER} rclcpp message std_msgs)
ament_export_targets(export_rclcpp_pubsub HAS_LIBRARY_TARGET)
ament_export_dependencies(ament_cmake rclcpp message)

ament_export_include_directories(include)
ament_export_libraries(rclcpp_pubsub)


install(
  DIRECTORY include/
  DESTINATION include
  )

install(
  TARGETS rclcpp_pubsub
  EXPORT export_rclcpp_pubsub
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

# install(TARGETS
#   rclcpp_pubsub
#   ${TALKER}
#   ${LISTENER}
#   DESTINATION lib/${PROJECT_NAME})

ament_package()
