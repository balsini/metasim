#include "message.hpp"

Message::Message(int l, Node * s, Node * d, int id, bool ack) :
  _len(l),
  _id(id),
  _ack(ack),
  _trans_time(0),
  _dst(d),
  _src(s),
  _dst_net_interf(0),
  _src_net_interf(0)
{}

Message::Message(const Message &other)
{
  _len = other.length();
  _id = other.id();
  _ack = other.isACK();
  _trans_time = other.transTime();
  _dst = other.destNode();
  _src = other.sourceNode();
  _dst_net_interf = other.destInterface();
  _src_net_interf = other.sourceInterface();
}

void Message::transTime(Tick t)
{
  _trans_time = t;
}

Tick Message::transTime() const
{
  return _trans_time;
}

int Message::length() const
{
  return _len;
}

void Message::sourceInterface(NetInterface *n)
{
  _src_net_interf = n;
}

void Message::destInterface(NetInterface *n)
{
  _dst_net_interf = n;
}

NetInterface * Message::sourceInterface() const
{
  return _src_net_interf;
}

NetInterface * Message::destInterface() const
{
  return _dst_net_interf;
}

Node * Message::sourceNode() const
{
  return _src;
}

Node * Message::destNode() const
{
  return _dst;
}
