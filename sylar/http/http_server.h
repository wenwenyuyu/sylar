/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-20 15:34:06
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-21 14:12:55
 * @FilePath     : /sylar/http/http_server.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-20 15:34:06
 */
#ifndef __SYLAR_HTTP_SERVER_H__
#define __SYLAR_HTTP_SERVER_H__

#include "sylar/http/servlet.h"
#include "sylar/iomanager.h"
#include "sylar/socket.h"
#include "sylar/tcp_server.h"
#include <memory>
namespace sylar {
namespace http {
class HttpServer : public TcpServer {
public:
  typedef std::shared_ptr<HttpServer> ptr;
  HttpServer(bool keepalive = false, IOManager *worker = IOManager::GetThis(),
             IOManager *accept_worker = IOManager::GetThis());

  ServletDispatch::ptr getServletDispatch() const { return m_dispatch; }
  void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v; }
protected:
  virtual void handleClient(Socket::ptr client) override;
private:
  bool m_isKeepalive;
  ServletDispatch::ptr m_dispatch;
};
}
}
#endif