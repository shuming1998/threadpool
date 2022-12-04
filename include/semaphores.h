#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <mutex>
#include <atomic>
#include <condition_variable>

// 基于互斥锁和条件变量实现的信号量类，用于线程通信
class Semaphore {
public:
  Semaphore(int limit = 0);
  ~Semaphore();
  // 获取一个信号量资源
  void wait();
  // 增加一个信号量资源
  void post();

private:
  std::atomic_bool isExit_;
  int resLimit_;
  std::mutex mtx_;
  std::condition_variable cond_;
};

#endif