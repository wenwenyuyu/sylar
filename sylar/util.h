/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-03 15:07:58
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-04-06 14:40:36
 * @FilePath     : /sylar/util.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-03 15:07:58
 */
#ifndef __UTIL_H__
#define __UTIL_H__

#include <cstdint>
#include <sched.h>
#include <stdint.h>
#include <string>
#include <vector>
namespace sylar {

pid_t getThreadId();
uint32_t getFiberId();

void BackTrace(std::vector<std::string> &bt, int size = 64, int skip = 1);
std::string BackTraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

uint64_t GetCurrentMS();
uint64_t GetCurrentUS();
}
#endif