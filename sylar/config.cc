/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-03-04 16:18:30
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-03-17 15:06:11
 * @FilePath     : /sylar/config.cc
 * @Description  : 对ConfigVarMap的基本函数实现，通过读取yaml中的内容修改map中的信息
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-03-04 16:18:30
 */
#include "config.h"
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <yaml-cpp/node/node.h>

namespace sylar {
/**
 * @func: LookupBase
 * @param {string} &name
 * @return ConfigVarBase::ptr
 * @description: 用name从ConfigVarMap获取相应的信息
 */
ConfigVarBase::ptr Config::LookupBase(const std::string &name) {
  RWMutexType::ReadLock lock(GetMutex());
  auto it = GetDatas().find(name);
  return it == GetDatas().end() ? nullptr : it->second;
}

// "A.B" 10
//  A:
//    B: 10
//
/**
 * @func: ListAllMember
 * @param：const std::string &prefix, const YAML::Node &node, std::list<std::pair<std::string, const YAML::Node>> &output
 * @return {*}
 * @description: 将yaml中的格式如{A: B: C:10} 修改成A.B.C：10；同时获得所有的node信息并输入到output中
 */
static void
ListAllMember(const std::string &prefix, const YAML::Node &node,
              std::list<std::pair<std::string, const YAML::Node>> &output) {
  if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") !=
      std::string::npos) {
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
        << " Config invaild name " << prefix << "." << node;
    return;
  }
  
  output.push_back(std::make_pair(prefix, node));
  //递归调用函数，获取子集信息
  if (node.IsMap()) {
    for (auto it = node.begin(); it != node.end(); ++it) {
      ListAllMember(prefix.empty() ? it->first.Scalar()
                                   : prefix + "." + it->first.Scalar(),
                    it->second, output);
    }
  }
}
/**
 * @func: 
 * @param {Node} &root
 * @return {*}
 * @description: 从yaml文档中读取相应的信息；读取之后，若全局map中已经有该信息则修改全局map的内容为yaml文档中的内容
 */
void Config::LoadFromYaml(const YAML::Node &root) {
  std::list<std::pair<std::string, const YAML::Node>> all_nodes;
  ListAllMember("", root, all_nodes);
  //获得yaml中的全部信息
  for (auto &node : all_nodes) {
    std::string key = node.first;
    if (key.empty()) {
      continue;
    }
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    ConfigVarBase::ptr var = LookupBase(key);
    //如果全局map中有该项信息，则修改成yaml中的内容
    if (var) {
      if (node.second.IsScalar()) {
        var->fromString(node.second.Scalar());
      } else {
        std::stringstream ss;
        ss << node.second;
        var->fromString(ss.str());
      }
    }
  }
}

void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb) {
  RWMutexType::WriteLock lock(GetMutex());
  for (auto i = GetDatas().begin(); i != GetDatas().end(); ++i) {
    cb(i->second);
  }
}
}