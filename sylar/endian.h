/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-04-24 16:33:04
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-04-24 16:54:21
 * @FilePath     : /sylar/endian.h
 * @Description  : 网络字节序
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-04-24 16:33:04
 */

#ifndef __SYLAR_ENDIAN_H__
#define __SYLAR_ENDIAN_H__

#include <cstdint>
#include <type_traits>
#include <byteswap.h>
#define SYLAR_LITTLE_ENDIAN 1
#define SYLAR_BIG_ENDIAN 2

namespace sylar {

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value) {
  return (T)bswap_64((uint64_t)value);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
  return (T)bswap_32((uint32_t)value);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
  return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIANS
#define SYLAR_BYTE_ORDER SYLAR_BIG_ENDIAN
#else
#define SYLAR_BYTE_ORDER SYLAR_LITTLE_ENDIAN
#endif

// 网络协议使用大端序，如果主机为大端序则不需要转换
#if SYLAR_BYTE_ORDER == SYLAR_BIG_ENDIAN
template <class T> T byteswapOnLittleEndian(T t) { return t; }

template <class T> T byteswapOnBigEndian(T t) { return byteswap(t); }

#else
template <class T> T byteswapOnLittleEndian(T t) { return byteswap(t); }

template <class T> T byteswapOnBigEndian(T t) { return t; }

#endif

}

#endif