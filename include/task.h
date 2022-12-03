#ifndef TASK_H
#define TASK_H

#include "result.h"
#include "any.h"

class Task {
public:
  Task();
  ~Task() = default;
  void exec();
  void setResult(Result *res);
  virtual Any run() = 0;

private:
  // Result 中已有 Task 的智能指针，这里不能用智能指针，否则会交叉引用
  // Result 的生命周期一定长于 task，因为 result 对象要接收 task 的返回值
  Result *result_;
};


#endif