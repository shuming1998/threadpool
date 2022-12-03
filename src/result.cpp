#include "task.h"
#include "result.h"

Result::Result(std::shared_ptr<Task> task, bool isVaild)
    : task_(task)
    , isValid_(isVaild) {
  task_->setResult(this);
}

Any Result::get() {
  if (!isValid_) {
    return "";
  }
  // 线程池中的线程未完成 task 时，会阻塞用户线程
  sem_.wait();
  return std::move(any_);
}

// 执行任务的线程
void Result::setVal(Any any) {
  // 存储 task 的返回值
  this->any_ = std::move(any);
  // 已经获取任务的返回值，增加信号量资源
  sem_.post();
}