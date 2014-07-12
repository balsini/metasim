#include <metasim.hpp>

#include "node.hpp"

Node::Node(const std::string &name,
           std::pair<double, double> position) :
  Entity(name)
{
  _position = position;
  _consumed = 0;
}

void Node::newRun()
{
  _consumed = 0;
}

void Node::setInterval(const std::shared_ptr<RandomVar> &i)
{
  _period = i;
}

int Node::consumed()
{
  return _consumed;
}

Source::Source(const std::string &n,
               std::pair <double, double> position,
               const std::shared_ptr<RandomVar> &a,
               int packetsToProduce) :
  Node(n, position),
  _produced(0),
  _max_produced(packetsToProduce),
  _prodEvent(*this)
{
  _at = a;
}

std::pair <double, double> Node::position()
{
  return _position;
}

void Node::netInterface(std::shared_ptr<WifiInterface> &n)
{
  _net_interf = n;
}

std::shared_ptr<WifiInterface> Node::netInterface()
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
  if (_produced < _max_produced) {
    if (_dest.size() == 0)
      throw NodeException("Node messages has no destination");
    if (_net_interf == nullptr)
      throw NodeException("Node has no Network Interface");

    _produced++;

    UniformVar toRand(0, _dest.size());
    UniformVar idRand(0, 32767);
    auto destinationNode = _dest.at(toRand.get());

    //std::cout << this->getName() << ": sending message to: " <<  destinationNode->getName() << std::endl;
    int msgLen = 256;

    auto m = unique_ptr<Message>(new Message(msgLen, this, destinationNode.get(), idRand.get()));
    m->transTime(Tick(msgLen));

    _net_interf->send(m);

    if (_produced < _max_produced)
      _prodEvent.post(SIMUL.getTime() + Tick(_period->get()));
  }
}

void Node::put(std::unique_ptr<Message> &m)
{
  _consumed++;
  //std::cout << this->getName() << ": received message from: " << m->sourceNode()->getName() << std::endl;
}

void Source::addDest(std::shared_ptr<Node> n)
{
  _dest.push_back(n);
}

int Source::produced()
{
  return _produced;
}
