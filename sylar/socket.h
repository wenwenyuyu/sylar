/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-04-26 13:28:41
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-19 15:20:00
 * @FilePath     : /sylar/socket.h
 * @Description  : socket封装
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-04-26 13:28:41
 */
#ifndef __SYLAR_SOCKET_H__
#define __SYLAR_SOCKET_H__

#include "sylar/address.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <sys/socket.h>
namespace sylar {
class Socket : public std::enable_shared_from_this<Socket> {
public:
  typedef std::shared_ptr<Socket> ptr;
  typedef std::weak_ptr<Socket> weak_ptr;

  enum Type { TCP = SOCK_STREAM, UDP = SOCK_DGRAM };

  enum Family { IPv4 = AF_INET, IPv6 = AF_INET6, UNIX = AF_UNIX };

  static Socket::ptr CreateTCP(Address::ptr address);
  static Socket::ptr CreateUDP(Address::ptr address);
  static Socket::ptr CreateTCPSocket();
  static Socket::ptr CreateUDPSocket();
  static Socket::ptr CreateTCPSocket6();
  static Socket::ptr CreateUDPSocket6();
  static Socket::ptr CreateUnixTCPSocket();
  static Socket::ptr CreateUnixUDPSocket();

  Socket(const Socket &) = delete;
  Socket(const Socket &&) = delete;
  Socket operator=(const Socket &) = delete;
  Socket operator=(const Socket &&) = delete;

  // 获得类型、协议；并不初始化m_sock
  Socket(int family, int type, int protocol = 0);
  ~Socket();

  int64_t getSendTimeout();
  void setSendTimeout(int64_t v);
  int64_t getRecvTimeout();
  void setRecvTimeout(int64_t v);

  bool getOption(int level, int option, void *result, size_t *len);
  template <class T> bool getOption(int level, int option, T &result) {
    size_t length = sizeof(T);
    return getOption(level, option, &result, &length);
  }
  bool setOption(int level, int option, const void *result, size_t len);
  template <class T> bool setOption(int level, int option, const T &result) {
    return setOption(level, option, &result, sizeof(result));
  }

  // 通过accept获得m_sock
  Socket::ptr accept();
  // 根据fd初始化m_sock
  bool init(int sock);
  bool bind(const Address::ptr addr);
  bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);
  bool listen(int backlog = SOMAXCONN);
  bool close();

  int send(const void *buffer, size_t length, int flags = 0);
  int send(const iovec *buffers, size_t length, int flags = 0);
  int sendTo(const void *buffer, size_t length, const Address::ptr to,
             int flags = 0);
  int sendTo(const iovec *buffers, size_t length, const Address::ptr to,
             int flags = 0);

  int recv(void *buffer, size_t length, int flags = 0);
  int recv(iovec *buffers, size_t length, int flags = 0);
  int recvFrom(void *buffer, size_t length, Address::ptr from, int flags = 0);
  int recvFrom(iovec *buffers, size_t length, Address::ptr from, int flags = 0);

  Address::ptr getRemoteAddress();
  Address::ptr getLocalAddress();

  int getFamily() const {return m_family;};
  int getType() const {return m_type;};
  int getProtocol() const {return m_protocol;}

  bool isConnected() const {return m_isConnected;}
  bool isValid() const;
  int getError();

  std::ostream &dump(std::ostream &os) const;
  std::string ToString() const;
  int getSocket() const {return m_sock;}

  bool cancelRead();
  bool cancelWrite();
  bool cancelAccept();
  bool cancelAll();

private:
  // 新建完socket并进行初始化
  void initSock();
  // 新建一个socket
  void newSock();
private:
  int m_sock;
  int m_family;
  int m_type;
  int m_protocol;
  bool m_isConnected;
  // 本地地址
  Address::ptr m_localAddress;
  // 远端地址
  Address::ptr m_remoteAddress;
};

std::ostream &operator<<(std::ostream &os, const Socket &sock);

}
#endif