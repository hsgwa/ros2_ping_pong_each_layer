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

// #include "rosidl_generator_c/message_type_support_struct.h"  //
// get_message_typesupport_handle
#include "rosidl_runtime_c/message_type_support_struct.h" // get_message_typesupport_handle

#include "rosidl_typesupport_cpp/identifier.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp" // rosidl_typesupport_cpp::get_message_type_support_handle

#include "rosidl_typesupport_fastrtps_c/identifier.h" // for rosidl_typesupport_fastrtps_c__identifier
#include "rosidl_typesupport_fastrtps_cpp/identifier.hpp" // for rosidl_typesupport_fastrtps_cpp::typesupport_identifier
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h" // for message_type_support_callbacks_t

#include "my_message_type_support.hpp"

#include <iostream>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

//#include "fastrtps/config.h"
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

#include "rosidl_typesupport_cpp/identifier.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp" // rosidl_typesupport_cpp::get_message_type_support_handle

#include "rosidl_typesupport_fastrtps_c/identifier.h" // for rosidl_typesupport_fastrtps_c__identifier
#include "rosidl_typesupport_fastrtps_cpp/identifier.hpp" // for rosidl_typesupport_fastrtps_cpp::typesupport_identifier
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h" // for message_type_support_callbacks_t

#include "fastrtps_pubsub/fastrtps_pubsub.h"
#include "my_message_type_support.hpp"

using Domain = eprosima::fastrtps::Domain;
using IPLocator = eprosima::fastrtps::rtps::IPLocator;
using Locator_t = eprosima::fastrtps::rtps::Locator_t;
using Participant = eprosima::fastrtps::Participant;
using ParticipantAttributes = eprosima::fastrtps::ParticipantAttributes;
using StatefulReader = eprosima::fastrtps::rtps::StatefulReader;
using Subscriber = eprosima::fastrtps::Subscriber;
using SubscriberListener = eprosima::fastrtps::SubscriberListener;
using namespace std::chrono_literals;

#define RMW_FASTRTPS_CPP_TYPESUPPORT_C rosidl_typesupport_fastrtps_c__identifier
#define RMW_FASTRTPS_CPP_TYPESUPPORT_CPP                                       \
  rosidl_typesupport_fastrtps_cpp::typesupport_identifier

// configuration
static const size_t domain_id = 0;
static const char *local_namespace = "/ns";
static const char *const ros_topic_prefix = "rt"; // "rt"

inline eprosima::fastrtps::string_255
_create_topic_name(const char *prefix, const char *base,
                   const char *suffix = nullptr) {
  std::ostringstream topicName;
  if (prefix) {
    topicName << prefix;
  }
  topicName << base;
  if (suffix) {
    topicName << suffix;
  }
  return topicName.str();
}

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

struct SerializedData {
  bool is_cdr_buffer; // Whether next field is a pointer to a Cdr or to a plain
                      // ros message
  void *data;
  const void *impl; // RMW implementation specific data
};

FastrtpsPublisher::FastrtpsPublisher(int argc, char *argv[], char *node_name, char *topic_name) {
  ////////////////////////////////
  // rmw_create_node
  // As node is not contained in FastRTPS design,
  // we only need to get Participant
  ////////////////////////////////
  ParticipantAttributes participantAttrs;
  Domain::getDefaultParticipantAttributes(participantAttrs);
  participantAttrs.domainId = static_cast<uint32_t>(domain_id);
  participantAttrs.rtps.setName(node_name);

  Locator_t local_network_interface_locator;
  // if localhost only
  // static const std::string local_ip_name("127.0.0.1");
  // local_network_interface_locator.kind = 1;
  // local_network_interface_locator.port = 0;
  // IPLocator::setIPv4(local_network_interface_locator, local_ip_name);
  // participantAttrs.rtps.builtin.metatrafficUnicastLocatorList.push_back(
  //     local_network_interface_locator);
  // participantAttrs.rtps.builtin.initialPeersList.push_back(local_network_interface_locator);

  participantAttrs.rtps.builtin.readerHistoryMemoryPolicy =
      eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
  participantAttrs.rtps.builtin.writerHistoryMemoryPolicy =
      eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

  size_t length = strlen(node_name) + strlen("name=;") +
                  strlen(local_namespace) + strlen("namespace=;") + 1;
  participantAttrs.rtps.userData.resize(length);
  snprintf(reinterpret_cast<char *>(participantAttrs.rtps.userData.data()),
           length, "name=%s;namespace=%s;", node_name, local_namespace);

  // rmw_fastrtps create_node
  participant_ = Domain::createParticipant(participantAttrs, nullptr);

  ////////////////////////////////
  // rmw_create_publisher
  ////////////////////////////////
  // I don't know why, but segv happens in the following code:
  //   const rosidl_message_type_support_t *type_support =
  //      rosidl_typesupport_cpp::get_message_type_support_handle<std_msgs::msg::String>();
  //   auto callbacks = static_cast<const message_type_support_callbacks_t
  //   *>(type_support->data); callbacks.message_namespace_  // segv here

  const rosidl_message_type_support_t *type_supports =
      rosidl_typesupport_cpp::get_message_type_support_handle<
          message::msg::Data>();
  const rosidl_message_type_support_t *type_support =
      get_message_typesupport_handle(type_supports,
                                     RMW_FASTRTPS_CPP_TYPESUPPORT_C);
  if (!type_support) {
    type_support = get_message_typesupport_handle(
        type_supports, RMW_FASTRTPS_CPP_TYPESUPPORT_CPP);
  }

  if (!type_support) {
    std::cout << "cannot get type_support " << std::endl;
    return;
  }

  auto callbacks =
      static_cast<const message_type_support_callbacks_t *>(type_support->data);
  std::string type_name = _create_type_name(callbacks);
  data_.is_cdr_buffer = false;
  data_.impl =
      static_cast<const message_type_support_callbacks_t *>(type_support->data);

  // TODO: create topicDataType for std_msgs::msg::dds_::String_
  rmw_fastrtps_shared_cpp::TypeSupport *message_type =
      new MyMessageTypeSupport(callbacks);
  eprosima::fastrtps::Domain::registerType(participant_, message_type);

  eprosima::fastrtps::PublisherAttributes publisherParam;
  Domain::getDefaultPublisherAttributes(publisherParam);
  publisherParam.qos.m_publishMode.kind =
      eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;
  publisherParam.qos.m_reliability.kind =
      eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
  publisherParam.qos.m_durability.kind =
      eprosima::fastrtps::VOLATILE_DURABILITY_QOS;
  publisherParam.qos.m_liveliness.kind =
      eprosima::fastrtps::AUTOMATIC_LIVELINESS_QOS;
  publisherParam.historyMemoryPolicy =
      eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
  publisherParam.topic.topicKind = eprosima::fastrtps::rtps::NO_KEY;
  publisherParam.topic.topicDataType = type_name;
  publisherParam.topic.topicName =
      _create_topic_name(ros_topic_prefix, topic_name);
  publisherParam.topic.historyQos.kind =
      eprosima::fastrtps::KEEP_LAST_HISTORY_QOS; // see rmw_fastrtps qos.cpp

  // create publisher
  publisher_ = Domain::createPublisher(participant_, publisherParam, nullptr);
  if (!publisher_) {
    std::cout << "cannot create publisher" << std::endl;
    return;
  }
}

void FastrtpsPublisher::publish(message::msg::Data &msg) {
  data_.data = &msg;
  if (!publisher_->write(&data_)) {
    std::cout << "cannot publish data" << std::endl;
    return;
  }
}

FastrtpsPublisher::~FastrtpsPublisher() {
  Domain::removeParticipant(participant_);
  }

FastrtpsSubscriber::~FastrtpsSubscriber() {
  Domain::removeParticipant(participant_);
}

void FastrtpsSubscriber::SubListener::onNewDataMessage(eprosima::fastrtps::Subscriber *sub) {
  // lock -> unlock -> notify_all is followed by ConditionalScopedLock,
  // see ConditionalScopedLock in rmw_fastrtps custom_event_info.hpp.
  // TODO: care detachCondition(conditionMutex, conditionVariable may be
  // nullptr).

  // std::cout << "onNewDataMessage" << std::endl;
  uint64_t unread_count = sub->get_unread_count();
  // std::cout << "onNewDataMessage try to lock" << std::endl;
  conditionMutex_->lock();
  // std::cout << "onNewDataMessage get lock" << std::endl;
  data_.store(unread_count, std::memory_order_relaxed);

  conditionMutex_->unlock();
  conditionVariable_->notify_all();
}

// SubscriberListener
void FastrtpsSubscriber::SubListener::onSubscriptionMatched(
    eprosima::fastrtps::Subscriber * /*sub*/,
    eprosima::fastrtps::rtps::MatchingInfo & /*info*/) {
  // std::cout << "onSubscriptionMatched" << std::endl;
}

void FastrtpsSubscriber::SubListener::attachCondition(std::mutex *conditionMutex,
                                  std::condition_variable *conditionVariable) {
  conditionMutex_ = conditionMutex;
  conditionVariable_ = conditionVariable;
}

bool FastrtpsSubscriber::SubListener::hasData() const {
  return data_.load(std::memory_order_relaxed) > 0;
}

void FastrtpsSubscriber::SubListener::data_taken(eprosima::fastrtps::Subscriber *sub) {
  // TODO: care lock
  uint64_t unread_count = sub->get_unread_count();
  data_.store(unread_count, std::memory_order_relaxed);
}

FastrtpsSubscriber::FastrtpsSubscriber(int argc, char *argv[], char *node_name, char *topic_name) {
  ////////////////////////////////
  // rmw_create_node
  // As node is not contained in FastRTPS design,
  // we only need to get Participant
  ////////////////////////////////
  Domain::getDefaultParticipantAttributes(participantAttrs_);
  participantAttrs_.domainId = static_cast<uint32_t>(domain_id);
  participantAttrs_.rtps.setName(node_name);

  // if localhost only
  // static const std::string local_ip_name("127.0.0.1");
  // local_network_interface_locator.kind = 1;
  // local_network_interface_locator.port = 0;
  // IPLocator::setIPv4(local_network_interface_locator, local_ip_name);
  // participantAttrs.rtps.builtin.metatrafficUnicastLocatorList.push_back(
  //     local_network_interface_locator);
  // participantAttrs.rtps.builtin.initialPeersList.push_back(local_network_interface_locator);

  participantAttrs_.rtps.builtin.readerHistoryMemoryPolicy =
      eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
  participantAttrs_.rtps.builtin.writerHistoryMemoryPolicy =
      eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

  size_t length = strlen(node_name) + strlen("name=;") +
                  strlen(local_namespace) + strlen("namespace=;") + 1;
  participantAttrs_.rtps.userData.resize(length);
  snprintf(reinterpret_cast<char *>(participantAttrs_.rtps.userData.data()),
           length, "name=%s;namespace=%s;", node_name, local_namespace);

  participant_ = Domain::createParticipant(participantAttrs_, nullptr);

  ////////////////////////////////
  // WaitSet (__rmw_create_wait_set)
  ////////////////////////////////
  // rmw_fastrtps implements wait mechanism by wait_for and notify_all in
  // std::conditional_variable. These variables are allocated in
  // __rmw_create_wait_set.

  ////////////////////////////////
  // rmw_create_subscription
  ////////////////////////////////
  // rmw_subscription_t *rmw_subscription = rmw_create_subscription(
  //     rmw_node,  <- node
  //     ts,        <- type_supports
  //     topic_name,
  //     &qos_profile,
  //     &subscription_option);
  const rosidl_message_type_support_t *type_supports =
      rosidl_typesupport_cpp::get_message_type_support_handle<
          message::msg::Data>();
  const rosidl_message_type_support_t *type_support =
      get_message_typesupport_handle(type_supports,
                                     RMW_FASTRTPS_CPP_TYPESUPPORT_C);
  if (!type_support) {
    type_support = get_message_typesupport_handle(
        type_supports, RMW_FASTRTPS_CPP_TYPESUPPORT_CPP);
  }
  if (!type_support) {
    std::cout << "cannot get type_support " << std::endl;
    return;
  }

  serializedData_.is_cdr_buffer = false;
  serializedData_.impl = type_support->data;

  auto callbacks =
      static_cast<const message_type_support_callbacks_t *>(type_support->data);
  std::string type_name = _create_type_name(callbacks);

  rmw_fastrtps_shared_cpp::TypeSupport *message_type =
      new MyMessageTypeSupport(callbacks);
  eprosima::fastrtps::Domain::registerType(participant_, message_type);

  eprosima::fastrtps::SubscriberAttributes subscriberParam;
  Domain::getDefaultSubscriberAttributes(subscriberParam);

  subscriberParam.qos.m_reliability.kind =
      eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
  subscriberParam.qos.m_durability.kind =
      eprosima::fastrtps::VOLATILE_DURABILITY_QOS;
  subscriberParam.qos.m_liveliness.kind =
      eprosima::fastrtps::AUTOMATIC_LIVELINESS_QOS;
  subscriberParam.historyMemoryPolicy =
      eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
  subscriberParam.topic.topicKind = eprosima::fastrtps::rtps::NO_KEY;
  subscriberParam.topic.topicDataType = type_name;
  subscriberParam.topic.topicName =
      _create_topic_name(ros_topic_prefix, topic_name);

  subscriber_ =
      Domain::createSubscriber(participant_, subscriberParam, &listener_);
}

void FastrtpsSubscriber::spin(std::chrono::nanoseconds timeout) {
  message::msg::Data data;

  listener_.attachCondition(&condition_mutex_, &condition_);
  while (1) {

    {
    // std::cout << "lock condition_mutex in main loop" << std::endl;
    std::unique_lock<std::mutex> lock(condition_mutex_);
    // TODO: set predicate i.e. check data already arrived.
    bool is_timeout = !condition_.wait_for(
        lock, timeout, [this] { return listener_.hasData(); });
    if (is_timeout) {
      return;
    }
    // std::cout << "try to lock.unlock in main loop" << std::endl;
    lock.unlock();
    // std::cout << "done lock.unlock in main loop" << std::endl;

    // skip calling listener->detachCondition.
    // I didn't catch up rmw_fastrtps lock mechanism,
    // so it may be something wrong in my code

    // rmw_take
    eprosima::fastrtps::SampleInfo_t sinfo;
    // message::msg::Data message;
    serializedData_.data = &data;

    // why takeNextData can work with &data,
    // typesupport doesn't know SerializedData, so who set this???
    // -> publisher write this struct.
    // -> By unsetting data.is_cdr_buffer or data.impl, takeNext fails with
    //    "Deserialization of data failed -> Function deserialize_change".
    // -> who knows/reads is_cdr_buffer or impl.

    if (!subscriber_->takeNextData(&serializedData_, &sinfo)) {
      std::cout << "oops, cannot takeNextData" << std::endl;
      return;
    }
    listener_.data_taken(subscriber_);
    callback_(&data, hist_, timeSeries_, pub_);
    }
  }
}

void FastrtpsSubscriber::spin_once(std::chrono::nanoseconds timeout) {
  message::msg::Data data;

  listener_.attachCondition(&condition_mutex_, &condition_);

  // std::cout << "lock condition_mutex in main loop" << std::endl;
  std::unique_lock<std::mutex> lock(condition_mutex_);
  // TODO: set predicate i.e. check data already arrived.
  bool is_timeout =
      !condition_.wait_for(lock, timeout, [this] { return listener_.hasData(); });
  if (is_timeout) {
    return;
  }
  // std::cout << "try to lock.unlock in main loop" << std::endl;
  lock.unlock();
  // std::cout << "done lock.unlock in main loop" << std::endl;

  // skip calling listener->detachCondition.
  // I didn't catch up rmw_fastrtps lock mechanism,
  // so it may be something wrong in my code

  // rmw_take
  eprosima::fastrtps::SampleInfo_t sinfo;
  // message::msg::Data message;
  serializedData_.data = &data;

  // why takeNextData can work with &data,
  // typesupport doesn't know SerializedData, so who set this???
  // -> publisher write this struct.
  // -> By unsetting data.is_cdr_buffer or data.impl, takeNext fails with
  //    "Deserialization of data failed -> Function deserialize_change".
  // -> who knows/reads is_cdr_buffer or impl.

  if (!subscriber_->takeNextData(&serializedData_, &sinfo)) {
    std::cout << "oops, cannot takeNextData" << std::endl;
    return;
  }
  listener_.data_taken(subscriber_);
  callback_(&data, hist_, timeSeries_, pub_);

}

void FastrtpsSubscriber::set_callback(void (*callback)(message::msg::Data *data, HistReport *hist,
                     TimeSeriesReport *timeSeries, PublisherBase *pub),
    HistReport *hist, TimeSeriesReport *timeSeries, PublisherBase *pub) {
  callback_ = callback;
  hist_ = hist;
  timeSeries_ = timeSeries;
  pub_ = pub;
}
