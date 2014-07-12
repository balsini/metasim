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
  std::vector<std::shared_ptr<Node>> _nodes;
  std::shared_ptr<TraceAscii> _interfacesTrace;
  /**
   * For each node, checks if other nodes are in range.
   * The neighbours are added to the node link,
   * to generate its broadcast domain
   */
  void generateLinks();

  const std::shared_ptr<Node> createNode(std::pair <int,int> p,
                                         double radius,
                                         unsigned int nodeId,
                                         const std::string &name);
  const std::shared_ptr<Source> createNode(std::pair <int,int> p,
                                           double radius,
                                           unsigned int nodeId,
                                           const std::string &name,
                                           const std::shared_ptr<RandomVar> &a);
  const std::shared_ptr<WifiInterface> createInterface(const std::shared_ptr<Node> &n,
                                                       const std::string &name, double radius);
  void createLink(const std::string &name, const std::shared_ptr<WifiInterface> &n_int);

public:
  Experiment();

  /**
   * Creates a node, its interface and the link of the interface
   * @param p node position
   * @param radius node radius
   * @return pointer to created object
   */
  const std::shared_ptr<Node> addNode(std::pair <int,int> p, double radius);

  /**
   * Creates a source node, its interface and the link of the interface
   * @param p node position
   * @param radius node radius
   * @param a message generation frequency
   * @return pointer to created object
   */
  const std::shared_ptr<Source> addNode(std::pair <int,int> p, double radius, const std::shared_ptr<RandomVar> &a);

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
