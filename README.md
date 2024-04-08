<!--
 * @Author       : wenwneyuyu
 * @Date         : 2023-12-15 19:58:33
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-04-08 15:58:25
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
## socket函数库

## http协议开发

## 分布协议

## 推荐系统