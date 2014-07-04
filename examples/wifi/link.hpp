#ifndef __LINK_HPP__
#define __LINK_HPP__

#include <vector>
#include <string>

#include <entity.hpp>
#include <event.hpp>

#include "netinterface.hpp"
#include "node.hpp"

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
  std::vector<WifiInterface *> _contending;

public:
  WifiLink(const std::string &name);
  virtual ~WifiLink();

  virtual void send(std::unique_ptr<Message> &m);

  void addNode(Node * n);

  WifiInterface * getRightInterface();
  WifiInterface * getDownInterface();
};

#endif
