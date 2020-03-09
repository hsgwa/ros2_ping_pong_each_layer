#include <iostream>

//#include "fastrtps/config.h"
#include "fastrtps/Domain.h"
#include "fastrtps/rtps/common/Locator.h"
#include "fastrtps/participant/Participant.h"
#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/subscriber/SubscriberListener.h"
#include "fastrtps/subscriber/SampleInfo.h"
#include "fastrtps/attributes/SubscriberAttributes.h"
#include "fastrtps/utils/IPLocator.h"

#include "fastrtps/rtps/RTPSDomain.h"

#include "fastrtps/rtps/reader/RTPSReader.h"
#include "fastrtps/rtps/reader/StatefulReader.h"
#include "fastrtps/rtps/reader/ReaderListener.h"
#include "fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h"

#include "std_msgs/msg/string.hpp"

#include "rosidl_generator_c/message_type_support_struct.h"  // get_message_typesupport_handle

#include "rosidl_typesupport_cpp/identifier.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp"  // rosidl_typesupport_cpp::get_message_type_support_handle

#include "rosidl_typesupport_fastrtps_c/identifier.h"  // for rosidl_typesupport_fastrtps_c__identifier
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"  // for message_type_support_callbacks_t
#include "rosidl_typesupport_fastrtps_cpp/identifier.hpp"  // for rosidl_typesupport_fastrtps_cpp::typesupport_identifier

#include "my_message_type_support.hpp"

using Domain = eprosima::fastrtps::Domain;
using IPLocator = eprosima::fastrtps::rtps::IPLocator;
using Locator_t = eprosima::fastrtps::rtps::Locator_t;
using Participant = eprosima::fastrtps::Participant;
using ParticipantAttributes = eprosima::fastrtps::ParticipantAttributes;
using StatefulReader = eprosima::fastrtps::rtps::StatefulReader;
using Publisher = eprosima::fastrtps::Publisher;

#define RMW_FASTRTPS_CPP_TYPESUPPORT_C   rosidl_typesupport_fastrtps_c__identifier
#define RMW_FASTRTPS_CPP_TYPESUPPORT_CPP rosidl_typesupport_fastrtps_cpp::typesupport_identifier

inline
eprosima::fastrtps::string_255
_create_topic_name(
  const char * prefix,
  const char * base,
  const char * suffix = nullptr)
{
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

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  // configuration
  const size_t domain_id = 0;
  const char * local_namespace = "/ns";
  const char * node_name = "fastrtps_publisher";
  const char * topic_name = "/ns/topic";
  const char * const ros_topic_prefix = "rt"; // "rt"

  ////////////////////////////////
  // rmw_create_node
  // As node is not contained in FastRTPS design,
  // we only need to get Participant
  ////////////////////////////////
  ParticipantAttributes participantAttrs;
  Domain::getDefaultParticipantAttributes(participantAttrs);
  participantAttrs.rtps.builtin.domainId = static_cast<uint32_t>(domain_id);
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
  Participant * participant = Domain::createParticipant(participantAttrs, nullptr);

  ////////////////////////////////
  // rmw_create_publisher
  ////////////////////////////////
  // I don't know why, but segv happens in the following code:
  //   const rosidl_message_type_support_t *type_support =
  //      rosidl_typesupport_cpp::get_message_type_support_handle<std_msgs::msg::String>();
  //   auto callbacks = static_cast<const message_type_support_callbacks_t *>(type_support->data);
  //   callbacks.message_namespace_  // segv here
  const rosidl_message_type_support_t *type_supports =
      rosidl_typesupport_cpp::get_message_type_support_handle<std_msgs::msg::String>();
  const rosidl_message_type_support_t *type_support = get_message_typesupport_handle(
    type_supports, RMW_FASTRTPS_CPP_TYPESUPPORT_C);
  if (!type_support) {
    type_support = get_message_typesupport_handle(
        type_supports, RMW_FASTRTPS_CPP_TYPESUPPORT_CPP);
  }
  if (!type_support) {
    std::cout << "cannot get type_support " << std::endl;
    return -1;
  }
  auto callbacks = static_cast<const message_type_support_callbacks_t *>(type_support->data);
  std::string type_name = _create_type_name(callbacks);
  std::cout << "type_name: " << type_name << std::endl;

  // TODO: create topicDataType for std_msgs::msg::dds_::String_
  rmw_fastrtps_shared_cpp::TypeSupport *message_type = new MyMessageTypeSupport(callbacks);
  eprosima::fastrtps::Domain::registerType(participant, message_type);

  eprosima::fastrtps::PublisherAttributes publisherParam;
  Domain::getDefaultPublisherAttributes(publisherParam);
  publisherParam.qos.m_publishMode.kind = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;
  publisherParam.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
  publisherParam.qos.m_durability.kind = eprosima::fastrtps::VOLATILE_DURABILITY_QOS;
  publisherParam.qos.m_liveliness.kind = eprosima::fastrtps::AUTOMATIC_LIVELINESS_QOS;
  publisherParam.historyMemoryPolicy =
      eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
  publisherParam.topic.topicKind = eprosima::fastrtps::rtps::NO_KEY;
  publisherParam.topic.topicDataType = type_name;
  publisherParam.topic.topicName = _create_topic_name(ros_topic_prefix, topic_name);
  std::cout << "topicName: " << publisherParam.topic.topicName << std::endl;
  publisherParam.topic.historyQos.kind = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS; // see rmw_fastrtps qos.cpp

  // create publisher
  Publisher *publisher = Domain::createPublisher(participant, publisherParam, nullptr);
  if (!publisher) {
    std::cout << "cannot create publisher" << std::endl;
    return -1;
  }

  // publish
  for(int i=0; i<10; i++) {
    std_msgs::msg::String msg;
    msg.data = "Hello World";
    rmw_fastrtps_shared_cpp::SerializedData data;
    data.is_cdr_buffer = false;
    data.data = &msg;
    data.impl = callbacks;
    if (!publisher->write(&data)) {
      std::cout << "cannot publish data" << std::endl;
      return -1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}
