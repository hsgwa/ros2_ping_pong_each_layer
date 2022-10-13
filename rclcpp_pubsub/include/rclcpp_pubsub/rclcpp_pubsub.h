#include "rclcpp/rclcpp.hpp"
#include "message/msg/data.hpp"
#include "layer_pubsub_common/interface.hpp"
#include <rclcpp/publisher.hpp>

class RclcppPublisher : public PublisherBase {
public:
  RclcppPublisher(int argc, char *argv[], char *node_name, char *topic_name);
  ~RclcppPublisher(){};

  void publish(message::msg::Data &msg) override;

private:
  std::shared_ptr<rclcpp::Node> node_;
  rclcpp::Publisher<message::msg::Data>::SharedPtr pub_;
  std::shared_ptr<rclcpp::executors::SingleThreadedExecutor> exec_;
};


class RclcppSubscriber : public SubscriberBase {
public:
  RclcppSubscriber(int argc, char *argv[], char *node_name, char *topic_name);
  ~RclcppSubscriber(){};

  void set_callback(void (*callback)(message::msg::Data *data, HistReport *hist,
                                     TimeSeriesReport *timeSeries,
                                     PublisherBase *pub),
                    HistReport *hist, TimeSeriesReport *timeSeries,
                    PublisherBase *pub) override;

  void spin(std::chrono::nanoseconds timeout) override;
  void spin_once(std::chrono::nanoseconds timeout) override;

private:
  void sub_callback(const message::msg::Data::UniquePtr msg);
  HistReport *hist_;
  TimeSeriesReport *timeSeries_;
  PublisherBase *pub_;
  void (*callback_)(message::msg::Data *data, HistReport *hist,
                   TimeSeriesReport *timeSeries, PublisherBase *pub);

  std::shared_ptr<rclcpp::executors::SingleThreadedExecutor> exec_;
  std::shared_ptr<rclcpp::Node> node_;
  rclcpp::Subscription<message::msg::Data>::SharedPtr sub_;
  struct timespec last_exec_;
  char *topic_name_;
  char *node_name_;
  bool done_callback_;
};

