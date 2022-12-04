#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <condition_variable>

#include "noncopyable.h"
#include "thread.h"
#include "task.h"

enum class PoolMode {
  MODE_FIXED,
  MODE_CACHED,
};

class ThreadPool : noncopyable{
public:
  ThreadPool();
  ~ThreadPool();
  // 开启线程池
  void start(int initThreadSize = std::thread::hardware_concurrency());
  // 设置任务队列上限的阈值
  void setTaskQueueMaxSize(int maxSize_);
  // 生产任务，提交到任务队列
  Result submitTask(std::shared_ptr<Task> pt);
  // 设置线程池工作模式
  void setMode(PoolMode mode = PoolMode::MODE_FIXED);
  // cached 模式下，设置线程池最大线程数
  void setMaxThreadSize(int maxSize);
  // 设置 cached 模式下多余线程的最长空闲时间
  void setMaxThreadFreeTime_(size_t time);
private:
  // 线程函数
  void threadFunc(size_t threadId);
  // 线程池运行状态
  bool isRunningState() const;

  std::unordered_map<size_t, std::unique_ptr<Thread>> threadsMap_;
  std::vector<std::unique_ptr<Thread> > threadVector_;  // 线程列表
  size_t initThreadSize_;               // 初始线程数
  size_t maxThreadSize_;                // 最大线程数
  size_t maxThreadFreeTime_;            // cached 模式下，多余线程的最大空闲时间
  std::atomic_int idleThreadSize_;      // 当前空闲线程数量

  // 用户传入对象的周期可能很短，不能直接传裸指针
  std::queue<std::shared_ptr<Task> > taskQueue_;  // 任务队列
  std::atomic_uint taskSize_;                     // 任务数量
  size_t taskQueueMaxSize_;                       // 任务队列上限

  std::mutex taskQueueMutex_;                     // 保证任务队列的线程安全
  std::condition_variable queueNotFull_;          // 任务队列未满
  std::condition_variable queueNotEmpty_;         // 任务队列未空
  std::condition_variable waitForWorkFinished_;   // 等待线程执行结束

  PoolMode poolMode_;                   // 当前线程工作模式
  std::atomic_bool isPoolRunning_;      // 当前线程是否启动
};


#endif