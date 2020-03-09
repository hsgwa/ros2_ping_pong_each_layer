#include <iostream>
#include <chrono>
#include <thread>

#include "rcl/rcl.h"
#include "rcl/remap.h"
#include "rcl/arguments.h"
#include "rcl/error_handling.h"
#include "std_msgs/msg/string.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"
#include "rmw/node_security_options.h"
#include "rmw/error_handling.h"
#include "rmw/validate_full_topic_name.h"

#define CHECK_RCL_RET(val, fname) if (RCL_RET_OK != val) { \
    std::cout << fname << " fail: " << rcl_get_error_string().str << std::endl; \
    return -1; \
  }
#define CHECK_RMW_RET(val, fname) if (RMW_RET_OK != val) { \
    std::cout << fname << " fail: " << rmw_get_error_string().str << std::endl; \
    return -1; \
  }
#define CHECK_VALIDATE(val, where)   if (RMW_NODE_NAME_VALID != val) {        \
    const char * msg = rmw_node_name_validation_result_string(validation_result); \
    std::cout << where << " fail: " << msg << std::endl; \
    return RCL_RET_NODE_INVALID_NAME; \
  } \


int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  // configuration
  const char * local_namespace = "/ns";
  const char * node_name = "node_name";
  const char * topic_name = "/ns/topic";  // "/ns" should be equal to "local_namespace"

  // global resources
  rmw_ret_t rmw_ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();

  ////////////////////////////////
  // init
  // rcl_init_options_init in rcl/.../init.c
  ////////////////////////////////
  uint64_t next_instance_id = 0;
  rmw_init_options_t rmw_init_options = rmw_get_zero_initialized_init_options();
  rmw_ret = rmw_init_options_init(&rmw_init_options, allocator);

  rmw_init_options.instance_id = next_instance_id;
  rmw_context_t rmw_context = rmw_get_zero_initialized_context();
  rmw_ret = rmw_init(&rmw_init_options, &rmw_context);
  CHECK_RMW_RET(rmw_ret, "rmw_init");

  ////////////////////////////////
  // create node
  // rcl_get_zero_initialized_node in rcl/.../node.c
  ////////////////////////////////
  // Skip validation  as local_namespace must begin "/" when rmw_create_node(),
  /*
    int validation_result = 0;
    rmw_ret = rmw_validate_node_name(node_name, &validation_result, NULL);
    CHECK_RMW_RET(rmw_ret, "rmw_validate_node_name");
    CHECK_VALIDATE(validation_result, "node_name");

    rmw_ret = rmw_validate_namespace(local_namespace, &validation_result, NULL);
    CHECK_RMW_RET(rmw_ret, "rmw_validate_namespace");
    CHECK_VALIDATE(validation_result, "namespace");
  */

  size_t domain_id = 0;
  rmw_node_security_options_t node_security_options =
      rmw_get_zero_initialized_node_security_options();
  node_security_options.enforce_security = RMW_SECURITY_ENFORCEMENT_PERMISSIVE;
  rmw_node_t *rmw_node = rmw_create_node(
      &rmw_context,
      node_name, local_namespace, domain_id, &node_security_options, true);

  const rmw_guard_condition_t * rmw_graph_guard_condition =
      rmw_node_get_graph_guard_condition(rmw_node);

  ////////////////////////////////
  // init publisher
  // rcl_publisher_init in rcl/.../publisher.c
  ////////////////////////////////
  const rosidl_message_type_support_t *ts =
    rosidl_typesupport_cpp::get_message_type_support_handle<std_msgs::msg::String>();

  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  qos_profile.liveliness = RMW_QOS_POLICY_LIVELINESS_AUTOMATIC;
  rmw_publisher_options_t publisher_option = rmw_get_default_publisher_options();

  // omit since no remappled_topic_name created
  /*
  rcl_ret_t rcl_ret;
  char * remapped_topic_name = NULL;
  rcl_arguments_t rcl_arguments = rcl_get_zero_initialized_arguments();
  rcl_ret = rcl_parse_arguments(0, nullptr, allocator, &rcl_arguments);
  CHECK_RCL_RET(rcl_ret, "rcl_parse_arguments");

  rcl_ret = rcl_remap_topic_name(
      &rcl_arguments, nullptr, topic_name,
      rmw_node->name, rmw_node->namespace_, allocator, &remapped_topic_name);
  CHECK_RCL_RET(rcl_ret, "remap_topic_name");

  rmw_ret = rmw_validate_full_topic_name(remapped_topic_name, &validation_result, NULL);
  CHECK_RMW_RET(rmw_ret, "rmw_validate_full_topic_name");
  CHECK_VALIDATE(validation_result, "topic");
  */

  rmw_publisher_t *rmw_publisher = rmw_create_publisher(
      rmw_node,
      ts,
      topic_name, // remapped_topic_name,
      &qos_profile,
      &publisher_option);
  if (!rmw_publisher) {
    std::cout << "cannot create publisher" << std::endl;
    return -1;
  }

  ////////////////////////////////
  // publish
  // rcl_publish in rcl/.../publisher.c
  ////////////////////////////////
  std_msgs::msg::String msg;
  for(int i=0; i<100; i++) {
    std::cout << "puslish" << std::to_string(i) << std::endl;
    msg.data = "HelloWorld_rmw" + std::to_string(i);
    rmw_ret = rmw_publish(rmw_publisher, &msg, nullptr);
    CHECK_RMW_RET(rmw_ret, "rmw_publish");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  /*
  if (NULL != remapped_topic_name) {
    allocator.deallocate(remapped_topic_name, allocator.state);
  }
  */
  rmw_ret = rmw_destroy_publisher(rmw_node, rmw_publisher);
  CHECK_RMW_RET(rmw_ret, "rmw_destroy_publisher");
  rmw_ret = rmw_destroy_node(rmw_node);
  CHECK_RMW_RET(rmw_ret, "rmw_destroy_node");

  return 0;
}
