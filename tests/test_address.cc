/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-04-26 10:22:27
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-04-26 10:51:45
 * @FilePath     : /tests/test_address.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-04-26 10:22:27
 */
#include "sylar/address.h"
#include "sylar/log.h"
#include "sylar/util.h"
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <sys/socket.h>
#include <utility>
#include <vector>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test() {
  std::vector<sylar::Address::ptr> addrs;

  bool v = sylar::Address::Lookup(addrs, "www.baidu.com:ftp", AF_INET, SOCK_STREAM);

  if (!v) {
    SYLAR_LOG_ERROR(g_logger) << "look up failed";
    return;
  }

  for (size_t i = 0; i < addrs.size(); i++) {
    SYLAR_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
  }
}

void test_iface() {
  std::multimap<std::string, std::pair<sylar::Address::ptr, uint32_t>> results;

  bool v = sylar::Address::GetInterfaceAddress(results);
  if (!v) {
    SYLAR_LOG_ERROR(g_logger) << "GetInterfaceAddress failed";
    return;
  }

  for (auto &i : results) {
    SYLAR_LOG_INFO(g_logger)
        << i.first << " - " << i.second.first->toString() << " - " << i.second.second;
  }
}

void test_ipv4() {
  // auto addr = sylar::IPAddress::Create("www.baidu.com");
  auto addr = sylar::IPAddress::Create("127.0.0.1");
  if (addr) {
    SYLAR_LOG_INFO(g_logger) <<addr->toString();
  }
}
int main() {
  test();
  test_iface();
  test_ipv4();
  return 0;
}