/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-14 15:52:41
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-15 15:17:25
 * @FilePath     : /sylar/http/http_parser.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-14 15:52:41
 */
#ifndef __SYLAR_PARSER_H__
#define __SYLAR_PARSER_H__

#include "sylar/http/http.h"
#include "sylar/http/http11_parser.h"
#include "sylar/http/httpclient_parser.h"
#include <cstddef>
#include <cstdint>
#include <memory>
namespace sylar {
namespace http {

class HttpRequestParser {
public:
  typedef std::shared_ptr<HttpRequestParser> ptr;
  HttpRequestParser();
  size_t execute(char *data, size_t len);
  int isFinish();
  int hasError();

  HttpRequest::ptr getData() const { return m_data; }
  int getError() const { return m_error; }
  void setError(int v) { m_error = v; }
  uint64_t getContentLength();
private:
  http_parser m_parser;
  HttpRequest::ptr m_data;
  int m_error;
};

class HttpResponseParser {
public:
  typedef std::shared_ptr<HttpResponseParser> ptr;
  HttpResponseParser();
  size_t execute(char *data, size_t len);
  int isFinish();
  int hasError();

  HttpResponse::ptr getData() const { return m_data; }
  int getError() const { return m_error; }
  void setError(int v) { m_error = v; }
  uint64_t getContentLength();
private:
  httpclient_parser m_parser;
  HttpResponse::ptr m_data;
  int m_error;
};

}
}
#endif