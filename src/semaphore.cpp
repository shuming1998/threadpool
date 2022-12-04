#include "semaphores.h"

Semaphore::Semaphore(int limit) : resLimit_(limit), isExit_(false) {}

Semaphore::~Semaphore() {
  isExit_ = true;
}

// 获取一个信号量资源
void Semaphore::wait() {
  //! 如果信号量资源已经析构，就直接返回
  if (isExit_) {
    return;
  }
  std::unique_lock<std::mutex> lock(mtx_);
  // 阻塞等待，直到信号量有资源
  cond_.wait(lock, [&]() { return resLimit_ > 0; });
  --resLimit_;
}

// 增加一个信号量资源
void Semaphore::post() {
  //! 如果信号量资源已经析构，就直接返回
  if (isExit_) {
    return;
  }
  std::unique_lock<std::mutex> lock(mtx_);
  ++resLimit_;
  //! 如果用局部 result 对象接收 submitTask 的结果，result 对象析构后，result 中的
  //! 成员 semaphore 析构 =》semaphore 中的 condition_variable 析构
  //! 而 Linux下 condition_variable 的析构函数为空函数，没有释放资源
  //! notify_all 会失效，任务线程会阻塞在这里
  cond_.notify_all();
}