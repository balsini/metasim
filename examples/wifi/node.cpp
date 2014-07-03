#include <metasim.hpp>

#include "node.hpp"

using namespace MetaSim;

/*-----------------------------------------------------*/

Node::Node(const std::string &name,
           std::pair<double, double> position) :
  Entity(name)
{
  _net_interf = nullptr;
  _position = position;
  _consumed = 0;
}

void Node::newRun()
{
  _consumed = 0;
}

void Source::newRun()
{
  //std::cout << "Source::newRun()" << std::endl;
  _produced = 0;
  _prodEvent.post(Tick(_at->get()));
}

void Node::onMessageReceived(Message *m)
{
  // simply, record the fact that the message has succesfully been
  // received.

  DBGTAG(_NODE_DBG, getName() + "::onMessageReceived()");
}

void Source::produce()
{
  UniformVar lenRand(0,2400);
  UniformVar toRand(0, _dest.size());
  UniformVar idRand(0, 32767);
  int to = toRand.get();

  //std::cout << this->getName() << ": sending message to: " <<  _dest.at(to)->getName() << std::endl;
  int msgLen = lenRand.get();

  Message * m = new Message(msgLen, this, _dest.at(to), idRand.get());
  m->setTransTime(Tick(msgLen));

  DBGENTER(_NODE_DBG);
  DBGPRINT("dest node = " << _dest.at(to)->getName());

  _net_interf->send(m);
  _produced++;
  _prodEvent.post(SIMUL.getTime() + Tick(_at->get()));
}



void Node::put(Message * m)
{
  _consumed++;
  //std::cout << this->getName() << ": received message from: " << m->sourceNode()->getName() << std::endl;
}
