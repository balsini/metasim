#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include <metasim.hpp>

#define _MESSAGE_DBG "EthernetLink"

class Node;
class NetInterface;

#include "netinterface.hpp"

class Message
{
  int _len;

  //  partito dal nodo e arrivato all'interfaccia
  MetaSim::Tick _start_time;
  // partito dall'interfaccia e arrivato sul link
  MetaSim::Tick _trans_time;
  // fine della trasmission, arrivo sull'interfaccia destinazione
  MetaSim::Tick _arr_time;

  Node *_dst;
  Node *_src;

  bool _ack;
  NetInterface *_dst_net_interf;
  NetInterface *_src_net_interf;

public:
  Message(int l, Node * s, Node * d, bool ack = false);

  void setStartTime(MetaSim::Tick t);
  MetaSim::Tick getStartTime();

  void setTransTime(MetaSim::Tick t);
  MetaSim::Tick getTransTime();

  void setArrTime(MetaSim::Tick t);
  MetaSim::Tick getArrTime();

  int length();

  bool isACK() { return _ack; }

  void sourceInterface(NetInterface *n);
  void destInterface(NetInterface *n);

  NetInterface *sourceInterface();
  NetInterface *destInterface();

  Node *sourceNode();
  Node *destNode();
};

#endif
