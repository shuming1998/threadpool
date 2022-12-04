#include "Logger.h"
#include "threadpool.h"

#include <functional>
#include <iostream>
#include <chrono>

const int TASK_MAX_SIZE = INT32_MAX;
const int THREAD_MAX_SIZE = 1024;

// 线程池构造
ThreadPool::ThreadPool()
    : initThreadSize_(0)
    , taskSize_(0)
    , idleThreadSize_(0)
    , maxThreadFreeTime_(60)
    , maxThreadSize_(THREAD_MAX_SIZE)
    , taskQueueMaxSize_(TASK_MAX_SIZE)
    , poolMode_(PoolMode::MODE_FIXED)
    , isPoolRunning_(false) {}

// 线程池析构，将线程池相关的线程资源全部回收
ThreadPool::~ThreadPool() {
  isPoolRunning_ = false;
  // 线程间通信，等待线程池中所有线程(阻塞|执行中|执行后刚进while循环)返回
  std::unique_lock<std::mutex> lock(taskQueueMutex_);
  // 如果任务线程先获取了锁，需要将任务线程的 queueNotEmpty_ 唤醒
  queueNotEmpty_.notify_all();
  // 等待线程池为空
  waitForWorkFinished_.wait(lock, [&](){ return threadsMap_.size() == 0; });
}

// 线程函数：从任务队列中消费任务
void ThreadPool::threadFunc(size_t threadId) {
  auto threadLastWorkTime = std::chrono::high_resolution_clock().now();
  // 线程持续从任务队列取任务，线程池析构时，必须把所有任务执行完
  for (;;) {
    std::shared_ptr<Task> taskPtr;
    {
      //! 先获取锁，控制锁的粒度，只需要在操作任务队列时加锁，应和执行任务分开
      std::unique_lock<std::mutex> lock(taskQueueMutex_);
      // 线程池析构的时候，如果是主线程先获得锁，必须再在任务线程中判断一下池是否运行
      // 如果不判断，任务列表为空时，任务线程可能一直阻塞在 queueNotEmpty_ 上(3|执行后刚进 while 循环)
      while (taskQueue_.size() == 0) {
        if (!isPoolRunning_) {
          // 线程执行任务完成，回收(1|阻塞的线程 2|执行任务的线程)
          threadsMap_.erase(threadId);
          waitForWorkFinished_.notify_all();
          //! 结束线程函数就是结束线程
          return;
        }
        // cached模式下，回收当前线程池中空闲时间超过阈值的多余线程
        // 每秒钟返回一次，判断是否超时
        if (poolMode_ == PoolMode::MODE_CACHED) {
          // 条件变量超时返回
          if (std::cv_status::timeout == queueNotEmpty_.wait_for(lock, std::chrono::seconds(1))) {
            auto now = std::chrono::high_resolution_clock().now();
            auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - threadLastWorkTime);
            if (dur.count() >= maxThreadFreeTime_ && threadsMap_.size() > initThreadSize_) {
              // 回收当前线程
              --idleThreadSize_;
              // 将当前线程对象从列表删除
              std::cout << "threadsMap size: " << threadsMap_.size() << '\n';
              std::cout << "Timeout! Delete thread\n";
              threadsMap_.erase(threadId);
              --idleThreadSize_;
              // 结束线程函数
              return;
            }
          }
        } else if (poolMode_ == PoolMode::MODE_FIXED) {
          // fixed 模式下，等待任务队列非空queueNotEmpty条件
          queueNotEmpty_.wait(lock);
        }
        // 线程池即将结束，回收线程资源(1|阻塞的线程)
        // if (!isPoolRunning_) {
        //   threadsMap_.erase(threadId);
        //   waitForWorkFinished_.notify_all();
        //   return;
        // }
      }
      // 空闲线程数减一
      --idleThreadSize_;
      // 从任务队列取任务并执行
      taskPtr = taskQueue_.front();
      taskQueue_.pop();
      --taskSize_;
      // 有任务被取出，通知生产者生产新任务
      queueNotFull_.notify_all();
      // 如果任务队列不为空，通知其他消费者消费任务
      if (!taskQueue_.empty()) {
        queueNotEmpty_.notify_all();
      }
    } // 控制锁的粒度
    if (taskPtr != nullptr) {
      // 执行任务，将任务的返回值通过 setVal 方法传递给 Result
      taskPtr->exec();
    }
    // 任务已做完，空闲线程数 +1
    ++idleThreadSize_;
    // 更新线程执行任务结束后的时间
    threadLastWorkTime = std::chrono::high_resolution_clock().now();
  }
}

// 开启线程池
void ThreadPool::start(int initThreadSize) {
  isPoolRunning_ = true;
  initThreadSize_ = initThreadSize;
  // 创建线程
  for (int i = 0; i < initThreadSize_; ++i) {
    std::unique_ptr<Thread> ptr = std::make_unique<Thread>(Thread(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1)));
    // unique_ptr 不允许拷贝构造和赋值，所以直接转为右值
    threadsMap_.emplace(ptr->getId(), std::move(ptr));
  }
  // 启动线程
  for (int i = 0; i < initThreadSize_; ++i) {
    threadsMap_[i]->start();
    ++idleThreadSize_;
  }
}

// cached 模式下，设置线程池最大线程数
void ThreadPool::setMaxThreadSize(int maxSize) {
  if (isRunningState()) {
    return;
  }
  maxThreadSize_ = maxSize;
}

void ThreadPool::setMaxThreadFreeTime_(size_t time) {
  if (isRunningState()) {
    return;
  }
  if (poolMode_ == PoolMode::MODE_CACHED) {
    maxThreadFreeTime_ = time;
  }
}

// cached 模式下，设置任务队列上限的阈值
void ThreadPool::setTaskQueueMaxSize(int maxSize_) {
  if (isRunningState()) {
    return;
  }
  if (poolMode_ == PoolMode::MODE_CACHED) {
    taskQueueMaxSize_ = maxSize_;
  }
}

void ThreadPool::setMode(PoolMode mode) {
  if (isRunningState()) {
    return;
  }
  poolMode_ = mode;
}

bool ThreadPool::isRunningState() const {
  return isPoolRunning_;
}

// 生产任务，提交到任务队列
// 用户提交的任务各不相同，所以传基类的智能指针
Result ThreadPool::submitTask(std::shared_ptr<Task> pt) {
  // 获取锁
  std::unique_lock<std::mutex> lock(taskQueueMutex_);
  // 持续等待  持续等待指定时间    持续等待直到指定的时间
  //  wait  |    wait_for    |     wait_until
  // 用户提交任务阻塞时长不能超过 1s
  if (!queueNotFull_.wait_for(lock, std::chrono::seconds(1), \
      [&](){ return taskQueue_.size() < taskQueueMaxSize_; })) {
    // 因到达等待时长(设置为1s)而返回，但此时条件仍未满足
    std::cerr << "Task queue is full, submit task failed!\n";
    return Result(pt, false);
  }
  // 任务队列有空余可插入
  taskQueue_.emplace(pt);
  ++taskSize_;

  // 放入新任务后，任务列表非空，通知消费线程消费任务
  queueNotEmpty_.notify_all();

  // 若处于 cached 模式，判断任务数量和空闲线程数量是否合理，以此决定是否创建新线程
  if (poolMode_ == PoolMode::MODE_CACHED
      && taskSize_ > idleThreadSize_
      && threadsMap_.size() < maxThreadSize_) {
    // 创建新线程对象
    std::cout << "Create new thread!\n";
    auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
    size_t threadId = ptr->getId();
    threadsMap_.emplace(threadId, std::move(ptr));
    // 启动新线程
    threadsMap_[threadId]->start();
    ++idleThreadSize_;
  }

  // 返回任务的 Result 对象
  return Result(pt);
}


