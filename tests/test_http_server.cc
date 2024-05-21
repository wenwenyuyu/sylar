/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-20 15:50:33
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-21 14:29:32
 * @FilePath     : /tests/test_http_server.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-20 15:50:33
 */

#include "sylar/address.h"
#include "sylar/http/http.h"
#include "sylar/http/http_server.h"
#include "sylar/http/http_session.h"
#include "sylar/iomanager.h"
#include "sylar/hook.h"

void run() {
  sylar::http::HttpServer::ptr server(new sylar::http::HttpServer);
  sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
  while (!server->bind(addr)) {
    sleep(2);
  }
  auto ad = server->getServletDispatch();
  ad->addServlet("/sylar/xx", [](sylar::http::HttpRequest::ptr req,
                                 sylar::http::HttpResponse::ptr rsp,
                                 sylar::http::HttpSession::ptr session) {
    rsp->setBody(req->toString());
    return 0;
  });

  ad->addGlobServlet("/sylar/*", [](sylar::http::HttpRequest::ptr req,
                                 sylar::http::HttpResponse::ptr rsp,
                                 sylar::http::HttpSession::ptr session) {
    rsp->setBody("Glob:\r\n" + req->toString());
    return 0;
  });
   
  server->start();
}

int main() {
  sylar::IOManager iom(2);
  iom.schedule(run);
  return 0;
}