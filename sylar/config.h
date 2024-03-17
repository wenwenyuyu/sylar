/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-04 14:20:59
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-03-17 14:58:08
 * @FilePath     : /sylar/config.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-04 14:20:59
 */

#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include "log.h"
#include "sylar/mutex.h"
#include "util.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <boost/lexical_cast.hpp>
#include <unordered_set>
#include <utility>
#include <vector>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

namespace sylar {

class ConfigVarBase {
public:
  typedef std::shared_ptr<ConfigVarBase> ptr;
  ConfigVarBase(const std::string &name, const std::string &description)
      : m_name(name), m_description(description) {
    std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }
  virtual ~ConfigVarBase() {}

  const std::string &getName() const { return m_name; }
  const std::string &getDescription() const { return m_description; }

  virtual std::string toString() = 0;
  virtual bool fromString(const std::string &val) = 0;
  virtual std::string getTypeName () const = 0;
protected:
  std::string m_name;
  std::string m_description;
};

//将类型F转化为类型T
template <class F, class T> class LexicalCast {
public:
  T operator()(const F &v) {
    return boost::lexical_cast<T>(v);
  }
};

//特化
template <class T> class LexicalCast<std::string, std::vector<T>> {
public:
  std::vector<T> operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    typename std::vector<T> vec;
    std::stringstream ss;
    for (std::size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      vec.push_back(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

template <class T> class LexicalCast<std::vector<T>,std::string> {
public:
  std::string operator()(const std::vector<T> &v) {
    YAML::Node node;
    for (auto &i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <class T> class LexicalCast<std::string, std::list<T>> {
public:
  std::list<T> operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    typename std::list<T> l;
    std::stringstream ss;
    for (std::size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      l.push_back(LexicalCast<std::string, T>()(ss.str()));
    }
    return l;
  }
};

template <class T> class LexicalCast<std::list<T>,std::string> {
public:
  std::string operator()(const std::list<T> &v) {
    YAML::Node node;
    for (auto &i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <class T> class LexicalCast<std::string, std::set<T>> {
public:
  std::set<T> operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    typename std::set<T> s;
    std::stringstream ss;
    for (std::size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      s.insert(LexicalCast<std::string, T>()(ss.str()));
    }
    return s;
  }
};

template <class T> class LexicalCast<std::set<T>,std::string> {
public:
  std::string operator()(const std::set<T> &v) {
    YAML::Node node;
    for (auto &i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <class T> class LexicalCast<std::string, std::unordered_set<T>> {
public:
  std::unordered_set<T> operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    typename std::unordered_set<T> s;
    std::stringstream ss;
    for (std::size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      s.insert(LexicalCast<std::string, T>()(ss.str()));
    }
    return s;
  }
};

template <class T> class LexicalCast<std::unordered_set<T>,std::string> {
public:
  std::string operator()(const std::unordered_set<T> &v) {
    YAML::Node node;
    for (auto &i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <class T> class LexicalCast<std::string, std::map<std::string, T>> {
public:
  std::map<std::string, T> operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    typename std::map<std::string, T> m;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); ++it) {
      ss.str("");
      ss << it->second;
      m.insert(std::make_pair(it->first.Scalar(),
                                LexicalCast<std::string, T>()(ss.str())));
    }
    return m;
  }
};

template <class T> class LexicalCast<std::map<std::string, T>,std::string> {
public:
  std::string operator()(const std::map<std::string, T> &v) {
    YAML::Node node;
    for (auto &i : v) {
      node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <class T> class LexicalCast<std::string, std::unordered_map<std::string, T>> {
public:
  std::unordered_map<std::string, T> operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    typename std::unordered_map<std::string, T> m;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); ++it) {
      ss.str("");
      ss << it->second;
      m.insert(std::make_pair(it->first.Scalar(),
                                LexicalCast<std::string, T>()(ss.str())));
    }
    return m;
  }
};

template <class T> class LexicalCast<std::unordered_map<std::string, T>,std::string> {
public:
  std::string operator()(const std::unordered_map<std::string, T> &v) {
    YAML::Node node;
    for (auto &i : v) {
      node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <class T, class FromStr = LexicalCast<std::string, T>,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
          class ToStr = LexicalCast<T,std::string>>
class ConfigVar : public ConfigVarBase {
public:
  typedef std::shared_ptr<ConfigVar<T>> ptr;
  typedef std::function<void(const T& old_value, const T& new_value)> on_change_cb;
  typedef RWMutex RWMutexType;
  
  ConfigVar(const std::string &name, const T &default_value,
            const std::string &description)
      : ConfigVarBase(name, description), m_val(default_value) {}

  std::string toString() override {
    try {
      // return boost::lexical_cast<std::string>(m_val);
      RWMutexType::ReadLock lock(m_mutex);
      return ToStr()(m_val);
    } catch (std::exception &e) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
          << "ConfigVar::toString exception " << e.what()
          << " convert: " << typeid(m_val).name() << " to string";
    }
    return "";
  }

  bool fromString(const std::string &val) override {
    try {
      // m_val = boost::lexical_cast<T>(val);
      setValue(FromStr()(val));
    } catch (std::exception &e) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
          << "ConfigVar::fromString exception " << e.what()
          << " convert: string to " << typeid(m_val).name();
    }
    return false;
  }

  const T getValue() {
    RWMutexType::WriteLock lock(m_mutex);
    return m_val;
  }

  void setValue(const T &val) {
    {
      RWMutexType::ReadLock lock(m_mutex);
      if (val == m_val) {
        return;
      }
      for (auto &cb : m_cbs) {
        cb.second(m_val, val);
      }
    }

    RWMutexType::WriteLock lock(m_mutex);
    m_val = val;
  }
  
  std::string getTypeName() const override { return typeid(T).name(); }

  void addListener(on_change_cb cb) {
    static std::uint64_t s_fun_id = 0;
    RWMutexType::WriteLock lock(m_mutex);
    s_fun_id++;
    m_cbs[s_fun_id] = cb;
  }

  void delListener(std::uint64_t key) {
    RWMutexType::WriteLock lock(m_mutex);
    m_cbs.erase(key);
  }

  on_change_cb getListener(std::uint64_t key) {
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_cbs.find(key);
    return it == m_cbs.end() ? nullptr : it->second;
  }

  void clearListers() {
    RWMutexType::WriteLock lock(m_mutex);
    m_cbs.clear();
  }
  
private:
  T m_val;
  std::map<std::uint64_t, on_change_cb> m_cbs;
  RWMutexType m_mutex;
};

class Config {
public:
  typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;
  typedef RWMutex RWMutexType;
  /**
   * @func: Lookup
   * @return {*}
   * @description: 从全局map表中获取信息，若没有则进行创建
   */
  template <class T>
  static typename ConfigVar<T>::ptr
  Lookup(const std::string &name, const T &default_value,
         const std::string &description = "") {
    RWMutexType::WriteLock lock(GetMutex());
    auto it = GetDatas().find(name);
    if (it != GetDatas().end()) {
      auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
      if (tmp) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name=" << name << " exists";
        return tmp;
      } else {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
            << "Lookup name=" << name << " exists but type not "
            << typeid(T).name() << " real type = " << it->second->getTypeName()
            << " " << it->second->toString();
        return nullptr;
      }
    }

    if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid " << name;
      throw std::invalid_argument(name);
    }

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "create config: " << name;
    typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
    GetDatas()[name] = v;
    return v;
  }

  template <class T>
  static typename ConfigVar<T>::ptr Lookup(const std::string &name) {
    RWMutexType::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    if (it == GetDatas().end()) {
      return nullptr;
    }
    return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
  }

  static void LoadFromYaml(const YAML::Node &root);
  static ConfigVarBase::ptr LookupBase(const std::string &name);
  static void Visit(std::function<void(ConfigVarBase::ptr)>);

private:
  //静态成员没有固定的初始顺序，防止在调用静态成员变量时，该变量没有初始化，可以使用静态成员函数初始化
  static ConfigVarMap &GetDatas() {
    static ConfigVarMap m_datas;
    return m_datas;
  }

  static RWMutexType &GetMutex() {
    static RWMutexType m_mutex;
    return m_mutex;
  }
};
}
#endif