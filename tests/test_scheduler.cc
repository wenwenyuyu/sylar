/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-22 15:33:29
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-04-02 17:05:00
 * @FilePath     : /tests/test_scheduler.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-22 15:33:29
 */
#include "sylar/log.h"
#include "sylar/scheduler.h"
#include "sylar/util.h"
#include <unistd.h>

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber() {
    static int s_count = 5;
    SYLAR_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        sylar::Scheduler::GetThis()->schedule(&test_fiber, sylar::getThreadId());
    }
}

void test_fiber1() { SYLAR_LOG_INFO(g_logger) << "test_fiber"; }

void test1() {
    SYLAR_LOG_INFO(g_logger) << "main";
    sylar::Scheduler sc(3, true, "test");
    sc.start();
    sleep(2);
    SYLAR_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber);
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << "over";
}


int main(int argc, char **argv) {
  //   sylar::Scheduler sc;
  //   sc.start();
  //   sc.schedule(&test_fiber1);
  //   sc.stop();
  test1();

  return 0;
}