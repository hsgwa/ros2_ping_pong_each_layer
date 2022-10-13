#include <iostream>
#include <chrono>
#include <thread>

#include "message/msg/data.hpp"
#include "rcl/error_handling.h"
#include "rcl/rcl.h"
#include "rcl_pubsub/rcl_pubsub.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "message/msg/data.hpp"
#include "rcl/error_handling.h"
#include "rcl/rcl.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

#include "rmw/types.h"

#define CHECK_RCL_RET(val, fname) if (RCL_RET_OK != val) { \
    std::cout << fname << " fail: " << rcl_get_error_string().str << std::endl; \
    return; \
  }

using namespace std::chrono;

// configuration
static const char *namespace_ = "";

static void print_rmw_qos_profile(const rmw_qos_profile_t *qos) {
  std::cout << "qos" << std::endl;
  std::cout << "qos->history: " << qos->history << std::endl;
  std::cout << "qos->reliability: " << qos->reliability << std::endl;
  std::cout << "qos->durability: " << qos->durability << std::endl;
  // std::cout << "qos->deadline: " << qos->deadline << std::endl;
  // std::cout << "qos->lifespan: " << qos->lifespan << std::endl;
  std::cout << "qos->liveliness: " << qos->liveliness << std::endl;
  // std::cout << "qos->liveliness_lease_duration: " <<
  // qos->liveliness_lease_duration << std::endl;
}

RclPublisher::RclPublisher(int argc, char *argv[], char *node_name, char *topic_name) {
    // global resources
    context_ = rcl_get_zero_initialized_context();
    rcl_ret_t ret;

    // rcl init
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    ret = rcl_init_options_init(&init_options, rcl_get_default_allocator());
    CHECK_RCL_RET(ret, "rcl_init_options_init");

    ret = rcl_init(argc, argv, &init_options, &context_);
    CHECK_RCL_RET(ret, "rcl_init");

    // create node
    node_ = rcl_get_zero_initialized_node();
    rcl_node_options_t node_ops = rcl_node_get_default_options();
    ret = rcl_node_init(&node_, node_name, namespace_, &context_, &node_ops);
    CHECK_RCL_RET(ret, "rcl_node_init");

    rcl_node_impl_t *impl = node_.impl;

    // create publisher
    const rosidl_message_type_support_t *ts =
        rosidl_typesupport_cpp::get_message_type_support_handle<
            message::msg::Data>();
    rcl_publisher_options_t publisher_options =
        rcl_publisher_get_default_options();
    publisher_ = rcl_get_zero_initialized_publisher();
    ret = rcl_publisher_init(&publisher_, &node_, ts, topic_name, &publisher_options);
    CHECK_RCL_RET(ret, "rcl_publisher_init");

    const rmw_qos_profile_t *act_qos =
        rcl_publisher_get_actual_qos(&publisher_);
    // print_rmw_qos_profile(act_qos);
  }

RclPublisher::~RclPublisher() {
  rcl_ret_t ret;
  // free resources
  ret = rcl_publisher_fini(&publisher_, &node_);
  CHECK_RCL_RET(ret, "rcl_publisher_fini");
  ret = rcl_node_fini(&node_);
  CHECK_RCL_RET(ret, "rcl_node_fini");
  ret = rcl_shutdown(&context_);
  CHECK_RCL_RET(ret, "rcl_shutdown");
}

void RclPublisher::publish(message::msg::Data &msg) {
  rcl_ret_t ret;
  ret = rcl_publish(&publisher_, &msg, nullptr);
  CHECK_RCL_RET(ret, "rcl_publish");
}


// class RclSubscriber {
//  public:
RclSubscriber::RclSubscriber(int argc, char *argv[], char *node_name, char *topic_name) {
  // global resources
  context_ = rcl_get_zero_initialized_context();
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  // rcl init
  rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
  ret = rcl_init_options_init(&init_options, allocator);
  CHECK_RCL_RET(ret, "rcl_init_options_init");

  ret = rcl_init(argc, argv, &init_options, &context_);
  CHECK_RCL_RET(ret, "rcl_init");

  // create node
  node_ = rcl_get_zero_initialized_node();
  rcl_node_options_t node_ops = rcl_node_get_default_options();
  ret = rcl_node_init(&node_, node_name, namespace_, &context_, &node_ops);
  CHECK_RCL_RET(ret, "rcl_node_init");

  rcl_node_impl_t *impl = node_.impl;

  // create subscriber
  const rosidl_message_type_support_t *ts =
      rosidl_typesupport_cpp::get_message_type_support_handle<
          message::msg::Data>();
  rcl_subscription_options_t subscription_options =
      rcl_subscription_get_default_options();
  subscription_ = rcl_get_zero_initialized_subscription();
  ret = rcl_subscription_init(&subscription_, &node_, ts, topic_name,
                              &subscription_options);
  CHECK_RCL_RET(ret, "rcl_subscription_init");

  // wait_set
  wait_set_ = rcl_get_zero_initialized_wait_set();
  size_t num_sub = 1;
  size_t num_pub = 0;
  size_t num_timer = 0;
  size_t num_client = 0;
  size_t num_service = 0;
  size_t num_event = 0;
  ret = rcl_wait_set_init(&wait_set_, num_sub, num_pub, num_timer, num_client,
                          num_service, num_event, &context_, allocator);
  CHECK_RCL_RET(ret, "rcl_wait_set_init");
}
RclSubscriber::~RclSubscriber() {
  rcl_ret_t ret;
  // free resources
  ret = rcl_wait_set_fini(&wait_set_);
  CHECK_RCL_RET(ret, "rcl_wait_set_fini");
  ret = rcl_subscription_fini(&subscription_, &node_);
  CHECK_RCL_RET(ret, "rcl_subscription_fini");
  ret = rcl_node_fini(&node_);
  CHECK_RCL_RET(ret, "rcl_node_fini");
  ret = rcl_shutdown(&context_);
  CHECK_RCL_RET(ret, "rcl_shutdown");
}

void RclSubscriber::spin_once(std::chrono::nanoseconds timeout) {

  message::msg::Data data;
  rcl_ret_t ret;

  auto timeout_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();
  ret = rcl_wait_set_clear(&wait_set_);
  ret = rcl_wait_set_add_subscription(&wait_set_, &subscription_, nullptr);
  ret = rcl_wait(&wait_set_, timeout_ns);

  if (RCL_RET_TIMEOUT == ret) {
    return;
  }

  for (size_t j = 0; j < wait_set_.size_of_subscriptions; j++) {
    if (wait_set_.subscriptions[j]) {
      // auto message = std::allocate_shared<void,
      // std::allocator<void>>(alloc); auto message =
      // std::allocate_shared<void>(alloc);
      rmw_message_info_t message_info;
      ret = rcl_take(&subscription_, &data, &message_info, NULL);
      callback_(&data, hist_, timeSeries_, pub_);
    }
  }
}

void RclSubscriber::spin(std::chrono::nanoseconds timeout) {

  message::msg::Data data;
  rcl_ret_t ret;

  auto timeout_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();
  while (1) {
    ret = rcl_wait_set_clear(&wait_set_);
    ret = rcl_wait_set_add_subscription(&wait_set_, &subscription_, nullptr);
    ret = rcl_wait(&wait_set_,timeout_ns);

    if (RCL_RET_TIMEOUT == ret) {
      return;
    }

    for (size_t j = 0; j < wait_set_.size_of_subscriptions; j++) {
      if (wait_set_.subscriptions[j]) {
        // auto message = std::allocate_shared<void,
        // std::allocator<void>>(alloc); auto message =
        // std::allocate_shared<void>(alloc);
        rmw_message_info_t message_info;
        ret = rcl_take(&subscription_, &data, &message_info, NULL);
        callback_(&data, hist_, timeSeries_, pub_);
      }
    }
  }
}

void RclSubscriber::set_callback(
    void (*callback)(message::msg::Data *data, HistReport *hist,
                     TimeSeriesReport *timeSeries, PublisherBase *pub),
    HistReport *hist, TimeSeriesReport *timeSeries, PublisherBase *pub) {

  callback_ = callback;
  hist_ = hist;
  timeSeries_ = timeSeries;
  pub_ = pub;
}
