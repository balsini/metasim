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
  _collision_detected(false),
  _status(IDLE),
  _corrupted_messages(0),
  _wait_for_DIFS_evt(),
  _wait_for_backoff_evt(),
  _data_received_evt(),
  _start_trans_evt(),
  _end_trans_evt(),
  _wait_for_SIFS_evt()
{
  //std::cout << "WifiInterface::WifiInterface: Registering handler" << std::endl;

  //register_handler(_collision_evt, this, &WifiInterface::onCollision);
  register_handler(_start_trans_evt, this, &WifiInterface::onStartTrans);
  register_handler(_wait_for_DIFS_evt, this, &WifiInterface::onDIFSElapsed);
  register_handler(_wait_for_SIFS_evt, this, &WifiInterface::onSIFSElapsed);
  register_handler(_data_received_evt, this, &WifiInterface::onMessageReceived);
  //register_handler(_end_trans_evt, this, &WifiInterface::onEndTrans);

  _radius = radius;
  //_coll = 0;

  //std::cout << "WifiInterface::WifiInterface: DONE" << std::endl;
}

WifiInterface::~WifiInterface()
{}

void WifiInterface::newRun()
{
  _ack_queue.clear();
  _out_queue.clear();
  _collision_detected = false;
  _status = IDLE;
  _corrupted_messages = 0;
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

WifiInterface * WifiInterface::routingProtocol(Node * n)
{
  double myPosX = std::get<0>(_node->position()),
      myPosY = std::get<1>(_node->position()),
      hisPosX = std::get<0>(n->position()),
      hisPosY = std::get<1>(n->position());

  if (myPosX == hisPosX) {
    // Send Bottom
    return _link->getRightInterface();
  } else if (myPosY == hisPosY) {
    // Send Down
    return _link->getDownInterface();
  }
  return 0;
}

void WifiInterface::send(Message *m)
{
  DBGENTER(_ETHINTER_DBG);

  _out_queue.push_back(m);

  m->sourceInterface(this);
  m->destInterface(routingProtocol(m->destNode()));

  if (_status == IDLE) {
    _status = WAITING_FOR_DIFS;
    _wait_for_DIFS_evt.post(SIMUL.getTime() + _DIFS);
  }
}

void WifiInterface::sendACK(Message * m)
{
  std::cout << "Awaiting for SIFS" << std::endl;
  _ack_queue.push_back(m);

  _status = WAITING_FOR_SIFS;
  _wait_for_SIFS_evt.post(SIMUL.getTime() + _SIFS);
}

void WifiInterface::onTransmit(Event * e)
{
  DBGENTER(_ETHINTER_DBG);

  //if (_link->isBusy())
  //  onCollision();
  //else
  //  _link->contend(this, _out_queue.front());
}

void WifiInterface::onDIFSElapsed(MetaSim::Event * e)
{
  std::cout << "DIFS elapsed!" << std::endl;
  if (_collision_detected) {
    std::cout << "Collision detected, managing backoff!" << std::endl;
  } else {
    std::cout << "No collision detected, starting transmission!" << std::endl;
    _link->send(_out_queue.front());
  }
}

void WifiInterface::onSIFSElapsed(MetaSim::Event * e)
{
  std::cout << "SIFS elapsed! Sending ACK" << std::endl;
  _link->send(_ack_queue.front());
  _ack_queue.pop_front();
}

void WifiInterface::onStartTrans(Event * e)
{
  std::cout << "Started Transmission!" << std::endl;
}

void WifiInterface::onMessageSent(MetaSim::Event * e)
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

void WifiInterface::onMessageReceived(Event * e)
{
  DBGENTER(_ETHINTER_DBG);

  std::cout << "Message received" << std::endl;

  if (_collision_detected) {
    _collision_detected = false;
    return;
  }

  // Check message interface address
  if (_incoming_message->destInterface() == this) {
    // Frame is for me, ACK must be sent

    if (_incoming_message->isACK()) {
      std::cout << "ACK received" << std::endl;
      _out_queue.pop_front();
    } else {
      Message * m_ack = new Message(10,
                                    _node,
                                    _incoming_message->destInterface()->node(),
                                    true);
      m_ack->destInterface(_incoming_message->sourceInterface());
      m_ack->sourceInterface(this);

      sendACK(m_ack);

      // Check if it has to be forwarded
      if (_incoming_message->destNode() == _node) {
        // Message is for my node
        _node->put(_incoming_message);
      } else {
        // Message must be forwarded
        send(_incoming_message);
      }
    }
  }
}

void WifiInterface::receive(Message * m)
{
  DBGTAG(_ETHINTER_DBG, getName() + "::get()");

  std::cout << "Incoming communication" << std::endl;

  switch(_status) {
    case IDLE:
      _incoming_message = m;
      _data_received_evt.post(SIMUL.getTime() + m->getTransTime());
      _status = RECEIVING_MESSAGE;
      break;
    case WAITING_FOR_DIFS:
      _collision_detected = true;
      break;
    case WAITING_FOR_SIFS:
      break;
    case RECEIVING_MESSAGE:
      _collision_detected = true;
      _corrupted_messages++;
      _data_received_evt.drop();
      return;
    default: break;
  }
}

