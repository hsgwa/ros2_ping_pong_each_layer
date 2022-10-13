#include "rmw_pubsub/rmw_pubsub.h"

#define CHECK_RCL_RET(val, fname) if (RCL_RET_OK != val) { \
    std::cout << fname << " fail: " << rcl_get_error_string().str << std::endl; \
    return ; \
  }
#define CHECK_RMW_RET(val, fname) if (RMW_RET_OK != val) { \
    std::cout << fname << " fail: " << rmw_get_error_string().str << std::endl; \
    return ; \
  }
#define CHECK_VALIDATE(val, where)   if (RMW_NODE_NAME_VALID != val) {        \
    const char * msg = rmw_node_name_validation_result_string(validation_result); \
    std::cout << where << " fail: " << msg << std::endl; \
    return RCL_RET_NODE_INVALID_NAME; \
  }

// configuration
static const char *local_namespace = "";

static void print_rmw_qos_profile(const rmw_qos_profile_t *qos)
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

RmwSubscriber::RmwSubscriber(int argc, char *argv[], char *node_name, char *topic_name) {
  // global resources
  rmw_ret_t rmw_ret;
  allocator_ = rcl_get_default_allocator();
  ////////////////////////////////
  // init
  // rcl_init_options_init in rcl/.../init.c
  ////////////////////////////////
  next_instance_id_ = 1;
  rmw_init_options_ = rmw_get_zero_initialized_init_options();
  rmw_ret = rmw_init_options_init(&rmw_init_options_, allocator_);

  rmw_init_options_.instance_id = next_instance_id_;
  rmw_context_ = rmw_get_zero_initialized_context();

  rmw_init_options_.domain_id = 0;
  rmw_init_options_.instance_id = 1;
  static char enclave[] = "/";
  rmw_init_options_.enclave = enclave;
  size_t domain_id = 0;



  rmw_ret = rmw_init(&rmw_init_options_, &rmw_context_);
  CHECK_RMW_RET(rmw_ret, "rmw_init");


  ////////////////////////////////
  // create node
  // rcl_get_zero_initialized_node in rcl/.../node.c
  ////////////////////////////////
  // Skip validation  as local_namespace must begin "/" when
  // rmw_create_node(),
  /*
    int validation_result = 0;
    rmw_ret = rmw_validate_node_name(node_name, &validation_result, NULL);
    CHECK_RMW_RET(rmw_ret, "rmw_validate_node_name");
    CHECK_VALIDATE(validation_result, "node_name");

    rmw_ret = rmw_validate_namespace(local_namespace, &validation_result,
    NULL); CHECK_RMW_RET(rmw_ret, "rmw_validate_namespace");
    CHECK_VALIDATE(validation_result, "namespace");
  */

  // rmw_security_options_t node_security_options =
  //     rmw_get_zero_initialized_security_options();
  // node_security_options.enforce_security = RMW_SECURITY_ENFORCEMENT_PERMISSIVE;
  // rmw_node = rmw_create_node(&rmw_context, node_name, local_namespace,
  //                            domain_id, &node_security_options, true);


  rmw_node_ = rmw_create_node(&rmw_context_, node_name, local_namespace,
                             domain_id, true);

  ////////////////////////////////
  // init subscription
  // rcl_subscription_init in rcl/.../subscription.c
  ////////////////////////////////
  const rosidl_message_type_support_t *ts =
      rosidl_typesupport_cpp::get_message_type_support_handle<
          message::msg::Data>();

  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  qos_profile.liveliness = RMW_QOS_POLICY_LIVELINESS_AUTOMATIC;
  rmw_subscription_options_t subscription_option =
      rmw_get_default_subscription_options();

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

  rmw_ret = rmw_validate_full_topic_name(remapped_topic_name,
  &validation_result, NULL); CHECK_RMW_RET(rmw_ret,
  "rmw_validate_full_topic_name"); CHECK_VALIDATE(validation_result,
  "topic");
  */

  rmw_subscription_ =
      rmw_create_subscription(rmw_node_, ts,
                              topic_name, // remapped_topic_name,
                              &qos_profile, &subscription_option);
  if (!rmw_subscription_) {
    std::cout << "cannot create subscription" << std::endl;
    return;
  }
  rmw_qos_profile_t act_qos;
  rmw_ret = rmw_subscription_get_actual_qos(rmw_subscription_, &act_qos);
  CHECK_RMW_RET(rmw_ret, "rmw_subscription_get_actual_qos");
  // print_rmw_qos_profile(&act_qos);

  // wait set
  size_t max_conditions = 2; // 2 * number_of_subscriptions in
                             // rcl/.../wait.c, I don't know why "*2"...
  wait_set_ = rmw_create_wait_set(&rmw_context_, max_conditions);
  if (!wait_set_) {
    std::cout << "fail to rmw_create_wait_set" << std::endl;
    return;
  }
  rmw_subscriptions_.subscribers = (void **)malloc(sizeof(void *) * 1);

  // main loop
}

RmwSubscriber::~RmwSubscriber() {
  rmw_ret_t rmw_ret;
  // free resources
  /*
  if (NULL != remapped_topic_name) {
    allocator.deallocate(remapped_topic_name, allocator.state);
  }
  */
  free(rmw_subscriptions_.subscribers);
  rmw_ret = rmw_destroy_wait_set(wait_set_);
  CHECK_RMW_RET(rmw_ret, "rmw_destroy_wait_set");
  rmw_ret = rmw_destroy_subscription(rmw_node_, rmw_subscription_);
  CHECK_RMW_RET(rmw_ret, "rmw_destroy_subscription");
  rmw_ret = rmw_destroy_node(rmw_node_);
  CHECK_RMW_RET(rmw_ret, "rmw_destroy_node");
  rmw_ret = rmw_shutdown(&rmw_context_);
  CHECK_RMW_RET(rmw_ret, "rmw_shutdown");
  rmw_ret = rmw_context_fini(&rmw_context_);
  CHECK_RMW_RET(rmw_ret, "rmw_context_fini");

  // this->msg.reset(&this->message);
}

void RmwSubscriber::set_callback(void (*callback)(message::msg::Data *data,
                                          HistReport *hist,
                                          TimeSeriesReport *timeSeries, PublisherBase *pub),
                         HistReport *hist, TimeSeriesReport *timeSeries, PublisherBase *pub) {
  callback_ = callback;
  hist_ = hist;
  timeSeries_ = timeSeries;
  pub_ = pub;
}

void RmwSubscriber::spin_once(std::chrono::nanoseconds timeout) {
  message::msg::Data data;

  rmw_ret_t rmw_ret;

  rmw_subscriptions_.subscribers[0] = rmw_subscription_->data;
  rmw_subscriptions_.subscriber_count = 1;
  wait_timeout_.sec =
      std::chrono::duration_cast<std::chrono::seconds>(timeout).count();
  wait_timeout_.nsec =
      std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();

  rmw_ret = rmw_wait(&rmw_subscriptions_, // &rmw_subscriptions,
                     nullptr,            // guard_condition
                     nullptr,            // services
                     nullptr,            // clients
                     nullptr,            // event
                     wait_set_, &wait_timeout_);
  if (RMW_RET_OK == rmw_ret) {
    if (rmw_subscriptions_.subscribers[0] != NULL) {
      rmw_message_info_t message_info;
      bool taken = false;
      rmw_ret = rmw_take_with_info(rmw_subscription_, &data, &taken,
                                   &message_info, nullptr);
      CHECK_RMW_RET(rmw_ret, "rmw_take_with_info");
      callback_(&data, hist_, timeSeries_, pub_);
    } else {
      std::cout << "unknown fired!?" << std::endl;
    }
  } else if (RMW_RET_TIMEOUT == rmw_ret) {
    return;
  } else {
    std::cout << "rmw_wait fail: " << rmw_get_error_string().str << std::endl;
    return;
  }
}

void RmwSubscriber::spin(std::chrono::nanoseconds timeout) {
  message::msg::Data data;

  rmw_ret_t rmw_ret;

  rmw_subscriptions_.subscribers[0] = rmw_subscription_->data;
  rmw_subscriptions_.subscriber_count = 1;
  wait_timeout_.sec =
      std::chrono::duration_cast<std::chrono::seconds>(timeout).count();
  wait_timeout_.nsec =
      std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();

  while (1) {


    rmw_ret = rmw_wait(&rmw_subscriptions_, // &rmw_subscriptions,
                       nullptr,            // guard_condition
                       nullptr,            // services
                       nullptr,            // clients
                       nullptr,            // event
                       wait_set_, &wait_timeout_);
    if (RMW_RET_OK == rmw_ret) {
      if (rmw_subscriptions_.subscribers[0] != NULL) {
        rmw_message_info_t message_info;
        bool taken = false;
        rmw_ret = rmw_take_with_info(rmw_subscription_, &data, &taken,
                                     &message_info, nullptr);
        CHECK_RMW_RET(rmw_ret, "rmw_take_with_info");
        callback_(&data, hist_, timeSeries_, pub_);
      } else {
        std::cout << "unknown fired!?" << std::endl;
      }
    } else if (RMW_RET_TIMEOUT == rmw_ret) {
      return;
    } else {
      std::cout << "rmw_wait fail: " << rmw_get_error_string().str << std::endl;
      return;
    }
  }
}

RmwPublisher::RmwPublisher(int argc, char *argv[], char *node_name, char *topic_name) {
  // global resources
  rmw_ret_t rmw_ret;
  allocator_ = rcl_get_default_allocator();

  ////////////////////////////////
  // init
  // rcl_init_options_init in rcl/.../init.c
  ////////////////////////////////
  uint64_t next_instance_id = 1;
  rmw_init_options_ = rmw_get_zero_initialized_init_options();
  rmw_ret = rmw_init_options_init(&rmw_init_options_, allocator_);

  rmw_init_options_.instance_id = next_instance_id;
  rmw_context_ = rmw_get_zero_initialized_context();

  rmw_init_options_.domain_id = 0;
  rmw_init_options_.instance_id = 1;
  static char enclave[] = "/";
  rmw_init_options_.enclave = enclave;
  size_t domain_id = 0;

  rmw_ret = rmw_init(&rmw_init_options_, &rmw_context_);
  CHECK_RMW_RET(rmw_ret, "rmw_init");

  ////////////////////////////////
  // create node
  // rcl_get_zero_initialized_node in rcl/.../node.c
  ////////////////////////////////
  // Skip validation  as local_namespace must begin "/" when
  // rmw_create_node(),
  /*
    int validation_result = 0;
    rmw_ret = rmw_validate_node_name(node_name, &validation_result, NULL);
    CHECK_RMW_RET(rmw_ret, "rmw_validate_node_name");
    CHECK_VALIDATE(validation_result, "node_name");

    rmw_ret = rmw_validate_namespace(local_namespace, &validation_result,
    NULL); CHECK_RMW_RET(rmw_ret, "rmw_validate_namespace");
    CHECK_VALIDATE(validation_result, "namespace");
  */

  // rmw_security_options_t node_security_options =
  //     rmw_get_zero_initialized_security_options();
  // node_security_options.enforce_security = RMW_SECURITY_ENFORCEMENT_PERMISSIVE;
  // rmw_node = rmw_create_node(&rmw_context, node_name, local_namespace,
  //                            domain_id, &node_security_options, true);
  rmw_node_ = rmw_create_node(&rmw_context_, node_name, local_namespace,
                             domain_id, true);

  const rmw_guard_condition_t *rmw_graph_guard_condition =
      rmw_node_get_graph_guard_condition(rmw_node_);

  ////////////////////////////////
  // init publisher
  // rcl_publisher_init in rcl/.../publisher.c
  ////////////////////////////////
  const rosidl_message_type_support_t *ts =
      rosidl_typesupport_cpp::get_message_type_support_handle<
          message::msg::Data>();

  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  qos_profile.liveliness = RMW_QOS_POLICY_LIVELINESS_AUTOMATIC;
  rmw_publisher_options_t publisher_option =
      rmw_get_default_publisher_options();

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

  rmw_ret = rmw_validate_full_topic_name(remapped_topic_name,
  &validation_result, NULL); CHECK_RMW_RET(rmw_ret,
  "rmw_validate_full_topic_name"); CHECK_VALIDATE(validation_result,
  "topic");
  */

  rmw_publisher_ = rmw_create_publisher(rmw_node_, ts,
                                       topic_name, // remapped_topic_name,
                                       &qos_profile, &publisher_option);

  // if (!rmw_publisher) {
  //   std::cout << "cannot create publisher" << std::endl;
  //   return -1;
  // }
}
void RmwPublisher::publish(message::msg::Data &msg) {
  rmw_ret_t rmw_ret;
  rmw_ret = rmw_publish(rmw_publisher_, &msg, nullptr);
  CHECK_RMW_RET(rmw_ret, "rmw_publish");
}
RmwPublisher::~RmwPublisher() {
  rmw_ret_t rmw_ret;
  /*
  if (NULL != remapped_topic_name) {
    allocator.deallocate(remapped_topic_name, allocator.state);
  }
  */
  rmw_ret = rmw_destroy_publisher(rmw_node_, rmw_publisher_);
  CHECK_RMW_RET(rmw_ret, "rmw_destroy_publisher");
  rmw_ret = rmw_destroy_node(rmw_node_);
  CHECK_RMW_RET(rmw_ret, "rmw_destroy_node");
}

