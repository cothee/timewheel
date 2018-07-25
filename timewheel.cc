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

  int index = ((relative_time / interval_) + current_) % max_num_;
  //std::cout << "current: " << current_ << ", index:" << index << ", fd: " << event->fd_ << ", timeout:" << event->timeout_ << std::endl;
  to_add_->Push(event);
  //wheel_[index]->Push(event);
  return 0;
}

/*step forward by one interval*/
void Step() {
  current_ = (current_ + 1) % max_num_;

  std::shared_ptr<Timer<Event>> timer = to_do_->head_->next_;
  int index = current_;
  uint64_t timeout;
  while (timer) {
    timeout = timer->ele_->timeout_;
    index = ((relative_time / interval_) + current_) % max_num_;
    wheel_[index]->Push(event);

    timer = timer->next_;
  }
  to_do_->Clear();

  expired_->Push(wheel_[current_]);
  wheel_[current]->Clear();
}

std::shared_ptr<Event> TimeWheel::PopExpired() {
  return expired->Pop();
}

}// namespace mian
