#ifndef __LINK_HPP__
#define __LINK_HPP__

#include <exception>
#include <memory>
#include <vector>
#include <string>

#include <entity.hpp>
#include <event.hpp>

#include "netinterface.hpp"
#include "node.hpp"

class LinkException: public std::exception
{
  std::string err;
public:
  LinkException(const std::string &e)
  {
    err = e;
  }
  virtual const char * what() const throw()
  {
    return err.c_str();
  }
};

class Message;
class Node;

class Link
{
  std::string _name;
public:
  Link(const std::string &name);
  virtual ~Link();

  virtual void send(std::unique_ptr<Message> &m) = 0;
};

class WifiLink : public Link
{
  std::vector<std::shared_ptr<WifiInterface>> _interfaces;

public:
  WifiLink(const std::string &name);
  virtual ~WifiLink();

  virtual void send(std::unique_ptr<Message> & m);

  void addInterface(std::shared_ptr<WifiInterface> &i);

  std::shared_ptr<WifiInterface> getRightInterface();
  std::shared_ptr<WifiInterface> getDownInterface();
};

#endif
