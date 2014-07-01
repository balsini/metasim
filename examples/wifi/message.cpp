#include "message.hpp"
#include "netinterface.hpp"
#include "node.hpp"

using namespace std;
using namespace MetaSim;

Message::Message(int l, Node *s, Node *d) :
  _len(l), _dst(d), _src(s), _dst_net_interf(NULL),
  _src_net_interf(NULL)
{
  _dst_net_interf = _dst->netInterface();
  _src_net_interf = _src->netInterface();
  _start_time = 0;
  _trans_time = 0;
  _arr_time = 0;
}

void Message::setStartTime(Tick t)
{
  _start_time = t;
}

Tick Message::getStartTime()
{
  return _start_time;
}

void Message::setTransTime(Tick t)
{
  _trans_time = t;
}

Tick Message::getTransTime()
{
  return _trans_time;
}

void Message::setArrTime(Tick t)
{
  _arr_time = t;
}

Tick Message::getArrTime()
{
  return _arr_time;
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

NetInterface *Message::sourceInterface()
{
  return _src_net_interf;
}

NetInterface *Message::destInterface()
{
  return _dst_net_interf;
}

Node *Message::sourceNode()
{
  return _src;
}

Node *Message::destNode()
{
  return _dst;
}
