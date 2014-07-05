#ifndef __EXPERIMENT_HPP__
#define __EXPERIMENT_HPP__

#include <iostream>

#include <vector>
#include <string>
#include <utility>
#include <cmath>

#include "node.hpp"

class CollisionStat : public StatCount {
public:
  CollisionStat(const std::string &n) : StatCount(n) {}

  void probe(Event * e)
  {
    record(1);
  }

  void attach(Entity * e)
  {
    WifiInterface * l = dynamic_cast<WifiInterface *>(e);
    if (l == NULL)
      throw BaseExc("Please, specify a Wifi Interface!");

    //l->_collision_evt.addStat(this);
  }
};

class Experiment
{
  unsigned int nodeId;
  std::vector<Node *> _nodes;

  /**
   * For each node, checks if other nodes are in range.
   * The neighbours are added to the node link,
   * to generate its broadcast domain
   */
  void generateLinks();

  Node * createNode(std::pair <int,int> p,
                    double radius,
                    unsigned int nodeId,
                    const std::string &name);
  Source * createNode(std::pair <int,int> p,
                      double radius,
                      unsigned int nodeId,
                      const std::string &name,
                      const std::shared_ptr<RandomVar> &a);
  WifiInterface * createInterface(Node * n,
                                  const std::string &name, double radius);
  void createLink(const std::string &name, WifiInterface * n_int);

public:
  Experiment();

  /**
   * Creates a node, its interface and the link of the interface
   * @param p node position.
   * @param radius node radius.
   */
  Node * addNode(std::pair <int,int> p, double radius);

  Source * addNode(std::pair <int,int> p, double radius, const std::shared_ptr<RandomVar> &a);

  /**
   * Prepares all the data and starts the experiment
   */
  void start(double UMIN,
             double UMAX,
             double USTEP,
             int AVG_LEN,
             Tick SIM_LEN);
};

#endif
