/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-04 16:17:11
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-03-17 15:03:56
 * @FilePath     : /tests/test_config.cc
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-04 16:17:11 
 */

#include "../sylar/log.h"
#include "../sylar/config.h"
#include <cstddef>
#include <sstream>
#include <string>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>
#include <vector>


sylar::ConfigVar<int>::ptr g_int_value_config =
    sylar::Config::Lookup<int>("system.port", (int)8080, "system port");

sylar::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config =
    sylar::Config::Lookup("system.vec", std::vector<int>{1, 2}, "system.vec");

sylar::ConfigVar<std::list<int>>::ptr g_int_list_value_config =
    sylar::Config::Lookup("system.list", std::list<int>{1, 2}, "system.list");

sylar::ConfigVar<std::set<int>>::ptr g_int_set_value_config =
    sylar::Config::Lookup("system.set", std::set<int>{1, 2}, "system.set");

sylar::ConfigVar<std::unordered_set<int>>::ptr g_int_uset_value_config =
    sylar::Config::Lookup("system.uset", std::unordered_set<int>{1, 2},
                          "system.uset");

sylar::ConfigVar<std::map<std::string, int>>::ptr g_int_map_value_config =
    sylar::Config::Lookup("system.map", std::map<std::string, int>{{"aaa", 1}, {"bbb", 2}},
                          "system.map");

sylar::ConfigVar<std::unordered_map<std::string, int>>::ptr
    g_int_umap_value_config = sylar::Config::Lookup(
        "system.umap",
        std::unordered_map<std::string, int>{{"aaa", 1}, {"bbb", 2}},
        "system.umap");

void print_yaml(YAML::Node node, int level) {
  if (node.IsScalar()) {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
        << std::string(level * 4, ' ') << node.Scalar() << " Scalar - " << node.Type()
        << " - " << level;
  } else if (node.IsNull()) {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
        << std::string(level * 4, ' ') <<  " NULL - " << node.Type()
        << " - " << level;
  } else if (node.IsMap()) {
    for (auto it = node.begin(); it != node.end(); ++it) {
      SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
          << std::string(level * 4, ' ') << it->first << " Map- "
          << it->second.Type() << " - " << level;
      print_yaml(it->second, level + 1);
    }
  } else if (node.IsSequence()) {
    for (size_t i = 0; i < node.size(); i++) {
      SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
          << std::string(level * 4, ' ') << i << " - "
          << node[i].Type() << " Sequence- " << level;
      print_yaml(node[i], level + 1);      
    }
  }   
}
void test_yaml() {
  YAML::Node node =
      YAML::LoadFile("/home/wenwenyuyu/sylar/bin/config/test.yaml");
  print_yaml(node, 0);
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << node;
}

void test_config() {
#define XX(g_var, name, prefix)                                                \
  {                                                                            \
    auto v = g_var->getValue();                                                \
    for (auto &i : v) {                                                        \
      SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ":" << i;          \
    }                                                                          \
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())                                           \
        << #prefix " " #name " yaml: " << g_var->toString();                   \
  }

#define XX_M(g_var, name, prefix)                                              \
  {                                                                            \
    auto v = g_var->getValue();                                                \
    for (auto &i : v) {                                                        \
      SYLAR_LOG_INFO(SYLAR_LOG_ROOT())                                         \
          << #prefix " " #name ": {" << i.first << " - " << i.second << "}";   \
    }                                                                          \
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())                                           \
        << #prefix " " #name " yaml: " << g_var->toString();                   \
  }

  XX(g_int_vec_value_config, vec, before);
  XX(g_int_list_value_config, list, before);
  XX(g_int_set_value_config, set, before);
  XX(g_int_uset_value_config, uset, before);
  XX_M(g_int_map_value_config, map, before);
  XX_M(g_int_umap_value_config, umap, before);
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
      << "before:" << g_int_value_config->getValue();

  YAML::Node node =
      YAML::LoadFile("/home/wenwenyuyu/sylar/bin/config/test.yaml");
  sylar::Config::LoadFromYaml(node);

  SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
      << "after:" << g_int_value_config->getValue();
  XX(g_int_vec_value_config, vec, after);
  XX(g_int_list_value_config, list, after);
  XX(g_int_set_value_config, set, after);
  XX(g_int_uset_value_config, uset, after);
  XX_M(g_int_map_value_config, map, after);
  XX_M(g_int_umap_value_config, umap, after);

#undef XX
#undef XX_M
}

class Person {
public:
  std::string m_name ;
  int m_age = 0;
  bool m_sex = 0;

  std::string toString() const {
    std::stringstream ss;
    ss << "[Person name=" << m_name << " age=" << m_age << " sex=" << m_sex
       << "]";
    return ss.str();
  }

  bool operator==(const Person &oth) const {
    return m_name == oth.m_name && m_sex == oth.m_sex && m_age == oth.m_age;
  }

    
};

namespace sylar {
template <>
class LexicalCast<std::string, Person> {
public:
  Person operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    Person p;
    p.m_name = node["name"].as<std::string>();
    p.m_age = node["age"].as<int>();
    p.m_sex = node["sex"].as<bool>();
    return p;
  }
};

template <>
class LexicalCast<Person,std::string> {
public:
  std::string operator()(const Person &v) {
    YAML::Node node;
    node["name"] = v.m_name;
    node["age"] = v.m_age;
    node["sex"] = v.m_sex;
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};
} // namespace sylar

sylar::ConfigVar<Person>::ptr g_person =
    sylar::Config::Lookup("class.person", Person(), "class person");

sylar::ConfigVar<std::map<std::string, Person>>::ptr g_person_map =
    sylar::Config::Lookup("class.map", std::map<std::string, Person>(),
                          "class map person");

sylar::ConfigVar<std::map<std::string, std::vector<Person>>>::ptr g_person_vec_map =
    sylar::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Person>>(),
                          "class map person");
void test_class() {
#define XX(g_var, prefix)                                                      \
  {                                                                            \
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())                                           \
        << #prefix << g_var->getValue().toString() << " - "                    \
        << g_var->toString();                                                  \
  }

#define XX_MP(g_var, prefix)                                                   \
  {                                                                            \
    auto m = g_var->getValue();                                                \
    for (auto &i : m) {                                                        \
      SYLAR_LOG_INFO(SYLAR_LOG_ROOT())                                         \
          << #prefix " " << i.first << " - " << i.second.toString();           \
    }                                                                          \
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix << ": size=" << m.size();      \
  }
  g_person->addListener([](const Person &old_value,
                              const Person &new_value) {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "old_value=" << old_value.toString()
                                     << "new_value=" << new_value.toString();
  });

  XX(g_person, before);
  XX_MP(g_person_map, before);
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before " << g_person_vec_map->toString();

  YAML::Node node =
      YAML::LoadFile("/home/wenwenyuyu/sylar/bin/config/test.yaml");
  sylar::Config::LoadFromYaml(node);

  XX(g_person, after);
  XX_MP(g_person_map, after);
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after " << g_person_vec_map->toString();
  
#undef XX
#undef XX_MP
}

void test_log() {
  static sylar::Logger::ptr sys = SYLAR_LOG_NAME("system");
  SYLAR_LOG_INFO(sys) << "create system log";
  std::cout << "before " << sylar::LoggerMagr::getInstance()->toYamlString() << std::endl;
  YAML::Node node =
      YAML::LoadFile("/home/wenwenyuyu/sylar/bin/config/log.yaml");
  std::cout << "load file " << std::endl;
  sylar::Config::LoadFromYaml(node);
  std::cout << "=============" << std::endl;
  std::cout << sylar::LoggerMagr::getInstance()->toYamlString() << std::endl;
  std::cout << "=============" << std::endl;
  SYLAR_LOG_INFO(sys) << "create system log";
}

int main() {
  // test_config();
  // SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "main start";
  // test_class();
  test_log();
  sylar::Config::Visit([](sylar::ConfigVarBase::ptr val) {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
        << " name = " << val->getName()
        << " description = " << val->getDescription()
        << " type = " << val->getTypeName() 
        << " value = " << val->toString();
  
  });

  return 0;
}