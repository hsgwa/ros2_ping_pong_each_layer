#include "my_message_type_support.hpp"

std::string
_create_type_name(
  const message_type_support_callbacks_t * members)
{
  if (!members) {
    std::cout << "members handle is null" << std::endl;
    return "";
  }

  std::ostringstream ss;
  std::string message_namespace(members->message_namespace_);
  std::string message_name(members->message_name_);
  if (!message_namespace.empty()) {
    ss << message_namespace << "::";
  }
  ss << "dds_::" << message_name << "_";
  return ss.str();
}

MyMessageTypeSupport::MyMessageTypeSupport(const message_type_support_callbacks_t * members)
{
  assert(members);

  m_isGetKeyDefined = false;
  max_size_bound_ = false;

  std::string name = _create_type_name(members);
  this->setName(name.c_str());

  set_members(members);
}

void MyMessageTypeSupport::set_members(const message_type_support_callbacks_t * members)
{
  members_ = members;

  // Fully bound by default
  max_size_bound_ = true;
  auto data_size = static_cast<uint32_t>(members->max_serialized_size(max_size_bound_));

  // A fully bound message of size 0 is an empty message
  if (max_size_bound_ && (data_size == 0) ) {
    has_data_ = false;
    ++data_size;  // Dummy byte
  } else {
    has_data_ = true;
  }

  // Total size is encapsulation size + data size
  m_typeSize = 4 + data_size;
}

size_t MyMessageTypeSupport::getEstimatedSerializedSize(const void * ros_message, const void * impl) const
{
  if (max_size_bound_) {
    return m_typeSize;
  }

  assert(ros_message);
  assert(impl);

  auto callbacks = static_cast<const message_type_support_callbacks_t *>(impl);

  // Encapsulation size + message size
  return 4 + callbacks->get_serialized_size(ros_message);
}

bool MyMessageTypeSupport::serializeROSmessage(
  const void * ros_message, eprosima::fastcdr::Cdr & ser, const void * impl) const
{
  assert(ros_message);
  assert(impl);

  // Serialize encapsulation
  ser.serialize_encapsulation();

  // If type is not empty, serialize message
  if (has_data_) {
    auto callbacks = static_cast<const message_type_support_callbacks_t *>(impl);
    return callbacks->cdr_serialize(ros_message, ser);
  }

  // Otherwise, add a dummy byte
  ser << (uint8_t)0;
  return true;
}

bool MyMessageTypeSupport::deserializeROSmessage(
  eprosima::fastcdr::Cdr & deser, void * ros_message, const void * impl) const
{
  assert(ros_message);
  assert(impl);

  // Deserialize encapsulation.
  deser.read_encapsulation();

  // If type is not empty, deserialize message
  if (has_data_) {
    auto callbacks = static_cast<const message_type_support_callbacks_t *>(impl);
    return callbacks->cdr_deserialize(deser, ros_message);
  }

  // Otherwise, consume dummy byte
  uint8_t dump = 0;
  deser >> dump;
  (void)dump;

  return true;
}
