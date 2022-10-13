
#include <iostream>

#include "fastrtps/Domain.h"
#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/attributes/SubscriberAttributes.h"
#include "fastrtps/participant/Participant.h"
#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/rtps/common/Locator.h"
#include "fastrtps/subscriber/SampleInfo.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/subscriber/SubscriberListener.h"
#include "fastrtps/utils/IPLocator.h"

#include "fastrtps/rtps/RTPSDomain.h"

#include "fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h"
#include "fastrtps/rtps/reader/RTPSReader.h"
#include "fastrtps/rtps/reader/ReaderListener.h"
#include "fastrtps/rtps/reader/StatefulReader.h"

#include "message/msg/data.hpp"

#include "rosidl_runtime_c/message_type_support_struct.h" // get_message_typesupport_handle

#include "rosidl_typesupport_cpp/identifier.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp" // rosidl_typesupport_cpp::get_message_type_support_handle

#include "rosidl_typesupport_fastrtps_c/identifier.h" // for rosidl_typesupport_fastrtps_c__identifier
#include "rosidl_typesupport_fastrtps_cpp/identifier.hpp" // for rosidl_typesupport_fastrtps_cpp::typesupport_identifier
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h" // for message_type_support_callbacks_t

#include "my_message_type_support.hpp"

#include "layer_pubsub_common/interface.hpp"

using Domain = eprosima::fastrtps::Domain;
using IPLocator = eprosima::fastrtps::rtps::IPLocator;
using Locator_t = eprosima::fastrtps::rtps::Locator_t;
using Participant = eprosima::fastrtps::Participant;
using ParticipantAttributes = eprosima::fastrtps::ParticipantAttributes;
using StatefulReader = eprosima::fastrtps::rtps::StatefulReader;
using Publisher = eprosima::fastrtps::Publisher;

#define RMW_FASTRTPS_CPP_TYPESUPPORT_C rosidl_typesupport_fastrtps_c__identifier
#define RMW_FASTRTPS_CPP_TYPESUPPORT_CPP                                       \
  rosidl_typesupport_fastrtps_cpp::typesupport_identifier

class FastrtpsPublisher : public PublisherBase {
public:
  FastrtpsPublisher(int argc, char *argv[], char *node_name, char *topic_name);
  ~FastrtpsPublisher();

  void publish(message::msg::Data &msg);

private:
  Publisher *publisher_;
  rmw_fastrtps_shared_cpp::SerializedData data_;
  Participant *participant_;
};


class FastrtpsSubscriber : public SubscriberBase {
public:
  FastrtpsSubscriber(int argc, char *argv[], char *node_name, char *topic_name);
  ~FastrtpsSubscriber();

  void spin(std::chrono::nanoseconds timeout) override;
  void spin_once(std::chrono::nanoseconds timeout) override;

  void set_callback(void (*callback)(message::msg::Data *data, HistReport *hist,
                                     TimeSeriesReport *timeSeries,
                                     PublisherBase *pub),
                    HistReport *hist, TimeSeriesReport *timeSeries,
                    PublisherBase *pub) override;

private:
  ParticipantAttributes participantAttrs_;
  Locator_t local_network_interface_locator_;

  Participant *participant_;
  eprosima::fastrtps::Subscriber *subscriber_;
  std::condition_variable condition_;
  std::mutex condition_mutex_;
  rmw_fastrtps_shared_cpp::SerializedData serializedData_;

  message::msg::Data::UniquePtr msg_;

  void (*callback_)(message::msg::Data *data, HistReport *hist,
                    TimeSeriesReport *timeSeries, PublisherBase *pub);
  HistReport *hist_;
  TimeSeriesReport *timeSeries_;
  PublisherBase *pub_;

public:
  class SubListener : public eprosima::fastrtps::SubscriberListener {
  public:
    void onNewDataMessage(eprosima::fastrtps::Subscriber *sub) final;
    void onSubscriptionMatched(
        eprosima::fastrtps::Subscriber * /*sub*/,
        eprosima::fastrtps::rtps::MatchingInfo & /*info*/) final;

    void attachCondition(std::mutex *conditionMutex,
                         std::condition_variable *conditionVariable);

    bool hasData() const;

    void data_taken(eprosima::fastrtps::Subscriber *sub);

  private:
    std::condition_variable *conditionVariable_;
    std::mutex *conditionMutex_;
    std::atomic_size_t data_;
   } listener_;
};
