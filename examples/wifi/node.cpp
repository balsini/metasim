#include <metasim.hpp>

#include "node.hpp"

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

void Node::setInterval(const std::shared_ptr<RandomVar> &i)
{
  _at = i;
}

int Node::consumed()
{
  return _consumed;
}

Source::Source(const std::string &n,
               std::pair <double, double> position,
               const std::shared_ptr<RandomVar> &a) :
  Node(n, position),
  _produced(0),
  _prodEvent(*this)
{
  _at = a;
}

std::pair <double, double> Node::position()
{
  return _position;
}

void Node::netInterface(WifiInterface * n)
{
  _net_interf = n;
}

WifiInterface * Node::netInterface()
{
  return _net_interf;
}

void Source::newRun()
{
  //std::cout << "Source::newRun()" << std::endl;
  _produced = 0;
  _prodEvent.post(Tick(_at->get()));
}

void Source::produce()
{
  if (_produced < 100) {
    if (_dest.size() == 0)
      throw NodeException("Node messages has no destination");
    if (_net_interf == nullptr)
      throw NodeException("Node has no Network Interface");

    UniformVar toRand(0, _dest.size());
    UniformVar idRand(0, 32767);
    Node * destinationNode = _dest.at(toRand.get());

    //std::cout << this->getName() << ": sending message to: " <<  destinationNode->getName() << std::endl;
    int msgLen = 512;

    auto m = unique_ptr<Message>(new Message(msgLen, this, destinationNode, idRand.get()));
    m->transTime(Tick(msgLen));

    _net_interf->send(m);
    _produced++;
    _prodEvent.post(SIMUL.getTime() + Tick(_at->get()));
  }
}

void Node::put(Message * m)
{
  _consumed++;
  //std::cout << this->getName() << ": received message from: " << m->sourceNode()->getName() << std::endl;
}

void Source::addDest(Node * n)
{
  _dest.push_back(n);
}

int Source::produced()
{
  return _produced;
}
