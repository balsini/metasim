#ifndef __NODE_HPP__
#define __NODE_HPP__

#include <memory>
#include <vector>
#include <string>

#include "link.hpp"
#include "message.hpp"
#include "netinterface.hpp"

#include <metasim.hpp>

#define _NODE_DBG "Node"

class Node : public MetaSim::Entity
{
  WifiInterface * _net_interf;

  std::auto_ptr<MetaSim::RandomVar> _interval;

  std::pair <double, double> _position;
  double _radius;

public:
  MetaSim::GEvent<Node> _recv_evt;
  MetaSim::GEvent<Node> _send_evt;

  Node(const std::string &name,
       std::pair <double, double> position,
       double radius);

  std::pair <double, double> position();
  double radius();

  // Net interface management
  WifiInterface * netInterface();
  void netInterface(WifiInterface * n);

  void setInterval(std::auto_ptr<MetaSim::RandomVar> i);

  void onMessageReceived(Message *m);
  void onReceive(MetaSim::Event *e);
  void onSend(MetaSim::Event *e);

  void newRun();
  void endRun();
};

#endif
