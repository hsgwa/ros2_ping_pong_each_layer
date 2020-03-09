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

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  // configuration
  const char * local_namespace = "/ns";
  const char * node_name = "rmw_subscriber";
  const char * topic_name = "/ns/topic";  // "/ns" shoud be equal to local_namespace

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

  ////////////////////////////////
  // init subscription
  // rcl_subscription_init in rcl/.../subscription.c
  ////////////////////////////////
  const rosidl_message_type_support_t *ts =
    rosidl_typesupport_cpp::get_message_type_support_handle<std_msgs::msg::String>();

  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  qos_profile.liveliness = RMW_QOS_POLICY_LIVELINESS_AUTOMATIC;
  rmw_subscription_options_t subscription_option = rmw_get_default_subscription_options();

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

  rmw_subscription_t *rmw_subscription = rmw_create_subscription(
      rmw_node,
      ts,
      topic_name, // remapped_topic_name,
      &qos_profile,
      &subscription_option);
  if (!rmw_subscription) {
    std::cout << "cannot create subscription" << std::endl;
    return -1;
  }
  std::cout << "actual topic name: " << rmw_subscription->topic_name << std::endl;
  rmw_qos_profile_t act_qos;
  rmw_ret = rmw_subscription_get_actual_qos(rmw_subscription, &act_qos);
  CHECK_RMW_RET(rmw_ret, "rmw_subscription_get_actual_qos");
  print_rmw_qos_profile(&act_qos);


  // wait set
  size_t max_conditions = 2; // 2 * number_of_subscriptions in rcl/.../wait.c, I don't know why "*2"...
  rmw_wait_set_t *wait_set = rmw_create_wait_set(&rmw_context, max_conditions);
  if(!wait_set) {
    std::cout << "fail to rmw_create_wait_set" << std::endl;
    return -1;
  }
  rmw_subscriptions_t rmw_subscriptions;
  rmw_subscriptions.subscribers = (void**)malloc(sizeof(void*) * 1);

  // main loop
  rmw_time_t wait_timeout;
  wait_timeout.sec = 1;
  wait_timeout.nsec = 0;

  for(int i=0; i<10; i++) {
    rmw_subscriptions.subscribers[0] = rmw_subscription->data;
    rmw_subscriptions.subscriber_count = 1;
    rmw_ret = rmw_wait(&rmw_subscriptions, // &rmw_subscriptions,
                       nullptr,  // guard_condition
                       nullptr,  // services
                       nullptr,  // clients
                       nullptr,  // event
                       wait_set,
                       &wait_timeout);
    if(RMW_RET_OK == rmw_ret) {
      if (rmw_subscriptions.subscribers[0] != NULL) {
        std_msgs::msg::String message;
        rmw_message_info_t message_info;
        bool taken = false;
        rmw_ret = rmw_take_with_info(
            rmw_subscription, &message, &taken, &message_info, nullptr);
        CHECK_RMW_RET(rmw_ret, "rmw_take_with_info");
        std::cout << "subscriptions fired. message=" << message.data.c_str() << std::endl;
      } else {
        std::cout << "unknown fired!?" << std::endl;
      }
    } else if(RMW_RET_TIMEOUT == rmw_ret) {
      std::cout << "timeout" << std::endl;
    } else {
      std::cout << "rmw_wait fail: " << rmw_get_error_string().str << std::endl;
      break;
    }
  }

  // free resources
  /*
  if (NULL != remapped_topic_name) {
    allocator.deallocate(remapped_topic_name, allocator.state);
  }
  */
  free(rmw_subscriptions.subscribers);
  rmw_ret = rmw_destroy_wait_set(wait_set);
  CHECK_RMW_RET(rmw_ret, "rmw_destroy_wait_set");
  rmw_ret = rmw_destroy_subscription(rmw_node, rmw_subscription);
  CHECK_RMW_RET(rmw_ret, "rmw_destroy_subscription");
  rmw_ret = rmw_destroy_node(rmw_node);
  CHECK_RMW_RET(rmw_ret, "rmw_destroy_node");
  rmw_ret = rmw_shutdown(&rmw_context);
  CHECK_RMW_RET(rmw_ret, "rmw_shutdown");
  rmw_ret = rmw_context_fini(&rmw_context);
  CHECK_RMW_RET(rmw_ret, "rmw_context_fini");

  return 0;
}
