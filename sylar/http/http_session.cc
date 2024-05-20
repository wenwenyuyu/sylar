/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-20 13:43:48
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-20 16:21:42
 * @FilePath     : /sylar/http/http_session.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-20 13:43:48
 */
#include "sylar/http/http_session.h"
#include "sylar/http/http_parser.h"
#include "sylar/streams/socket_stream.h"
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>

namespace sylar {
namespace http {
HttpSession::HttpSession(Socket::ptr sock, bool ower)
    : SocketStream(sock, ower) {}

HttpRequest::ptr HttpSession::recvRequest() {
  HttpRequestParser::ptr parser(new HttpRequestParser);
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    //uint64_t buff_size = 100;
    std::shared_ptr<char> buffer(
            new char[buff_size], [](char* ptr){
                delete[] ptr;
            });
    char* data = buffer.get();
    int offset = 0;
    do {
        int len = read(data + offset, buff_size - offset);
        if(len <= 0) {
            close();
            return nullptr;
        }
        len += offset;
        size_t nparse = parser->execute(data, len);
        if(parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparse;
        if(offset == (int)buff_size) {
            close();
            return nullptr;
        }
        if(parser->isFinish()) {
            break;
        }
    } while(true);
    int64_t length = parser->getContentLength();
    if(length > 0) {
        std::string body;
        body.resize(length);

        int len = 0;
        if(length >= offset) {
            memcpy(&body[0], data, offset);
            len = offset;
        } else {
            memcpy(&body[0], data, length);
            len = length;
        }
        length -= offset;
        if(length > 0) {
            if(readFixSize(&body[len], length) <= 0) {
                close();
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }

    return parser->getData();
}

int HttpSession::sendResponse(HttpResponse::ptr rsp) {
  std::stringstream ss;
  ss << *rsp;
  std::string data = ss.str();
  return writeFixSize(data.c_str(), data.size());
}
}
}