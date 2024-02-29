/*
 * @Author       : wenwneyuyu
 * @Date         : 2023-12-15 21:34:57
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-02-29 20:10:53
 * @FilePath     : /sylar/log.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2023-12-15 21:34:57
 */
#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include <stdint.h>
#include <string>
#include <memory>
#include <list>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace sylar{
class Logger;
class LogFormatter;

//日志事件
class LogEvent{
public:
  typedef std::shared_ptr<LogEvent> ptr;
  LogEvent(const char* file, uint32_t line, uint32_t elapse,
           uint32_t threadId, uint32_t fiberId, uint64_t time);

  const char* getFile() const {return m_file;}
  uint32_t getLine() const {return m_line;}
  uint32_t getElapse() const {return m_elapse;}
  uint32_t getThreadId() const {return m_threadId;}
  uint32_t getFiberId() const {return m_fiberId;}
  uint32_t getTime() const {return m_time;}
  const std::string getContent() const {return m_ss.str();}
  const std::stringstream& getSS() const {return m_ss;}
private:
  //文件名
  const char* m_file = nullptr;
  //行号
  uint32_t m_line = 0;
  //程序启动到现在的开始时间
  uint32_t m_elapse = 0;
  //线程id
  uint32_t m_threadId = 0;
  //协程id
  uint32_t m_fiberId = 0;
  //时间戳
  uint64_t m_time = 0;
  //内容
  std::stringstream m_ss;
};

//日志等级
class LogLevel{
public:
  enum Level{
    UNKNOW = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
  };
  static const char *ToString(LogLevel::Level level);
};

//日志格式
class LogFormatter{
public:
  typedef std::shared_ptr<LogFormatter> ptr;
  LogFormatter(const std::string& pattern);

  std::string format(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event);
  void init();

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
};

//输出地
class LogAppender{
public:
  typedef std::shared_ptr<LogAppender> ptr;
  virtual ~LogAppender(){};
  virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
  void setFormatter(LogFormatter::ptr format) {m_formatter = format;}
  LogFormatter::ptr getFormatter(){return m_formatter;}
protected:
  LogLevel::Level m_level = LogLevel::DEBUG;
  LogFormatter::ptr m_formatter;
};

class StdoutLogAppender : public LogAppender{
public:
  typedef std::shared_ptr<StdoutLogAppender> ptr;
  void log(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override;
};

class FileAppender : public LogAppender{
public:
  typedef std::shared_ptr<FileAppender> ptr;
  FileAppender(const std::string &filename);
  void log(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override;
  bool reopen();
private:
  std::string m_filename;
  std::ofstream m_filestream;
};

//日志器
//包括输出者、输出等级和输出目的地
class Logger : public std::enable_shared_from_this<Logger>{
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

  LogLevel::Level getLevel() const {return m_level;}
  void setLevel(LogLevel::Level val) {m_level = val;}

  const std::string getName() const {return m_name;}
  void setName(const std::string &val) { m_name = val; }

  const LogFormatter::ptr getFormatter() const {return m_formatter;}
private:
  std::string m_name;
  LogLevel::Level m_level;
  std::list<LogAppender::ptr> m_appenders;
  LogFormatter::ptr m_formatter;
};



}

#endif // !DEBUG__SYLAR_LOG_H__

