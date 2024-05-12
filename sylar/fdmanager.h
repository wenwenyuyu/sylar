/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-04-21 19:21:42
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-12 20:22:35
 * @FilePath     : /sylar/fdmanager.h
 * @Description  : 为了保存被hoo fd的集合
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-04-21 19:21:42
 */
#ifndef __SYLAR_FDMANAGER_H__
#define __SYLAR_FDMANAGER_H__

#include "sylar/mutex.h"
#include "sylar/singleton.h"
#include <boost/type.hpp>
#include <cstdint>
#include <memory>
#include <vector>
namespace sylar {

class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
  typedef std::shared_ptr<FdCtx> ptr;
  FdCtx(int fd);
  ~FdCtx();

  bool init();
  bool isInit() const { return m_isInit; }
  bool isSocket() const { return m_isSocket; }
  bool isClose() const { return m_isClosed; }
  bool close();

  void setUserNonblock(bool v) { m_sysNonblock = v; }
  bool getUserNonblock() const { return m_userNonblock; }

  void setSysNonblock(bool v) { m_sysNonblock = v; }
  bool getSysNonblock() const { return m_sysNonblock; }

  void setTimeout(int type, uint64_t v);
  uint64_t getTimeout(int type);

private:
  // 是否初始化
  bool m_isInit : 1;
  // 是否是socket
  bool m_isSocket : 1;
  // 是否hook非阻塞
  bool m_sysNonblock : 1;
  // 是否用户主动设置非阻塞
  bool m_userNonblock : 1;
  // 是否关闭
  bool m_isClosed : 1;
  // 文件句柄
  int m_fd;
  // 读超时时间
  uint64_t m_recvTimeout;
  // 写超时时间
  uint64_t m_sendTimeout;
};

class FdManager {
public:
  typedef RWMutex RWMutexType;

  FdManager();

  FdCtx::ptr get(int fd, bool auto_create = false);
  void del(int fd);

private:
  // 读写锁
  RWMutexType m_mutex;
  // 文件句柄合集
  std::vector<FdCtx::ptr> m_datas;
};

// 文件句柄管理单例模式
typedef Singleton<FdManager> FdMgr;
}

#endif