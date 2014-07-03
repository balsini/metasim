#include <metasim.hpp>

#include "link.hpp"
#include "message.hpp"

using namespace MetaSim;

Link::Link(const std::string & name)
{
  _name = name;
}

Link::~Link() {}

WifiLink::WifiLink(const std::string &name)
  : Link(name) {}

WifiLink::~WifiLink() {}

void WifiLink::send(Message *m)
{
  std::cout << "Broadcasting message to:" << std::endl;
  for (auto o : _interfaces) {
    std::cout << o->getName() << std::endl;
    o->receive(m);
  }
  std::cout << "DONE" << std::endl;
}

void WifiLink::addNode(Node * n)
{
  _interfaces.push_back(n->netInterface());
  for (auto o : _interfaces) {
    std::cout << o->getName() << std::endl;
  }
}

WifiInterface * WifiLink::getRightInterface() {
  WifiInterface * i = _interfaces.front();
  double maxRight = std::get<0>(_interfaces.front()->node()->position());

  // Search for the rightest
  for (auto o : _interfaces) {
    if (maxRight < std::get<0>(o->node()->position())) {
      maxRight = std::get<0>(o->node()->position());
      i = o;
    }
  }

  return i;
}

WifiInterface * WifiLink::getDownInterface() {
  WifiInterface * i = _interfaces.front();
  double maxDown = std::get<1>(_interfaces.front()->node()->position());

  // Search for the rightest
  for (auto o : _interfaces) {
    if (maxDown < std::get<1>(o->node()->position())) {
      maxDown = std::get<1>(o->node()->position());
      i = o;
    }
  }

  return i;
}
