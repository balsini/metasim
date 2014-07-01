#ifndef __NETINTERFACE_HPP__
#define __NETINTERFACE_HPP__

#include <deque>
#include <vector>
#include <string>

#include <metasim.hpp>

#define _ETHINTER_DBG "WifiInterface"

class Node;
class Message;
class WifiLink;

class NetInterface : public MetaSim::Entity
{
protected:
  Node *_node;

public:
  NetInterface(const std::string &name, Node * const &n);
  virtual ~NetInterface();

  virtual void send(Message *m) = 0;
  virtual Message * receive(Node *n) = 0;

  virtual void onMessageSent(Message *m) = 0;
  virtual void onMessageReceived(Message *m) = 0;
};

class WifiInterface : public NetInterface
{
protected:
  std::deque<Message *> _queue;
  std::vector<Message *> _received;
  std::vector<Node *> _blocked;

  WifiLink * _link;

  int _cont_per;
  int _backoff;
  int _coll;

public:

  MetaSim::GEvent<WifiInterface> _trans_evt;

  WifiInterface(const std::string &name, Node * n);
  virtual ~WifiInterface();

  void link(WifiLink * l);
  WifiLink * link();

  MetaSim::Tick nextTransTime();

  virtual void send(Message* m);
  virtual void onCollision();
  virtual void onTransmit(MetaSim::Event* e);
  virtual Message* receive(Node* n);
  virtual void onMessageSent(Message* m);
  virtual void onMessageReceived(Message* m);

  void newRun();
  void endRun();
};

#endif
