/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-22 14:21:25
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-23 13:45:19
 * @FilePath     : /tests/test_http_connection.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-22 14:21:25
 */
#include "sylar/address.h"
#include "sylar/http/http.h"
#include "sylar/http/http_connection.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"
#include "sylar/socket.h"
#include "sylar/util.h"
#include <fstream>
#include <mutex>

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_pool() {
  sylar::http::HttpConnectionPool::ptr pool(new sylar::http::HttpConnectionPool(
      "www.sylar.top", "", 80, 10, 1000 * 30, 5));

  sylar::IOManager::GetThis()->addTimer(
      1000,
      [pool]() {
        auto r = pool->doGet("/", 300);
        SYLAR_LOG_INFO(g_logger) << r->toString();
      },
      true);
}

void run() {
  sylar::Address::ptr addr =
      sylar::Address::LookupAnyIPAddress("www.sylar.top:80");
  if (!addr) {
    SYLAR_LOG_INFO(g_logger) << "get addr error";
    return;
  }

  sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
  bool rt = sock->connect(addr);
  if (!rt) {
    SYLAR_LOG_INFO(g_logger) << "connect " << *addr << " failed";
    return;
  }

  sylar::http::HttpConnection::ptr conn(new sylar::http::HttpConnection(sock));
  sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
  req->setPath("/blog/");
  req->setHeader("host", "www.sylar.top");
  SYLAR_LOG_INFO(g_logger) << "req:" << std::endl << *req;

  conn->sendRequest(req);
  auto rsp = conn->recvResponse();

  if(!rsp) {
    SYLAR_LOG_INFO(g_logger) << "recv response error";
    return;
  }
  SYLAR_LOG_INFO(g_logger) << "rsp:" << std::endl << *rsp;

  SYLAR_LOG_INFO(g_logger) << "=================================";

  auto r = sylar::http::HttpConnection::DoGet("http://www.sylar.top/blog/", 300);
  SYLAR_LOG_INFO(g_logger) << "result=" << r->result << " error=" << r->error
                           << " rsp="
                           << (r->response ? r->response->toString() : "");

  SYLAR_LOG_INFO(g_logger) << "=========================";
  test_pool();
}

int main() {
  sylar::IOManager iom(2);
  iom.schedule(run);
  return 0;
}