#include <iostream>
#include <ctime>
#include <thread>
#include <chrono>
#include "timewheel.h"


int max = 100;
peanuts::TimeWheel tw(10,1);

void AddTimer(){
  int i = 0;
  while (i < max) {
    srand((unsigned)time(NULL));
    tw.Add(i, rand() % 10 + 1);
    tw.Remove(i - (rand() % 10 + 1));
    std::this_thread::sleep_for(std::chrono::seconds(rand() % 3 + 1));
    //tw.Tick();
    i++;
  }
}

void tick(){
  int count = 0;
  peanuts::Event event;
  while(count < max) {
    while ((tw.PopExpired(&event))) {
        count++;
        std::cout << "Expired--count:" << count << ",fd:" << event.fd_ << ", timeout: " << event.timeout_ << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(tw.GetInterval()));
    tw.Tick();
  }
}

int main() {
  std::thread thread1(tick);
  std::thread thread2(AddTimer);
  thread1.join();
  thread2.join();
}
