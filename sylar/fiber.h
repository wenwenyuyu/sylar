/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-18 14:09:10
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-04-08 15:22:32
 * @FilePath     : /sylar/fiber.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-18 14:09:10
 */

#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__

#include <cstdint>
#include <functional>
#include <memory>
#include <ucontext.h>

namespace sylar {

// 将协程作为调度的主体
// 每个线程拥有一个主协程负责调度子协程，主协程不进行任何工作，只是创建或销毁子协程，不需要任何栈空间，不能够主动创建
// 子协程可以由线程主动创建，创建后当前线程获得运行的子协程指针，当子协程运行结束后将子协程返回到主协程处

class Fiber : public std::enable_shared_from_this<Fiber> {
  friend class Scheduler;
public:
  typedef std::shared_ptr<Fiber> ptr;

  //协程状态
  enum State {
    INIT,
    HOLD,
    EXEC,
    TERM,
    READY,
    EXCEPT
  };

  // 创建子协程
  Fiber(std::function<void()> cb, std::uint32_t size = 0,
        bool use_caller = false);
  // 析构函数
  ~Fiber();
  // 将子协程运行的函数重置
  void reset(std::function<void()> cb);
  // 从子协程转换到主协程运行
  void swapOut();
  // 全局函数，将当前协程转换到主协程
  static void wait();
  // 从主协程恢复到子协程退出的地方继续运行
  void resume();
  void call();
  void back();
  // 所有协程的运行函数
  static void Func();
  static void MainFunc();
  static void CallerMainFunc();

  // 获得当前运行的协程，若当前没有设置主协程则初始化主协程
  static Fiber::ptr GetThis();
  static void SetThis(Fiber *f);

  uint64_t getId() const { return m_id; }
  State getState() const { return m_state; }
  static uint64_t TotalFibers();
  static uint64_t GetId();

  // static void MainFunc();
  // static void CallerMainFunc();
  // void call();
  // void back();

private:
  Fiber();
private:
  std::uint64_t m_id = 0;
  ucontext_t m_ctx;
  void *m_stack = nullptr;
  std::uint32_t m_stacksize = 0;
  std::function<void()> m_cb;
  State m_state = INIT;
};

}

#endif