#ifndef MIAN_TIMEWHELL_INCLUDE_H_
#define MIAN_TIMEWHELL_INCLUDE_H_

#include <vector>
#include <memory>
#include <atomic>
#include <mutex>

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

  class TimeWheel;
  class TimerList {
    public:
      TimerList() {
        head_ = std::make_shared<Timer<Event>>();
        tail_ = nullptr;
      }
    
      ~TimerList() {
      }

      void Push(std::shared_ptr<Event> item) {
        std::shared_ptr<Timer<Event>> ptr(new Timer<Event>(item));
        ptr->next_ = head_->next_;
        if (!head_->next_) {
          tail_ = ptr;
        }
        head_->next_ = ptr;
      }

      void Push(std::shared_ptr<Timer<Event>> timer) {
        if (timer != nullptr) {
          timer->next_ = head_->next_;
          if (!head_->next_) {
            tail_= timer;
          }
          head_->next_ = timer;
        }
      
      }

      void Push(std::shared_ptr<TimerList> list) {
          if (tail_) {
            tail_->next_ = list->head_->next_;
          } else {
            head_->next_ = list->head_->next_;
          }
      }

      std::shared_ptr<Event> Pop() {
        std::shared_ptr<Timer<Event>> ptr = head_->next_;
        if (!ptr) {
          return nullptr;
        }
        head_->next_ = ptr->next_;
        std::shared_ptr<Event> event = ptr->ele_;
        if (ptr == tail_) {
          tail_ = nullptr;
        }
        ptr.reset();
        return event;
      }

      std::shared_ptr<Event> Top() {
        std::shared_ptr<Timer<Event>> ptr = head_->next_;
        return (ptr ? ptr->ele_ : nullptr);
      }

      void Clear() {
        head_->next_ = nullptr;
        tail_ = nullptr;
      }

      private:
        std::shared_ptr<Timer<Event>> head_;
        std::shared_ptr<Timer<Event>> tail_;
        TimerList(const TimerList& tl) = delete;
        void operator=(const TimerList& tl) = delete;

        friend TimeWheel;

  };

  class TimeWheel {
    public:
      TimeWheel(uint32_t max_num, uint64_t interval = 1);
      ~TimeWheel();

      int Add(std::shared_ptr<Event> event, uint64_t relative_time);
      int Add(int fd, int ev_type, uint64_t relative_time);

      std::shared_ptr<Event> PopExpired();
      void Step();

      uint64_t GetMaxInterval() const {
        return (max_num_ - 1) * interval_;
      }
      uint64_t GetInterval() const {
        return interval_;
      }

    private:
      int current_; // current wheel index

      const uint32_t max_num_;
      const uint64_t interval_;  //the interval per tick
      
      std::vector<std::shared_ptr<TimerList>> wheel_;
      std::shared_ptr<TimerList> to_add_;
      std::shared_ptr<TimerList> expired_;   //expired timeout list

      std::mutex add_mutex_;
      std::mutex expired_mutex_;

      TimeWheel(const TimeWheel& t) = delete;
      void operator =(const TimeWheel& t) = delete;
  };

}// namespace mian


#endif   //MIAN_TIMEWHEEL_INCLUDE_H_
