/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-03 15:08:03
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-03-03 15:22:57
 * @FilePath     : /sylar/util.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-03 15:08:03
 */
#include "util.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace sylar {

pid_t getThreadId() {
  return syscall(SYS_gettid);
}

uint32_t getFiberId() {
  return 0;
};
}