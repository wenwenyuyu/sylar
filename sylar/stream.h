/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-20 10:16:02
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-20 10:30:25
 * @FilePath     : /sylar/stream.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-20 10:16:02
 */
#ifndef __SYLAR_STREAM_H__
#define __SYLAR_STREAM_H__

#include "sylar/bytearray.h"
#include <cstddef>
#include <memory>

namespace sylar {
class Stream {
public:
  typedef std::shared_ptr<Stream> ptr;

  virtual ~Stream();

  virtual int read(void *buffer, size_t length) = 0;
  virtual int read(ByteArray::ptr ba, size_t length) = 0;
  virtual int readFixSize(void *buffer, size_t length);
  virtual int readFixSize(ByteArray::ptr ba, size_t length);
  virtual int write(const void *buffer, size_t length) = 0;
  virtual int write(ByteArray::ptr ba, size_t length) = 0;
  virtual int writeFixSize(const void *buffer, size_t length);
  virtual int writeFixSize(ByteArray::ptr ba, size_t length);
  virtual void close() = 0;
};
} // namespace sylar

#endif