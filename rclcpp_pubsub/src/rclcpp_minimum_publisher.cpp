#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "message/msg/sample.hpp"

using namespace std::chrono_literals;

const char * node_name = "rclcpp_publisher";
const char * namespace_ = "ns";
const char * topic_name = "topic";
const int qos = 10;

class MinimumPublisherSampleMsg : public rclcpp::Node
{
public:
  MinimumPublisherSampleMsg() : Node(node_name, namespace_), count(0)
  {
    publisher = this->create_publisher<message::msg::Sample>(topic_name, qos);

    timer = this->create_wall_timer(500ms,
                                    [this]() -> void
                                      {
                                        std::cout << "send message::msg::Sample" << std::endl;
                                        message::msg::Sample msg;
                                        msg.a = 1;
                                        msg.b = 2;
                                        this->count++;
                                        publisher->publish(msg);
                                      });
  }

private:
  rclcpp::TimerBase::SharedPtr timer;
  rclcpp::Publisher<message::msg::Sample>::SharedPtr publisher;
  int count;
};

class MinimumPublisher : public rclcpp::Node
{
public:
  MinimumPublisher() : Node(node_name, namespace_), count(0)
  {

    publisher = this->create_publisher<std_msgs::msg::String>(topic_name, qos);

    timer = this->create_wall_timer(500ms,
                                    [this]() -> void
                                      {
                                        std::cout << "send std_msgs::msg::String" << std::endl;
                                        std_msgs::msg::String msg;
                                        msg.data = "HelloWorld" + std::to_string(this->count);
                                        this->count++;
                                        publisher->publish(msg);
                                      });
  }

private:
  rclcpp::TimerBase::SharedPtr timer;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher;
  int count;
};

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  // rclcpp::spin(std::make_shared<>(MinimumPublisherSampleMsg));
  rclcpp::spin(std::make_shared<MinimumPublisher>());
  rclcpp::shutdown();

  return 0;
}
