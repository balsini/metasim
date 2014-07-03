#ifndef __NODE_HPP__
#define __NODE_HPP__

#include <memory>
#include <vector>
#include <string>

#include "link.hpp"
#include "message.hpp"
#include "netinterface.hpp"

#include <metasim.hpp>

using namespace MetaSim;

class Message;
class WifiInterface;

#define _NODE_DBG "Node"

/**
 * The Node class.
 * The Node receives messages only if it is the
 * message's destination.
 */
class Node : public Entity
{
  int _consumed;

protected:
  WifiInterface * _net_interf;
  std::auto_ptr<RandomVar> _interval;
  std::pair <double, double> _position;

public:
  GEvent<Node> _recv_evt;

  /**
   * Node creator
   * @param name name of the node
   * @param position position of the node
   * @param radius radius of the node
   */
  Node(const std::string &name,
       std::pair <double, double> position);

  /**
   * Gets node position
   * @return node position
   */
  std::pair <double, double> position() { return _position; }

  /**
   * Gets node radius
   * @return node radius
   */
  double radius();

  /**
   * The net interface connected to the node
   * @returns pointer to the node interface
   */
  WifiInterface * netInterface() { return _net_interf; }
  /**
   * Sets the node's wifi network interface
   * @param n WifiInterface pointer
   */

  void netInterface(WifiInterface * n) { _net_interf = n; }

  /**
   * Sets the time interval
   * @param i time interval random value
   */
  void setInterval(std::auto_ptr<RandomVar> i) { _interval = i; }

  virtual void put(Message * m) { _consumed++; }

  void onMessageReceived(Message * m);

  virtual void newRun();
  virtual void endRun() {}
};

/**
 * Source node.
 * It produces messages at random times, with random lengths.
 * Each message is pushed into node's wireless interface.
 */
class Source : public Node
{
  /**
   * @brief _at message production speed
   */
  RandomVar * _at;
  /**
   * @brief _produced number of produced messages
   */
  int _produced;

  /**
   * @brief _dest destination nodes
   */
  std::vector<Node *> _dest;

public:
  /**
   * @brief The ProduceEvent class
   */
  class ProduceEvent : public Event
  {
    /**
     * @brief _n destination node
     */
    Source &_n;
  public:
    /**
     * @brief ProduceEvent constructor
     * @param n destination node
     */
    ProduceEvent(Source &n) : Event(), _n(n) {}
    /**
     * @brief doit Performs the action: produces
     *   and sends message
     */
    virtual void doit() { _n.produce(); }
  };

  /**
   * @brief _prodEvent event generator
   */
  ProduceEvent _prodEvent;

  /**
   * Source constructor
   * @param d destination node
   * @param position node position
   * @param a message production speed
   * @param n node name
   */
  Source(const std::string &n,
         std::pair <double, double> position,
         RandomVar * a) :
    Node(n, position),
    _at(a),
    _produced(0),
    _prodEvent(*this) {}

  /**
   * Adds a new destination node
   * @param n destination node
   */
  void addDest(Node * n) { _dest.push_back(n); }

  /**
   * Produces a message and sends it to the
   * network interface
   */
  void produce();

  virtual void newRun();
  virtual void endRun() {}
};

#endif
