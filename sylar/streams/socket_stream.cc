/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-20 13:04:46
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-20 19:04:07
 * @FilePath     : /sylar/streams/socket_stream.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-20 13:04:46
 */
#include "sylar/streams/socket_stream.h"
#include "sylar/log.h"
#include "sylar/util.h"
#include <bits/types/struct_iovec.h>
#include <vector>

namespace sylar {
static Logger::ptr g_logger = SYLAR_LOG_ROOT();

SocketStream::SocketStream(Socket::ptr sock, bool owner)
    : m_socket(sock), m_owner(owner) {}


SocketStream::~SocketStream() {
    if(m_owner && m_socket) {
        m_socket->close();
    }
}

bool SocketStream::isConnected() const {
    return m_socket && m_socket->isConnected();
}

int SocketStream::read(void* buffer, size_t length) {
    if(!isConnected()) {
        return -1;
    }
    SYLAR_LOG_INFO(g_logger) << "SockerStream start to read data";
    return m_socket->recv(buffer, length);
}

int SocketStream::read(ByteArray::ptr ba, size_t length) {
    if(!isConnected()) {
        return -1;
    }
    std::vector<iovec> iovs;
    ba->getWriteBuffers(iovs, length);
    int rt = m_socket->recv(&iovs[0], iovs.size());
    if(rt > 0) {
        ba->setPosition(ba->getPosition() + rt);
    }
    return rt;
}
int SocketStream::write(const void *buffer, size_t length) {
    if(!isConnected()) {
        return -1;
    }
    return m_socket->send(buffer, length);
}

int SocketStream::write(ByteArray::ptr ba, size_t length) {
    if(!isConnected()) {
        return -1;
    }
    std::vector<iovec> iovs;
    ba->getReadBuffers(iovs, length);
    int rt = m_socket->send(&iovs[0], iovs.size());
    if(rt > 0) {
        ba->setPosition(ba->getPosition() + rt);
    }
    return rt;
}

void SocketStream::close() {
  if (m_socket) {
    SYLAR_LOG_INFO(g_logger) << "SocketStream::close()";
    m_socket->close();
  }
}
}