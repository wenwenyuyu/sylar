#include "log.h"
#include <cctype>
#include <cstddef>
#include <memory>
#include <tuple>
#include <map>
#include <functional>
#include <algorithm>
#include <utility>

namespace sylar{
    /******************logEvent start********************/
    /**
     * @func: LogEvent
     * @return {*}
     * @description: 初始化
     */
    LogEvent::LogEvent(const char* file, uint32_t line, uint32_t elapse,
           uint32_t threadId, uint32_t fiberId, uint64_t time)
           :m_file(file)
           ,m_line(line)
           ,m_elapse(elapse)
           ,m_threadId(threadId)
           ,m_fiberId(fiberId)
           ,m_time(time){
    }

    /******************Logger start********************/
    Logger::Logger(const std::string &name)
        :m_name(name)
        ,m_level(LogLevel::DEBUG){
        m_formatter.reset(new LogFormatter("%d [%p] <%f:%l> %m %n"));
    }

    void Logger::log(LogLevel::Level level, LogEvent::ptr event){
        if(level >= m_level){
            auto self = shared_from_this();
            for(auto &i : m_appenders){
                i->log(self, level, event);
            }
        }
    }

    void Logger::debug(LogEvent::ptr event){
        log(LogLevel::DEBUG, event);
    }

    void Logger::info(LogEvent::ptr event){
        log(LogLevel::INFO, event);
    }

    void Logger::warn(LogEvent::ptr event){
        log(LogLevel::WARN, event);
    }

    void Logger::error(LogEvent::ptr event){
        log(LogLevel::ERROR, event);
    }

    void Logger::fatal(LogEvent::ptr event){
        log(LogLevel::FATAL, event);
    }

    void Logger::addAppender(LogAppender::ptr appender){
        if(!appender->getFormatter()){
            appender->setFormatter(m_formatter);
        }
        m_appenders.push_back(appender);
    }

    void Logger::delAppender(LogAppender::ptr appender){
        auto it = find(m_appenders.begin(), m_appenders.end(), appender);
        if(it != m_appenders.end()){
            it = m_appenders.erase(it);
        }
    }

    const char *LogLevel::ToString(LogLevel::Level level){
        switch(level){
    #define XX(name) \
        case LogLevel::name:\
            return #name;   \
            break;
            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);
    #undef XX
            default:
                return "NUKNOW";
        }
        return "UNKNOW";
    }

    /******************FileAppender start********************/
    FileAppender::FileAppender(const std::string &filename){
        m_filename = filename;
    }

    void FileAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event){
        if(m_level >= level){
            m_filestream << m_formatter->format(logger,level, event);
        }
    }

    bool FileAppender::reopen(){
        if(m_filestream){
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        return !!m_filestream; 
    }

    /******************StdoutLogAppender start********************/
    void StdoutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event){
        if(m_level >= level){
            std::cout << m_formatter->format( logger, level, event);
        }
    }

    /******************Format start********************/
    class MessageFormatItem : public LogFormatter::FormatItem{
    public:
        MessageFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem{
    public:
        LevelFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override {
            os << LogLevel::ToString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem{
    public:
        ElapseFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getElapse();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem{
    public:
        NameFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override {
            os << logger->getName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem{
    public:
        ThreadIdFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getThreadId();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem{
    public:
        FiberIdFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getFiberId();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem{
    public:
      DateTimeFormatItem(const std::string &format = "%Y:%m:%d %H:%m:%S")
          : m_format(format) {
            if(m_format.empty()) {
                m_format = "%Y:%m:%d %H:%m:%S";
            }
        }

        void format(std::ostream& os, std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override {
                      struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }
    private:
        std::string m_format;
    };

    class FileNameFormatItem : public LogFormatter::FormatItem{
    public:
        FileNameFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem{
    public:
        LineFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem{
    public:
        NewLineFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem{
    public:
        StringFormatItem(const std::string& str = "")
            :FormatItem(str), m_string(str) {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override {
            os << m_string;
        }
    private:
        std::string m_string;
    };
    /**
     * @func: LogFormatter
     * @param {string&} pattern
     * @return {*}
     * @description: 通过pattern的日志格式，解析分为不同的item
     */
    LogFormatter::LogFormatter(const std::string& pattern)
        :m_pattern(pattern){
        init();
    }
    /**
     * @func: format
     * @param {ptr} event
     * @return {string}
     * @description: 将事件通过不同的item解析成字符串
     */
    std::string LogFormatter::format(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event){
        std::stringstream ss;
        for(auto& i : m_items){
            i->format(ss, logger, level, event);
        }
        return ss.str();
    }
    /**
     * @func: init
     * @return {*}
     * @description: 根据输入的pattern（[%d{HH\:mm\:ss\:SSS}][%p] (%c\:%L) -
%m%n）解析成相应的item
     *  %d{%Y-%m-%d %H:%M:%S} [%p] %f %l %m %n
        (d) - (%Y-%m-%d %H:%M:%S) - (1)
        ( [) - () - (0)
        (p) - () - (1)
        (] ) - () - (0)
        (f) - () - (1)
        ( ) - () - (0)
        (l) - () - (1)
        ( ) - () - (0)
        (m) - () - (1)
        ( ) - () - (0)
        (n) - () - (1)
     */
    void LogFormatter::init(){
        //格式有以下几种 %xxx %xxx{} %%
        //init函数主要解析上述格式，将其放入一个<str, fmt, status>的tuple中

        std::vector<std::tuple<std::string, std::string, int> >vec;
        std::string raw;
        for (size_t i = 0; i < m_pattern.size(); ++i) {
            if (m_pattern[i] != '%') {
                raw.append(1, m_pattern[i]);
                continue;
            }

            //m_pattern[i] == '%'
            if ((i + 1) < m_pattern.size() && m_pattern[i + 1] == '%') {
                raw.append(1, '%');
                continue;
            }

            //m_pattern[i] == '%'
            size_t n = i + 1;
            std::string str, fmt;
            int fmt_status = 0;
            std::size_t fmt_begin = 0;

            while (n < m_pattern.size()) {
                //%d
                if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
                    && m_pattern[n] != '}')) {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    break;
                }

                if (fmt_status == 0) {
                    if (m_pattern[n] == '{') {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        fmt_begin = n;
                        fmt_status = 1;
                        ++n;
                        continue;
                    }
                } else if (fmt_status == 1) {
                    if (m_pattern[n] == '}') {
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        fmt_status = 0;
                        ++n;
                        break;
                    }
                }
                ++n;
                if(n == m_pattern.size()) {
                    if(str.empty()) {
                        str = m_pattern.substr(i + 1);
                    }
                }
            }
            
            if (!raw.empty()) {
              vec.push_back(std::make_tuple(std::move(raw), std::string(), 0));
              i = n - 1;
            }

            if (fmt_status == 0) {
              vec.push_back(std::make_tuple(std::move(str), std::move(fmt), 1));
              i = n - 1;
            } else if(fmt_status == 1) {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }

        }

        static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
        #define XX(str, C) \
            {#str, [](const std::string& fmt){return FormatItem::ptr(new C(fmt)); }}
                XX(m, MessageFormatItem),
                XX(p, LevelFormatItem),
                XX(r, ElapseFormatItem),
                XX(n, NewLineFormatItem), 
                XX(c, NameFormatItem),
                XX(t, ThreadIdFormatItem),
                XX(d, DateTimeFormatItem),
                XX(f, FileNameFormatItem),
                XX(l, LineFormatItem),
                XX(F, FiberIdFormatItem),
        #undef XX
        };

        for(auto& i : vec) {
            if(std::get<2>(i) == 0) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            } else {
                auto it = s_format_items.find(std::get<0>(i));
                if(it == s_format_items.end()) {
                  m_items.push_back(FormatItem::ptr(new StringFormatItem(
                      "<<error_format %" + std::get<0>(i) + ">>")));
                } else {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
                
            }
            //std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
        }
    //std::cout << m_items.size() << std::endl;

    }

    

}