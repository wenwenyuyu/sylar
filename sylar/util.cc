/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-03 15:08:03
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-20 19:58:19
 * @FilePath     : /sylar/util.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-03 15:08:03
 */
#include "util.h"
#include "sylar/fiber.h"
#include "sylar/log.h"
#include <bits/types/struct_timeval.h>
#include <sys/time.h>
#include <cstddef>
#include <cstdlib>
#include <pthread.h>
#include <sstream>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <execinfo.h>
#include <vector>

namespace sylar {

sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

pid_t getThreadId() {
  return syscall(SYS_gettid);
}

uint32_t getFiberId() { return sylar::Fiber::GetFiberId(); };


void BackTrace(std::vector<std::string> &bt, int size, int skip) {
  void **buff = (void **)malloc(sizeof(void *) * size);
  int nptrs = ::backtrace(buff, size);

  char **strings = ::backtrace_symbols(buff, nptrs);
  if (strings == NULL) {
    SYLAR_LOG_ERROR(g_logger) << "backtrace symbols error";
    return;
  }

  for (int i = skip; i < nptrs; ++i) {
    bt.push_back(strings[i]);
  }

  free(strings);
  free(buff);
}

std::string BackTraceToString(int size, int skip, const std::string &prefix) {
  std::vector<std::string> bt;
  BackTrace(bt, size, skip);
  std::stringstream ss;
  ss << std::endl;
  for (size_t i = 0; i < bt.size(); ++i) {
    ss << prefix << bt[i] << std::endl;
  }
  return ss.str();
}

// tv_sec : 秒
// tv_usec : 微秒
uint64_t GetCurrentMS() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

uint64_t GetCurrentUS() {
    struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}
}