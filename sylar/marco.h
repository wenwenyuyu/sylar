/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-18 13:52:51
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-04-26 15:02:32
 * @FilePath     : /sylar/marco.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-18 13:52:51
 */
#ifndef __SYLAR_MARCO_H__
#define __SYLAR_MARCO_H__

#include <assert.h>
#include "util.h"

#if defined __GNUC__ || __llvm__
#define SYLAR_LIKELY(x) __builtin_expect(!!(x), 1)
#define SYLAR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define SYLAR_LIKELY(x) (x)
#define SYLAR_UNLIKELY(x) (x)
#endif

#define SYLAR_ASSERT(x)                                                        \
  if (SYLAR_UNLIKELY(!(x))) {                                                                  \
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())                                          \
        << "\nASSERTION: " #x                                                  \
        << "\nbacktrace:" << sylar::BackTraceToString(100, 2, "    ");         \
    assert(x);                                                                 \
  }


#define SYLAR_ASSERT2(x, w)                                                    \
  if (SYLAR_UNLIKELY(!(x))) {                                                                  \
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())                                          \
        << "\nASSERTION: " #x << "    " #w                                     \
        << "\nbacktrace:" << sylar::BackTraceToString(100, 2, "    ");         \
    assert(x);                                                                 \
  }

#endif