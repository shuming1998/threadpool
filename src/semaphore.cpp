#include "semaphores.h"

Semaphore::Semaphore(int limit) : resLimit_(limit) {}


// 获取一个信号量资源
void Semaphore::wait() {
  std::unique_lock<std::mutex> lock(mtx_);
  // 阻塞等待，直到信号量有资源
  cond_.wait(lock, [&]() { return resLimit_ > 0; });
  --resLimit_;
}

// 增加一个信号量资源
void Semaphore::post() {
  std::unique_lock<std::mutex> lock(mtx_);
  ++resLimit_;
  cond_.notify_all();
}