#include <bits/stdint-uintn.h>
#include <cstdio>
#include <ctime>
// #include <rclcpp/node.hpp>
#include <sched.h>
#define _GNU_SOURCE
#include <thread>
#include <pthread.h>

#include "osrf_testing_tools_cpp/memory_tools/memory_tools.hpp"
#include "osrf_testing_tools_cpp/scope_exit.hpp"

#include "fastrtps_pubsub/fastrtps_pubsub.h"
#include "rcl_pubsub/rcl_pubsub.h"
#include "rclcpp_pubsub/rclcpp_pubsub.h"
#include "rmw_pubsub/rmw_pubsub.h"

#include "rttest/rttest.h"
#include "rttest/utils.hpp"

#include "util.hpp"

#include <getopt.h> // for getopt_long
#include <unistd.h> // for getopt

#include "layer_pubsub_common/interface.hpp"

using namespace std;

void on_unexpected_memory(
    osrf_testing_tools_cpp::memory_tools::MemoryToolsService &service) {
  service.print_backtrace();
}

struct Params {
  rttest_params rt;
  std::string layer;
  std::string type;
  std::string side;
  std::string ping_hist_filename;
  std::string ping_topn_filename;
  std::string ping_timeseries_filename;
  std::string pong_hist_filename;
  std::string pong_topn_filename;
  std::string pong_timeseries_filename;
};

Params get_params(int argc, char *argv[]);

void ping(int argc, char **argv, const Params &params);
void pong(int argc, char **argv, const Params &params);

static auto timeout = std::chrono::milliseconds(1000);
template <typename CallbackT> void sampleFunc(CallbackT &&callback) {
  callback();
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  Params params = get_params(argc, argv);
  using namespace std;

  if (params.layer == "rclcpp") {
    rclcpp::init(argc, argv);
  }

  osrf_testing_tools_cpp::memory_tools::initialize();
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
      { osrf_testing_tools_cpp::memory_tools::uninitialize(); });

  osrf_testing_tools_cpp::memory_tools::on_unexpected_calloc(
      on_unexpected_memory);
  osrf_testing_tools_cpp::memory_tools::on_unexpected_free(
      on_unexpected_memory);
  osrf_testing_tools_cpp::memory_tools::on_unexpected_malloc(
      on_unexpected_memory);
  osrf_testing_tools_cpp::memory_tools::on_unexpected_realloc(
      on_unexpected_memory);

  osrf_testing_tools_cpp::memory_tools::enable_monitoring_in_all_threads();

  if (rttest_lock_and_prefault_dynamic() != 0) {
    fprintf(stderr, "Couldn't lock all cached virtual memory. errno = %d \n",
      errno);
    fprintf(stderr, "Pagefaults from reading pages not yet mapped into RAM "
			  "will be recorded.\n");
  }

  if (params.type == "intra") {
    std::thread s(pong, argc, argv, params);
    std::thread p(ping, argc, argv, params);

    s.join();
    p.join();
  } else if ( params.type == "inter") {
    if (params.side == "ping") {
      std::thread p(ping, argc, argv, params);
      p.join();
    } else if (params.side == "pong") {
      std::thread s(pong, argc, argv, params);
      s.join();
    } else {
      std::cout << "side is not either ping/pong." << std::endl;
    }
  } else {
    std::cout << "type is not either intra/inter." << std::endl;
  }

  if (params.layer == "rclcpp") {
    rclcpp::shutdown();
  }
  return 0;
}

Params get_params(int argc, char *argv[]) {
  Params params;
  const struct option longopts[] = {
      {"layer", required_argument, 0, 'l'},
      {"type", required_argument, 0, 'p'},
      {"side", required_argument, 0, 's'},
      {"ping_hist_filename", required_argument, 0, 'h'},
      {"ping_topn_filename", required_argument, 0, 'n'},
      {"ping_timeseries_filename", required_argument, 0, 't'},
      {"pong_hist_filename", required_argument, 0, 'i'},
      {"pong_topn_filename", required_argument, 0, 'o'},
      {"pong_timeseries_filename", required_argument, 0, 'u'},
      {0, 0, 0, 0}};

  opterr = 0;
  optind = 1;
  int longindex = 0;
  int c;

  const std::string optstring = "+";
  while ((c = getopt_long(argc, argv, optstring.c_str(), longopts,
                          &longindex)) != -1) {
    switch (c) {
    case ('p'):
      params.type = optarg;
      break;
    case ('s'):
      params.side = optarg;
      break;
    case ('l'):
      params.layer = optarg;
      break;
    case ('h'):
      params.ping_hist_filename = optarg;
      break;
    case ('n'):
      params.ping_topn_filename = optarg;
      break;
    case ('t'):
      params.ping_timeseries_filename = optarg;
      break;
    case ('i'):
      params.pong_hist_filename = optarg;
      break;
    case ('o'):
      params.pong_topn_filename = optarg;
      break;
    case ('u'):
      params.pong_timeseries_filename = optarg;
      break;
    }
  }
  argc -= optind - 2;
  argv += optind - 2;

  if (rttest_read_args(argc, argv) != 0) {
    perror("Couldn't read arguments for rttest");
  }
  rttest_get_params(&params.rt);
  return params;
}

void pong(int argc, char **argv, const Params &params) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(1, &cpuset);
  int rc = sched_setaffinity(0 , sizeof(cpu_set_t), &cpuset);
  if (rc != 0) {
  	std::cerr << "Error calling pthread_setaffinity_np : " << rc << std::endl;
  }

  rttest_set_sched_priority(97, SCHED_RR);
  std::shared_ptr<SubscriberBase> sub;
  std::shared_ptr<PublisherBase> pub;
  char ping_topic_name[] = "/ping";
  char pong_topic_name[] = "/pong";

  char pong_node_name[] = "rclcpp_pong_publisher";
  char sub_node_name[] = "rclcpp_pong_subscriber";

  if (params.layer == "rclcpp") {
    sub = std::make_shared<RclcppSubscriber>(argc, argv, sub_node_name, ping_topic_name);
    pub = std::make_shared<RclcppPublisher>(argc, argv, pong_node_name, pong_topic_name);
  } else if (params.layer == "rcl") {
    sub = std::make_shared<RclSubscriber>(argc, argv, sub_node_name, ping_topic_name);
    pub = std::make_shared<RclPublisher>(argc, argv, pong_node_name, pong_topic_name);
  } else if (params.layer == "rmw") {
    sub = std::make_shared<RmwSubscriber>(argc, argv, sub_node_name, ping_topic_name);
    pub = std::make_shared<RmwPublisher>(argc, argv, pong_node_name, pong_topic_name);
  } else if (params.layer == "dds") {
    sub = std::make_shared<FastrtpsSubscriber>(argc, argv, sub_node_name, ping_topic_name);
    pub = std::make_shared<FastrtpsPublisher>(argc, argv, pong_node_name, pong_topic_name);
  } else {
    cout << "layer name error";
    return;
  }

  HistReport *hist = nullptr;
  TimeSeriesReport *timeSeries = nullptr;
  if (!params.pong_hist_filename.empty() || !params.pong_timeseries_filename.empty()) {
    hist = new HistReport(5000);
  }
  if (!params.pong_timeseries_filename.empty()) {
    timeSeries = new TimeSeriesReport(params.rt.iterations);
  }

  uint64_t pub_latency[params.rt.iterations] = {0};
  int i;

  // wait before no_malloc_begin until other initialize finish.
  rttest_set_sched_priority(98, SCHED_RR);
  std::this_thread::sleep_for(timeout);
  osrf_testing_tools_cpp::memory_tools::expect_no_malloc_begin();

  auto timeout_ = std::chrono::nanoseconds(3000000000LL);

  // TODO: modify lambda
  sub->set_callback(
      [](message::msg::Data *msg, HistReport *hist,
         TimeSeriesReport *timeSeries, PublisherBase *pub) {
        static int i = 0;
        static struct timespec sub_time_spec;
        static uint64_t sub_time, latency;
        clock_gettime(CLOCK_MONOTONIC, &sub_time_spec);

        msg->time_sent_pong_ns = timespec_to_uint64(&sub_time_spec);
        pub->publish(*msg);

        sub_time = timespec_to_uint64(&sub_time_spec);
        latency = sub_time - msg->time_sent_ns;

        if (hist) {
          hist->add(latency);
        }
        if (timeSeries) {
          timeSeries->add(latency);
        }
        i++;
      },
      hist, timeSeries, pub.get());

  sub->spin(timeout_);
  osrf_testing_tools_cpp::memory_tools::expect_no_malloc_end();

  // export to csv files.
  if (!params.pong_hist_filename.empty()) {
    hist->histToCsv(params.pong_hist_filename);
  }
  if (!params.pong_topn_filename.empty()) {
    hist->topnToHist(params.pong_topn_filename);
  }
  if (!params.pong_timeseries_filename.empty()) {
    timeSeries->toCsv(params.pong_timeseries_filename);
  }
}

void ping(int argc, char **argv, const Params &params) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(2, &cpuset);
  int rc = sched_setaffinity(0 , sizeof(cpu_set_t), &cpuset);
  if (rc != 0) {
  	std::cerr << "Error calling pthread_setaffinity_np : " << rc << std::endl;
  }

  char pub_node_name[] = "rclcpp_ping_publisher";
  char sub_node_name[] = "rclcpp_ping_subscriber";

  char ping_topic_name[] = "/ping";
  char pong_topic_name[] = "/pong";

  rttest_set_sched_priority(97, SCHED_RR);
  std::shared_ptr<PublisherBase> pub;
  std::shared_ptr<SubscriberBase> sub;

  if (params.layer == "rclcpp") {
    pub = std::make_shared<RclcppPublisher>(argc, argv, pub_node_name, ping_topic_name);
    sub = std::make_shared<RclcppSubscriber>(argc, argv, sub_node_name, pong_topic_name);
  } else if (params.layer == "rcl") {
    pub = std::make_shared<RclPublisher>(argc, argv, pub_node_name, ping_topic_name);
    sub = std::make_shared<RclSubscriber>(argc, argv, sub_node_name, pong_topic_name);
  } else if (params.layer == "rmw") {
    pub = std::make_shared<RmwPublisher>(argc, argv, pub_node_name, ping_topic_name);
    sub = std::make_shared<RmwSubscriber>(argc, argv, sub_node_name, pong_topic_name);
  } else if (params.layer == "dds") {
    pub = std::make_shared<FastrtpsPublisher>(argc, argv, pub_node_name, ping_topic_name);
    sub = std::make_shared<FastrtpsSubscriber>(argc, argv, sub_node_name, pong_topic_name);
  } else {
    cout << "layer name error";
    return;
  }

  int count = 0;
  message::msg::Data msg;
  struct timespec wake_expected;
  struct timespec pub_time;
  struct timespec period;
  period.tv_sec = 1;
  clock_gettime(CLOCK_MONOTONIC, &wake_expected);
  int i;

  rttest_set_sched_priority(98, SCHED_RR);
  auto duration = toChronoDuration(params.rt.update_period);

  HistReport *hist = nullptr;
  TimeSeriesReport *timeSeries = nullptr;
  if (!params.ping_hist_filename.empty() || !params.ping_timeseries_filename.empty()) {
    hist = new HistReport(5000);
  }
  if (!params.ping_timeseries_filename.empty()) {
    timeSeries = new TimeSeriesReport(params.rt.iterations);
  }

  uint64_t pub_latency[params.rt.iterations] = {0};
  sub->set_callback(
      [](message::msg::Data *msg, HistReport *hist,
         TimeSeriesReport *timeSeries, PublisherBase *pub) {

        static int i = 0;
        static struct timespec sub_time_spec;
        static uint64_t sub_time, latency;

        clock_gettime(CLOCK_MONOTONIC, &sub_time_spec);
        sub_time = timespec_to_uint64(&sub_time_spec);
        latency = sub_time - msg->time_sent_ns;

        if (hist) {
          hist->add(latency);
        }
        if (timeSeries) {
          timeSeries->add(latency);
        }
        i++;
      },
      hist, timeSeries, nullptr);

  std::this_thread::sleep_for(timeout);
  std::this_thread::sleep_for(timeout); // wait until spin ready.
  osrf_testing_tools_cpp::memory_tools::expect_no_malloc_begin();

  auto timeout_ = std::chrono::nanoseconds(3000000000LL);
  for (i = 0; i < params.rt.iterations; i++) {
    std::this_thread::sleep_for(duration);
    clock_gettime(CLOCK_MONOTONIC, &pub_time);
    msg.data = count++;
    msg.time_sent_ns = timespec_to_uint64(&pub_time);
    pub->publish(msg);

    sub->spin_once(timeout_);
  }

  osrf_testing_tools_cpp::memory_tools::expect_no_malloc_end();

  // export to csv files.
  if (!params.ping_hist_filename.empty()) {
    hist->histToCsv(params.ping_hist_filename);
  }
  if (!params.ping_topn_filename.empty()) {
    hist->topnToHist(params.ping_topn_filename);
  }
  if (!params.ping_timeseries_filename.empty()) {
    timeSeries->toCsv(params.ping_timeseries_filename);
  }
}
