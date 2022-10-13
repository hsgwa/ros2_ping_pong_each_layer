#include <chrono>
#include <memory>
#include <mutex>

#include <rclcpp/node.hpp>
#include <rclcpp/strategies/allocator_memory_strategy.hpp>
#include <rclcpp/strategies/message_pool_memory_strategy.hpp>
#include <tlsf_cpp/tlsf.hpp>

#include "message/msg/data.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_pubsub/rclcpp_pubsub.h"
#include "rttest/utils.hpp"

using namespace std::chrono_literals;

const char *namespace_ = "ns";
const int qos_history_depth = 1;

template <typename T = void>
using TLSFAllocator = tlsf_heap_allocator<T>;

using rclcpp::memory_strategies::allocator_memory_strategy::
    AllocatorMemoryStrategy;
using rclcpp::strategies::message_pool_memory_strategy::
    MessagePoolMemoryStrategy;

using std::placeholders::_1;

class ExecutorSingleton {
private:
  ExecutorSingleton() = default;
  ~ExecutorSingleton() = default;

public:
  ExecutorSingleton(const ExecutorSingleton &) = delete;
  ExecutorSingleton &operator=(const ExecutorSingleton &) = delete;
  ExecutorSingleton(ExecutorSingleton &&) = delete;
  ExecutorSingleton &operator=(ExecutorSingleton &&) = delete;

  static std::shared_ptr<rclcpp::executors::SingleThreadedExecutor> get_instance(int argc, char *argv[]) {
    static std::shared_ptr<rclcpp::executors::SingleThreadedExecutor> exec = nullptr;
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    rclcpp::ExecutorOptions options;
    rclcpp::memory_strategy::MemoryStrategy::SharedPtr memory_strategy =
        std::make_shared<AllocatorMemoryStrategy<TLSFAllocator<void>>>();
    options.memory_strategy = memory_strategy;
    exec = std::make_shared<rclcpp::executors::SingleThreadedExecutor>(options);
    return exec;
  }
};


RclcppPublisher::RclcppPublisher(int argc, char *argv[], char *node_name, char *topic_name) :
    exec_(ExecutorSingleton::get_instance(argc, argv))
{
  node_ = std::make_shared<rclcpp::Node>(node_name, namespace_);
  pub_ = node_->create_publisher<message::msg::Data>(topic_name,
                                                     qos_history_depth);
  exec_->add_node(node_);
}

void RclcppPublisher::publish(message::msg::Data &msg) { pub_->publish(msg); }

RclcppSubscriber::RclcppSubscriber(int argc, char *argv[], char *node_name, char *topic_name) :
    exec_(ExecutorSingleton::get_instance(argc, argv))
{
  topic_name_ = topic_name;
  node_name_ = node_name;
}

void RclcppSubscriber::spin(std::chrono::nanoseconds timeout) {
  static struct timespec now, duration;

  uint64_t duration_ns;
  clock_gettime(CLOCK_MONOTONIC, &last_exec_);
  uint64_t timeout_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();

  while (rclcpp::ok()) {
    exec_->spin_once(timeout);
    clock_gettime(CLOCK_MONOTONIC, &now);
    subtract_timespecs(&last_exec_, &now, &duration);
    duration_ns = timespec_to_uint64(&duration);
    if (duration_ns >= timeout_ns) {
      return;
    }
  }
}

void RclcppSubscriber::spin_once(
    std::chrono::nanoseconds timeout) {

  static struct timespec last_exec, now, duration;

  uint64_t duration_ns;
  clock_gettime(CLOCK_MONOTONIC, &last_exec);
  uint64_t timeout_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();

  done_callback_ = false;
  while (rclcpp::ok() && !done_callback_) {
    exec_->spin_once(timeout);

    clock_gettime(CLOCK_MONOTONIC, &now);
    subtract_timespecs(&last_exec, &now, &duration);
    duration_ns = timespec_to_uint64(&duration);
    if (duration_ns >= timeout_ns) {
      return;
    }
 }
}

void RclcppSubscriber::sub_callback(const message::msg::Data::UniquePtr msg){
  callback_(msg.get(), hist_, timeSeries_, pub_);
  clock_gettime(CLOCK_MONOTONIC, &last_exec_);
  done_callback_ = true;
}

void RclcppSubscriber::set_callback(
    void (*callback)(message::msg::Data *data, HistReport *hist,
                     TimeSeriesReport *timeSeries, PublisherBase *pub),
    HistReport *hist, TimeSeriesReport *timeSeries, PublisherBase *pub) {

  callback_ = callback;
  hist_ = hist;
  timeSeries_ = timeSeries;
  pub_ = pub;
  node_ = rclcpp::Node::make_shared(node_name_, namespace_);
  sub_ = node_->create_subscription<message::msg::Data>(
      topic_name_, qos_history_depth, std::bind(&RclcppSubscriber::sub_callback, this, _1));

  exec_->add_node(node_);
}
