#include <iostream>
#include "timewheel.h"


namespace mian{

TimeWheel::TimeWheel(const uint32_t max, const uint64_t interval):
    max_num_(max + 1),
    interval_(interval),
    current_(0) {
    wheel_.reserve(max_num_);
    int num = max_num_;
    while (num-- > 0) {
      wheel_.push_back(std::unique_ptr<TimerList>(new TimerList()));
    }
}

TimeWheel::~TimeWheel() {

}

int TimeWheel::Add(const int fd, const int ev_type, const uint64_t relative_time) {
  return Add(std::shared_ptr<Event>(new Event(fd, ev_type, relative_time)), relative_time);
}

int TimeWheel::Add(const std::shared_ptr<Event> event, const uint64_t relative_time) {
  if (relative_time < interval_ || relative_time > max_num_ * interval_) {
    return -1;// return false 
  }

  int index = ((relative_time / interval_) + current_) % max_num_;
  //std::cout << "current: " << current_ << ", index:" << index << ", fd: " << event->fd_ << ", timeout:" << event->timeout_ << std::endl;
  wheel_[index]->Push(event);
  return 0;
}

std::shared_ptr<Event> TimeWheel::PopExpired() {
  return wheel_[current_]->Pop();
}

}// namespace mian
