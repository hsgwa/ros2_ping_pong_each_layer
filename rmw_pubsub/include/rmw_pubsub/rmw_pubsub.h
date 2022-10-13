
#include <chrono>
#include <iostream>
#include <thread>

#include "message/msg/data.hpp"
#include "rcl/arguments.h"
#include "rcl/error_handling.h"
#include "rcl/rcl.h"
#include "rcl/remap.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_full_topic_name.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

#include "rcl/rcl.h"
#include "rcl/remap.h"
#include "rcl/arguments.h"
#include "rcl/error_handling.h"
#include "message/msg/data.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"
#include "rmw/security_options.h"
#include "rmw/error_handling.h"
#include "rmw/validate_full_topic_name.h"

#include "layer_pubsub_common/interface.hpp"

class RmwPublisher : public PublisherBase {
 public:
  RmwPublisher(int argc, char *argv[], char *node_name, char *topic_name);
  ~RmwPublisher();

  void publish(message::msg::Data &msg) override;

private:
  rmw_publisher_t *rmw_publisher_;
  rmw_node_t *rmw_node_;
  rcl_allocator_t allocator_;
  rmw_init_options_t rmw_init_options_;
  rmw_context_t rmw_context_;
};

class RmwSubscriber : public SubscriberBase {
 public:
   RmwSubscriber(int argc, char *argv[], char *node_name, char *topic_name);
   ~RmwSubscriber();
   void spin(std::chrono::nanoseconds timeout) override;
   void spin_once(std::chrono::nanoseconds timeout) override;

   void set_callback(void (*callback)(message::msg::Data *data,
                                      HistReport *hist,
                                      TimeSeriesReport *timeSeries,
                                      PublisherBase *pub),
                     HistReport *hist, TimeSeriesReport *timeSeries,
                     PublisherBase *pub) override;

 private:
   rmw_subscriptions_t rmw_subscriptions_;
   rmw_subscription_t *rmw_subscription_;
   rmw_wait_set_t *wait_set_;
   rmw_node_t *rmw_node_;
   rmw_context_t rmw_context_;
   rmw_time_t wait_timeout_;
   rmw_init_options_t rmw_init_options_;
   rcl_allocator_t allocator_;
   uint64_t next_instance_id_;
   // message::msg::Data message;
   message::msg::Data::UniquePtr msg_;

   void (*callback_)(message::msg::Data *data, HistReport *hist,
                     TimeSeriesReport *timeSeries, PublisherBase *pub);
   HistReport *hist_;
   TimeSeriesReport *timeSeries_;
   PublisherBase *pub_;
};
