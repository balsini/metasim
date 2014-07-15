#include <algorithm>

#include <metasim.hpp>

#include <sstream>

#include "link.hpp"
#include "message.hpp"
#include "netinterface.hpp"
#include "node.hpp"

using namespace MetaSim;

NetInterface::NetInterface(const std::string &name, Node * n) :
  Entity(name), _node(n)
{}

NetInterface::~NetInterface()
{}

WifiInterface::WifiInterface(const std::string &name, double radius, Node * n) :
  NetInterface(name, n),
  _collision_detected(false),
  _status(IDLE),
  _corrupted_messages(0),
  _waitingForAck(false),
  _wait_for_DIFS_evt(),
  _wait_for_backoff_evt(),
  _data_received_evt(),
  _end_trans_evt(),
  _wait_for_SIFS_evt()
{
  register_handler(_wait_for_DIFS_evt, this, &WifiInterface::onDIFSElapsed);
  register_handler(_wait_for_SIFS_evt, this, &WifiInterface::onSIFSElapsed);
  register_handler(_data_received_evt, this, &WifiInterface::onDataReceived);
  register_handler(_end_trans_evt, this, &WifiInterface::onEndTrans);
  register_handler(_end_ACKtrans_evt, this, &WifiInterface::onEndACKTrans);
  register_handler(_wait_for_ACK_evt, this, &WifiInterface::onACKTimeElapsed);
  register_handler(_wait_for_backoff_evt, this, &WifiInterface::onBackoffTimeElapsed);

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

void WifiInterface::endRun()
{}

WifiInterface * WifiInterface::routingProtocol(Node * n)
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

void WifiInterface::onEndACKTrans(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": onEndACKTrans" << std::endl;

  _ack_queue.pop_front();

  if (not _data_received_evt.isInQueue())
    trySend();
}

void WifiInterface::onEndTrans(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": onEndTrans" << std::endl;

  status(WAITING_FOR_ACK);

  _waitingForAck = true;
  //_wait_for_ACK_evt.drop();
  //std::cout << "posting event 11" << std::endl;
  _wait_for_ACK_evt.post(SIMUL.getTime() + Tick(_ACK_timeout));
  //std::cout << "DONE" << std::endl;
}

void WifiInterface::onACKTimeElapsed(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": onACKTimeElapsed" << std::endl;

  incrementBackoff();
  // Retry to send the packet.

  _waitingForAck = false;

  status(WAITING_FOR_BACKOFF);
  _backoff_timer_init = SIMUL.getTime();
  _backoff_timer = Tick(getBackoff());

  /*
  std::cout << SIMUL.getTime() << " : Initializes backoff timer to: "
            << SIMUL.getTime() + Tick(getBackoff()) << std::endl;
            */

  if (not _data_received_evt.isInQueue()) {
    //std::cout << "posting event 9" << std::endl;
    //_wait_for_backoff_evt.drop();

    //std::cout << "Posting _wait_for_backoff_evt at: " << SIMUL.getTime() + _backoff_timer << std::endl;
    _wait_for_backoff_evt.post(SIMUL.getTime() + _backoff_timer);
    //std::cout << "DONE" << std::endl;
  }
}

void WifiInterface::onDIFSElapsed(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": onDIFSElapsed" << std::endl;

  if (_collision_detected) {
    _collision_detected = false;
    /*
    std::cout << this->getName() << ": Collision detected, managing backoff!"
              << std::endl;
*/

    //std::cout << SIMUL.getTime() << ": Last _data_received_evt at: " << _data_received_evt.getTime() << std::endl;

    status(WAITING_FOR_BACKOFF);

    // Initializes backoff timer
    _backoff_timer_init = SIMUL.getTime();
    _backoff_timer = Tick(getBackoff());

    if (not _data_received_evt.isInQueue()) {
      //std::cout << "posting event 10" << std::endl;
      _wait_for_backoff_evt.post(SIMUL.getTime() + _backoff_timer);
      //std::cout << "DONE" << std::endl;
    }
  } else {
    /*
    std::cout << this->getName()
              << ": No collision detected, starting transmission! Destination: "
              << _out_queue.front()->destInterface()->getName() << std::endl;
              */

    //_end_trans_evt.drop();

    status(SENDING_MESSAGE);

    //std::cout << "posting event 3" << std::endl;
    _end_trans_evt.post(SIMUL.getTime() + Tick(_out_queue.front()->transTime()));
    //std::cout << "DONE" << std::endl;
    _link->send(_out_queue.front());
  }

  //std::cout << "DONE" << std::endl;
}

void WifiInterface::onSIFSElapsed(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": onSIFSElapsed" << std::endl;

  //_end_ACKtrans_evt.drop();

  status(SENDING_ACK);

  //std::cout << "posting event 4 at: " << SIMUL.getTime() + Tick(_ack_queue.front()->transTime()) << std::endl;
  _end_ACKtrans_evt.post(SIMUL.getTime() + Tick(_ack_queue.front()->transTime()));
  //std::cout << "DONE" << std::endl;
  _link->send(_ack_queue.front());
}

void WifiInterface::onBackoffTimeElapsed(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": onBackoffTimeElapsed" << std::endl;

  //std::cout << _out_queue.size() << std::endl;

  status(SENDING_MESSAGE);

  //_end_trans_evt.drop();

  //std::cout << "posting event 1" << std::endl;
  _end_trans_evt.post(SIMUL.getTime() + Tick(_out_queue.front()->transTime()));
  //std::cout << "DONE" << std::endl;
  _link->send(_out_queue.front());
}

void WifiInterface::onDataReceived(Event * e)
{
  //std::cout << this->getName() << ": onDataReceived" << std::endl;

  switch (status()) {
    case RECEIVING_MESSAGE:

      if (not _collision_detected) {
        if (_incoming_message->destInterface() == this) {
          if (_waitingForAck
              and _incoming_message->isACK()
              and _incoming_message->id() == _out_queue.front()->id()) {

            //std::cout << "Correct ACK received" << std::endl;

            // ACK timer is disabled
            _wait_for_ACK_evt.drop();
            //_wait_for_backoff_evt.drop();

            // No more waiting for ACK
            _waitingForAck = false;
            // Pending message can be destroyed (succesfully sent)
            _out_queue.pop_front();

            trySend();

          } else {
            // A normal message is going to be received

            //std::cout << "\tNormal Message, send ACK after SIFS" << std::endl;

            auto m_ack_unique = std::unique_ptr<Message>(new Message(10,
                                                                     _node,
                                                                     _incoming_message->destInterface()->node(),
                                                                     _incoming_message->id(),
                                                                     true));
            m_ack_unique->transTime(5);
            m_ack_unique->destInterface(_incoming_message->sourceInterface());
            m_ack_unique->sourceInterface(this);

            sendACK(m_ack_unique);

            // Check if it has to be forwarded
            if (_incoming_message->destNode() == _node) {
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
          trySend();
        }
      } else {

        //std::cout << "Collision detected" << std::endl;

        _collision_detected = false;

        if (not _waitingForAck)
          trySend();
      }

      break;

    case WAITING_FOR_BACKOFF:

      // While waiting for backoff, all the messages have been
      // received, so it's now possible to resume the backoff
      // timer

      _backoff_timer_init = SIMUL.getTime();

      //std::cout << "posting event backoff onMessageReceived" << std::endl;
      //_wait_for_backoff_evt.drop();
      _wait_for_backoff_evt.post(SIMUL.getTime() + _backoff_timer);
      //std::cout << "DONE" << std::endl;

      break;

    case SENDING_ACK:

      status(IDLE);

      break;

    default: break;
  }
}

void WifiInterface::sendACK(std::unique_ptr<Message> &m)
{
  //std::cout << this->getName() << ": Awaiting for SIFS" << std::endl;

  status(WAITING_FOR_SIFS);

  _ack_queue.push_back(std::move(m));
  //std::cout << "posting event 5" << std::endl;
  _wait_for_SIFS_evt.post(SIMUL.getTime() + Tick(_SIFS));
  //std::cout << "DONE" << std::endl;
}

void WifiInterface::send(std::unique_ptr<Message> &m)
{
  //std::cout << "Sending message" << std::endl;

  // Updates messages MAC level addresses
  m->sourceInterface(this);
  m->destInterface(routingProtocol(m->destNode()));

  _out_queue.push_back(std::move(m));

  if (status() == IDLE)
    trySend();
}

void WifiInterface::trySend()
{
  //std::cout << "TrySend()" << std::endl;

  if (_out_queue.empty()) {
    status(IDLE);
  } else {
    status(WAITING_FOR_DIFS);

    //std::cout << "posting event 61 at time: " << SIMUL.getTime() + Tick(_DIFS) << std::endl;
    _wait_for_DIFS_evt.post(SIMUL.getTime() + Tick(_DIFS));
    //std::cout << "DONE" << std::endl;
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

  switch (status()) {
    case WAITING_FOR_ACK:
    case IDLE:

      status(RECEIVING_MESSAGE);

      _incoming_message = m;
      //_data_received_evt.drop();
      //std::cout << "posting event 7" << std::endl;
      _data_received_evt.post(SIMUL.getTime() + m->transTime());
      //std::cout << "DONE" << std::endl;

      break;

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
        _incoming_message = m;
        _data_received_evt.drop();
        _data_received_evt.post(SIMUL.getTime() + m->transTime());
      }

      break;

    case WAITING_FOR_BACKOFF:
      // Timer is decremented only when channel is sensed idle

      /*
      std::cout << SIMUL.getTime()
                << " : Timer is decremented only when channel is sensed idle: "
                << b << std::endl;
*/
      if (_wait_for_backoff_evt.isInQueue()) {
        // Need to delete the backoff event and decrement the timer value

        _wait_for_backoff_evt.drop();
        _backoff_timer = _backoff_timer - (SIMUL.getTime() - _backoff_timer_init);
      }

      if (_data_received_evt.isInQueue()) {
        if (_data_received_evt.getTime() < SIMUL.getTime() + m->transTime()) {
          _incoming_message = m;
          _data_received_evt.drop();
          //std::cout << "posting event 87" << std::endl;
          _data_received_evt.post(SIMUL.getTime() + m->transTime());
          //std::cout << "DONE" << std::endl;
        }
      } else {
        //std::cout << "posting event 88" << std::endl;
        _data_received_evt.post(SIMUL.getTime() + m->transTime());
        //std::cout << "DONE" << std::endl;
      }

      break;

    case WAITING_FOR_DIFS:

      //std::cout << SIMUL.getTime() << ": incoming transmission while waiting for DIFS: " << std::endl;
      _collision_detected = true;

    default:

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
        //std::cout << "posting event 8" << std::endl;
        _data_received_evt.post(SIMUL.getTime() + m->transTime());
        //std::cout << "DONE" << std::endl;
      }

      //std::cout << "DONE" << std::endl;

      break;
  }
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

    //if (getName() == "Interface_Node_5_[3,1]") {


/*
    std::cout << ss.str();

      std::cout << "_end_trans_evt :\t";
      if (_end_trans_evt.isInQueue()) std::cout << "ON";
      else std::cout << "OFF";
      std::cout << std::endl;

      std::cout << "_start_trans_evt :\t";
      if (_start_trans_evt.isInQueue()) std::cout << "ON";
      else std::cout << "OFF";
      std::cout << std::endl;

      std::cout << "_wait_for_DIFS_evt :\t";
      if (_wait_for_DIFS_evt.isInQueue()) std::cout << "ON";
      else std::cout << "OFF";
      std::cout << std::endl;

      std::cout << "_wait_for_SIFS_evt :\t";
      if (_wait_for_SIFS_evt.isInQueue()) std::cout << "ON";
      else std::cout << "OFF";
      std::cout << std::endl;

      std::cout << "_data_received_evt :\t";
      if (_data_received_evt.isInQueue()) std::cout << "ON";
      else std::cout << "OFF";
      std::cout << std::endl;

      std::cout << "_end_ACKtrans_evt :\t";
      if (_end_ACKtrans_evt.isInQueue()) std::cout << "ON";
      else std::cout << "OFF";
      std::cout << std::endl;

      std::cout << "_wait_for_ACK_evt :\t";
      if (_wait_for_ACK_evt.isInQueue()) std::cout << "ON";
      else std::cout << "OFF";
      std::cout << std::endl;

      std::cout << "_wait_for_backoff_evt :\t";
      if (_wait_for_backoff_evt.isInQueue()) std::cout << "ON";
      else std::cout << "OFF";
      std::cout << std::endl;
      */
    //}

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
