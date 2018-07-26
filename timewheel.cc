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
      wheel_.push_back(std::shared_ptr<TimerList>(new TimerList()));
    }
    to_add_ = std::shared_ptr<TimerList>(new TimerList());
    expired_ = std::shared_ptr<TimerList>(new TimerList());
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

  {
    std::lock_guard<std::mutex> add_lock(add_mutex_);
    to_add_->Push(event);
  }
  //std::cout << "current: " << current_ << ", index:" << index << ", fd: " << event->fd_ << ", timeout:" << event->timeout_ << std::endl;
  //wheel_[index]->Push(event);
  return 0;
}

/*step forward by one interval, make sure it is called just by one thread*/
void TimeWheel::Step() {
  {
    std::lock_guard<std::mutex> add_lock(add_mutex_);

    current_ = (current_ + 1) % max_num_;

    std::shared_ptr<Timer<Event>> timer = to_add_->head_->next_;
    std::shared_ptr<Timer<Event>> temp = nullptr;
  
    int index;
    uint64_t timeout;
    while (timer) {
      timeout = timer->ele_->timeout_;
      index = ((timeout / interval_) + current_) % max_num_;
      temp = timer->next_;
      
      /*timer->next_ was changed by Push*/
      wheel_[index]->Push(timer);
      timer = temp;
    }
    to_add_->Clear();
  }

  {
    std::lock_guard<std::mutex> expired_lock(expired_mutex_);
    expired_->Push(wheel_[current_]);
    wheel_[current_]->Clear();
  }
}

std::shared_ptr<Event> TimeWheel::PopExpired() {
  {
    std::lock_guard<std::mutex> expired_lock(expired_mutex_);
    return expired_->Pop();
  }
}

}// namespace mian
