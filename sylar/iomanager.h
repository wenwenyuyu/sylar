/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-04-01 16:42:07
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-04-08 15:43:00
 * @FilePath     : /sylar/iomanager.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-04-01 16:42:07
 */

#ifndef __SYLAR_IOMANAGER_H__
#define __SYLAR_IOMANAGER_H__


#include "sylar/fiber.h"
#include "sylar/mutex.h"
#include "sylar/scheduler.h"
#include "sylar/timer.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
namespace sylar {

class IOManager : public Scheduler, public TimerManager{
public:
  typedef std::shared_ptr<IOManager> ptr;
  typedef RWMutex RWMutexType;

  enum Event {
    NONE = 0x0,
    READ = 0x1,
    WRITE = 0x4
  };

private:
  // epoll主要对fd进行操作
  // 如果该fd可读，则触发读操作：将EventContext加入任务队列中
  // 如果该fd可写，则触发写操作
  // 在fd中封装回调函数，在触发事件时，将回调函数放入全局任务队列中
  struct FdContext {
    typedef Mutex MutexType;
    
    struct EventContext {
      Scheduler *scheduler = nullptr;
      Fiber::ptr fiber;
      std::function<void()> cb;
    };

    EventContext &getContext(Event event);
    void resetContext(EventContext &ctx);
    void triggerEvent(Event event);

    // 读任务
    EventContext read;
    // 写任务
    EventContext write;
    int fd;
    // 事件属性
    Event events = NONE;
    MutexType mutex;
  };
  
public:
  
  IOManager(std::size_t threads = 1, bool use_caller = true,
            const std::string &name = "");
  ~IOManager();

  int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
  bool delEvent(int fd, Event event);
  bool cancelEvent(int fd, Event event);
  bool cancelAll(int fd);

  static IOManager* GetThis();

protected:
  void tickle() override;
  bool stopping() override;
  bool stopping(uint64_t& timeout);
  void idle() override;
  void contextResize(std::size_t size);
  void onTimerInsertedAtFront() override;

private:
  // epoll套接字
  int m_epfd = 0;
  // pipe套接字，方便唤醒epoll_wait
  int m_ticklefds[2];
  RWMutexType m_mutex;

  std::atomic<std::size_t> m_pendingEventCount = {0};
  std::vector<FdContext*> m_fdContexts;
};
}
#endif