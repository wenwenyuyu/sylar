/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-04-16 14:26:28
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-12 20:17:34
 * @FilePath     : /sylar/hook.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-04-16 14:26:28
 */
#include "hook.h"
#include "sylar/config.h"
#include "sylar/fdmanager.h"
#include "sylar/fiber.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"
#include "sylar/mutex.h"
#include "sylar/scheduler.h"
#include "sylar/timer.h"
#include "sylar/util.h"
#include <asm-generic/socket.h>
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstdarg>
#include <cstdint>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <functional>
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();
// 连接延时配置
static sylar::ConfigVar<int>::ptr g_tcp_connect_timeout =
    sylar::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

// 是否进行hook?
static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX)                                                           \
  XX(sleep)                                                                    \
  XX(usleep)                                                                   \
  XX(nanosleep)                                                                \
  XX(socket)                                                                   \
  XX(connect)                                                                  \
  XX(accept)                                                                   \
  XX(read)                                                                     \
  XX(readv)                                                                    \
  XX(recv)                                                                     \
  XX(recvfrom)                                                                 \
  XX(recvmsg)                                                                  \
  XX(write)                                                                    \
  XX(writev)                                                                   \
  XX(send)                                                                     \
  XX(sendto)                                                                   \
  XX(sendmsg)                                                                  \
  XX(close)                                                                    \
  XX(fcntl)                                                                    \
  XX(ioctl)                                                                    \
  XX(getsockopt)                                                               \
  XX(setsockopt)

// hook函数初始化
void hook_init() {
  static bool is_inited = false;
  if (is_inited) {
    return;
  }
// 对函数进行hook
#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
  HOOK_FUN(XX);
#undef XX
}

// 设置连接超时，-1表示不超时
static uint64_t s_connect_timeout = -1;
struct _HookIniter {
  _HookIniter() {
    hook_init();
    s_connect_timeout = g_tcp_connect_timeout->getValue();

    g_tcp_connect_timeout->addListener(
        [](const int &old_value, const int &new_value) {
          SYLAR_LOG_INFO(g_logger) << "tcp connect timeout changed from "
                                   << old_value << " to " << new_value;
          s_connect_timeout = new_value;
    });
  }
};

// 静态初始化，确保在main函数之前初始化
static _HookIniter s_hook_init;
// 是否设置hook
bool is_hook_enable() { return t_hook_enable; }
// 设置hook
void set_hook_enable(bool flag) { t_hook_enable = flag; }

} // namespace sylar

// 条件定时器的条件
struct timer_info {
  int cancelled = 0;
};

// io操作的hook
template <typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char *hook_fun_name,
                     uint32_t event, int timeout_so, Args &&...args) {
  // 如果没有进行hook，则直接返回
  if (!sylar::t_hook_enable) {
    return fun(fd, std::forward<Args&&>(args)...);
  }

  // 获取相应的fd
  sylar::FdCtx::ptr ctx = sylar::FdMgr::getInstance()->get(fd);
  if (!ctx) {
    return fun(fd, std::forward<Args&&>(args)...);
  }

  if (ctx->isClose()) {
    errno = EBADF;
    return -1;
  }

  // 判断是否为套接字或者被显示设置了非阻塞模式
  if (!ctx->isSocket() || ctx->getUserNonblock()) {
    return fun(fd, std::forward<Args&&>(args)...);
  }

  // 获得超时时间
  uint64_t to = ctx->getTimeout(timeout_so);
  std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
  // 执行，因为是非阻塞模式，会立刻返回
  ssize_t n = fun(fd, std::forward<Args &&>(args)...);
  // 如果是被中断的
  while (n == -1 && errno == EINTR) {
    n = fun(fd, std::forward<Args&&>(args)...);
  }

  // 如果不可行则进行调度 
  if (n == -1 && errno == EAGAIN) {
    sylar::IOManager *iom = sylar::IOManager::GetThis();
    sylar::Timer::ptr timer;
    std::weak_ptr<timer_info> winfo(tinfo);

    // 如果设置了超时时间
    if (to != (uint64_t)-1) {
      // 设置条件定时器
      timer = iom->addConditionTimer(to, [winfo, fd, iom, event] {
        auto t = winfo.lock();
        // 如果已经执行过了，直接返回
        if (!t || t->cancelled) {
          return;
        }
        // 取消事件
        t->cancelled = ETIMEDOUT;
        iom->cancelEvent(fd, (sylar::IOManager::Event)event);
      }, winfo);
    }

    // 加入事件
    int rt = iom->addEvent(fd, (sylar::IOManager::Event)event);
    if (rt) {
      SYLAR_LOG_ERROR(sylar::g_logger)
          << hook_fun_name << " addEvent(" << fd << ", " << event << ")";
      if(timer) {
          timer->cancel();
      }
      return -1;
    } else {
      SYLAR_LOG_INFO(sylar::g_logger) << hook_fun_name << " trigger";
      // 重中之重 yield出去
      sylar::Fiber::wait();
      // 返回
      if (timer) {
        timer->cancel();
      }

      if (tinfo->cancelled) {
        errno = tinfo->cancelled;
        return -1;
      }

      goto retry;
    }
  }

  return n;
}
extern "C" {
#define XX(name) name##_fun name##_f = nullptr;
  HOOK_FUN(XX);
#undef XX

// 直接加入定时器中
unsigned int sleep(unsigned int seconds) {

  if (!sylar::t_hook_enable) {
    return sleep_f(seconds);
  }

  sylar::Fiber::ptr fiber = sylar::Fiber::GetThis();
  sylar::IOManager *iom = sylar::IOManager::GetThis();
  iom->addTimer(
      seconds * 1000,
      std::bind((void(sylar::Scheduler::*)(sylar::Fiber::ptr, int thread)) &
                    sylar::IOManager::schedule,
                iom, fiber, -1));
  sylar::Fiber::wait();
  return 0;
}

int usleep(useconds_t usec) {
  if (!sylar::t_hook_enable) {
    return usleep_f(usec);
  }
  
  sylar::Fiber::ptr fiber = sylar::Fiber::GetThis();
  sylar::IOManager *iom = sylar::IOManager::GetThis();
  iom->addTimer(
      usec / 1000,
      std::bind((void(sylar::Scheduler::*)(sylar::Fiber::ptr, int thread)) &
                    sylar::IOManager::schedule,
                iom, fiber, -1));
  sylar::Fiber::wait();
  return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
  if (!sylar::t_hook_enable) {
    return nanosleep_f(req, rem);
  }
  int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 /1000;
  sylar::Fiber::ptr fiber = sylar::Fiber::GetThis();
  sylar::IOManager *iom = sylar::IOManager::GetThis();
  iom->addTimer(
      timeout_ms / 1000,
      std::bind((void(sylar::Scheduler::*)(sylar::Fiber::ptr, int thread)) &
                    sylar::IOManager::schedule,
                iom, fiber, -1));
  sylar::Fiber::wait();
  return 0;
}

// hook的fd还要加入fdmgr中
int socket(int domain, int type, int protocol) {
  if (!sylar::t_hook_enable) {
    return socket_f(domain, type, protocol);
  }

  int fd = socket_f(domain, type, protocol);
  if (fd == -1) {
    return fd;
  }
  sylar::FdMgr::getInstance()->get(fd, true);
  return fd;
}

// 和do_io一样
int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen, uint64_t timeout_ms) {
  if (!sylar::t_hook_enable) {
    return connect_f(sockfd, addr, addrlen);
  }
  sylar::FdCtx::ptr ctx = sylar::FdMgr::getInstance()->get(sockfd);
  if (!ctx || ctx->isClose()) {
    errno = EBADF;
    return -1;
  }

  if (!ctx->isSocket()) {
    return connect_f(sockfd, addr, addrlen);
  }

  int n = connect_f(sockfd, addr, addrlen);
  if (n == 0) {
    return 0;
  } else if (n != -1 || errno != EINPROGRESS) {
    return n;
  }

  sylar::IOManager *iom = sylar::IOManager::GetThis();
  sylar::Timer::ptr timer;
  std::shared_ptr<timer_info> tinfo(new timer_info);
  std::weak_ptr<timer_info> winfo(tinfo);

  if (timeout_ms != (uint64_t)-1) {
    timer = iom->addConditionTimer(timeout_ms, [winfo, sockfd, iom] {
      auto t = winfo.lock();
      if (!t || t->cancelled) {
        return;
      }
      iom->cancelEvent(sockfd, sylar::IOManager::WRITE);
      t->cancelled = ETIMEDOUT;
    }, winfo);
  }

  int rt = iom->addEvent(sockfd, sylar::IOManager::WRITE);
  if (rt == 0) {
    SYLAR_LOG_ERROR(sylar::g_logger) << "connect trigger";
    sylar::Fiber::wait();
    if (timer) {
      timer->cancel();
    }
    if (tinfo->cancelled) {
      errno = tinfo->cancelled;
      return -1;
    }
  } else {
    if (timer) {
      timer->cancel();
    }
    SYLAR_LOG_ERROR(sylar::g_logger)
        << "connect addEvent(" << sockfd << ", WRITE) ERROR";
  }

  int error = 0;
  socklen_t len = sizeof(int);
  if (-1 == getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len)) {
    return -1;
  }

  if (!error) {
    return 0;
  } else {
    errno = error;
    return -1;
  }

  
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  return connect_with_timeout(sockfd, addr, addrlen, sylar::s_connect_timeout);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
  int fd = do_io(s, accept_f, "accept", sylar::IOManager::READ, SO_RCVTIMEO,
                 addr, addrlen);
  if (fd >= 0) {
    sylar::FdMgr::getInstance()->get(fd, true);
  }

  return fd;
      
}

ssize_t read(int fd, void *buf, size_t count) {
  return do_io(fd, read_f, "read", sylar::IOManager::READ, SO_RCVTIMEO, buf,
               count);
  
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
  return do_io(fd, readv_f, "readv", sylar::IOManager::READ, SO_RCVTIMEO, iov,
               iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
  return do_io(sockfd, recv_f, "recv", sylar::IOManager::READ, SO_RCVTIMEO, buf,
               len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen) {
  return do_io(sockfd, recvfrom_f, "recvfrom", sylar::IOManager::READ,
               SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
  return do_io(sockfd, recvmsg_f, "recvmsg", sylar::IOManager::READ, SO_RCVTIMEO, msg,
               flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
  return do_io(fd, write_f, "write", sylar::IOManager::WRITE, SO_SNDTIMEO, buf,
               count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
  return do_io(fd, writev_f, "writev", sylar::IOManager::WRITE, SO_SNDTIMEO,
               iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
  return do_io(s, send_f, "send", sylar::IOManager::WRITE, SO_SNDTIMEO, msg, len,
               flags);
} 

ssize_t sendto(int s, const void *msg, size_t len, int flags,
               const struct sockaddr *to, socklen_t tolen) {
  return do_io(s, sendto_f, "sendto", sylar::IOManager::WRITE, SO_SNDTIMEO, msg,
               len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
  return do_io(s, sendmsg_f, "sendmsg", sylar::IOManager::WRITE, SO_SNDTIMEO,
               msg, flags);
}

// 还要取消fd
int close(int fd) {
  if (!sylar::t_hook_enable) {
    return close_f(fd);
  }
  sylar::FdCtx::ptr ctx = sylar::FdMgr::getInstance()->get(fd);
  if (ctx) {
    auto iom = sylar::IOManager::GetThis();
    if (iom) {
      iom->cancelAll(fd);
    }
    sylar::FdMgr::getInstance()->del(fd);
  }
  return close_f(fd);
}
// fcntl，这里的O_NONBLOCK标志要特殊处理
// 因为所有参与协程调度的fd都会被设置成非阻塞模式
// 所以要在应用层维护好用户设置的非阻塞标志。
int fcntl(int fd, int cmd, ... /* arg */) {
  va_list va;
  va_start(va, cmd);
  switch (cmd) {
  case F_SETFL: {
    int arg = va_arg(va, int);
    va_end(va);
    sylar::FdCtx::ptr ctx = sylar::FdMgr::getInstance()->get(fd);
    if (!ctx || ctx->isClose() || !ctx->isSocket()) {
      return fcntl_f(fd, cmd, arg);
    }
    ctx->setUserNonblock(arg & O_NONBLOCK);
    if (ctx->getSysNonblock()) {
      arg |= O_NONBLOCK;
    }else {
      arg &= ~O_NONBLOCK;
    }
    return fcntl_f(fd, cmd, arg);
  } break;

  case F_GETFL: {
    va_end(va);
    int arg = fcntl_f(fd, cmd);
    sylar::FdCtx::ptr ctx = sylar::FdMgr::getInstance()->get(fd);
    if (!ctx || ctx->isClose() || !ctx->isSocket()) {
      return arg;
    }
    if (ctx->getUserNonblock()) {
      return arg | O_NONBLOCK;
    }else {
      return arg & ~O_NONBLOCK;
    }
  }
  case F_DUPFD:
  case F_DUPFD_CLOEXEC:
  case F_SETFD:
  case F_SETOWN:
  case F_SETSIG:
  case F_SETLEASE:
  case F_NOTIFY:
#ifdef F_SETPIPE_SZ
  case F_SETPIPE_SZ:
#endif
  {
    int arg = va_arg(va, int);
    va_end(va);
    return fcntl_f(fd, cmd, arg);
  }
    break;
  case F_GETFD:
  case F_GETOWN:
  case F_GETSIG:
  case F_GETLEASE:
#ifdef F_GETPIPE_SZ
  case F_GETPIPE_SZ:
#endif
  {
    va_end(va);
    return fcntl_f(fd, cmd);
  }
    break;
  case F_SETLK:
  case F_SETLKW:
  case F_GETLK:
  {
    struct flock *arg = va_arg(va, struct flock *);
    va_end(va);
    return fcntl_f(fd, cmd, arg);
  }
    break;
    
  case F_GETOWN_EX:
  case F_SETOWN_EX: {
    struct f_owner_exlock *arg = va_arg(va, struct f_owner_exlock *);
    va_end(va);
    return fcntl_f(fd, cmd, arg);
  }
    break;
  default:
    va_end(va);
    return fcntl_f(fd, cmd);
    
  }
}

// ioctl，同样要特殊处理FIONBIO命令，这个命令用于设置非阻塞，处理方式和上面的fcntl一样。
int ioctl(int d, unsigned long int request, ...) {
  va_list va;
  va_start(va, request);
  void *arg = va_arg(va, void *);
  va_end(va);

  if (FIONBIO == request) {
    bool user_noblock = !!*(int *)arg;
    sylar::FdCtx::ptr ctx = sylar::FdMgr::getInstance()->get(d);
    if (!ctx || ctx->isClose() || !ctx->isSocket()) {
      return ioctl_f(d, request, arg);
    }
    ctx->setUserNonblock(user_noblock);
  }
  return ioctl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval,
               socklen_t *optlen) {
  return getsockopt_f(sockfd, level, optname, optval, optlen);
}

// setsocketopt，这里要特殊处理SO_RECVTIMEO和SO_SNDTIMEO
// 在应用层记录套接字的读写超时，方便协程调度器获取。
int setsockopt(int sockfd, int level, int optname, const void *optval,
               socklen_t optlen) {
  if (!sylar::t_hook_enable) {
    return setsockopt_f(sockfd, level, optname, optval, optlen);
  }
  if (level == SOL_SOCKET) {
    if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
      sylar::FdCtx::ptr ctx = sylar::FdMgr::getInstance()->get(sockfd);
      if (ctx) {
        const timeval *v = (const timeval *)optval;
        ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
      }
    }
  }
  return setsockopt_f(sockfd, level, optname, optval, optlen);
}



}

