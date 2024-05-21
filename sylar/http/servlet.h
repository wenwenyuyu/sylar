/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-21 10:36:53
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-21 14:14:35
 * @FilePath     : /sylar/http/servlet.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-21 10:36:53
 */
#ifndef __SYLAR_SERVLET_H__
#define __SYLAR_SERVLET_H__

#include "sylar/http/http.h"
#include "sylar/http/http_session.h"
#include "sylar/mutex.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
namespace sylar {
namespace http {
class Servlet {
public:
  typedef std::shared_ptr<Servlet> ptr;

  Servlet(const std::string &name) : m_name(name) {}
  virtual ~Servlet(){};
  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         HttpSession::ptr session) = 0;
  const std::string &getName() const { return m_name; }
private:
std::string m_name;
};

class FunctionServlet : public Servlet {
public:
  typedef std::shared_ptr<FunctionServlet> ptr;
  typedef std::function<int32_t(HttpRequest::ptr request,
                                HttpResponse::ptr response,
                                HttpSession::ptr session)>
      callback;
  FunctionServlet(callback cb);
  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         HttpSession::ptr session) override;
private:
  callback m_cb;
};

class ServletDispatch : public Servlet {
public:
  typedef std::shared_ptr<ServletDispatch> ptr;
  typedef RWMutex RWMutexType;

  ServletDispatch();
  virtual int32_t handle(sylar::http::HttpRequest::ptr request
                   , sylar::http::HttpResponse::ptr response
                   , sylar::http::HttpSession::ptr session) override;

  void addServlet(const std::string &uri, Servlet::ptr slt);
  void addServlet(const std::string &uri, FunctionServlet::callback cb);
  void addGlobServlet(const std::string &uri, Servlet::ptr slt);
  void addGlobServlet(const std::string &uri, FunctionServlet::callback cb);

  void delServlet(const std::string &uri);
  void delGlobServlet(const std::string &uri);

  Servlet::ptr getDefault() const { return m_default; }
  void setDefault(Servlet::ptr v) { m_default = v;}

  Servlet::ptr getServlet(const std::string& uri);
  Servlet::ptr getGlobServlet(const std::string &uri);
  
  Servlet::ptr getMatchedServlet(const std::string& uri);

private:
  RWMutexType m_mutex;
  // uri(/sylar/xxx) -> servlet
  std::unordered_map<std::string, Servlet::ptr> m_datas;
  // uri(/sylar/*) ->servlet 
  std::vector<std::pair<std::string, Servlet::ptr>> m_globs;

  Servlet::ptr m_default;
};

class NotFoundServlet : public Servlet {
public:
  typedef std::shared_ptr<NotFoundServlet> ptr;

  NotFoundServlet();
  virtual int32_t handle(sylar::http::HttpRequest::ptr request,
                         sylar::http::HttpResponse::ptr response,
                         sylar::http::HttpSession::ptr session) override;
};
}
}
#endif