
#include <memory>
#include <iostream>
#include <chrono>
#include <thread>

#include "rcl/rcl.h"
#include "message/msg/data.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rcl/error_handling.h"

#include "layer_pubsub_common/interface.hpp"

typedef struct rcl_node_impl_t
{
  rcl_node_options_t options;
  size_t actual_domain_id;
  rmw_node_t * rmw_node_handle;
  rcl_guard_condition_t * graph_guard_condition;
  const char * logger_name;
  const char * fq_name;
} rcl_node_impl_t;


class RclSubscriber : public SubscriberBase {
 public:
  RclSubscriber(int argc, char *argv[], char *node_name, char *topic_name);
   ~RclSubscriber();

   void set_callback(void (*callback)(message::msg::Data *data,
                                      HistReport *hist,
                                      TimeSeriesReport *timeSeries,
                                      PublisherBase *pub),
                     HistReport *hist, TimeSeriesReport *timeSeries,
                     PublisherBase *pub) override;

   void spin(std::chrono::nanoseconds timeout) override;
   void spin_once(std::chrono::nanoseconds timeout) override;

 private:
   message::msg::Data::UniquePtr msg;
   void (*callback_)(message::msg::Data *data, HistReport *hist,
                     TimeSeriesReport *timeSeries, PublisherBase *pub);
   HistReport *hist_;
   TimeSeriesReport *timeSeries_;
   PublisherBase *pub_;
   rcl_wait_set_t wait_set_;
   rcl_subscription_t subscription_;
   rcl_node_t node_;
   rcl_context_t context_;
};

class RclPublisher : public PublisherBase {
public:
  RclPublisher(int argc, char *argv[], char *node_name, char *topic_name);
  ~RclPublisher();
  void publish(message::msg::Data &msg) ;

 private:
  rcl_publisher_t publisher_;
  rcl_context_t context_;
  rcl_node_t node_;
};
