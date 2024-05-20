/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-20 15:50:33
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-20 16:25:04
 * @FilePath     : /tests/test_http_server.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-20 15:50:33
 */

#include "sylar/address.h"
#include "sylar/http/http_server.h"
#include "sylar/iomanager.h"
#include "sylar/hook.h"

void run() {
  sylar::http::HttpServer::ptr server(new sylar::http::HttpServer);
  sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
  while (!server->bind(addr)) {
    sleep(2);
  }
  server->start();
}

int main() {
  sylar::IOManager iom(2);
  iom.schedule(run);
  return 0;
}