#ifndef RESULT_H
#define RESULT_H
#include <memory>
#include <atomic>
#include "any.h"
#include "semaphores.h"
class Task;
// 提交给线程池的任务完成后的返回值类型
class Result {
public:
  Result(std::shared_ptr<Task> task, bool isVaild = true);
  ~Result() = default;

  // 获取 task 执行完成后的返回值的真实类型
  void setVal(Any any);

  // 用户通过该方法获取 task 的返回值
  Any get();

private:
  Any any_;
  Semaphore sem_;
  std::shared_ptr<Task> task_;
  std::atomic_bool isValid_;
};



#endif