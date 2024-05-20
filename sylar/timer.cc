/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-04-06 14:20:32
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-20 17:01:04
 * @FilePath     : /sylar/timer.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-04-06 14:20:32
 */

#include "timer.h"
#include "sylar/log.h"
#include "sylar/util.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace sylar {
static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

// 定时器比较任务，通过m_next进行比较
bool Timer::Comparator::operator()(const Timer::ptr &lhs,
                                   const Timer::ptr &rhs) const {
  if (!lhs && !rhs) {
    return false;
  }

  if (!lhs) {
    return true;
  }

  if (!rhs) {
    return false;
  }

  if (lhs->m_next < rhs->m_next) {
    return true;
  }

  if (rhs->m_next < lhs->m_next) {
    return false;
  }
  return lhs.get() < rhs.get();

}

/**
 * @func: 
 * @param {uint64_t} ms
 * @return {*}
 * @description: 定时器创建
 */
Timer::Timer(uint64_t ms, std::function<void()> cb, bool recurring,
             TimerManager *manager)
    : m_recurring(recurring), m_ms(ms), m_cb(cb), m_manager(manager){
  m_next = GetCurrentMS() + ms;
}

Timer::Timer(uint64_t next) : m_next(next) {}

/**
 * @func: 
 * @return {*}
 * @description: 取消定时器
 */
bool Timer::cancel() {
  TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
  if (m_cb) {
    m_cb = nullptr;
    auto it = m_manager->m_timers.find(shared_from_this());
    m_manager->m_timers.erase(it);
    return true;
  }
  return false;
}

/**
 * @func: 
 * @return {*}
 * @description: 将定时器的next时间更新到从现在开始
 */
bool Timer::refresh() {
  TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
  if (!m_cb) {
    return false;
  }
  auto it = m_manager->m_timers.find(shared_from_this());
  if (it == m_manager->m_timers.end()) {
    return false;
  }

  m_manager->m_timers.erase(it);
  m_next = GetCurrentMS() + m_ms;
  m_manager->m_timers.insert(shared_from_this());
  return true;
}

/**
 * @func: 
 * @param {uint64_t} ms
 * @param {bool} from_now
 * @return {*}
 * @description: 重置定时器
 */
bool Timer::reset(uint64_t ms, bool from_now) {
  if (ms == m_ms && !from_now) {
    return true;
  }
  TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
  if (!m_cb) {
    return false;
  }
  auto it = m_manager->m_timers.find(shared_from_this());
  if (it == m_manager->m_timers.end()) {
    return false;
  }

  m_manager->m_timers.erase(it);
  uint64_t start = 0;
  if (from_now) {
    start = GetCurrentMS();
  } else {
    start = m_next - m_ms;
  }

  m_ms = ms;
  m_next = start + ms;
  m_manager->addTimer(shared_from_this(), lock);
  return true;
}

TimerManager::TimerManager() {
  m_previousTime = GetCurrentMS();
}

TimerManager::~TimerManager() {
  
}
/**
 * @func: 
 * @param {uint64_t} ms
 * @return {*}
 * @description: 增加定时器
 */
Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb,
                                  bool recurring) {
  Timer::ptr timer(new Timer(ms, cb, recurring, this));
  RWMutexType::WriteLock lock(m_mutex);
  addTimer(timer, lock);
  return timer;
}

static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
  std::shared_ptr<void> tmp = weak_cond.lock();
  if (tmp) {
    cb();
  }
}

Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb,
                             std::weak_ptr<void> weak_cond,
                             bool recurring) {
  return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
}
/**
 * @func: 
 * @return {*}
 * @description: 获得距离下一个定时器时间
 */
uint64_t TimerManager::getNextTimer() {
  RWMutexType::ReadLock lock(m_mutex);
  m_tickled = false;
  if (m_timers.empty()) {
    return ~0ull;
  }

  const Timer::ptr &next = *(m_timers.begin());
  uint64_t now_ms = GetCurrentMS();
  if (now_ms >= next->m_next) {
    return 0;
  } else {
    return next->m_next - now_ms;
  }
}
/**
 * @func: 
 * @param {vector<std::function<void()>>} &cbs
 * @return {*}
 * @description: 获得所有该触发的定时器任务列表
 */
void TimerManager::ListExpiredCb(std::vector<std::function<void()>> &cbs) {
    uint64_t now_ms = sylar::GetCurrentMS();
    std::vector<Timer::ptr> expired;
    {
        RWMutexType::ReadLock lock(m_mutex);
        if(m_timers.empty()) {
            return;
        }
    }
    RWMutexType::WriteLock lock(m_mutex);
    if(m_timers.empty()) {
        return;
    }
    bool rollover = detectClockRollover(now_ms);
    if(!rollover && ((*m_timers.begin())->m_next > now_ms)) {
        return;
    }

    Timer::ptr now_timer(new Timer(now_ms));
    auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer);
    while(it != m_timers.end() && (*it)->m_next == now_ms) {
        ++it;
    }
    expired.insert(expired.begin(), m_timers.begin(), it);
    m_timers.erase(m_timers.begin(), it);
    cbs.reserve(expired.size());

    for(auto& timer : expired) {
        cbs.push_back(timer->m_cb);
        if(timer->m_recurring) {
            timer->m_next = now_ms + timer->m_ms;
            m_timers.insert(timer);
        } else {
            timer->m_cb = nullptr;
        }
    }
}

void TimerManager::addTimer(Timer::ptr timer, RWMutexType::WriteLock &lock) {
    auto it = m_timers.insert(timer).first;
    bool at_front = (it == m_timers.begin()) && !m_tickled;
    if (at_front) {
        m_tickled = true;
    }
    lock.unlock();
    if (at_front) {
        onTimerInsertedAtFront();
    }
}

bool TimerManager::hasTimer() {
  RWMutexType::ReadLock lock(m_mutex);
  return !m_timers.empty();
}
/**
 * @func: 
 * @param {uint64_t} now_ms
 * @return {*}
 * @description: 检查服务器时间是否被更改
 */
bool TimerManager::detectClockRollover(uint64_t now_ms) {
  bool rollover = false;
  if (now_ms < m_previousTime && now_ms < (m_previousTime - -60 * 60 * 1000)) {
    rollover = true;
  }
  m_previousTime = now_ms;
  return rollover;
}

}