/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-20 10:20:43
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-20 10:27:21
 * @FilePath     : /sylar/stream.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-20 10:20:43
 */
#include "sylar/stream.h"
#include <cstdint>

namespace sylar {
Stream::~Stream() {
  
}

int Stream::readFixSize(void *buffer, size_t length) {
  size_t offset = 0;
  int64_t left = length;
  while (left > 0) {
    int64_t len = read(static_cast<char *>(buffer) + offset, left);
    if (len <= 0) {
      return len;
    }
    offset += len;
    left -= len;
  }
  return length;
}

int Stream::readFixSize(ByteArray::ptr ba, size_t length) {
  int64_t left = length;
  while (left > 0) {
    int64_t len = read(ba, left);
    if (len <= 0) {
      return len;
    }
    left -= len;
  }
  return length;
}

int Stream::writeFixSize(const void *buffer, size_t length) {
  size_t offset = 0;
  int64_t left = length;
  while (left > 0) {
    int64_t len = write((const char *)buffer + offset, left);
    if (len <= 0) {
      return len;
    }
    offset += len;
    left -= len;
  }
  return length;
}

int Stream::writeFixSize(ByteArray::ptr ba, size_t length) {
  int64_t left = length;
  while (left > 0) {
    int64_t len = write(ba, left);
    if (len <= 0) {
      return len;
    }
    left -= len;
  }
  return length;
}
}