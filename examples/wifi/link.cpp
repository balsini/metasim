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
  : Link(name)
{
  _interfaces.clear();
}

WifiLink::~WifiLink() {}

void WifiLink::send(std::unique_ptr<Message> &m)
{
  for (auto o : _interfaces) {
    //std::cout << "Broadcasting message to: " << o->getName() << std::endl;
    o->receive(m.get());
  }
}

void WifiLink::addInterface(std::shared_ptr<WifiInterface> &i)
{
  //std::cout << "Link: added interface" << std::endl;

  _interfaces.push_back(i);
}

std::shared_ptr<WifiInterface> WifiLink::getRightInterface()
{
  if (_interfaces.size() == 0)
    throw LinkException("Link is empty");

  std::shared_ptr<WifiInterface> i = _interfaces.front();
  double maxRight = std::get<1>(i->node()->position());

  // Search for the rightest
  for (auto o : _interfaces) {
    if (maxRight < std::get<1>(o->node()->position())) {
      maxRight = std::get<1>(o->node()->position());
      i = o;
    }
  }

  return i;
}

std::shared_ptr<WifiInterface> WifiLink::getDownInterface() {
  if (_interfaces.size() == 0)
    throw LinkException("Link is empty");

  std::shared_ptr<WifiInterface> i = _interfaces.front();
  double maxDown = std::get<0>(i->node()->position());

  // Search for the rightest
  for (auto o : _interfaces) {
    if (maxDown < std::get<0>(o->node()->position())) {
      maxDown = std::get<0>(o->node()->position());
      i = o;
    }
  }

  return i;
}
