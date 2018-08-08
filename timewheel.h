#ifndef PEANUTS_TIMEWHELL_INCLUDE_H_
#define PEANUTS_TIMEWHELL_INCLUDE_H_

#include <vector>
#include <memory>
#include <map>
#include <atomic>

#include <stdint.h>

namespace peanuts {
  struct Event {
    int fd_;
    uint32_t timeout_;
    
    Event():
      fd_(-1),
      timeout_(0) {
    }

    Event(int fd, uint32_t timeout):
      fd_(fd),
      timeout_(timeout) {
    }
  };

  template <typename T>
  struct Timer {
    T ele_;
    std::shared_ptr<Timer<T>> next_;
    std::weak_ptr<Timer<T>> prev_;

    Timer(): 
      next_(nullptr) {
    }

    explicit Timer(const T& ele): 
      next_(nullptr),
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

      std::shared_ptr<Timer<Event>> Push(const Event& item) {
        std::shared_ptr<Timer<Event>> ptr(new Timer<Event>(item));
        ptr->next_ = head_->next_;
        if (head_->next_ != nullptr) {
          head_->next_->prev_ = ptr;
        }
        ptr->prev_ = head_;
        head_->next_ = ptr;
        return ptr;
      }

      void Pop() {
        std::shared_ptr<Timer<Event>> ptr = head_->next_;
        if (!ptr) {
          return ;
        }
        head_->next_ = ptr->next_;
        if (ptr->next_ != nullptr) {
          ptr->next_->prev_ = head_;
        }
        ptr.reset();
      }

      const Event& Top() const {
        std::shared_ptr<Timer<Event>> ptr = head_->next_;
        return (ptr ? ptr->ele_ : head_->ele_);
      }

      bool Empty() const {
        return head_->next_ == nullptr;
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
      TimeWheel(uint32_t max_num, uint32_t interval = 1);
      ~TimeWheel();

      int Add(const Event& event);
      int Add(int fd, uint32_t relative_time);
      int Remove(int fd);
      int Remove(const Event& event);

      bool PopExpired(Event* ev);
      uint64_t GetMaxInterval() const {
        return (max_num_ - 1) * interval_;
      }
      uint64_t GetInterval() const {
        return interval_;
      }
      /*step forward by one interval*/
      void Tick() {
        current_ = (current_ + 1) % max_num_;
      }

    private:
      int current_; // current wheel index
      const uint32_t max_num_;
      const uint64_t interval_;  //the interval per tick
      std::vector<std::unique_ptr<TimerList>> wheel_;
      std::map<int, std::shared_ptr<Timer<Event>>> map_;

      TimeWheel(const TimeWheel& t) = delete;
      void operator =(const TimeWheel& t) = delete;
  };

}// namespace peanuts


#endif   //PEANUTS_TIMEWHEEL_INCLUDE_H_
