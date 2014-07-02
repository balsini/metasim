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

class NetInterface : public MetaSim::Entity
{
protected:
  Node *_node;

public:
  NetInterface(const std::string &name, Node * const &n);
  virtual ~NetInterface();

  virtual void send(Message * m) = 0;
  virtual Message * receive(Node * n) = 0;

  virtual void onMessageSent(Message * m) = 0;
  virtual void onMessageReceived(Message * m) = 0;
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
protected:
  std::deque<Message *> _out_queue;

  const int _c_wMin = 10;
  const int _c_wMax = 10;
  const int _DIFS = 28;
  const int _SIFS = 10;

  int _backoff_timer;
  int _c_w;

  WifiLink * _link;
  double _radius;

public:
  /**
   * A new message is willing to be sent, so DIFS has to be waited.
   */
  MetaSim::GEvent<WifiInterface> _wait_for_DIFS_evt;
  /**
   * After DIFS, if channel was not idle, a backoff time has to
   * be waited.
   */
  MetaSim::GEvent<WifiInterface> _wait_for_backoff_evt;
  /**
   * Some other node is transmitting.
   */
  MetaSim::GEvent<WifiInterface> _collision_evt;
  /**
   * Transmitting data.
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
  MetaSim::Tick nextTransTime();

  virtual void onCollision();
  virtual void onTransmit(MetaSim::Event * e);
  virtual void onStartTrans(MetaSim::Event * e);
  virtual void onDIFSElapsed(MetaSim::Event * e);
  //virtual void onEndTrans(MetaSim::Event * e);

  virtual void send(Message * m);
  virtual Message * receive(Node * n);
  virtual void onMessageSent(Message * m);
  virtual void onMessageReceived(Message * m);

  void newRun();
  void endRun();
};

#endif
