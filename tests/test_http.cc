/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-14 14:36:46
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-14 14:46:37
 * @FilePath     : /tests/test_http.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-14 14:36:46
 */
#include "sylar/http/http.h"
#include <iostream>

void test_request() {
  sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
  req->setHeader("host", "www.baidu.com");
  req->setBody("hello sylar");

  req->dump(std::cout) << std::endl;
}

void test_response() {
  sylar::http::HttpResponse::ptr rsp(new sylar::http::HttpResponse);
  rsp->setHeader("X-X", "sylar");
  rsp->setBody("hello sylar");
  rsp->setStatus((sylar::http::HttpStatus)400);
  rsp->setClose(false);
  rsp->dump(std::cout) << std::endl;
}
int main() {
  test_request();
  test_response();
  return 0;
}