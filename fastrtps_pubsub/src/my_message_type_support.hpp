#ifndef MY_MESSAGE_TYPE_SUPPORT_HPP_
#define MY_MESSAGE_TYPE_SUPPORT_HPP_

#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"  // for message_type_support_callbacks_t
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"

std::string
_create_type_name(
    const message_type_support_callbacks_t * members);


class MyMessageTypeSupport : public rmw_fastrtps_shared_cpp::TypeSupport
{
 public:
  explicit MyMessageTypeSupport(const message_type_support_callbacks_t * members);

  size_t getEstimatedSerializedSize(const void * ros_message, const void * impl) const;

  bool serializeROSmessage(
    const void * ros_message, eprosima::fastcdr::Cdr & ser, const void * impl) const;

  bool deserializeROSmessage(
    eprosima::fastcdr::Cdr & deser, void * ros_message, const void * impl) const;

protected:
  void set_members(const message_type_support_callbacks_t * members);

private:
  const message_type_support_callbacks_t * members_;
  bool has_data_;
};

#endif // MY_MESSAGE_TYPE_SUPPORT_HPP_
