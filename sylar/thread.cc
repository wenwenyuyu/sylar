/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-16 13:00:41
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-03-16 17:14:11
 * @FilePath     : /sylar/thread.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-16 13:00:41
 */

#include "thread.h"
#include "sylar/log.h"
#include "sylar/util.h"
#include <functional>
#include <pthread.h>

namespace sylar {
static thread_local Thread *t_thread = nullptr;
static thread_local std::string t_name = "UNKNOW";
static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

Thread::Thread(std::function<void()> cb, const std::string name)
    : m_name(name), m_cb(cb) {
  if (name.empty()) {
    m_name = "UNKNOW";
  }

  int rt = pthread_create(&m_thread, nullptr, &Thread::Run, this);
  if (rt) {
    SYLAR_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
            << " name=" << name;
    throw std::logic_error("pthread_create error");
  }

  m_sem.wait();
}

Thread::~Thread() {
  if (m_thread) {
    pthread_detach(m_thread);
  }
}

void Thread::join() {
  if (m_thread) {
    int rt = pthread_join(m_thread, nullptr);
    if (rt) {
        SYLAR_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
                << " name=" << m_name;
        throw std::logic_error("pthread_create error");
    }
    m_thread = 0;
  }
}

void *Thread::Run(void *args) {
  Thread *th = (Thread *)args;
  t_thread = th;
  t_name = t_thread->m_name;

  th->m_pid = sylar::getThreadId();
  pthread_setname_np(pthread_self(), th->m_name.substr(0, 15).c_str());
  std::function<void()> cb;
  // 使用swap而不使用复制，是为了防止函数里面有智能指针，增加引用，导致智能指针不会被释放
  cb.swap(th->m_cb);
  t_thread->m_sem.notify();  
  cb();

  return 0;
}

Thread *Thread::GetThis() {
  return t_thread;
}

const std::string &Thread::GetName() {
  return t_name;
}

void Thread::SetName(const std::string &name) {
  if (name.empty()) {
    return;
  }
  if (t_thread) {
    t_thread->m_name = name;
  }
  t_name = name;
}


}