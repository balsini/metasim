#include <metasim.hpp>

#include "link.hpp"
#include "message.hpp"

using namespace MetaSim;

Link::Link(const std::string &name) : Entity(name) {}

Link::~Link() {}

WifiLink::WifiLink(const std::string &name)
  : Link(name), _interfaces(), _contending(),
    _isBusy(false),
    _isContending(false),
    _isCollision(false),
    _contention_period(10),
    _message(0) {}

WifiLink::~WifiLink() {}

void WifiLink::newRun()
{
  _contending.clear();
  _isBusy = false;
  _isContending = false;
  _isCollision = false;
  _message = 0;
}

void WifiLink::send(Message *m)
{
  std::cout << "Broadcasting messages" << std::endl;
  for (auto o : _nodes)
    o->netInterface()->receive(m);
  std::cout << "DONE" << std::endl;
}

void WifiLink::contend(WifiInterface * wifi, Message * m)
{
  /*
  DBGENTER(_ETHLINK_DBG);

  if (_isContending) {
    //_end_contention_evt.drop();
    if (!_isCollision) {
      _isCollision = true;
      //_collision_evt.post(SIMUL.getTime() + 3);
    }
  } else {
    _isContending = true;
    _message = m;
    //_end_contention_evt.post(SIMUL.getTime() + _contention_period);
  }
  _contending.push_back(wifi);
  */
}

void WifiLink::onEndContention(Event *e)
{
  /*
  DBGENTER(_ETHLINK_DBG);

  _isContending = false;
  _isBusy = true;
  //_end_transmission_evt.post(SIMUL.getTime() + _message->length());

  _contending.clear();
  */
}


void WifiLink::addNode(Node * n)
{
  _nodes.push_back(n);
}

void WifiLink::onEndTransmission(Event *e)
{
  /*
  Message *m = _message;

  DBGENTER(_ETHLINK_DBG);

  NetInterface *dst = _message->destInterface();
  NetInterface *src = _message->sourceInterface();

  _isBusy = false;
  _message = 0;

  //dst->onMessageReceived(m);
  //src->onMessageSent(m);

*/
}
