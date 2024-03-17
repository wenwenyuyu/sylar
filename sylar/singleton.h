/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-03 20:56:25
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-03-13 15:04:09
 * @FilePath     : /sylar/singleton.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-03 20:56:25
 */
#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <cstdio>
#include <memory>
#include <iostream>
namespace sylar {

template <class T, class X = void, int N = 0>
class Singleton {
public:
  static T *getInstance() {
    static T v;
    return &v;
  }
};

template <class T, class X = void, int N = 0>
class SingletonPtr {
public:
  static std::shared_ptr<T> getInstance() {
    static std::shared_ptr<T> v(new T);
    return v;
  }
};

}
#endif