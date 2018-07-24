#ifndef MIAN_TIMEWHELL_INCLUDE_H_
#define MIAN_TIMEWHELL_INCLUDE_H_

#include <vector>
#include <memory>
#include <atomic>

#include <stdint.h>

namespace mian {
  enum EventType {
    READ_EV = 0x00,
    WRITE_EV = 0x01,
    PERSIST_EV = 0x02
  };

  struct Event {
    int fd_;
    int ev_type_;
    uint64_t timeout_;

    Event(int fd, int ev_type, uint64_t timeout):
      fd_(fd),
      ev_type_(ev_type),
      timeout_(timeout) {
    }
  };

  template <typename T>
  struct Timer {
    std::shared_ptr<T> ele_;
    std::shared_ptr<Timer<T>> next_;

    Timer(): next_(nullptr),
      ele_(nullptr){
    }

    explicit Timer(std::shared_ptr<T> ele): next_(nullptr),
      ele_(ele) {
    }
  };

  class TimerList {
    public:
      TimerList() {
        head_ = std::make_shared<Timer<Event>>();
      }
    
      ~TimerList() {
      }

      void Push(std::shared_ptr<Event> item) {
        std::shared_ptr<Timer<Event>> ptr(new Timer<Event>(item));
        ptr->next_ = head_->next_;
        head_->next_ = ptr;
      }

      std::shared_ptr<Event> Pop() {
        std::shared_ptr<Timer<Event>> ptr = head_->next_;
        if (!ptr) {
          return nullptr;
        }
        head_->next_ = ptr->next_;
        std::shared_ptr<Event> event = ptr->ele_;
        ptr.reset();
        return event;
      }

      std::shared_ptr<Event> Top() {
        std::shared_ptr<Timer<Event>> ptr = head_->next_;
        return (ptr ? ptr->ele_ : nullptr);
      }

      void Clear() {
        head_->next_ = nullptr;
      }

      private:
        std::shared_ptr<Timer<Event>> head_;
        TimerList(const TimerList& tl) = delete;
        void operator=(const TimerList& tl) = delete;

  };

  class TimeWheel {
    public:
      int current_; // current wheel index
      TimeWheel(uint32_t max_num, uint64_t interval = 1);
      ~TimeWheel();

      int Add(std::shared_ptr<Event> event, uint64_t relative_time);
      int Add(int fd, int ev_type, uint64_t relative_time);

      std::shared_ptr<Event> PopExpired();
      uint64_t GetMaxInterval() const {
        return (max_num_ - 1) * interval_;
      }
      uint64_t GetInterval() const {
        return interval_;
      }
      /*step forward by one interval*/
      void Step() {
        current_ = (current_ + 1) % max_num_;
      }

    private:
      const uint32_t max_num_;
      const uint64_t interval_;  //the interval per tick
      std::vector<std::unique_ptr<TimerList>> wheel_;
      std::shared_ptr<TimerList> expired_;   //expired timeout list

      TimeWheel(const TimeWheel& t) = delete;
      void operator =(const TimeWheel& t) = delete;
  };

}// namespace mian


#endif   //MIAN_TIMEWHEEL_INCLUDE_H_