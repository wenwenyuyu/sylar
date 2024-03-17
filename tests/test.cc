/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-02-28 13:37:18
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-03-13 15:02:13
 * @FilePath     : /tests/test.cc
 * @Description  : 测试文件
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-02-28 13:37:18
 */

#include "../sylar/log.h"
#include <ctime>
#include <iostream>
#include "../sylar/util.h"

int main() {
  std::cout << "main start" << std::endl;
  sylar::Logger::ptr logger(new sylar::Logger);
  logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));

  // sylar::FileAppender::ptr file_append(new sylar::FileAppender("./log.txt"));
  // logger->addAppender(file_append);
  // sylar::LogEvent::ptr event =
  //     std::make_shared<sylar::LogEvent>(__FILE__, __LINE__, 0,
  //     sylar::getThreadId(), sylar::getFiberId(), time(0));
  // event->getSS() << "hello sylar";
  // logger->log(sylar::LogLevel::DEBUG, event);

  // SYLAR_LOG_DEBUG(logger) << "hello sylar";
  // SYLAR_LOG_INFO(logger) << "info sylar";
  // SYLAR_LOG_FORMAT_LEVEL(logger, sylar::LogLevel::DEBUG, "%s", "hello");
  // SYLAR_LOG_FORMAT_DEBUG(logger, "test %s", "hello");

  auto l = sylar::LoggerMagr::getInstance()->getLogger("xx");
  SYLAR_LOG_INFO(l) << "logger manager";
  auto r = sylar::LoggerMagr::getInstance()->getLogger("xxx");
  SYLAR_LOG_INFO(r) << "logger manager";
  return 0;
}