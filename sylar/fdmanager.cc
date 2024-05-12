/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-04-21 19:44:02
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-12 20:21:52
 * @FilePath     : /sylar/fdmanager.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-04-21 19:44:02
 */
#include "fdmanager.h"
#include "sylar/hook.h"
#include <asm-generic/socket.h>
#include <cstdint>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace sylar {
FdCtx::FdCtx(int fd)
    : m_isInit(false), m_isSocket(false), m_sysNonblock(false),
      m_userNonblock(false), m_isClosed(false), m_fd(fd),
      m_recvTimeout(-1), m_sendTimeout(-1){
  init();
}

FdCtx::~FdCtx() {}
/**
 * @func: 
 * @return {*}
 * @description: 初始化函数，确保是socket、保证设置非阻塞
 */
bool FdCtx::init() {
  if (m_isInit) {
    return true;
  }
  m_recvTimeout = -1;
  m_sendTimeout = -1;

  struct stat fd_stat;
  if (-1 == fstat(m_fd, &fd_stat)) {
    m_isInit = false;
    m_isSocket = false;
  }else {
    m_isInit = true;
    m_isSocket = S_ISSOCK(fd_stat.st_mode);
  }

  if (m_isSocket) {
    int flag = fcntl_f(m_fd, F_GETFL, 0);
    if (!(flag & O_NONBLOCK)) {
      fcntl_f(m_fd, F_SETFL, flag | O_NONBLOCK);
    }
    m_sysNonblock = true;
  }else {
    m_sysNonblock = false;
  }

  m_userNonblock = false;
  m_isClosed = false;
  
  return m_isInit;
}

void FdCtx::setTimeout(int type, uint64_t v) {
  if (type == SO_RCVTIMEO) {
    m_recvTimeout = v;
  } else {
    m_sendTimeout = v;
  }
}

uint64_t FdCtx::getTimeout(int type) {
  if (type == SO_RCVTIMEO) {
    return m_recvTimeout;
  }
  return m_sendTimeout;
}

FdManager::FdManager() { m_datas.resize(64); }

FdCtx::ptr FdManager::get(int fd, bool auto_create) {
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_datas.size() <= fd) {
    if (!auto_create) {
      return nullptr;
    }
    m_datas.resize(m_datas.size() * 1.5);
  }
  else {
    if (m_datas[fd] || !auto_create) {
        return m_datas[fd];
    }
  }

  lock.unlock();

  RWMutexType::WriteLock lock2(m_mutex);
  FdCtx::ptr ctx(new FdCtx(fd));
  m_datas[fd] = ctx;
  return ctx;
}

void FdManager::del(int fd) {
  RWMutexType::WriteLock lock(m_mutex);
  if ((int)m_datas.size() <= fd) {
    return;
  }
  m_datas[fd].reset();
}
}