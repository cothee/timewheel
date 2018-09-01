#include <iostream>
#include <utility>
#include "timewheel.h"

namespace peanuts{

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

int TimeWheel::Add(const int fd, const uint32_t relative_time) {
  return Add({fd, relative_time});
}

int TimeWheel::Add(const Event& event) {
  if (event.timeout_ < interval_ || event.timeout_ > max_num_ * interval_) {
    return -1;// return false 
  }

  int index = ((event.timeout_ / interval_) + current_) % max_num_;
  {
    std::lock_guard<std::mutex> lock(map_mtx_);
    map_[event.fd_] = wheel_[index]->Push(event);
  }
  return 0;
}

bool TimeWheel::PopExpired(Event* ev) {
  if (wheel_[current_]->Empty()) {
    return false;
  }
  if (ev != NULL) {
    *ev = wheel_[current_]->Top();
    wheel_[current_]->Pop();
  }
  return true;
}

int TimeWheel::Remove(const int fd) {
  std::lock_guard<std::mutex> lock(map_mtx_);
  std::unordered_map<int, std::shared_ptr<Timer<Event>>>::iterator it = map_.find(fd);
  if (it != map_.end()) {
    std::shared_ptr<Timer<Event>> ptr = std::move(it->second);
    if (ptr) {
      (ptr->prev_.lock())->next_ = ptr->next_;
      if (ptr->next_) {
        ptr->next_->prev_ = ptr->prev_;
      }
      ptr.reset();
      return 0;
    }
  }
  return -1;
}


int TimeWheel::Remove(const Event& event) {
  return Remove(event.fd_);
}

}// namespace peanuts
