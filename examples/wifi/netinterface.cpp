#include <algorithm>

#include <metasim.hpp>

#include <sstream>

#include "link.hpp"
#include "message.hpp"
#include "netinterface.hpp"
#include "node.hpp"

using namespace MetaSim;

NetInterface::NetInterface(const std::string &name, const std::shared_ptr<Node> &n) :
  Entity(name), _node(n)
{}

NetInterface::~NetInterface()
{
}

WifiInterface::WifiInterface(const std::string &name, double radius, const std::shared_ptr<Node> &n) :
  NetInterface(name, n),
  _collision_detected(false),
  _status(IDLE),
  _corrupted_messages(0),
  _waitingForAck(false),
  _wait_for_DIFS_evt(),
  _wait_for_backoff_evt(),
  _data_received_evt(),
  _start_trans_evt(),
  _end_trans_evt(),
  _wait_for_SIFS_evt()
{
  register_handler(_start_trans_evt, this, &WifiInterface::onStartTrans);
  register_handler(_wait_for_DIFS_evt, this, &WifiInterface::onDIFSElapsed);
  register_handler(_wait_for_SIFS_evt, this, &WifiInterface::onSIFSElapsed);
  register_handler(_data_received_evt, this, &WifiInterface::onMessageReceived);
  register_handler(_end_trans_evt, this, &WifiInterface::onEndTrans);
  register_handler(_end_ACKtrans_evt, this, &WifiInterface::onEndACKTrans);
  register_handler(_wait_for_ACK_evt, this, &WifiInterface::onACKTimeElapsed);
  register_handler(_wait_for_backoff_evt, this, &WifiInterface::onBackoffTimeElapsed);

  _wifiTrace.reset();
  _radius = radius;
  _c_w = _c_wMin;
}

WifiInterface::~WifiInterface()
{}

void WifiInterface::newRun()
{
  status(IDLE);
  _waitingForAck = false;
  _c_w = _c_wMin;
  _ack_queue.clear();
  _out_queue.clear();
  _collision_detected = false;
  _corrupted_messages = 0;
}

void WifiInterface::link(const std::shared_ptr<WifiLink> l)
{
  _link = l;
}

const std::shared_ptr<WifiLink> WifiInterface::link()
{
  return _link;
}

void WifiInterface::endRun()
{}

const std::shared_ptr<WifiInterface> WifiInterface::routingProtocol(Node * n)
{
  //std::cout << "routingProtocol" << std::endl;
  double myPosR = std::get<0>(_node->position()),
      myPosC = std::get<1>(_node->position()),
      hisPosR = std::get<0>(n->position()),
      hisPosC = std::get<1>(n->position());

  try {
    if (myPosR == hisPosR) {
      // Send Bottom
      //std::cout << "rightInterface" << std::endl;
      return _link->getRightInterface();
    } else if (myPosC == hisPosC) {
      // Send Down
      //std::cout << "downInterface" << std::endl;
      return _link->getDownInterface();
    }
  } catch (exception& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}

void WifiInterface::onEndTrans(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": Transmission completed" << std::endl;

  status(WAITING_FOR_ACK);
  //_wait_for_ACK_evt.drop();
  _wait_for_ACK_evt.post(SIMUL.getTime() + Tick(_ACK_timeout));
}

void WifiInterface::onEndACKTrans(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": ACK Transmission completed" << std::endl;

  trySend();
}

void WifiInterface::onBackoffTimeElapsed(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": Backoff Time Elapsed" << std::endl;

  //std::cout << _out_queue.size() << std::endl;

  status(SENDING_MESSAGE);

  _end_trans_evt.drop();
  _end_trans_evt.post(SIMUL.getTime() + Tick(_out_queue.front()->transTime()));

  _link->send(_out_queue.front());
}

void WifiInterface::onACKTimeElapsed(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": ACK Timer elapsed!" << std::endl;

  incrementBackoff();
  // Retry to send the packet.
  status(WAITING_FOR_BACKOFF);
  /*
  std::cout << SIMUL.getTime() << " : Initializes backoff timer to: "
            << SIMUL.getTime() + Tick(getBackoff()) << std::endl;
            */
  _wait_for_backoff_evt.drop();
  _wait_for_backoff_evt.post(SIMUL.getTime() + Tick(getBackoff()));
}

void WifiInterface::onDIFSElapsed(MetaSim::Event * e)
{
  //std::cout << "DIFS elapsed!" << std::endl;
  if (_collision_detected) {

    _collision_detected = false;
/*
    std::cout << this->getName() << ": Collision detected, managing backoff!"
              << std::endl;
*/
    // Initializes backoff timer

    //std::cout << SIMUL.getTime() << ": Last _data_received_evt at: " << _data_received_evt.getTime() << std::endl;


    // Checks if there is still an incoming message
    if (_data_received_evt.isInQueue()) {
/*
      std::cout << this->getName() << ": Message transmission still pending!"
                << std::endl;
*/
      _wait_for_backoff_evt.post(_data_received_evt.getTime() + Tick(getBackoff()));
    } else {
      _wait_for_backoff_evt.post(SIMUL.getTime() + Tick(getBackoff()));
    }

    status(WAITING_FOR_BACKOFF);
  } else {
    /*
    std::cout << this->getName()
              << ": No collision detected, starting transmission! Destination: "
              << _out_queue.front()->destInterface()->getName() << std::endl;
              */

    //_end_trans_evt.drop();

    status(SENDING_MESSAGE);

    _end_trans_evt.post(SIMUL.getTime() + Tick(_out_queue.front()->transTime()));
    _link->send(_out_queue.front());
  }
}

void WifiInterface::onSIFSElapsed(MetaSim::Event * e)
{
  /*
  std::cout << this->getName() << ": SIFS elapsed! Sending ACK to "
            << _ack_queue.front()->destInterface()->getName() << std::endl;
*/
  //_end_ACKtrans_evt.drop();

  status(SENDING_ACK);

  _end_ACKtrans_evt.post(SIMUL.getTime() + Tick(_ack_queue.front()->transTime()));
  _link->send(_ack_queue.front());
  _ack_queue.pop_front();
}

void WifiInterface::onStartTrans(Event * e)
{
  /*
  std::cout << this->getName() << ": Started Transmission!" << std::endl;
  */
}

void WifiInterface::onMessageSent(MetaSim::Event * e)
{
  DBGENTER(_ETHINTER_DBG);

  trySend();
}

void WifiInterface::incrementBackoff()
{
  _c_w = (2 * _c_w) < _c_wMax ?
           2 * _c_w :
           _c_wMax;

  //std::cout << "Backoff incremented: " << _c_w << std::endl;
}

int WifiInterface::getBackoff()
{
  UniformVar backoffRand(1, _c_w);

  int b = backoffRand.get();

  //std::cout << "Backoff chosen: " << b << std::endl;
  return b;
}

void WifiInterface::onMessageReceived(Event * e)
{
  DBGENTER(_ETHINTER_DBG);

  if (status() == WAITING_FOR_BACKOFF) {
    return;
  }
  /*
  std::cout << this->getName() << ": Message received from "
            << _incoming_message->sourceInterface()->getName()
            << std::endl;
*/
  if (_collision_detected) {
    //std::cout << "\tCollision Detected, dropping" << std::endl;
    _collision_detected = false;

    trySend();
  } else if (_incoming_message->destInterface() == this) {
    // Frame is for me

    //std::cout << "\tCorrect Interface" << std::endl;
    if (_incoming_message->isACK()) {

      //std::cout << "\tACK received" << std::endl;

      if (_waitingForAck ) {
        //std::cout << "\tWaiting for ACK" << std::endl;

        if (_incoming_message->id() == _out_queue.front()->id()) {
          //std::cout << "\tcorrect ACK received" << std::endl;

          // ACK timer is disabled
          _wait_for_ACK_evt.drop();

          // No more waiting for ACK
          _waitingForAck = false;
          // Pending message can be destroyed (succesfully sent)
          _out_queue.pop_front();

        } else {
          //std::cout << "\tWRONG ACK received" << std::endl;
        }
      }

      trySend();
    } else {
      // Frame is a normal message,
      // ACK must be sent!

      //std::cout << "\tNormal Message, send ACK after SIFS" << std::endl;

      auto m_ack_unique = std::unique_ptr<Message>(new Message(10,
                                                               _node.get(),
                                                               _incoming_message->destInterface()->node().get(),
                                                               _incoming_message->id(),
                                                               true));
      m_ack_unique->transTime(5);
      m_ack_unique->destInterface(_incoming_message->sourceInterface());
      m_ack_unique->sourceInterface(this);

      sendACK(m_ack_unique);

      // Check if it has to be forwarded
      if (_incoming_message->destNode() == _node.get()) {
        // Message is for my node
        auto message_unique = std::unique_ptr<Message>(new Message(*_incoming_message));
        _node->put(message_unique);
      } else {
        // Message must be forwarded
        //std::cout << "Forwarding..." << std::endl;
        auto out_msg = std::unique_ptr<Message>(new Message(*_incoming_message));
        send(out_msg);
      }
    }
  } else {
    //std::cout << "\tWrong Interface, dropping" << std::endl;

    trySend();
  }
}

void WifiInterface::sendACK(std::unique_ptr<Message> &m)
{
  //std::cout << this->getName() << ": Awaiting for SIFS" << std::endl;

  status(WAITING_FOR_SIFS);

  _ack_queue.push_back(std::move(m));

  _wait_for_SIFS_evt.post(SIMUL.getTime() + Tick(_SIFS));
}

void WifiInterface::send(std::unique_ptr<Message> &m)
{
  DBGENTER(_ETHINTER_DBG);

  //std::cout << "Sending message" << std::endl;

  m->sourceInterface(this);
  m->destInterface(routingProtocol(m->destNode()).get());

  _out_queue.push_back(std::move(m));

  if (status() == IDLE)
    trySend();
}

void WifiInterface::trySend()
{
  //std::cout << "TrySend()" << std::endl;

  if (_out_queue.size() > 0) {
    if (status() != WAITING_FOR_DIFS) {
      status(WAITING_FOR_DIFS);
      //std::cout << "trySend(): posting event" << std::endl;
      if (!_wait_for_DIFS_evt.isInQueue())
        _wait_for_DIFS_evt.post(SIMUL.getTime() + Tick(_DIFS));
      //std::cout << "DONE" << std::endl;
    }
  } else {
    status(IDLE);
  }

  //std::cout << "DONE" << std::endl;
}

void WifiInterface::receive(const std::shared_ptr<Message> &m)
{
  /*
  std::cout << this->getName() << ": Incoming communication from "
            << m->sourceInterface()->getName() << std::endl;
*/

  MetaSim::Tick b;

  switch(status()) {
    case RECEIVING_MESSAGE:
      // The previously transferring message is now corrupted, so:
      // A collision has been detected
      _collision_detected = true;
      // The number of corrupted messages is increased for
      //statistics
      _corrupted_messages++;
      // Now we have to wait until the longest incoming transmission
      // is completed

      //std::cout << "Last _data_received_evt at: " << _data_received_evt.getTime() << std::endl;
      //std::cout << "Next _data_received_evt at: " << SIMUL.getTime() + m->transTime() << std::endl;

      if (_data_received_evt.getTime() < SIMUL.getTime() + m->transTime()) {
        _data_received_evt.drop();
        _data_received_evt.post(SIMUL.getTime() + m->transTime());
      }
      break;
    case WAITING_FOR_ACK:
      _waitingForAck = true;
    case IDLE:
      _incoming_message = m;
      //_data_received_evt.drop();
      _data_received_evt.post(SIMUL.getTime() + m->transTime());
      status(RECEIVING_MESSAGE);
      break;
    case WAITING_FOR_DIFS:
      _collision_detected = true;

      //std::cout << SIMUL.getTime() << ": incoming transmission while waiting for DIFS: " << std::endl;

      // Checks for the longest incoming message
      if (_data_received_evt.isInQueue()) {

        //std::cout << SIMUL.getTime() << ": there was a transmission already: " << _data_received_evt.getTime() << std::endl;

        //std::cout << SIMUL.getTime() << ": Last _data_received_evt at: " << _data_received_evt.getTime() << std::endl;
        //std::cout << SIMUL.getTime() << ": Next _data_received_evt at: " << SIMUL.getTime() + m->transTime() << std::endl;

        if (_data_received_evt.getTime() < SIMUL.getTime() + m->transTime()) {
          _incoming_message = m;
          _data_received_evt.drop();
          _data_received_evt.post(SIMUL.getTime() + m->transTime());
        }
      } else {
        _incoming_message = m;
        _data_received_evt.post(SIMUL.getTime() + m->transTime());
      }
      break;
    case WAITING_FOR_BACKOFF:
      // Timer is decremented only when channel is sensed idle
      b = _wait_for_backoff_evt.getTime();
      /*
      std::cout << SIMUL.getTime()
                << " : Timer is decremented only when channel is sensed idle: "
                << b << std::endl;
*/
      _wait_for_backoff_evt.drop();
      _wait_for_backoff_evt.post(b + m->transTime());
      break;
    case WAITING_FOR_SIFS:
      break;
    default: break;
  }
}

WifiInterfaceStatus WifiInterface::status()
{
  return _status;
}

void WifiInterface::status(WifiInterfaceStatus s)
{
  /*
  std::cout << SIMUL.getTime()
            << "\t: "
            << this->node()->getName()
            << "\t: "
            << status2string(status())
            << " -> "
            << status2string(s)
            << std::endl;
            */

  _status = s;

  if (_wifiTrace) {
    std::stringstream ss;
    ss << SIMUL.getTime()
       << "\t"
       << getName()
       << "\t"
       << status2string(status())
       << std::endl;

    std::cout << ss.str();
    _wifiTrace->record(ss.str().c_str());
  }
}

std::string WifiInterface::status2string(WifiInterfaceStatus s)
{
  switch(s) {
    case IDLE:
      return "IDLE";
      break;
    case WAITING_FOR_DIFS:
      return "WAITING_FOR_DIFS";
      break;
    case WAITING_FOR_SIFS:
      return "WAITING_FOR_SIFS";
      break;
    case WAITING_FOR_BACKOFF:
      return "WAITING_FOR_BACKOFF";
      break;
    case WAITING_FOR_ACK:
      return "WAITING_FOR_ACK";
      break;
    case SENDING_MESSAGE:
      return "SENDING_MESSAGE";
      break;
    case SENDING_ACK:
      return "SENDING_ACK";
      break;
    case RECEIVING_MESSAGE:
      return "RECEIVING_MESSAGE";
      break;
    default:
      return "!!! Status: unknown !!!";
      break;
  }
}
