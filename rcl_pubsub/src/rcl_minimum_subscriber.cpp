#include <memory>
#include <iostream>
#include <chrono>
#include <thread>

#include "rcl/rcl.h"
#include "std_msgs/msg/string.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rcl/error_handling.h"

#define CHECK_RCL_RET(val, fname) if (RCL_RET_OK != val) { \
    std::cout << fname << " fail: " << rcl_get_error_string().str << std::endl; \
    return -1; \
  }

using namespace std::chrono;

typedef struct rcl_node_impl_t
{
  rcl_node_options_t options;
  size_t actual_domain_id;
  rmw_node_t * rmw_node_handle;
  rcl_guard_condition_t * graph_guard_condition;
  const char * logger_name;
  const char * fq_name;
} rcl_node_impl_t;

int main(int argc, char *argv[])
{
  // configuration
  const char * node_name = "rcl_minimum_subscriber";
  const char * namespace_ = "ns";
  const char * topic = "topic";

  // global resources
  rcl_context_t context = rcl_get_zero_initialized_context();
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();

  // rcl init
  rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
  ret = rcl_init_options_init(&init_options, allocator);
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

  // create subscriber
  const rosidl_message_type_support_t *ts =
    rosidl_typesupport_cpp::get_message_type_support_handle<std_msgs::msg::String>();
  rcl_subscription_options_t subscription_options = rcl_subscription_get_default_options();
  rcl_subscription_t subscription = rcl_get_zero_initialized_subscription();
  ret = rcl_subscription_init(&subscription, &node, ts, topic, &subscription_options);
  CHECK_RCL_RET(ret, "rcl_subscription_init");

  // wait_set
  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  size_t num_sub = 1;
  size_t num_pub = 0;
  size_t num_timer = 0;
  size_t num_client = 0;
  size_t num_service = 0;
  size_t num_event = 0;
  ret = rcl_wait_set_init(&wait_set, num_sub, num_pub, num_timer, num_client,
                          num_service, num_event, &context, allocator);
  CHECK_RCL_RET(ret, "rcl_wait_set_init");

  while(true) {
    steady_clock::time_point time_bgn = steady_clock::now();
    ret = rcl_wait_set_clear(&wait_set);
    ret = rcl_wait_set_add_subscription(&wait_set, &subscription, nullptr);
    steady_clock::time_point time_ws = steady_clock::now();
    ret = rcl_wait(&wait_set, RCL_MS_TO_NS(1000));  // 1 [s]
    steady_clock::time_point time_end = steady_clock::now();
    std::cout << "set wait_set: " << duration_cast<milliseconds>(time_ws - time_bgn).count() << " [ms], "
              << "wait: " << duration_cast<milliseconds>(time_end - time_ws).count() << " [ms]" << std::endl;
    if (RCL_RET_TIMEOUT == ret) {
      std::cout << "timeouted" << std::endl;
    }
    for(size_t j=0; j<wait_set.size_of_subscriptions; j++) {
      if(wait_set.subscriptions[j]) {
        std_msgs::msg::String message;
        // auto message = std::allocate_shared<void, std::allocator<void>>(alloc);
        // auto message = std::allocate_shared<void>(alloc);
        rmw_message_info_t message_info;
        ret = rcl_take(&subscription, &message, &message_info, NULL);

        std::cout << "subscriptions[" << j << "] fired. message=" << message.data.c_str() << std::endl;
      }
    }
  }

  // free resources
  ret = rcl_wait_set_fini(&wait_set);
  CHECK_RCL_RET(ret, "rcl_wait_set_fini");
  ret = rcl_subscription_fini(&subscription, &node);
  CHECK_RCL_RET(ret, "rcl_subscription_fini");
  ret = rcl_node_fini(&node);
  CHECK_RCL_RET(ret, "rcl_node_fini");
  ret = rcl_shutdown(&context);
  CHECK_RCL_RET(ret, "rcl_shutdown");

  return 0;
}
