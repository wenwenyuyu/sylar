/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-20 15:40:17
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-21 14:21:53
 * @FilePath     : /sylar/http/http_server.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-20 15:40:17
 */
#include "sylar/http/http_server.h"
#include "sylar/http/http.h"
#include "sylar/http/http_session.h"
#include "sylar/http/servlet.h"
#include "sylar/log.h"
#include "sylar/util.h"
#include "sylar/tcp_server.h"

namespace sylar {
namespace http {
static Logger::ptr g_logger = SYLAR_LOG_ROOT();

HttpServer::HttpServer(bool keepalive, IOManager *worker,
                       IOManager *accept_worker)
    : TcpServer(worker, accept_worker, accept_worker),
      m_isKeepalive(keepalive) {
  m_dispatch.reset(new ServletDispatch);
}

void HttpServer::handleClient(Socket::ptr client) {
  HttpSession::ptr session(new HttpSession(client));
  do {
    // SYLAR_LOG_INFO(g_logger) << "start to recv request";
    auto req = session->recvRequest();
    // SYLAR_LOG_INFO(g_logger) << "request recv";
    if (!req) {
      SYLAR_LOG_DEBUG(g_logger)
          << "recv http request fail, errno=" << errno
          << " errstr=" << strerror(errno) << " cliet:" << *client
          << " keep_alive=" << m_isKeepalive;
      break;
    }

    HttpResponse::ptr rsp(
        new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
    // rsp->setBody("hello sylar");
    m_dispatch->handle(req, rsp, session);
    session->sendResponse(rsp);
  } while (m_isKeepalive);
  session->close();
  // SYLAR_LOG_INFO(g_logger) << "session close";
}
}
}