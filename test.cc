#include <iostream>
#include <ctime>
#include <thread>
#include <chrono>
#include "timewheel.h"


int max = 100;
mian::TimeWheel tw(10,1);

void AddTimer(){
  int i = 0;
  while (i < max) {
    srand((unsigned)time(NULL));
    tw.Add(i, 0x00,rand() % 10 + 1);
    std::this_thread::sleep_for(std::chrono::seconds(rand() % 3 + 1));
    i++;
  }
}

void tick(){
  int count = 0;
  std::shared_ptr<mian::Event> event = nullptr;
  while(count < max) {
    while ((event = tw.PopExpired()) != nullptr) {
        count++;
        std::cout << "Expired--count:" << count << " current:" << tw.current_ << ",fd:" << event->fd_ << ", timeout: " << event->timeout_ << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(tw.GetInterval()));
    tw.Step();
  }
}

int main() {
  std::thread thread1(tick), thread2(AddTimer);
  thread1.join();
  thread2.join();
}
