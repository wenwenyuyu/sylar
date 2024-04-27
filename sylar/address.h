/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-04-24 14:26:34
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-04-27 19:33:57
 * @FilePath     : /sylar/address.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-04-24 14:26:34
 */
#ifndef __SYLAR_ADDRESS_H__
#define __SYLAR_ADDRESS_H__

#include <cstdint>
#include <map>
#include <memory>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <utility>
#include <vector>

namespace sylar {

class IPAddress;

class Address {
public:
  typedef std::shared_ptr<Address> ptr;
  virtual ~Address();

  static bool Lookup(std::vector<Address::ptr> &result, const std::string &host,
                     int family = AF_UNSPEC, int type = 0, int protocol = 0);  
  static Address::ptr LookupAny(const std::string &host, int family = AF_UNSPEC,
                                int type = 0, int protocol = 0);
  static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string &host,
                                                       int family = AF_UNSPEC,
                                                       int type = 0,
                                                       int protocol = 0);

  static bool GetInterfaceAddress(
      std::multimap<std::string, std::pair<Address::ptr, uint32_t>> &result,
      int family = AF_UNSPEC);
  static bool
  GetInterfaceAddress(std::vector<std::pair<Address::ptr, uint32_t>> &result,
                      const std::string &iface, int family = AF_UNSPEC);
  

  static Address::ptr Create(const sockaddr* address, socklen_t addrlen);
  int getFamily() const;
  virtual const sockaddr *getAddr() const = 0;
  virtual socklen_t getAddrLen() const = 0;

  virtual std::ostream &insert(std::ostream &os) const = 0;
  std::string toString();

  bool operator<(const Address &rhs) const;
  bool operator==(const Address &rhs) const;
  bool operator!=(const Address &rhs) const;
};

class IPAddress : public Address {
public:
  typedef std::shared_ptr<IPAddress> ptr;

  static IPAddress::ptr Create(const char *address, uint16_t port = 0);
  virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
  virtual IPAddress::ptr networkAddress(uint32_t prefix_len) = 0;
  virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

  virtual uint32_t getPort() const = 0;
  virtual void setPort(uint16_t v) = 0;
};

class IPv4Address : public IPAddress {
public:
  typedef std::shared_ptr<IPv4Address> ptr;
  static IPv4Address::ptr Create(const char *address, uint16_t port);
  IPv4Address(const sockaddr_in& address);
  IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

  IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
  IPAddress::ptr networkAddress(uint32_t prefix_len) override;
  IPAddress::ptr subnetMask(uint32_t prefix_len) override;

  const sockaddr *getAddr() const override;
  socklen_t getAddrLen() const override;
  std::ostream &insert(std::ostream &os) const override;

  uint32_t getPort() const override;
  void setPort(uint16_t v) override;

private:
  sockaddr_in m_addr;
};

class IPv6Address : public IPAddress {
public:
  typedef std::shared_ptr<IPv6Address> ptr;
  static IPv6Address::ptr Create(const char* address , uint16_t port = 0);  
  IPv6Address();
  IPv6Address(const sockaddr_in6 &address);
  IPv6Address(const uint8_t address[16], uint16_t port = 0);

  IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
  IPAddress::ptr networkAddress(uint32_t prefix_len) override;
  IPAddress::ptr subnetMask(uint32_t prefix_len) override;

  const sockaddr *getAddr() const override;
  socklen_t getAddrLen() const override;
  std::ostream &insert(std::ostream &os) const override;

  uint32_t getPort() const override;
  void setPort(uint16_t v) override;

private:
  sockaddr_in6 m_addr;
};

class UnixAddress : public Address {
public:
  typedef std::shared_ptr<UnixAddress> ptr;
  UnixAddress();
  UnixAddress(const std::string &path);
  const sockaddr *getAddr() const override;
  socklen_t getAddrLen() const override;
  void setAddrLen(const socklen_t &len) { m_length = len; }
  std::ostream &insert(std::ostream &os) const override;
private:
  struct sockaddr_un m_addr;
  socklen_t m_length;
};

class UnknownAddress : public Address {
public:
  typedef std::shared_ptr<UnknownAddress> ptr;
  UnknownAddress(int family);
  UnknownAddress(const sockaddr& addr);
  const sockaddr *getAddr() const override;
  socklen_t getAddrLen() const override;
  std::ostream &insert(std::ostream &os) const override;
private:
  sockaddr m_addr;
};

}
#endif