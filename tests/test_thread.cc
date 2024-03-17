/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-16 13:36:08
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-03-17 13:55:08
 * @FilePath     : /tests/test_thread.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-16 13:36:08
 */

#include "sylar/mutex.h"
#include "sylar/thread.h"
#include "sylar/log.h"
#include "sylar/config.h"
#include "sylar/util.h"
#include <string>
#include <vector>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();
int count = 0;
sylar::RWMutex m_mutex;

void func1() {
  SYLAR_LOG_INFO(g_logger) << "name: " << sylar::Thread::GetName()
                           << " this.name: "
                           << sylar::Thread::GetThis()->getName()
                           << " id: " << sylar::getThreadId()
                           << " this.id: " << sylar::Thread::GetThis()->getId();
  for (int i = 0; i < 2; i++) {
    sylar::RWMutex::WriteLock lock(m_mutex);
    count++;    
  }

}

int main() {
  SYLAR_LOG_INFO(g_logger) << "thread test begin";

  std::vector<sylar::Thread::ptr> thrs;

  for (int i = 0; i < 5; i++) {
    sylar::Thread::ptr thr(
        new sylar::Thread(func1, "name_" + std::to_string(i)));
    thrs.push_back(thr);
  }

  for (int i = 0; i < 5; i++) {
    thrs[i]->join();
  }

  SYLAR_LOG_INFO(g_logger) << "count: " << count;
  SYLAR_LOG_INFO(g_logger) << "thread test end";
  return 0;
}