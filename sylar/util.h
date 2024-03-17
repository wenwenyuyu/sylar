/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-03 15:07:58
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-03-03 15:21:57
 * @FilePath     : /sylar/util.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-03 15:07:58
 */
#ifndef __UTIL_H__
#define __UTIL_H__

#include <sched.h>
#include <stdint.h>
namespace sylar {

pid_t getThreadId();
uint32_t getFiberId();
}
#endif