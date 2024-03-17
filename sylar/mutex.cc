/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-16 16:04:57
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-03-17 15:09:30
 * @FilePath     : /sylar/mutex.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-16 16:04:57
 */

#include "mutex.h"
#include <atomic>
#include <pthread.h>
#include <semaphore.h>
#include <stdexcept>

namespace sylar {
Semaphore::Semaphore(std::uint32_t count) {
  if (sem_init(&m_sem, 0, count)) {
    throw std::logic_error("sem_init error");
  }
}

Semaphore::~Semaphore() {
  sem_destroy(&m_sem);
}

void Semaphore::wait() {
  if (sem_wait(&m_sem)) {
    throw std::logic_error("sem_wait error");
  }
}

void Semaphore::notify() {
  if (sem_post(&m_sem)) {
    throw std::logic_error("sem_post error");
  }
}

Mutex::Mutex() { pthread_mutex_init(&m_mutex, nullptr); }

Mutex::~Mutex() { pthread_mutex_destroy(&m_mutex); }

void Mutex::lock() { pthread_mutex_lock(&m_mutex); }

void Mutex::unlock() { pthread_mutex_unlock(&m_mutex); }

RWMutex::RWMutex() { pthread_rwlock_init(&m_lock, nullptr); }

RWMutex::~RWMutex() { pthread_rwlock_destroy(&m_lock); }

void RWMutex::rdlock() { pthread_rwlock_rdlock(&m_lock); }

void RWMutex::wrlock() { pthread_rwlock_wrlock(&m_lock); }

void RWMutex::unlock() { pthread_rwlock_unlock(&m_lock); }

Spinlock::Spinlock() { pthread_spin_init(&m_mutex, 0); }

Spinlock::~Spinlock() { pthread_spin_destroy(&m_mutex); }

void Spinlock::lock() { pthread_spin_lock(&m_mutex); }

void Spinlock::unlock() { pthread_spin_unlock(&m_mutex); }

CASLock::CASLock() { m_mutex.clear(); }

CASLock::~CASLock() {  }

void CASLock::lock() {
  while (std::atomic_flag_test_and_set_explicit(&m_mutex,
                                                std::memory_order_acquire))
    ;
}

void CASLock::unlock() {
  std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
}

}