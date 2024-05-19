/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-19 14:57:33
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-19 15:28:49
 * @FilePath     : /sylar/tcp_server.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-19 14:57:33
 */
#ifndef __SYLAR_TCP_SERVER_H__
#define __SYLAR_TCP_SERVER_H__

#include "sylar/address.h"
#include "sylar/iomanager.h"
#include "sylar/socket.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
namespace sylar {

class TcpServer : public std::enable_shared_from_this<TcpServer> {
public:
  typedef std::shared_ptr<TcpServer> ptr;
  TcpServer(IOManager *worker = IOManager::GetThis(),
            IOManager *io_worker = IOManager::GetThis(),
            IOManager *accept_worker = IOManager::GetThis());
  virtual ~TcpServer();

  virtual bool bind(Address::ptr addr);
  virtual bool bind(const std::vector<Address::ptr> &addrs,
                    std::vector<Address::ptr> &fails);
  virtual bool start();
  virtual bool stop();

  uint64_t getReadTime() const { return m_recvTimeout; }
  std::string getName() const { return m_name; }
  void setReadTimeOut(uint64_t v) { m_recvTimeout = v; }
  void setName(const std::string &v) { m_name = v; }

  bool isStop() const { return m_isStop; }

protected:
  virtual void handleClient(Socket::ptr client);
  virtual void startAccept(Socket::ptr sock);
private:
  std::vector<Socket::ptr> m_socks;

  IOManager *m_worker;
  IOManager *m_ioWorker;

  IOManager *m_acceptWorker;

  uint64_t m_recvTimeout;

  std::string m_name;
  std::string m_type = "tcp";
  bool m_isStop;
};
} // namespace sylar

#endif