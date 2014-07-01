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

  std::vector<Node *> _nodes;

  void generateLinks();

public:
  Experiment();

  void addNode(std::pair <int,int> p, double radius);

  void start();
};

#endif
