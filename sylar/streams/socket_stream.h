/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-20 10:57:27
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-20 13:16:13
 * @FilePath     : /sylar/streams/socket_stream.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-20 10:57:27
 */
#ifndef __SYLAR_SOCKET_STREAM_H__
#define __SYLAR_SOCKET_STREAM_H__

#include "sylar/bytearray.h"
#include "sylar/socket.h"
#include "sylar/stream.h"
#include <memory>

namespace sylar {
class SocketStream : public Stream {
public:
  typedef std::shared_ptr<SocketStream> ptr;
  SocketStream(Socket::ptr sock, bool owner = true);
  ~SocketStream();

  virtual int read(void* buffer, size_t length) override;
  virtual int read(ByteArray::ptr ba, size_t length) override;
  virtual int write(const void *buffer, size_t length) override;
  virtual int write(ByteArray::ptr ba, size_t length) override;
  virtual void close() override;

  Socket::ptr getSocket() const { return m_socket; }
  bool isConnected() const;
protected:
  Socket::ptr m_socket;
  bool m_owner;
};
}
#endif