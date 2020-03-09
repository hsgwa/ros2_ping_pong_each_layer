#include <iostream>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "message/msg/sample.hpp"

const char * node_name = "rclcpp_subscriber";
const char * namespace_ = "ns";
const char * topic_name = "topic";
const int qos = 10;

class MinimumSubscriberSampleMsg : public rclcpp::Node
{
public:
  MinimumSubscriberSampleMsg() : Node(node_name, namespace_)
  {
    subscriber = this->create_subscription<message::msg::Sample>(
        topic_name,
        qos,
        [this](const message::msg::Sample::UniquePtr msg) {
          RCLCPP_INFO(this->get_logger(), "I heard: [%d]", msg->a);
        });
  }

private:
  rclcpp::Subscription<message::msg::Sample>::SharedPtr subscriber;
};

class MinimumSubscriber : public rclcpp::Node
{
public:
  MinimumSubscriber() : Node(node_name, namespace_)
  {
    subscriber = this->create_subscription<std_msgs::msg::String>(
        topic_name,
        qos,
        [this](const std_msgs::msg::String::UniquePtr msg) {
          RCLCPP_INFO(this->get_logger(), "I heard: [%s]", msg->data.c_str());
        });
  }

private:
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscriber;
};

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  // rclcpp::spin(std::make_shared<MinimumSubscriberSampleMsg>());
  rclcpp::spin(std::make_shared<MinimumSubscriber>());
  rclcpp::shutdown();

  return 0;
}
