/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-22 15:53:48
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-22 15:55:48
 * @FilePath     : /tests/test_uri.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-22 15:53:48
 */
#include "sylar/uri.h"
#include <iostream>
#include <ostream>

int main() {
  sylar::Uri::ptr uri =
      sylar::Uri::Create("http://www.sylar.top/test/uri?id=100&name=sylar#frg");
  std::cout << uri->toString() << std::endl;
  auto addr = uri->createAddress();
  std::cout << *addr << std::endl;
  return 0;
}