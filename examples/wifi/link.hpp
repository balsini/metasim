#ifndef __LINK_HPP__
#define __LINK_HPP__

#include <vector>
#include <string>

#include <entity.hpp>
#include <event.hpp>

#include "netinterface.hpp"
#include "node.hpp"

const char* const _ETHLINK_DBG = "EthernetLink";

class Message;

class Link : public MetaSim::Entity
{
public:
  Link(const std::string &name);
  virtual ~Link();

  virtual void send(Message * m) = 0;
};

class WifiLink : public Link
{
  std::vector<WifiInterface *> _interfaces;
  std::vector<WifiInterface *> _contending;

  bool _isBusy;
  bool _isContending;
  bool _isCollision;
  int _contention_period;

  Message *_message;
  std::vector<Node *> _nodes;

public:
  WifiLink(const std::string &name);
  virtual ~WifiLink();

  bool isBusy();

  virtual void send(Message *m);

  void addNode(Node * n);

  void contend(WifiInterface *eth, Message *m);
  void onEndContention(MetaSim::Event *e);
  void onCollision(MetaSim::Event *e);
  void onEndTransmission(MetaSim::Event *e);

  void setContentionPeriod(int p) { _contention_period = p; }
  int getContentionPeriod() { return _contention_period; }

  void newRun();
  void endRun();
};

#endif
