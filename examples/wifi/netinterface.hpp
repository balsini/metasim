#ifndef __NETINTERFACE_HPP__
#define __NETINTERFACE_HPP__

#include <deque>
#include <string>

#include <metasim.hpp>

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
  SENDING_MESSAGE,
  RECEIVING_MESSAGE
};

class NetInterface : public MetaSim::Entity
{
protected:
  Node *_node;

public:
  NetInterface(const std::string &name, Node * const &n);
  virtual ~NetInterface();

  virtual Node * node() = 0;
  virtual void send(Message * m) = 0;
  virtual void receive(Message * n) = 0;
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
 *   --- if no ACK is received, the backoff time is incremented.
 * The receiver:
 *   If the frame has been received, an ACK is sent after SIFS time.
 *   This ACK is used to solve the hidden terminal problem.
 *
 * The backoff time is a random value taken from the backoff
 * interval. This interval is [0, c_w-1], where c_w is initialized
 * with c_wMin and, if no ACK is received, the interval is updated
 * as c_w = min[2*c_w, c_wMax].
 *
 * DIFS = SIFS + (2 * Slot time)
 *
 * In this case, will be taken as example the IEEE 802.11n protocol,
 * with DIFS = 28 and SIFS = 10.
 */
class WifiInterface : public NetInterface
{
  /**
   * Sends ACK message to sender interface
   * @param m ACK message
   */
  void sendACK(Message * m);

protected:
  std::deque<Message *> _out_queue;
  std::deque<Message *> _ack_queue;

  const int _c_wMin = 10;
  const int _c_wMax = 10;
  const int _DIFS = 28;
  const int _SIFS = 10;

  bool _collision_detected;
  int _backoff_timer;
  int _c_w;

  Message * _incoming_message;
  WifiLink * _link;
  double _radius;

  WifiInterfaceStatus _status;

  unsigned int _corrupted_messages;
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
   * Transmitting data
   */
  MetaSim::GEvent<WifiInterface> _start_trans_evt;
  /**
   * Data transmission completed
   */
  MetaSim::GEvent<WifiInterface> _end_trans_evt;
  /**
   * Waiting for SIFS to send the ACK
   */
  MetaSim::GEvent<WifiInterface> _wait_for_SIFS_evt;

  WifiInterface(const std::string &name, double radius, Node * n);
  virtual ~WifiInterface();

  void link(WifiLink * l);
  WifiLink * link();

  double radius() { return _radius; }
  Node * node() { return _node; }
  MetaSim::Tick nextTransTime();

  virtual void onStartTrans(MetaSim::Event * e);
  virtual void onDIFSElapsed(MetaSim::Event * e);
  virtual void onSIFSElapsed(MetaSim::Event * e);
  virtual void onEndTrans(MetaSim::Event * e);

  WifiInterface * routingProtocol(Node * n);
  void node(Node * n) {_node = n; }
  /**
   * Node calls this function, requesting the interface
   * to send the message.
   * @param m message to be sent
   */
  virtual void send(Message * m);
  /**
   * Link calls this function, meaning that the message is
   * sent from the link to the interface.
   * @param m received message
   */
  virtual void receive(Message * m);
  /**
   * This function is called when the transmission is completed.
   * Collisions are checked here.
   * @param m
   */
  void onMessageReceived(MetaSim::Event * e);
  void onMessageSent(MetaSim::Event * e);

  void printStatus();

  void newRun();
  void endRun();
};

#endif
