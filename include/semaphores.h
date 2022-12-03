#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <mutex>
#include <condition_variable>

// 基于互斥锁和条件变量实现的信号量类，用于线程通信
class Semaphore {
public:
  Semaphore(int limit = 0);
  ~Semaphore() = default;
  // 获取一个信号量资源
  void wait();
  // 增加一个信号量资源
  void post();

private:
  int resLimit_;
  std::mutex mtx_;
  std::condition_variable cond_;
};

#endif