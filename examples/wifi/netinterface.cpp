#include <algorithm>

#include <metasim.hpp>

#include "link.hpp"
#include "message.hpp"
#include "netinterface.hpp"
#include "node.hpp"

using namespace MetaSim;

NetInterface::NetInterface(const std::string &name, Node* const &n) :
  Entity(name), _node(n)
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
  register_handler(_start_trans_evt, this, &WifiInterface::onStartTrans);
  register_handler(_wait_for_DIFS_evt, this, &WifiInterface::onDIFSElapsed);
  register_handler(_wait_for_SIFS_evt, this, &WifiInterface::onSIFSElapsed);
  register_handler(_data_received_evt, this, &WifiInterface::onMessageReceived);
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
  _c_w = _c_wMin;
  _ack_queue.clear();
  _out_queue.clear();
  _collision_detected = false;
  _status = IDLE;
  _corrupted_messages = 0;
}

void WifiInterface::link(WifiLink * l)
{
  _link = l;
}

WifiLink * WifiInterface::link()
{
  return _link;
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

void WifiInterface::onEndTrans(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": Transmission completed" << std::endl;

  _status = IDLE;
  _wait_for_ACK_evt.drop();
  _wait_for_ACK_evt.post(SIMUL.getTime() + Tick(_ACK_time));
}

void WifiInterface::onEndACKTrans(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": ACK Transmission completed" << std::endl;

  _status = IDLE;
}

void WifiInterface::onBackoffTimeElapsed(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": Backoff Time Elapsed" << std::endl;

  //std::cout << _out_queue.size() << std::endl;
  _end_trans_evt.drop();
  _end_trans_evt.post(SIMUL.getTime() + Tick(_out_queue.front()->transTime()));

  _link->send(_out_queue.front());
  _status = SENDING_MESSAGE;
}

void WifiInterface::onACKTimeElapsed(MetaSim::Event * e)
{
  //std::cout << this->getName() << ": ACK Timer elapsed!" << std::endl;

  incrementBackoff();
  // Retry to send the packet.
  _status = WAITING_FOR_BACKOFF;
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
    /*
    std::cout << this->getName() << ": Collision detected, managing backoff!"
              << std::endl;
              */
    _status = WAITING_FOR_BACKOFF;

    // Initializes backoff timer
    _wait_for_backoff_evt.drop();
    _wait_for_backoff_evt.post(SIMUL.getTime() + Tick(getBackoff()));
  } else {
    /*
    std::cout << this->getName()
              << ": No collision detected, starting transmission! Destination: "
              << _out_queue.front()->destInterface()->getName() << std::endl;
              */

    _end_trans_evt.drop();
    _end_trans_evt.post(SIMUL.getTime() + Tick(_out_queue.front()->transTime()));
    _link->send(_out_queue.front());
    _status = SENDING_MESSAGE;
  }
}

void WifiInterface::onSIFSElapsed(MetaSim::Event * e)
{
  /*
  std::cout << this->getName() << ": SIFS elapsed! Sending ACK to "
            << _ack_queue.front()->destInterface()->getName() << std::endl;
*/
  //_end_ACKtrans_evt.drop();
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

  _status = IDLE;
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

  /*
  std::cout << this->getName() << ": Message received from "
            << _incoming_message->sourceInterface()->getName()
            << std::endl;
            */

  if (_collision_detected) {
    //std::cout << "\tCollision Detected, dropping" << std::endl;
    _collision_detected = false;
    _status = IDLE;
  } else if (_incoming_message->destInterface() == this) {
    // Frame is for me, ACK must be sent

    //std::cout << "\tCorrect Interface" << std::endl;
    if (_incoming_message->isACK()) {
      //std::cout << "\tACK received" << std::endl;
      if (_incoming_message->id() == _out_queue.front()->id()) {
        //std::cout << "\tACK is correct" << std::endl;
        _wait_for_ACK_evt.drop();
        _out_queue.pop_front();
      }
      _status = IDLE;
    } else {
      //std::cout << "\tNormal Message" << std::endl;

      Message * m_ack = new Message(10,
                                    _node,
                                    _incoming_message->destInterface()->node(),
                                    _incoming_message->id(),
                                    true);
      auto m_ack_unique = std::unique_ptr<Message>(m_ack);
      m_ack_unique->destInterface(_incoming_message->sourceInterface());
      m_ack_unique->sourceInterface(this);

      sendACK(m_ack_unique);

      // Check if it has to be forwarded
      if (_incoming_message->destNode() == _node) {
        // Message is for my node
        //_node->put(_incoming_message);
        _status = IDLE;
      } else {
        // Message must be forwarded
        auto out_msg = std::unique_ptr<Message>(_incoming_message);
        send(out_msg);
      }
    }
  } else {
    //std::cout << "\tWrong Interface, dropping" << std::endl;
    _status = IDLE;
  }

  //if (_status == IDLE)
  //  trySend();
}

void WifiInterface::sendACK(std::unique_ptr<Message> &m)
{
  //std::cout << this->getName() << ": Awaiting for SIFS" << std::endl;
  _ack_queue.push_back(std::move(m));

  _status = WAITING_FOR_SIFS;
  _wait_for_SIFS_evt.post(SIMUL.getTime() + Tick(_SIFS));
}

void WifiInterface::send(std::unique_ptr<Message> &m)
{
  DBGENTER(_ETHINTER_DBG);

  //std::cout << "Sending message" << std::endl;

  m->sourceInterface(this);
  m->destInterface(routingProtocol(m->destNode()));

  _out_queue.push_back(std::move(m));

  trySend();
}

void WifiInterface::trySend()
{
  //std::cout << "TrySend()" << std::endl;

  if (_out_queue.size() > 0 && _status == IDLE) {
    _status = WAITING_FOR_DIFS;
    _wait_for_DIFS_evt.post(SIMUL.getTime() + Tick(_DIFS));
  }

  //std::cout << "DONE" << std::endl;
}

void WifiInterface::receive(Message * m)
{
  DBGTAG(_ETHINTER_DBG, getName() + "::get()");

  /*
  std::cout << this->getName() << ": Incoming communication from "
            << m->sourceInterface()->getName() << std::endl;
*/

  MetaSim::Tick b;

  //printStatus();
  switch(_status) {
    case RECEIVING_MESSAGE:
      _collision_detected = true;
      _corrupted_messages++;
      _data_received_evt.drop();
      _data_received_evt.post(SIMUL.getTime() + m->transTime());
    case IDLE:
      _incoming_message = m;
      _data_received_evt.drop();
      _data_received_evt.post(SIMUL.getTime() + m->transTime());
      _status = RECEIVING_MESSAGE;
      break;
    case WAITING_FOR_DIFS:
      _collision_detected = true;
      break;
    case WAITING_FOR_SIFS:
      break;
    case WAITING_FOR_BACKOFF:
      // Timer is decremented only when channel is sensed idle
      b = _wait_for_backoff_evt.getTime();


      //std::cout << SIMUL.getTime()
      //          << " : Timer is decremented only when channel is sensed idle: "
      //          << b << std::endl;

      _wait_for_backoff_evt.drop();
      _wait_for_backoff_evt.post(b + m->transTime());
      break;
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
