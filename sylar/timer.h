/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-04-06 14:20:38
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-04-08 15:51:44
 * @FilePath     : /sylar/timer.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-04-06 14:20:38
 */

#ifndef __SYLAR_TIMER_H__
#define __SYLAR_TIMER_H__

#include "sylar/mutex.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <vector>
namespace sylar {

// 定时器事件，规定在m_ms秒后发生m_cb事件
class TimerManager;
class Timer : public std::enable_shared_from_this<Timer> {
  friend class TimerManager;

public:
  typedef std::shared_ptr<Timer> ptr;

  // 取消该定时任务
  bool cancel();
  // 刷新该定时任务
  bool refresh();
  // 重置该定时任务
  bool reset(uint64_t ms, bool from_now);

private:
  Timer(uint64_t ms, std::function<void()> cb, bool recurring,
        TimerManager *manager);
  Timer(uint64_t next);

private:
  // 是否为循环事件
  bool m_recurring = false;
  // 定时了多少微妙
  uint64_t m_ms = 0;
  // 多少小时后运行（将定时器定义的微秒转化到真实时间）
  uint64_t m_next = 0;
  std::function<void()> m_cb;
  TimerManager *m_manager = nullptr;

private:
  // 确保可以比较
  struct Comparator {
    bool operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const;
  };

};

class TimerManager {
  friend class Timer;

public:
  typedef RWMutex RWMutexType;

  TimerManager();
  virtual ~TimerManager();

  // 增加定时器
  Timer::ptr addTimer(uint64_t ms, std::function<void()> cb,
                      bool recurring = false);
  // 增加条件定时器
  Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb,
                               std::weak_ptr<void> weak_cond,
                               bool recurring = false);
  // 下一个任务还有多少时间触发
  uint64_t getNextTimer();
  // 获得所有该触发的定时器任务
  void ListExpiredCb(std::vector<std::function<void()>> &cbs);

protected:
  // 增加的定时器是第一个定时器，需要通知一下
  virtual void onTimerInsertedAtFront() = 0;
  void addTimer(Timer::ptr val, RWMutexType::WriteLock &lock);
  bool hasTimer();
private:
  bool detectClockRollover(uint64_t now_ms);
private:
  RWMutexType m_mutex;
  // set比较器
  std::set<Timer::ptr, Timer::Comparator> m_timers;
  bool m_tickled = false;
  uint64_t m_previousTime = 0;
};

}

#endif