<!--
 * @Author       : wenwneyuyu
 * @Date         : 2023-12-15 19:58:33
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-12 20:55:40
 * @FilePath     : /README.md
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2023-12-15 19:58:33
-->
# sylar

## 开发环境
ubuntu 20.04 

gcc 9.4.0

cmake 3.16.3

## 项目路径
bin   ---二进制

build ---中间文件路径

cmake ---cmake文件夹

CMakeLists.txt  --cmake定义文件

lib   -- 库的输出路径

Makefile

sylar -- 源代码路径

tests -- 测试路径


## 日志系统

    Logger（定义日志类别）
    |
    |------------Formatter（日志格式）
    |
    Appender（日志输出地方）
Logger类具有多个LogAppender进行内容输出
```cpp
class Logger : public std::enable_shared_from_this<Logger>{
  friend class LoggerManager;
  public:
  typedef std::shared_ptr<Logger> ptr;
  Logger(const std::string& name = "root");

  void log(LogLevel::Level level, LogEvent::ptr event);
  void debug(LogEvent::ptr event);
  void info(LogEvent::ptr event);
  void warn(LogEvent::ptr event);
  void error(LogEvent::ptr event);
  void fatal(LogEvent::ptr event);

  void addAppender(LogAppender::ptr appender);
  void delAppender(LogAppender::ptr appender);
  void clearAppenders();
  
  LogLevel::Level getLevel() const {return m_level;}
  void setLevel(LogLevel::Level val) {m_level = val;}

  const std::string getName() const {return m_name;}
  void setName(const std::string &val) { m_name = val; }

  const LogFormatter::ptr getFormatter() const { return m_formatter; }
  void setFormatter(LogFormatter::ptr val);
  void setFormatter(const std::string &val);

  std::string toYamlString();
private:
  std::string m_name;
  LogLevel::Level m_level;
  std::list<LogAppender::ptr> m_appenders;
  LogFormatter::ptr m_formatter;
  Logger::ptr m_root;
};
```
LogAppender类是个父类，子类有FileLogAppender和StdoutLogAppender
```cpp
class LogAppender {
  friend class Logger;
public:
  typedef std::shared_ptr<LogAppender> ptr;
  virtual ~LogAppender(){};
  virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   LogEvent::ptr event) = 0;
  virtual std::string toYamlString() = 0;

  void setFormatter(LogFormatter::ptr format);
  LogFormatter::ptr getFormatter() { return m_formatter; }
  LogLevel::Level getLevel() const { return m_level; }
  void setLevel(LogLevel::Level level) { m_level = level; }
  
protected:
  LogLevel::Level m_level = LogLevel::DEBUG;
  LogFormatter::ptr m_formatter;
  bool has_formatter = false;
};
```
Formatter类用来解析打印格式，将其保存到FromatItem中
```cpp
class LogFormatter{
public:
  typedef std::shared_ptr<LogFormatter> ptr;     
  LogFormatter(const std::string& pattern);

  std::string format(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event);
  void init();
  bool isError() { return m_error; }
  
  const std::string getPattern() const {return m_pattern;}
  class FormatItem{
  public:
    typedef std::shared_ptr<FormatItem> ptr;
    FormatItem(const std::string& fmt = ""){};
    virtual ~FormatItem(){};
    //virtual std::string format(LogEvent::ptr event) = 0;
    virtual void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) = 0;
  };
private:
  std::string m_pattern;
  std::vector<FormatItem::ptr> m_items;
  bool m_error = false;
};
```

LogEvent类为日志事件，为真正要打印的数据
```cpp
class LogEvent{
public:
  typedef std::shared_ptr<LogEvent> ptr;
  LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
           const char* file, uint32_t line, uint32_t elapse,
           uint32_t threadId, uint32_t fiberId, uint64_t time);

  const char* getFile() const {return m_file;}
  uint32_t getLine() const {return m_line;}
  uint32_t getElapse() const {return m_elapse;}
  uint32_t getThreadId() const {return m_threadId;}
  uint32_t getFiberId() const {return m_fiberId;}
  uint32_t getTime() const {return m_time;}
  std::string getContent() { return m_ss.str(); }
  
  std::stringstream &getSS() { return m_ss; }
  std::shared_ptr<Logger> getLogger() const { return m_logger; }
  LogLevel::Level getLevel() const { return m_level; }

  void format(const char *fmt, ...);
  void format(const char *fmt, va_list al);
private:
  const char* m_file = nullptr;   //文件名
  uint32_t m_line = 0;            //行号
  uint32_t m_elapse = 0;          //程序启动到现在的开始时间
  uint32_t m_threadId = 0;        //线程id
  uint32_t m_fiberId = 0;         //协程id
  uint64_t m_time = 0;            //时间戳
  std::stringstream m_ss;         // 内容

  std::shared_ptr<Logger> m_logger;
  LogLevel::Level m_level;
};
```
LogEventWrap类为真正的打印日志类，通过其析构函数进行打印
```cpp
class LogEventWrap {
public:
  LogEventWrap(const LogEvent::ptr& event);
  ~LogEventWrap();
  std::stringstream &getSS();
  const LogEvent::ptr getEvent() const {return m_event;}
private:
  LogEvent::ptr m_event;
};
```

日志Logger初始化逻辑： 

1）调用logger的构造函数

2）调用formatter构造函数，并解析pattern，填充m_items数组

3）为logger添加appender

日志打印逻辑：

1）构造logEvent类

2）将输出信息输入到logEvent的stringstream中

3）logger进行format该event   -->  logger中的appender进行format  --> appender中的logFormatter进行format --> format中的items进行最后的format

4）调用LogEventWrap类虚构函数进行打印

LoggerManager类用来管理全局的日志，并创建一个单例模式
```cpp
class LoggerManager {
public:
  LoggerManager();
  void init();

  Logger::ptr getLogger(const std::string &name);
  Logger::ptr getRoot() const { return m_root; }

  std::string toYamlString();
private:
  std::map<std::string, Logger::ptr> m_loggers;
  Logger::ptr m_root;
};

typedef sylar::Singleton<LoggerManager> LoggerMagr;
```

## 配置系统
Config --> yaml

本系统中的ConfigVar格式为 < name > < value > < description >

例如 "system.port" | 8080  | "system.port"

基础的类为ConfigVarBase，实现name和description
```cpp
class ConfigVarBase {
public:
  typedef std::shared_ptr<ConfigVarBase> ptr;
  ConfigVarBase(const std::string &name, const std::string &description)
  virtual ~ConfigVarBase() {}

  const std::string &getName() const {}
  const std::string &getDescription() const {}
  
  virtual std::string toString() = 0;
  virtual bool fromString(const std::string &val) = 0;
  virtual std::string getTypeName () const = 0;
protected:
  std::string m_name;
  std::string m_description;
};
```

value类型不一致，需要继承ConfigVarBase实现模板类ConfigVar；

同时要获得value的值并输出，需要实现相应的函数功能toString()、fromString()

注意到T类型不单单为普通类型，可能还有stl或结构体类型，需要对模板进行相应的操作(对一些转换进行偏特化)

当全局配置进行修改时，可以通过回调函数进行修改相应的变量
```cpp
template <class T, class FromStr , class ToStr>
class ConfigVar : public ConfigVarBase {
public:
  typedef std::shared_ptr<ConfigVar<T>> ptr;
  typedef std::function<void(const T& old_value, const T& new_value)> on_change_cb;
  
  ConfigVar(const std::string &name, const T &default_value,
            const std::string &description)

  std::string toString() override {
    //将m_val转化为string类型输出
  }

  bool fromString(const std::string &val) override {
    //将string类型转化为T类型
  }

  const T getValue() const {}
  void setValue(const T &val) {
    //使用cb来打印set信息
  }
  
  std::string getTypeName() const override {}

  void addListener(std::uint64_t key, on_change_cb cb) {}
  void delListener(std::uint64_t key) { }
  on_change_cb getListener(std::uint64_t key) {}
  
private:
  T m_val;
  std::map<std::uint64_t, on_change_cb> m_cbs;
};
```
模板偏特化的一种类型：
```cpp
template <class T> class LexicalCast<std::string, std::unordered_map<std::string, T>> {
public:
  std::unordered_map<std::string, T> operator()(const std::string &v) {
    //将string类型通过YAML::Node转化为unordered_map类型
  }
};

template <class T> class LexicalCast<std::unordered_map<std::string, T>,std::string> {
public:
  std::string operator()(const std::unordered_map<std::string, T> &v) {
    //unordered_map读入到YAML::Node中并输出到stringstream中
  }
};
```
最后一个Config类保存了全局配置信息
```cpp
class Config {
public:
  typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;

  //获取配置  
  template <class T>
  static typename ConfigVar<T>::ptr
  Lookup(const std::string &name, const T &default_value,
         const std::string &description = "") {
    //获得相应的配置内容，如果没有的话则创建相应内容
  }

  //从yaml文件中读取信息
  static void LoadFromYaml(const YAML::Node &root);
  static ConfigVarBase::ptr LookupBase(const std::string &name);
  
private:
  // static成员变量
  static ConfigVarMap m_datas;
};
```
一些yaml-cpp的操作：
```cpp
YAML::Node node = YAML::LoadFile("");

node.IsMap()
for(auto it = node.begin(); it != node.end(); ++it)
    it->first, it->second

node.IsSequence()
for(size_t i = 0; i < node.size(); ++i)

node.IsScalar()
```

## 线程库、信号量和互斥量

Thread类较为简单，要搞清楚Thread注册函数之后的运行逻辑

同时还为每个线程设置两个thread_local类型变量，用来保存this
```cpp
class Thread {
public:
  typedef std::shared_ptr<Thread> ptr;
  Thread(const Thread &) = delete;
  Thread(const Thread &&) = delete;
  Thread &operator=(const Thread &) = delete;
  Thread &operator=(const Thread &&) = delete;

  Thread(std::function<void()> cb, const std::string name);
  ~Thread();

  void join();
  pid_t getId() const { return m_pid; }
  const std::string &getName() const { return m_name; }
  
  static Thread *GetThis();
  static const std::string &GetName() ;
  static void SetName(const std::string &);
  static void *Run(void * args);
private:
  pid_t m_pid = 0;
  pthread_t m_thread = -1;
  std::string m_name;
  std::function<void()> m_cb;

  Semaphore m_sem;
};
```
信号量和互斥量使用pthread库中的信号量和互斥量

注意使用RAII技巧给互斥量初始化

```CPP
class Semaphore {
public:
  Semaphore(const Semaphore &) = delete;
  Semaphore(const Semaphore &&) = delete;
  Semaphore &operator=(const Semaphore &) = delete;
  Semaphore &operator=(const Semaphore &&) = delete;

  Semaphore(std::uint32_t count = 0);
  ~Semaphore();

  void wait();
  void notify();
private:
  sem_t m_sem;
};
/**
 * @func:
 * @return {*}
 * @description: 使用模板类的方式来统一管理mutex，不需要mutex显示加锁和解锁。
 * 模板类生成时自动加锁，生命周期结束时自动解锁。RAII的编程思想
 */
template <class T> struct ScopedLockImpl {
public:
  ScopedLockImpl(T &mutex) : m_mutx(mutex) {
    m_mutx.lock();
    locked = true;
  }

  ~ScopedLockImpl() {
    unlock();
  }

  void lock() {
    if (!locked) {
      m_mutx.lock();
      locked = true;
    }
  }

  void unlock() {
    if (locked) {
      m_mutx.unlock();
      locked = false;
    }
  }
  
private:
  T &m_mutx;
  bool locked;
};

template <class T> struct ReadScopedLockImpl {
public:
  ReadScopedLockImpl(T &mutex) : m_mutex(mutex) {
    m_mutex.rdlock();
    locked = true;
  }

  ~ReadScopedLockImpl() { unlock(); }

  void lock() {
    if (!locked) {
      m_mutex.rdlock();
      locked = true;
    }
  }

  void unlock() {
    if (locked) {
      m_mutex.unlock();
      locked = false;
    }
  }

private:
  T &m_mutex;
  bool locked;
};

template <class T> struct WriteScopedLockImpl {
public:
  WriteScopedLockImpl(T &mutex) : m_mutex(mutex) {
    m_mutex.wrlock();
    locked = true;
  }

  ~WriteScopedLockImpl() { unlock(); }
  
  void lock() {
    if (!locked) {
      m_mutex.wrlock();
      locked = true;
    }
  }

  void unlock() {
    if (locked) {
      m_mutex.unlock();
      locked = false;
    }
  }
  
private:
  T &m_mutex;
  bool locked;
};

class Mutex {
public:
  Mutex(const Mutex &) = delete;
  Mutex(const Mutex &&) = delete;
  Mutex &operator=(const Mutex &) = delete;
  Mutex &operator=(const Mutex &&) = delete;

  typedef ScopedLockImpl<Mutex> Lock;

  Mutex();
  ~Mutex();
  void lock();
  void unlock();
private:
  pthread_mutex_t m_mutex;
};

// 读多写少使用读写锁
class RWMutex {
public:
  RWMutex(const RWMutex &) = delete;
  RWMutex(const RWMutex &&) = delete;
  RWMutex &operator=(const RWMutex &) = delete;
  RWMutex &operator=(const RWMutex &&) = delete;

  typedef ReadScopedLockImpl<RWMutex> ReadLock;
  typedef WriteScopedLockImpl<RWMutex> WriteLock;

  RWMutex();
  ~RWMutex();

  void rdlock();
  void wrlock();
  void unlock();
private:
  pthread_rwlock_t m_lock;
};

// 使用自旋锁防止互斥锁陷入内核中，性能损耗
class Spinlock {
public:
  Spinlock(const Spinlock &) = delete;
  Spinlock(const Spinlock &&) = delete;
  Spinlock &operator=(const Spinlock &) = delete;
  Spinlock &operator=(const Spinlock &&) = delete;

  typedef ScopedLockImpl<Spinlock> Lock;

  Spinlock();
  ~Spinlock();
  void lock();
  void unlock();
private:
  pthread_spinlock_t m_mutex;
};

// 原子锁
class CASLock {
public:
  CASLock(const CASLock &) = delete;
  CASLock(const CASLock &&) = delete;
  CASLock &operator=(const CASLock &) = delete;
  CASLock &operator=(const CASLock &&) = delete;

  typedef ScopedLockImpl<CASLock> Lock;

  CASLock();
  ~CASLock();
  void lock();
  void unlock();
private:
  volatile std::atomic_flag m_mutex;
};
```


## 协程库封装
使用无栈协程进行封装

确保在一个线程内有一个主协程和多个子协程；主协程仅仅进行调度功能，如子协程运行结束后返回到主协程中

    thread--> main fiber <---> sub fiber
                  |
                  |
                  |
              sub fiber  
在主协程中进行子协程的创建和调度
```cpp
class Fiber : public std::enable_shared_from_this<Fiber> {
  friend class Scheduler;
public:
  typedef std::shared_ptr<Fiber> ptr;

  enum State {
    INIT,
    HOLD,
    EXEC,
    TERM,
    READY,
    EXCEPT
  };

  Fiber(std::function<void()> cb, std::uint32_t size = 0);
  ~Fiber();
  void reset(std::function<void()> cb);
  void swapOut();
  static void wait();
  void resume();
  static void Func();
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

```

在协程的基础上，进行调度器的封装

一个调度器包括多个线程池，多个任务队列

线程池中的线程用来循环遍历任务队列，查看是否有任务可以进行

若该任务队列没有任何任务，则调用提供的初始化任务

```cpp
class Scheduler {
public:
  typedef std::shared_ptr<Scheduler> ptr;
  typedef Mutex MutexType;

  Scheduler(std::size_t threads = 1, bool use_caller = true, const std::string& name = "");
  virtual ~Scheduler();

  const std::string &getName() const { return m_name; }
  
  void start();
  void stop();

  static Scheduler *GetThis();
  static Fiber* GetMainFiber();

  template <class FiberOrCb> void schedule(FiberOrCb fc, int thr = -1) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      need_tickle = scheduNoLock(fc, thr);
    }

    if (need_tickle) {
      tickle();
    }
  }

protected:
  /**
   * @func: 
   * @return {*}
   * @description: 通知协程调度器有任务了
   */  
  virtual void tickle();

  /**
   * @func: 
   * @return {*}
   * @description: 协程调度开始函数
   */  
  void run();

  /**
   * @func: 
   * @return {*}
   * @description: 协程调度器是否可以停止了
   */  
  virtual bool stopping();

  /**
   * @func: 
   * @return {*}
   * @description: 协程调度器无任务时执行idle协程
   */
  virtual void idle();

  void setThis();

private:
  template <class FiberOrCb> bool scheduNoLock(FiberOrCb fc, int thr) {
    bool need_tickle = m_fibers.empty();
    FiberAndThread ft(fc, thr);
    if (ft.m_fiber || ft.m_cb) {
      m_fibers.push_back(ft);
    }
    return need_tickle;
  }

private:
  struct FiberAndThread{
    // 协程
    Fiber::ptr m_fiber;
    // 任务
    std::function<void()> m_cb;
    // 指定运行线程id
    int m_threadId;

    FiberAndThread() { m_threadId = -1; }

    FiberAndThread(Fiber::ptr f, int thr) : m_fiber(f), m_threadId(thr) {}

    FiberAndThread(Fiber::ptr *f, int thr) : m_threadId(thr) {
      m_fiber = std::move(*f);
    }

    FiberAndThread(std::function<void()> cb, int thr)
        : m_cb(cb), m_threadId(thr) {}
    FiberAndThread(std::function<void()> *cb, int thr) : m_threadId(thr) {
      m_cb = std::move(*cb);
    }

    void reset() {
      m_fiber = nullptr;
      m_cb = nullptr;
      m_threadId = -1;
    }

  };

private:
  // 调度器的互斥锁
  MutexType m_mutex;
  // 线程池
  std::vector<Thread::ptr> m_threads;
  // 任务消息队列
  std::list<FiberAndThread> m_fibers;
  // 主协程，设置use_caller为true时创造
  Fiber::ptr m_rootFiber;
  // 调度器名称
  std::string m_name;

protected:
  std::vector<int> m_threadIds;
  // 线程数量
  std::size_t m_threadCount = 0;
  std::atomic<std::size_t> m_activeThreadCount = {0};
  std::atomic<std::size_t> m_idleThreadCount = {0};
  // 是否停止
  bool m_stopping = true;
  bool auto_stopping = false;
  // 主线程id
  int m_rootThread = 0;
};

```

在调度器的基础上，使用epoll进行io异步处理

```cpp
class IOManager : public Scheduler, public TimerManager{
public:
  typedef std::shared_ptr<IOManager> ptr;
  typedef RWMutex RWMutexType;

  enum Event {
    NONE = 0x0,
    READ = 0x1,
    WRITE = 0x4
  };

private:
  // epoll主要对fd进行操作
  // 如果该fd可读，则触发读操作：将EventContext加入任务队列中
  // 如果该fd可写，则触发写操作
  // 在fd中封装回调函数，在触发事件时，将回调函数放入全局任务队列中
  struct FdContext {
    typedef Mutex MutexType;
    
    struct EventContext {
      Scheduler *scheduler = nullptr;
      Fiber::ptr fiber;
      std::function<void()> cb;
    };

    EventContext &getContext(Event event);
    void resetContext(EventContext &ctx);
    void triggerEvent(Event event);

    // 读任务
    EventContext read;
    // 写任务
    EventContext write;
    int fd;
    // 事件属性
    Event events = NONE;
    MutexType mutex;
  };
  
public:
  
  IOManager(std::size_t threads = 1, bool use_caller = true,
            const std::string &name = "");
  ~IOManager();

  int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
  bool delEvent(int fd, Event event);
  bool cancelEvent(int fd, Event event);
  bool cancelAll(int fd);

  static IOManager* GetThis();

protected:
  void tickle() override;
  bool stopping() override;
  bool stopping(uint64_t& timeout);
  void idle() override;
  void contextResize(std::size_t size);
  void onTimerInsertedAtFront() override;

private:
  // epoll套接字
  int m_epfd = 0;
  // pipe套接字，方便唤醒epoll_wait
  int m_ticklefds[2];
  RWMutexType m_mutex;

  std::atomic<std::size_t> m_pendingEventCount = {0};
  std::vector<FdContext*> m_fdContexts;
};

```

增加定时器操作
```cpp
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

```
## Hook模块
hook系统底层和socket相关的API，socket IO相关的API，以及sleep系列的API。hook的开启控制是线程粒度的，可以自由选择。通过hook模块，可以使一些不具异步功能的API，展现出异步的性能，如MySQL。

hook的目的是在不重新编写代码的情况下，把老代码中的socket IO相关的API都转成异步，以提高性能。hook和IO协程调度是密切相关的，如果不使用IO协程调度器，那hook没有任何意义，考虑IOManager要在一个线程上按顺序调度以下协程：

协程1：sleep(2) 睡眠两秒后返回。

协程2：在scoket fd1 上send 100k数据。

协程3：在socket fd2 上recv直到数据接收成功。

在未hook的情况下，IOManager要调度上面的协程，流程是下面这样的：


1. 调度协程1，协程阻塞在sleep上，等2秒后返回，这两秒内调度线程是被协程1占用的，其他协程无法在当前线程上调度。

2. 调度协徎2，协程阻塞send 100k数据上，这个操作一般问题不大，因为send数据无论如何都要占用时间，但如果fd迟迟不可写，那send会阻塞直到套接字可写，同样，在阻塞期间，其他协程也无法在当前线程上调度。

3. 调度协程3，协程阻塞在recv上，这个操作要直到recv超时或是有数据时才返回，期间调度器也无法调度其他协程。

上面的调度流程最终总结起来就是，协程只能按顺序调度，一旦有一个协程阻塞住了，那整个调度线程也就阻塞住了，其他的协程都无法在当前线程上执行。像这种一条路走到黑的方式其实并不是完全不可避免，以sleep为例，调度器完全可以在检测到协程sleep后，将协程yield以让出执行权，同时设置一个定时器，2秒后再将协程重新resume。这样，调度器就可以在这2秒期间调度其他的任务，同时还可以顺利的实现sleep 2秒后再继续执行协程的效果，send/recv与此类似。在完全实现hook后，IOManager的执行流程将变成下面的方式：


1. 调度协程1，检测到协程sleep，那么先添加一个2秒的定时器，定时器回调函数是在调度器上继续调度本协程，接着协程yield，等定时器超时。

2. 因为上一步协程1已经yield了，所以协徎2并不需要等2秒后才可以执行，而是立刻可以执行。同样，调度器检测到协程send，由于不知道fd是不是马上可写，所以先在IOManager上给fd注册一个写事件，回调函数是让当前协程resume并执行实际的send操作，然后当前协程yield，等可写事件发生。

3. 上一步协徎2也yield了，可以马上调度协程3。协程3与协程2类似，也是给fd注册一个读事件，回调函数是让当前协程resume并继续recv，然后本协程yield，等事件发生。

4. 等2秒超时后，执行定时器回调函数，将协程1 resume以便继续执行。

5. 等协程2的fd可写，一旦可写，调用写事件回调函数将协程2 resume以便继续执行send。

6. 等协程3的fd可读，一旦可读，调用回调函数将协程3 resume以便继续执行recv。

上面的4、5、6步都是异步的，调度线程并不会阻塞，IOManager仍然可以调度其他的任务，只在相关的事件发生后，再继续执行对应的任务即可。并且，由于hook的函数签名与原函数一样，所以对调用方也很方便，只需要以同步的方式编写代码，实现的效果却是异步执行的，效率很高。


总而言之，在IO协程调度中对相关的系统调用进行hook，可以让调度线程尽可能得把时间片都花在有意义的操作上，而不是浪费在阻塞等待中。


hook的重点是在替换API的底层实现的同时完全模拟其原本的行为，因为调用方是不知道hook的细节的，在调用被hook的API时，如果其行为与原本的行为不一致，就会给调用方造成困惑。比如，所有的socket fd在进行IO调度时都会被设置成NONBLOCK模式，如果用户未显式地对fd设置NONBLOCK，那就要处理好fcntl，不要对用户暴露fd已经是NONBLOCK的事实，这点也说明，除了IO相关的函数要进行hook外，对fcntl, setsockopt之类的功能函数也要进行hook，才能保证API的一致性。

首先是socket fd上下文和FdManager的实现，这两个类用于记录fd上下文和保存全部的fd上下文，它们的关键实现如下

```cpp

/**
 * @brief 文件句柄上下文类
 * @details 管理文件句柄类型(是否socket)
 *          是否阻塞,是否关闭,读/写超时时间
 */
class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
    typedef std::shared_ptr<FdCtx> ptr;
    /**
     * @brief 通过文件句柄构造FdCtx
     */
    FdCtx(int fd);
    /**
     * @brief 析构函数
     */
    ~FdCtx();
    ....
private:
    /// 是否初始化
    bool m_isInit: 1;
    /// 是否socket
    bool m_isSocket: 1;
    /// 是否hook非阻塞
    bool m_sysNonblock: 1;
    /// 是否用户主动设置非阻塞
    bool m_userNonblock: 1;
    /// 是否关闭
    bool m_isClosed: 1;
    /// 文件句柄
    int m_fd;
    /// 读超时时间毫秒
    uint64_t m_recvTimeout;
    /// 写超时时间毫秒
    uint64_t m_sendTimeout;
};
 
/**
 * @brief 文件句柄管理类
 */
class FdManager {
public:
    typedef RWMutex RWMutexType;
    /**
     * @brief 无参构造函数
     */
    FdManager();
 
    /**
     * @brief 获取/创建文件句柄类FdCtx
     * @param[in] fd 文件句柄
     * @param[in] auto_create 是否自动创建
     * @return 返回对应文件句柄类FdCtx::ptr
     */
    FdCtx::ptr get(int fd, bool auto_create = false);
 
    /**
     * @brief 删除文件句柄类
     * @param[in] fd 文件句柄
     */
    void del(int fd);
private:
    /// 读写锁
    RWMutexType m_mutex;
    /// 文件句柄集合
    std::vector<FdCtx::ptr> m_datas;
};
 
/// 文件句柄单例
typedef Singleton<FdManager> FdMgr;
```
FdCtx类在用户态记录了fd的读写超时和非阻塞信息，其中非阻塞包括用户显式设置的非阻塞和hook内部设置的非阻塞，区分这两种非阻塞可以有效应对用户对fd设置/获取NONBLOCK模式的情形。

另外注意一点，FdManager类对FdCtx的寻址采用了和IOManager中对FdContext的寻址一样的寻址方式，直接用fd作为数组下标进行寻址。



接下来是hook的整体实现。首先定义线程局部变量t_hook_enable，用于表示当前线程是否启用hook，使用线程局部变量表示hook模块是线程粒度的，各个线程可单独启用或关闭hook。然后是获取各个被hook的接口的原始地址， 这里要借助dlsym来获取。sylar使用了一套宏来简化编码，这套宏的实现如下：
```cpp
#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)
 
extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX
}
 
void hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}
```

最关键的do_io模板,基础的IO操作都使用该hook模板
```cpp
template <typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char *hook_fun_name,
                     uint32_t event, int timeout_so, Args &&...args) {
  // 如果没有进行hook，则直接返回
  if (!sylar::t_hook_enable) {
    return fun(fd, std::forward<Args&&>(args)...);
  }

  // 获取相应的fd
  sylar::FdCtx::ptr ctx = sylar::FdMgr::getInstance()->get(fd);
  if (!ctx) {
    return fun(fd, std::forward<Args&&>(args)...);
  }

  if (ctx->isClose()) {
    errno = EBADF;
    return -1;
  }

  // 判断是否为套接字或者被显示设置了非阻塞模式
  if (!ctx->isSocket() || ctx->getUserNonblock()) {
    return fun(fd, std::forward<Args&&>(args)...);
  }

  // 获得超时时间
  uint64_t to = ctx->getTimeout(timeout_so);
  std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
  // 执行，因为是非阻塞模式，会立刻返回
  ssize_t n = fun(fd, std::forward<Args &&>(args)...);
  // 如果是被中断的
  while (n == -1 && errno == EINTR) {
    n = fun(fd, std::forward<Args&&>(args)...);
  }

  // 如果不可行则进行调度 
  if (n == -1 && errno == EAGAIN) {
    sylar::IOManager *iom = sylar::IOManager::GetThis();
    sylar::Timer::ptr timer;
    std::weak_ptr<timer_info> winfo(tinfo);

    // 如果设置了超时时间
    if (to != (uint64_t)-1) {
      // 设置条件定时器
      timer = iom->addConditionTimer(to, [winfo, fd, iom, event] {
        auto t = winfo.lock();
        // 如果已经执行过了，直接返回
        if (!t || t->cancelled) {
          return;
        }
        // 取消事件
        t->cancelled = ETIMEDOUT;
        iom->cancelEvent(fd, (sylar::IOManager::Event)event);
      }, winfo);
    }

    // 加入事件
    int rt = iom->addEvent(fd, (sylar::IOManager::Event)event);
    if (rt) {
      SYLAR_LOG_ERROR(sylar::g_logger)
          << hook_fun_name << " addEvent(" << fd << ", " << event << ")";
      if(timer) {
          timer->cancel();
      }
      return -1;
    } else {
      SYLAR_LOG_INFO(sylar::g_logger) << hook_fun_name << " trigger";
      // 重中之重 yield出去
      sylar::Fiber::wait();
      // 返回
      if (timer) {
        timer->cancel();
      }

      if (tinfo->cancelled) {
        errno = tinfo->cancelled;
        return -1;
      }

      goto retry;
    }
  }

  return n;
}

```

## socket函数库

### Address模块
提供网络地址相关的类，支持与网络地址相关的操作，一共有以下几个类：

Address：所有网络地址的基类，抽象类，对应sockaddr类型，但只包含抽象方法，不包含具体的成员。除此外，Address作为地址类还提供了网络地址查询及网卡地址查询功能。

IPAddress：IP地址的基类，抽象类，在Address基础上，增加了IP地址相关的端口以及子网掩码、广播地址、网段地址操作，同样是只包含抽象方法，不包含具体的成员。

IPv4Address：IPv4地址类，实体类，表示一个IPv4地址，对应sockaddr_in类型，包含一个sockaddr_in成员，可以操作该成员的网络地址和端口，以及获取子码掩码等操作。

IPv6Address：IPv6地址类，实体类，与IPv4Address类似，表示一个IPv6地址，对应sockaddr_in6类型，包含一个sockaddr_in6成员。

UnixAddreess：Unix域套接字类，对应sockaddr_un类型，同上。

UnknownAddress：表示一个未知类型的套接字地址，实体类，对应sockaddr类型，这个类型与Address类型的区别是它包含一个sockaddr成员，并且是一个实体类。

### Socket模块
套接字类，表示一个套接字对象。

对于套接字类，需要关注以下属性：

1. 文件描述符
2. 地址类型（AF_INET, AF_INET6等）
3. 套接字类型（SOCK_STREAM, SOCK_DGRAM等）
4. 协议类型（这项其实可以忽略）
5. 是否连接（针对TCP套接字，如果是UDP套接字，则默认已连接）
6. 本地地址和对端的地址

套接字类应提供以下方法：

1. 创建各种类型的套接字对象的方法（TCP套接字，UDP套接字，Unix域套接字）
2. 设置套接字选项，比如超时参数
3. bind/connect/listen方法，实现绑定地址、发起连接、发起监听功能 
4. accept方法，返回连入的套接字对象
5. 发送、接收数据的方法
6. 获取本地地址、远端地址的方法
7. 获取套接字类型、地址类型、协议类型的方法
8. 取消套接字读、写的方法

## ByteArray
字节数组容器，提供基础类型的序列化与反序列化功能。

ByteArray的底层存储是固定大小的块，以链表形式组织。每次写入数据时，将数据写入到链表最后一个块中，如果最后一个块不足以容纳数据，则分配一个新的块并添加到链表结尾，再写入数据。ByteArray会记录当前的操作位置，每次写入数据时，该操作位置按写入大小往后偏移，如果要读取数据，则必须调用setPosition重新设置当前的操作位置。

ByteArray支持基础类型的序列化与反序列化功能，并且支持将序列化的结果写入文件，以及从文件中读取内容进行反序列化。ByteArray支持以下类型的序列化与反序列化：

1. 固定长度的有符号/无符号8位、16位、32位、64位整数
2. 不固定长度的有符号/无符号32位、64位整数
3. float、double类型
4. 字符串，包含字符串长度，长度范围支持16位、32位、64位。
5. 字符串，不包含长度。

以上所有的类型都支持读写。

ByteArray还支持设置序列化时的大小端顺序。
## http协议开发

