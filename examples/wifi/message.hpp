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
  int _id;

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
  Message(int l, Node * s, Node * d, int id, bool ack = false);

  void setTransTime(MetaSim::Tick t);
  MetaSim::Tick getTransTime();

  int length();

  bool isACK() { return _ack; }
  int id() { return _id; }

  void sourceInterface(NetInterface *n);
  void destInterface(NetInterface *n);

  NetInterface *sourceInterface();
  NetInterface *destInterface();

  Node *sourceNode();
  Node *destNode();
};

#endif
