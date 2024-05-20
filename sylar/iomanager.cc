/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-04-01 16:42:01
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-20 20:02:05
 * @FilePath     : /sylar/iomanager.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-04-01 16:42:01
 */
#include "iomanager.h"
#include "sylar/fiber.h"
#include "sylar/log.h"
#include "sylar/marco.h"
#include "sylar/scheduler.h"
#include "sylar/util.h"
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

namespace sylar {
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

/**
 * @func: 
 * @return {*}
 * @description: 根据属性获得相应的事件上下文
 */
IOManager::FdContext::EventContext &
IOManager::FdContext::getContext(Event event) {
  SYLAR_LOG_INFO(g_logger) << "IOManager::FdContext::getContext";
  switch (event) {
  case IOManager::READ:
    return read;
  case IOManager::WRITE:
    return write;
  default:
    SYLAR_ASSERT2(false, "getContext");
  }
  throw std::invalid_argument("getContext invalid event");
}

/**
 * @func: 
 * @param {EventContext} &ctx
 * @return {*}
 * @description: 重置事件上下文
 */
void IOManager::FdContext::resetContext(EventContext &ctx) {
  SYLAR_LOG_INFO(g_logger) << "IOManager::FdContext::resetContext";
  ctx.scheduler = nullptr;
  ctx.fiber.reset();
  ctx.cb = nullptr;
}

/**
 * @func: 
 * @param {Event} event
 * @return {*}
 * @description: 将读写事件的cb或fiber放入全局任务队列中
 */
void IOManager::FdContext::triggerEvent(Event event) {
  SYLAR_ASSERT(events & event);
  events = (Event)(events & ~event);

  // 获得相应事件的上下文
  EventContext &ctx = getContext(event);
  // 将事件的回调函数放入全局任务队列中
  if (ctx.cb) {
    SYLAR_LOG_INFO(g_logger) << "IOManager::FdContext::triggerEvent cb";
    ctx.scheduler->schedule(&ctx.cb);
  } else {
    SYLAR_LOG_INFO(g_logger) << "IOManager::FdContext::triggerEvent fiber = " << ctx.fiber->getId();
    ctx.scheduler->schedule(&ctx.fiber);
  }
  ctx.scheduler = nullptr;
  return;
}

/**
 * @func:
 * @return {*}
 * @description:
构造函数，初始化epoll；同时使用pipe沟通epoll和io调度器，将tickfd加入到epoll中；
并将fdContext队列初始化，开始运行
 */
IOManager::IOManager(std::size_t threads, bool use_caller,
                     const std::string &name)
    : Scheduler(threads, use_caller, name) {
  SYLAR_LOG_INFO(g_logger) << "IOManager::IOManager";
  m_epfd = epoll_create(5000);
  SYLAR_ASSERT(m_epfd > 0);

  int rt = pipe(m_ticklefds);
  SYLAR_ASSERT(!rt);

  epoll_event event;
  memset(&event, 0, sizeof(epoll_event));
  event.events = EPOLLIN | EPOLLET;
  event.data.fd = m_ticklefds[0];

  // 不要写成F_SETFD
  // 将pipe文件设置为NONBLOCK，防止在while循环中阻塞
  rt = fcntl(m_ticklefds[0], F_SETFL, O_NONBLOCK);
  SYLAR_ASSERT(!rt);

  // 将pipe加入epoll中，方便唤醒
  rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_ticklefds[0], &event);
  SYLAR_ASSERT(!rt);

  contextResize(64);
  start();
}

/**
 * @func: 
 * @return {*}
 * @description: 停止并将句柄删除
 */
IOManager::~IOManager() {
  stop();
  close(m_epfd);
  close(m_ticklefds[0]);
  close(m_ticklefds[1]);

  for (std::size_t i = 0; i < m_fdContexts.size(); i++) {
    if (m_fdContexts[i]) {
      delete m_fdContexts[i];
    }
  }
}

/**
 * @func: 
 * @param {size_t} size
 * @return {*}
 * @description: 扩展fd列表
 */
void IOManager::contextResize(std::size_t size) {
  m_fdContexts.resize(size);
  for (size_t i = 0; i < m_fdContexts.size(); i++) {
    if (!m_fdContexts[i]) {
      m_fdContexts[i] = new FdContext;
      m_fdContexts[i]->fd = i;
    }
  }
}

/**
 * @func:
 * @param {int} fd
 * @param {Event} event
 * @param {function<void()>} cb
 * @return {*}
 * @description: 为fd中加入事件 
 */
int IOManager::addEvent(int fd, Event event, std::function<void()> cb) {
  // 在全局fd列表中获得相应的FdContext
  SYLAR_LOG_INFO(g_logger) << "IOManager::addEvent";
  FdContext *fd_ctx = nullptr;
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() > fd) {
    fd_ctx = m_fdContexts[fd];
    lock.unlock();
  } else {
    lock.unlock();
    RWMutexType::WriteLock lock2(m_mutex);
    contextResize(m_fdContexts.size() * 2);
    fd_ctx = m_fdContexts[fd];
  }

  // 确保该FdContext没有event属性
  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  
  if (fd_ctx->events & event) {
    SYLAR_LOG_ERROR(g_logger)
        << "addEvent assert fd = " << fd << " event = " << event
        << " fd_ctx.event = " << fd_ctx->events;
    SYLAR_ASSERT(!(fd_ctx->events & event));
  }

  // 使用epoll_ctl进行操作
  int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
  epoll_event epevent;
  epevent.events = EPOLLET | fd_ctx->events | event;
  epevent.data.ptr = fd_ctx;
  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt) {
    SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << "," << op << ","
                              << fd << "," << epevent.events << "):" << rt
                              << " (" << errno << ")" << strerror(errno) << ")";
    return -1;
  }

  ++m_pendingEventCount;

  // 修改此FdContext，创建EventContext的内容
  fd_ctx->events = (Event)(fd_ctx->events | event);
  FdContext::EventContext &event_ctx = fd_ctx->getContext(event);
  SYLAR_ASSERT(!event_ctx.cb && !event_ctx.fiber && !event_ctx.scheduler);
  event_ctx.scheduler = Scheduler::GetThis();
  if (cb) {
    event_ctx.cb = std::move(cb);
  } else {
    event_ctx.fiber = Fiber::GetThis();
    SYLAR_ASSERT(event_ctx.fiber->getState() == Fiber::EXEC);
  }
  
  return 0;
}

/**
 * @func: 
 * @param {int} fd
 * @param {Event} event
 * @return {*}
 * @description: 把fd中某个事件删除
 */
bool IOManager::delEvent(int fd, Event event) {
  SYLAR_LOG_INFO(g_logger) << "IOManager::delEvent";
  // 获得FdContext
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
    return false;
  }

  FdContext *fd_ctx = m_fdContexts[fd];
  lock.unlock();

  // 确保该FdContext中有event事件
  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if (!(fd_ctx->events & event)) {
    return false;
  }

  // 进行操作
  Event new_events = (Event)(fd_ctx->events & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = EPOLLET | new_events;
  epevent.data.ptr = fd_ctx;
  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt) {
    SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << "," << op << ","
                              << fd << "," << epevent.events << "):" << rt
                              << " (" << errno << ")" << strerror(errno) << ")";
    return false;
  }

  // 修改fd_ctx
  --m_pendingEventCount;
  fd_ctx->events = new_events;
  FdContext::EventContext &event_ctx = fd_ctx->getContext(event);
  fd_ctx->resetContext(event_ctx);
  return true;
}

/**
 * @func: 
 * @param {int} fd
 * @param {Event} event
 * @return {*}
 * @description: 触发并取消事件
 */
bool IOManager::cancelEvent(int fd, Event event) {
  SYLAR_LOG_INFO(g_logger) << "IOManager::cancelEvent";
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
    return false;
  }

  FdContext *fd_ctx = m_fdContexts[fd];
  lock.unlock();

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if (!(fd_ctx->events & event)) {
    return false;
  }

  // 取消事件
  Event new_events = (Event)(fd_ctx->events & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = EPOLLET | new_events;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt) {
    SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << "," << op << ","
                              << fd << "," << epevent.events << "):" << rt
                              << " (" << errno << ")" << strerror(errno) << ")";
    return false;
  }

  // 触发事件
  fd_ctx->triggerEvent(event);
  --m_pendingEventCount;
  return true;
}

/**
 * @func: 
 * @param {int} fd
 * @return {*}
 * @description: 触发并取消所有事件
 */
bool IOManager::cancelAll(int fd) {
  SYLAR_LOG_INFO(g_logger) << "IOManager::cancelAll";
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
    return false;
  }

  FdContext *fd_ctx = m_fdContexts[fd];
  lock.unlock();

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if (!fd_ctx->events) {
    return false;
  }

  int op = EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = 0;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt) {
    SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << "," << op << ","
                              << fd << "," << epevent.events << "):" << rt
                              << " (" << errno << ")" << strerror(errno) << ")";
    return false;
  }

  if (fd_ctx->events & READ) {
    fd_ctx->triggerEvent(READ);
    --m_pendingEventCount;
  }

  if (fd_ctx->events & WRITE) {
    fd_ctx->triggerEvent(WRITE);
    --m_pendingEventCount;
  }
  
  SYLAR_ASSERT(fd_ctx->events == 0)
  return true;
}

IOManager *IOManager::GetThis() {
  return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

/**
 * @func: 
 * @return {*}
 * @description: 通知epoll有内容到来，通过pipe沟通
 */
void IOManager::tickle() {
  if (!hasIdleThreads()) {
    return;
  }

  //SYLAR_LOG_INFO(g_logger) << "tickle";
  int rt = write(m_ticklefds[1], "T", 1);
  SYLAR_ASSERT(rt == 1);
}

bool IOManager::stopping(uint64_t &timeout) {
  timeout = getNextTimer();
  return timeout == ~0ull && m_pendingEventCount == 0 && Scheduler::stopping();

}
/**
 * @func: 
 * @return {*}
 * @description: 确保可以停止
 */
bool IOManager::stopping() {
  uint64_t timeout = 0;
  return stopping(timeout);
}

/**
 * @func: 
 * @return {*}
 * @description: epoll_wait，在获得event之后，触发该event的操作，并删除该fd相应的事件
 */
void IOManager::idle() {
  epoll_event *events = new epoll_event[64]();
  std::shared_ptr<epoll_event> shared_events(
      events, [](epoll_event *ptr) { delete[] ptr; });

  while (true) {

    uint64_t next_timeout = 0;

    if (stopping(next_timeout)) {
      SYLAR_LOG_INFO(g_logger)
          << "name = " << getName() << " idle stopping exit";
      break;
    }

    // epoll_wait等待
    int rt = 0;
    do {
      
      static const int MAX_TIMEOUT = 3000;
      if(next_timeout != ~0ull) {
        next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
      } else {
        next_timeout = MAX_TIMEOUT;
      }
      SYLAR_LOG_INFO(g_logger) << "epoll wait next_timeout = " << next_timeout;
      rt = epoll_wait(m_epfd, events, 64, (int)next_timeout);

      if (rt < 0 && errno == EINTR) {

      } else {
        break;
      }
    } while (true);

    std::vector<std::function<void()>> cbs;
    ListExpiredCb(cbs);
    if (!cbs.empty()) {
      schedule(cbs.begin(), cbs.end());
      cbs.clear();
    }

    for (int i = 0; i < rt; i++) {
      epoll_event &event = events[i];

      // 如果是tickle的话
      if (event.data.fd == m_ticklefds[0]) {
        uint8_t dummy[256];
        //SYLAR_LOG_INFO(g_logger) << "pipe read";
        while (read(m_ticklefds[0], dummy, sizeof(dummy)) > 0)
           ;
        continue;
      }

      // 如果是其他fd则获得该events的属性并进行操作
      FdContext *fd_ctx = (FdContext *)event.data.ptr;
      FdContext::MutexType::Lock lock(fd_ctx->mutex);
      if (event.events & (EPOLLERR | EPOLLHUP)) {
        event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
      }
      int real_events = NONE;
      if (event.events & EPOLLIN) {
        real_events |= READ;
      }
      if (event.events & EPOLLOUT) {
        real_events |= WRITE;
      }

      if ((fd_ctx->events & real_events) == NONE) {
        continue;
      }
      
      // 修改fd_ctx剩余的事件
      int left_events = (fd_ctx->events & ~real_events);
      int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
      event.events = EPOLLET | left_events;

      int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
      if (rt2) {
        SYLAR_LOG_ERROR(g_logger)
            << "epoll_ctl(" << m_epfd << "," << op << "," << fd_ctx->fd << ","
            << event.events << "):" << rt << " (" << errno << ")"
            << strerror(errno) << ")";
        continue;
      }

      if (real_events & READ) {
        SYLAR_LOG_INFO(g_logger) << "epoll trigger read";
        fd_ctx->triggerEvent(READ);
        m_pendingEventCount--;
      }

      if (real_events & WRITE) {
        SYLAR_LOG_INFO(g_logger) << "epoll trigger write";
        fd_ctx->triggerEvent(WRITE);
        m_pendingEventCount--;
      }
    } 

    // 做完一次记得跳出idle协程，去运行任务
    // 否则会导致死循环    
    Fiber::ptr cur = Fiber::GetThis();
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();
  }
}

void IOManager::onTimerInsertedAtFront() {
  //SYLAR_LOG_INFO(g_logger) << "onTimerInsertedAtFront";
  tickle();
}

}