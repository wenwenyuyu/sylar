/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-21 10:15:35
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-04-16 15:20:42
 * @FilePath     : /sylar/scheduler.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-21 10:15:35
 */
#ifndef __SYLAR_SCHEDULER_H__
#define __SYLAR_SCHEDULER_H__

#include "sylar/fiber.h"
#include "sylar/mutex.h"
#include "sylar/thread.h"
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>
namespace sylar {

// 初始化函数：如果设置了use_caller则自动将调度器的线程也纳入线程池中
// start：将所有的线程绑定到run函数中
// run：首先设置该线程的主协程，接着开始遍历任务队列，若有任务则取出任务，并通过主协程创造出子协程运行该任务；若没有任务，则运行idle协程
// schedule：往任务队列中加入任务，若任务队列为空，则通知线程有任务来了
// stop： 将所有的线程detach，并且保证idle协程结束，线程在idle协程结束后可以结束
// 析构函数：释放内存

class Scheduler {
public:
  typedef std::shared_ptr<Scheduler> ptr;
  typedef Mutex MutexType;

  Scheduler(std::size_t threads = 1, bool use_caller = true, const std::string& name = "");
  virtual ~Scheduler();

  const std::string &getName() const { return m_name; }
  
  void start();
  void stop();

  static Scheduler *GetThis();
  static Fiber* GetMainFiber();

  // 调度器增加任务，若当前的任务队列为空则通知所有的线程
  template <class FiberOrCb> void schedule(FiberOrCb fc, int thr = -1) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      need_tickle = scheduNoLock(fc, thr);
    }

    if (need_tickle) {
      tickle();
    }
  }

  // 通过迭代器增加所有的任务
  template<class InputIterator>
  void schedule(InputIterator begin, InputIterator end) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      while(begin != end) {
        need_tickle = scheduNoLock(&*begin, -1) || need_tickle;
        ++begin;
      }
    }
    if(need_tickle) {
      tickle();
    }
  }

protected:
  /**
   * @func: 
   * @return {*}
   * @description: 通知协程调度器有任务了
   */  
  virtual void tickle();

  /**
   * @func: 
   * @return {*}
   * @description: 协程调度开始函数
   */  
  void run();

  /**
   * @func: 
   * @return {*}
   * @description: 协程调度器是否可以停止了
   */  
  virtual bool stopping();

  /**
   * @func: 
   * @return {*}
   * @description: 协程调度器无任务时执行idle协程
   */
  virtual void idle();

  void setThis();

  bool hasIdleThreads() { return m_idleThreadCount > 0; }
  
private:
  template <class FiberOrCb> bool scheduNoLock(FiberOrCb fc, int thr) {
    bool need_tickle = m_fibers.empty();
    FiberAndThread ft(fc, thr);
    if (ft.m_fiber || ft.m_cb) {
      m_fibers.push_back(ft);
    }
    return need_tickle;
  }

private:
  // 任务队列中存储的任务，包括协程、回调函数和要求指定的线程运行
  struct FiberAndThread{
    // 协程
    Fiber::ptr m_fiber;
    // 任务
    std::function<void()> m_cb;
    // 指定运行线程id
    int m_threadId;

    FiberAndThread() { m_threadId = -1; }

    FiberAndThread(Fiber::ptr f, int thr) : m_fiber(f), m_threadId(thr) {}

    FiberAndThread(Fiber::ptr *f, int thr) : m_threadId(thr) {
      m_fiber = std::move(*f);
    }

    FiberAndThread(std::function<void()> cb, int thr)
        : m_cb(cb), m_threadId(thr) {}
    FiberAndThread(std::function<void()> *cb, int thr) : m_threadId(thr) {
      m_cb = std::move(*cb);
    }

    void reset() {
      m_fiber = nullptr;
      m_cb = nullptr;
      m_threadId = -1;
    }

  };

private:
  // 调度器的互斥锁
  MutexType m_mutex;
  // 线程池
  std::vector<Thread::ptr> m_threads;
  // 任务消息队列
  std::list<FiberAndThread> m_fibers;
  // 主协程，设置use_caller为true时创造
  Fiber::ptr m_rootFiber;
  // 调度器名称
  std::string m_name;

protected:
  std::vector<int> m_threadIds;
  // 线程数量
  std::size_t m_threadCount = 0;
  std::atomic<std::size_t> m_activeThreadCount = {0};
  std::atomic<std::size_t> m_idleThreadCount = {0};
  // 是否停止
  bool m_stopping = true;
  bool auto_stopping = false;
  // 主线程id
  int m_rootThread = 0;
};

}
#endif