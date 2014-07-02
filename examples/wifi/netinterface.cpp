#include <algorithm>

#include <metasim.hpp>

#include "link.hpp"
#include "message.hpp"
#include "netinterface.hpp"
#include "node.hpp"

using namespace MetaSim;

NetInterface::NetInterface(const std::string &name, Node* const &n) :
  Entity(name)
{}

NetInterface::~NetInterface()
{
}

WifiInterface::WifiInterface(const std::string &name, double radius, Node * n) :
  NetInterface(name, n),
  _wait_for_DIFS_evt(),
  _wait_for_backoff_evt(),
  _collision_evt(),
  _start_trans_evt(),
  _end_trans_evt(),
  _wait_for_SIFS_evt()
{
  //std::cout << "WifiInterface::WifiInterface: Registering handler" << std::endl;

  //register_handler(_collision_evt, this, &WifiInterface::onCollision);
  register_handler(_start_trans_evt, this, &WifiInterface::onStartTrans);
  register_handler(_wait_for_DIFS_evt, this, &WifiInterface::onDIFSElapsed);
  //register_handler(_end_trans_evt, this, &WifiInterface::onEndTrans);

  _radius = radius;
  //_coll = 0;

  //std::cout << "WifiInterface::WifiInterface: DONE" << std::endl;
}

WifiInterface::~WifiInterface()
{}

void WifiInterface::newRun()
{
  vector<Message *>::iterator i;

  _out_queue.clear();
  //for (i = _received.begin(); i != _received.end(); ++i) delete (*i);

}

void WifiInterface::link(WifiLink * l)
{
  _link = l;
  //_cont_per = _link->getContentionPeriod();
  //_backoff = _cont_per;
}

WifiLink * WifiInterface::link()
{
  return _link;
}

void WifiInterface::endRun()
{}

void WifiInterface::send(Message *m)
{
  DBGENTER(_ETHINTER_DBG);

  _out_queue.push_back(m);

  if (_out_queue.size() == 1)
    _wait_for_DIFS_evt.post(SIMUL.getTime() + _DIFS);
  else
    DBGPRINT("Message enqueued");
}

void WifiInterface::onTransmit(Event *e)
{
  DBGENTER(_ETHINTER_DBG);

  if (_link->isBusy())
    onCollision();
  else
    _link->contend(this, _out_queue.front());
}

void WifiInterface::onDIFSElapsed(MetaSim::Event * e)
{
  std::cout << "DIFS elapsed!" << std::endl;
}

void WifiInterface::onStartTrans(Event * e)
{
  std::cout << "Started Transmission!" << std::endl;
}

void WifiInterface::onCollision()
{
  DBGENTER(_ETHINTER_DBG);

  //_trans_evt.post(SIMUL.getTime() + nextTransTime());


}

void WifiInterface::onMessageSent(Message *m)
{
  DBGENTER(_ETHINTER_DBG);

  _out_queue.pop_front();

  //_coll = 0;
  //_backoff = _cont_per;

  //if (!_queue.empty()) _trans_evt.process();
}

Tick WifiInterface::nextTransTime()
{
  DBGTAG(_ETHINTER_DBG, getName() + "::nextTransTime()");

  //_coll++;

  //if (_coll <= 10) _backoff *= 2;

  UniformVar a(1, _c_w);

  return (Tick) a.get();
}

void WifiInterface::onMessageReceived(Message *m)
{
  DBGENTER(_ETHINTER_DBG);
  /*
  vector<Node *>::iterator i = find(_blocked.begin(), _blocked.end(), m->destNode());

  if (i != _blocked.end()) {
    (*i)->onMessageReceived(m);
    _blocked.erase(i);
  }
  else
    _received.push_back(m);
    */
}

Message * WifiInterface::receive(Node *n)
{
  DBGTAG(_ETHINTER_DBG, getName() + "::get()");
  /*
  vector<Message *>::iterator i = _received.begin();
  Message *m = NULL;

  while (i != _received.end()) {
    if ((*i)->destNode() == n) {
      m = *i;
      _received.erase(i);
      return m;
    }
    else ++i;
  }
  _blocked.push_back(n);
  */
  return nullptr;
}

