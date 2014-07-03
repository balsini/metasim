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
  for (auto o : _interfaces) {
    //std::cout << "Broadcasting message to: " << o->getName() << std::endl;
    o->receive(m);
  }
}

void WifiLink::addNode(Node * n)
{
  _interfaces.push_back(n->netInterface());
}

WifiInterface * WifiLink::getRightInterface() {
  WifiInterface * i = _interfaces.front();
  double maxRight = std::get<1>(_interfaces.front()->node()->position());

  // Search for the rightest
  for (auto o : _interfaces) {
    if (maxRight < std::get<1>(o->node()->position())) {
      maxRight = std::get<1>(o->node()->position());
      i = o;
    }
  }

  return i;
}

WifiInterface * WifiLink::getDownInterface() {
  WifiInterface * i = _interfaces.front();
  double maxDown = std::get<0>(_interfaces.front()->node()->position());

  // Search for the rightest
  for (auto o : _interfaces) {
    if (maxDown < std::get<0>(o->node()->position())) {
      maxDown = std::get<0>(o->node()->position());
      i = o;
    }
  }

  return i;
}
