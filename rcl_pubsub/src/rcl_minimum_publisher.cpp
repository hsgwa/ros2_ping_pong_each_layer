#include <iostream>
#include <chrono>
#include <thread>

#include "rcl/rcl.h"
#include "rcl/error_handling.h"
#include "std_msgs/msg/string.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

#include "rmw/types.h"

#define CHECK_RCL_RET(val, fname) if (RCL_RET_OK != val) { \
    std::cout << fname << " fail: " << rcl_get_error_string().str << std::endl; \
    return -1; \
  }

typedef struct rcl_node_impl_t
{
  rcl_node_options_t options;
  size_t actual_domain_id;
  rmw_node_t * rmw_node_handle;
  rcl_guard_condition_t * graph_guard_condition;
  const char * logger_name;
  const char * fq_name;
} rcl_node_impl_t;


void print_rmw_qos_profile(const rmw_qos_profile_t *qos)
{
  std::cout << "qos" << std::endl;
  std::cout << "qos->history: " << qos->history << std::endl;
  std::cout << "qos->reliability: " << qos->reliability << std::endl;
  std::cout << "qos->durability: " << qos->durability << std::endl;
  // std::cout << "qos->deadline: " << qos->deadline << std::endl;
  // std::cout << "qos->lifespan: " << qos->lifespan << std::endl;
  std::cout << "qos->liveliness: " << qos->liveliness << std::endl;
  // std::cout << "qos->liveliness_lease_duration: " << qos->liveliness_lease_duration << std::endl;
}

/**
 * TODO:
 *   - error check
 *   - use rcl_wait (really need?)
 *   - use real-time sleep
 */
int main(int argc, char *argv[])
{
  // configuration
  const char * node_name = "rcl_publisher";
  const char * namespace_ = "ns";
  const char * topic = "topic";

  // global resources
  rcl_context_t context = rcl_get_zero_initialized_context();
  rcl_ret_t ret;

  // rcl init
  rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
  ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
  CHECK_RCL_RET(ret, "rcl_init_options_init");

  ret = rcl_init(argc, argv, &init_options, &context);
  CHECK_RCL_RET(ret, "rcl_init");

  // create node
  rcl_node_t node = rcl_get_zero_initialized_node();
  rcl_node_options_t node_ops = rcl_node_get_default_options();
  ret = rcl_node_init(&node, node_name, namespace_, &context, &node_ops);
  CHECK_RCL_RET(ret, "rcl_node_init");

  rcl_node_impl_t* impl = node.impl;
  std::cout << "node fq_name: " << impl->fq_name << std::endl;

  // create publisher
  const rosidl_message_type_support_t *ts =
    rosidl_typesupport_cpp::get_message_type_support_handle<std_msgs::msg::String>();
  rcl_publisher_options_t publisher_options = rcl_publisher_get_default_options();
  rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
  ret = rcl_publisher_init(&publisher, &node, ts, topic, &publisher_options);
  CHECK_RCL_RET(ret, "rcl_publisher_init");

  std::cout << "topic name: " << rcl_publisher_get_topic_name(&publisher) << std::endl;
  const rmw_qos_profile_t *act_qos = rcl_publisher_get_actual_qos(&publisher);
  print_rmw_qos_profile(act_qos);

  // TODO: spin by rcl_wait()
  std_msgs::msg::String msg;
  for(int i=0; ; i++) {
    std::cout << "puslish" << std::endl;
    msg.data = "HelloWorld" + std::to_string(i);
    ret = rcl_publish(&publisher, &msg, nullptr);
    CHECK_RCL_RET(ret, "rcl_publish");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // free resources
  ret = rcl_publisher_fini(&publisher, &node);
  CHECK_RCL_RET(ret, "rcl_publisher_fini");
  ret = rcl_node_fini(&node);
  CHECK_RCL_RET(ret, "rcl_node_fini");
  ret = rcl_shutdown(&context);
  CHECK_RCL_RET(ret, "rcl_shutdown");
}
