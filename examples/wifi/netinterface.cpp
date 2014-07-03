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
  register_handler(_end_trans_evt, this, &WifiInterface::onEndTrans);

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
  double myPosR = std::get<0>(_node->position()),
      myPosC = std::get<1>(_node->position()),
      hisPosR = std::get<0>(n->position()),
      hisPosC = std::get<1>(n->position());

  if (myPosR == hisPosR) {
    // Send Bottom
    return _link->getRightInterface();
  } else if (myPosC == hisPosC) {
    // Send Down
    return _link->getDownInterface();
  }
  return 0;
}

void WifiInterface::onEndTrans(MetaSim::Event * e)
{
  std::cout << "Transmission completed" << std::endl;

  _status = IDLE;
}

void WifiInterface::onDIFSElapsed(MetaSim::Event * e)
{
  std::cout << "DIFS elapsed!" << std::endl;
  if (_collision_detected) {
    std::cout << this->getName() << ": Collision detected, managing backoff!" << std::endl;
    _status = WAITING_FOR_BACKOFF;
    //TODO
  } else {
    std::cout << this->getName() << ": No collision detected, starting transmission! Destination: " << _out_queue.front()->destInterface()->getName() << std::endl;

    _end_trans_evt.post(SIMUL.getTime() + _out_queue.front()->getTransTime());
    _link->send(_out_queue.front());
    _status = SENDING_MESSAGE;
  }
}

void WifiInterface::onSIFSElapsed(MetaSim::Event * e)
{
  std::cout << this->getName() << ": SIFS elapsed! Sending ACK to " << _ack_queue.front()->destInterface()->getName() << std::endl;

  _end_trans_evt.post(SIMUL.getTime() + _ack_queue.front()->getTransTime());
  _link->send(_ack_queue.front());
  _ack_queue.pop_front();
}

void WifiInterface::onStartTrans(Event * e)
{
  std::cout << this->getName() << ": Started Transmission!" << std::endl;
}

void WifiInterface::onMessageSent(MetaSim::Event * e)
{
  DBGENTER(_ETHINTER_DBG);

  _out_queue.pop_front();
  _status = IDLE;
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

  std::cout << this->getName() << ": Message received from "
            << _incoming_message->sourceInterface()->getName()
            << std::endl;

  if (_collision_detected) {
    std::cout << "\tCollision Detected, dropping" << std::endl;
    _collision_detected = false;
    return;
  }

  // Check message interface address
  if (_incoming_message->destInterface() == this) {
    // Frame is for me, ACK must be sent

    std::cout << "\tCorrect Interface" << std::endl;
    if (_incoming_message->isACK()) {
      std::cout << "\tACK received" << std::endl;
      _out_queue.pop_front();
    } else {
      std::cout << "\tNormal Message" << std::endl;
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
  } else {
    std::cout << "\tWrong Interface, dropping" << std::endl;
  }
  _status = IDLE;
}

void WifiInterface::sendACK(Message * m)
{
  std::cout << this->getName() << ": Awaiting for SIFS" << std::endl;
  _ack_queue.push_back(m);

  _status = WAITING_FOR_SIFS;
  _wait_for_SIFS_evt.post(SIMUL.getTime() + _SIFS);
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

void WifiInterface::receive(Message * m)
{
  DBGTAG(_ETHINTER_DBG, getName() + "::get()");

  std::cout << this->getName() << ": Incoming communication from " << m->sourceInterface()->getName() << std::endl;

  printStatus();

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


void WifiInterface::printStatus()
{
  switch(_status) {
    case IDLE:
      std::cout << "IDLE" << std::endl;
      break;
    case WAITING_FOR_DIFS:
      std::cout << "WAITING_FOR_DIFS" << std::endl;
      break;
    case WAITING_FOR_SIFS:
      std::cout << "WAITING_FOR_SIFS" << std::endl;
      break;
    case SENDING_MESSAGE:
      std::cout << "SENDING_MESSAGE" << std::endl;
      break;
    case RECEIVING_MESSAGE:
      std::cout << "RECEIVING_MESSAGE" << std::endl;
      break;
    case WAITING_FOR_BACKOFF:
      std::cout << "WAITING_FOR_BACKOFF" << std::endl;
      break;
    default:
      std::cout << "!!! Status: unknown !!!" << std::endl;
      break;
  }
}
