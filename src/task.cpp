#include "task.h"

Task::Task() : result_(nullptr) {}

void Task::exec() {
  if (result_) {
    result_->setVal(run()); // 此处会发生多态调用
  }
}

void Task::setResult(Result *res) {
  result_ = res;
}