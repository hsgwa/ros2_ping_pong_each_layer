#pragma once

#include <chrono>

#include "message/msg/data.hpp"
#include "util.hpp"

class PublisherBase {
public:
  virtual void publish(message::msg::Data &msg){};
  virtual ~PublisherBase(){};
};

class SubscriberBase {
public:
  virtual void set_callback(
      void (*callback)(message::msg::Data *data, HistReport *hist,
                       TimeSeriesReport *timeSeries, PublisherBase *pub),
      HistReport *hist, TimeSeriesReport *timeSeries, PublisherBase *pub){};

  virtual void spin(std::chrono::nanoseconds timeout){};
  virtual void spin_once(std::chrono::nanoseconds timeout){};

  virtual ~SubscriberBase(){};
};

