#ifndef __NETINTERFACE_HPP__
#define __NETINTERFACE_HPP__

#include <memory>
#include <deque>
#include <string>

#include <metasim.hpp>
#include <trace.hpp>

#include "message.hpp"
#include "link.hpp"
#include "node.hpp"
#include "netinterface.hpp"

class WifiLink;

#define _ETHINTER_DBG "WifiInterface"

enum WifiInterfaceStatus {
  IDLE,
  WAITING_FOR_DIFS,
  WAITING_FOR_SIFS,
  WAITING_FOR_BACKOFF,
  WAITING_FOR_ACK,
  SENDING_MESSAGE,
  SENDING_ACK,
  RECEIVING_MESSAGE
};

class NetInterface : public MetaSim::Entity
{
protected:
  std::shared_ptr<Node> _node;

public:
  NetInterface(const std::string &name, const std::shared_ptr<Node> &n);
  virtual ~NetInterface();

  virtual const std::shared_ptr<Node> node() = 0;
  virtual void send(std::unique_ptr<Message> &m) = 0;
  virtual void receive(const std::shared_ptr<Message> &n) = 0;
};

/**
 * The WifiInterface class is placed between the Node (owner)
 * and the WifiLink classes.
 * From one side, it receives messages from the Node and tries
 * to send them through the link.
 * From the other side, it receives messages from the neighbour
 * interfaces and checks the message destination, so:
 * - if the Node is the correct destination: then the message
 *   is forwarded to it.
 * - otherwise:
 * --- if the destination is such that the message has to be
 *       forwarded, then it is.
 * --- otherwise, the message is ignored.
 *
 * It implements a simple CSMA/CA protocol, so:
 * The sender:
 *   Checks if the channel was free for DIFS time;
 *   - If the condition is verified, the frame is sent;
 *   - Otherwise
 *   --- a timer is activated with a certain backoff time
 *   --- the timer is decremented every time the channel is
 *         found idle;
 *   --- when the timer triggers, the frame is sent;
 *   --- if no ACK is received within ACK_timeout, the backoff time
 *         is incremented.
 * The receiver:
 *   If the frame has been received, an ACK is sent after SIFS time.
 *   This ACK is used to solve the hidden terminal problem.
 *
 * The backoff time is a random value taken from the backoff
 * interval. This interval is [0, c_w-1], where c_w is initialized
 * with c_wMin and, if no ACK is received, the interval is updated
 * as c_w = min[2*c_w, c_wMax].
 *
 * DIFS = SIFS + (2 * SlotTime)
 *
 * In this case, will be taken as example the IEEE 802.11n protocol,
 * with DIFS = 28, SIFS = 10 and SlotTime = 9.
 *
 * ACK_timeout = SIFS + ACKTransmissionDuration + SlotTime
 */
class WifiInterface : public NetInterface
{
  /**
   * Sends ACK message to sender interface
   * @param m ACK message
   */
  void sendACK(std::unique_ptr<Message> & m);
  void incrementBackoff();
  int getBackoff();

protected:
  std::deque<std::unique_ptr<Message>> _out_queue;
  std::deque<std::unique_ptr<Message>> _ack_queue;

  const int _ACKTransmissionDuration = 10;

  const int _c_wMin = 31;
  const int _c_wMax = 1023;

  const int _SlotTime = 9;
  const int _SIFS = 10;
  const int _DIFS = _SIFS + (2 * _SlotTime);

  const int _ACK_timeout = _SIFS + _ACKTransmissionDuration + _SlotTime;

  bool _collision_detected;
  MetaSim::Tick _backoff_timer;
  MetaSim::Tick _backoff_timer_init;
  int _c_w;

  std::shared_ptr<Message> _incoming_message;
  std::shared_ptr<WifiLink> _link;

  double _radius;
  WifiInterfaceStatus _status;
  int _corrupted_messages;
  bool _waitingForAck;

  std::shared_ptr<MetaSim::TraceAscii> _wifiTrace;

  void status(WifiInterfaceStatus s);

  void trySend();

public:
  /**
   * A new message is willing to be sent, so DIFS has to be waited
   */
  MetaSim::GEvent<WifiInterface> _wait_for_DIFS_evt;
  /**
   * After DIFS, if channel was not idle, a backoff time has to
   * be waited
   */
  MetaSim::GEvent<WifiInterface> _wait_for_backoff_evt;
  /**
   * Some other node is transmitting
   */
  MetaSim::GEvent<WifiInterface> _data_received_evt;
  /**
   * Data transmission completed
   */
  MetaSim::GEvent<WifiInterface> _end_trans_evt;
  /**
   * ACK transmission completed
   */
  MetaSim::GEvent<WifiInterface> _end_ACKtrans_evt;
  /**
   * Waiting for SIFS to send the ACK
   */
  MetaSim::GEvent<WifiInterface> _wait_for_SIFS_evt;
  /**
   * Waiting for destination to send back the ACK
   */
  MetaSim::GEvent<WifiInterface> _wait_for_ACK_evt;

  WifiInterface(const std::string &name, double radius, const std::shared_ptr<Node> &n);
  virtual ~WifiInterface();

  void link(const std::shared_ptr<WifiLink> l) { _link = l; }
  const std::shared_ptr<WifiLink> link() { return _link; }
  double radius() { return _radius; }
  virtual const std::shared_ptr<Node> node() { return _node; }

  virtual void onDIFSElapsed(MetaSim::Event * e);
  virtual void onSIFSElapsed(MetaSim::Event * e);
  virtual void onEndTrans(MetaSim::Event * e);
  virtual void onEndACKTrans(MetaSim::Event * e);
  virtual void onDataReceived(MetaSim::Event * e);
  virtual void onACKTimeElapsed(MetaSim::Event * e);
  virtual void onBackoffTimeElapsed(MetaSim::Event * e);

  const std::shared_ptr<WifiInterface> routingProtocol(Node * n);
  /**
   * Node calls this function, requesting the interface
   * to send the message.
   * @param m message to be sent
   */
  virtual void send(std::unique_ptr<Message> &m);
  /**
   * Link calls this function, meaning that the message is
   * sent from the link to the interface.
   * @param m received message
   */
  virtual void receive(const std::shared_ptr<Message> &m);

  std::string status2string(WifiInterfaceStatus s);

  void addTrace(const std::shared_ptr<MetaSim::TraceAscii> &t) { _wifiTrace = t; }
  WifiInterfaceStatus status() { return _status; }

  void newRun();
  void endRun();
};

#endif
