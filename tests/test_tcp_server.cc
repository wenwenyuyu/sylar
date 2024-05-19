/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-19 15:36:06
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-19 16:03:49
 * @FilePath     : /tests/test_tcp_server.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-19 15:36:06
 */
#include "sylar/tcp_server.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"
#include "sylar/util.h"
#include "sylar/hook.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run() {
  auto addr = sylar::Address::LookupAny("0.0.0.0:8033");
  SYLAR_LOG_INFO(g_logger) << *addr;
  std::vector<sylar::Address::ptr> addrs;
  addrs.push_back(addr);
    //addrs.push_back(addr2);

  sylar::TcpServer::ptr tcp_server(new sylar::TcpServer);
  std::vector<sylar::Address::ptr> fails;
  while(!tcp_server->bind(addrs, fails)) {
      sleep(2);
  }
  tcp_server->start();
}

int main(int argc, char** argv) {
    sylar::IOManager iom(2);
    iom.schedule(run);
    return 0;
}