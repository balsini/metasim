#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include <metasim.hpp>

#define _MESSAGE_DBG "EthernetLink"

class Node;
class NetInterface;

#include "netinterface.hpp"
#include "node.hpp"

class Message
{
  /**
   * Message length
   */
  int _len;

  /**
   * Message identifier.
   * A message and its associated ACK message
   * have the same message id
   */
  int _id;

  /**
   * Message type: ACK or normal message
   */
  bool _ack;

  /**
   * Time needed for message transmission
   */
  MetaSim::Tick _trans_time;

  /**
   * Destination node
   */
  Node * _dst;

  /**
   * Source node
   */
  Node * _src;

  /**
   * Destination net interface (MAC level)
   */
  NetInterface * _dst_net_interf;

  /**
   * Source net interface (MAC level)
   */
  NetInterface * _src_net_interf;

public:
  Message(int l,
          Node * s,
          Node * d,
          int id = 0,
          bool ack = false);

  /**
   * Sets the number of ticks needed for message
   * transmission
   * @param t message transmission time (in ticks)
   */
  void setTransTime(MetaSim::Tick t);

  /**
   * Returns the number of ticks required for transmitting
   * the message
   * @return ticks required for message transmission
   */
  MetaSim::Tick getTransTime();

  /**
   * Message length
   * @return the length of the message
   */
  int length();

  /**
   * Checks the message type
   * @return true if the message is an ACK, false if it
   *   is a normal message
   */
  bool isACK() { return _ack; }

  /**
   * Message identifier, needed to link a message to its ACK
   * @return the message id
   */
  int id() { return _id; }

  /**
   * Sets the MAC level source net interface
   * @param n source net interface pointer
   */
  void sourceInterface(NetInterface * n);

  /**
   * Sets the MAC level destination net interface
   * @param n destination net interface pointer
   */
  void destInterface(NetInterface * n);

  /**
   * Gets the MAC level source net interface
   * @param n source net interface pointer
   */
  NetInterface * sourceInterface();

  /**
   * Gets the MAC level destination net interface
   * @param n destination net interface pointer
   */
  NetInterface * destInterface();

  /**
   * Gets the source node
   * @return source node
   */
  Node * sourceNode();

  /**
   * Gets the destination node
   * @return destination node
   */
  Node * destNode();
};

#endif
