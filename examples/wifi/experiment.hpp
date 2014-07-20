#ifndef __EXPERIMENT_HPP__
#define __EXPERIMENT_HPP__

#include <iostream>

#include <vector>
#include <string>
#include <utility>
#include <cmath>

#include "node.hpp"

class Experiment
{
  unsigned int nodeId;
  bool _traces;
  std::vector<std::unique_ptr<Node>> _nodes;
  /**
   * For each node, checks if other nodes are in range.
   * The neighbours are added to the node link,
   * to generate its broadcast domain
   */
  void generateLinks();

  Node * createNode(std::pair<double, double> p,
                    double radius,
                    unsigned int nodeId,
                    const std::string &name);
  Source * createNode(std::pair <double,double> p,
                      double radius,
                      unsigned int nodeId,
                      const std::string &name,
                      const std::shared_ptr<RandomVar> &a);
  WifiInterface * createInterface(Node * n,
                                  const std::string &name, double radius);
  void createLink(const std::string &name, WifiInterface * n_int);

public:
  Experiment(bool traces = false);
  ~Experiment();

  /**
   * Creates a node, its interface and the link of the interface
   * @param p node position
   * @param radius node radius
   * @return pointer to created object
   */
  Node * addNode(std::pair <double,double> p, double radius);

  /**
   * Creates a source node, its interface and the link of the interface
   * @param p node position
   * @param radius node radius
   * @param a message generation frequency
   * @return pointer to created object
   */
  Source * addNode(std::pair <double,double> p, double radius, const std::shared_ptr<RandomVar> &a);

  /**
   * Prepares all the data and starts the experiment
   */
  void start(unsigned int period_i,
             unsigned int period_s,
             unsigned int period_e,
             Tick SIM_LEN = INT64_MAX);

  const std::vector<std::unique_ptr<Node>> &nodes() { return _nodes; }
};

#endif
