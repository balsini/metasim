#ifndef __LINK_HPP__
#define __LINK_HPP__

#include <exception>
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

const char* const _ETHLINK_DBG = "EthernetLink";

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
  std::vector<WifiInterface *> _interfaces;

public:
  WifiLink(const std::string &name);
  virtual ~WifiLink();

  virtual void send(std::unique_ptr<Message> &m);

  void addInterface(WifiInterface * i);

  WifiInterface * getRightInterface();
  WifiInterface * getDownInterface();
};

#endif
