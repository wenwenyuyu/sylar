/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-20 13:43:39
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-20 15:36:50
 * @FilePath     : /sylar/http/http_session.h
 * @Description  :
 * Copyright 2024 OBKoro1, All Rights Reserved.
 * 2024-05-20 13:43:39
 */
#ifndef __SYLAR_HTTP_SESSION_H__
#define __SYLAR_HTTP_SESSION_H__

#include "sylar/http/http.h"
#include "sylar/socket.h"
#include "sylar/streams/socket_stream.h"
#include <memory>

namespace sylar {
namespace http {
class HttpSession : public SocketStream {
public:
  typedef std::shared_ptr<HttpSession> ptr;

  HttpSession(Socket::ptr sock, bool ower = true);
  HttpRequest::ptr recvRequest();
  int sendResponse(HttpResponse::ptr rsp);
};
}
} // namespace sylar

#endif