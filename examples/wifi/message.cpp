#include "message.hpp"

Message::Message(int l, Node *s, Node *d, int id, bool ack) :
  _len(l),
  _id(id),
  _ack(ack),
  _trans_time(0),
  _dst(d),
  _src(s)
{}

void Message::setTransTime(Tick t)
{
  _trans_time = t;
}

Tick Message::getTransTime()
{
  return _trans_time;
}

int Message::length()
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

NetInterface * Message::sourceInterface()
{
  return _src_net_interf;
}

NetInterface * Message::destInterface()
{
  return _dst_net_interf;
}

Node * Message::sourceNode()
{
  return _src;
}

Node * Message::destNode()
{
  return _dst;
}
