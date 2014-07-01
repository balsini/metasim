#include "experiment.hpp"

Experiment::Experiment()
{}

/**
 * For each node, checks if other nodes are in range.
 * The neighbours are added to the node link,
 * to generate its broadcast domain
 */
void Experiment::generateLinks()
{
  std::cout << "\n\nGenerating Links\n\n" << std::endl;

  for (auto n : _nodes) {
    std::cout << "Node: " << n->getName() << std::endl;
    for (auto other : _nodes) {
      if (other != n) {
        double d;
        d = sqrt(pow(std::get<0>(n->position()) - std::get<0>(other->position()), 2)
                 + pow(std::get<1>(n->position()) - std::get<1>(other->position()), 2));

        std::cout << "\t" << other->getName() << " -- distance: " << d << std::endl;
        if (d < n->radius()) {
          std::cout << "\t\tIN RANGE" << std::endl;
          n->netInterface()->link()->addNode(other);
        }
      }
    }
  }
}

/**
 * Creates a node, its interface and the link of the interface
 * @param p node position.
 * @param radius node radius.
 */
void Experiment::addNode(std::pair <int,int> p, double radius)
{
  ///////////////////////////
  // Node Creation
  ///////////////////////////

  static unsigned int nodeId = 0;
  std::string name = "Node_";

  name.append(std::to_string(++nodeId));

  std::cout << "Creating node:\t\t" << name << std::endl;
  std::cout << "\tPosition:\t[ " << p.first << " , " << p.second << " ]" << std::endl;
  std::cout << "\tRadius:\t\t" << radius << std::endl;

  Node * n = new Node(name, p, radius);
  _nodes.push_back(n);

  ///////////////////////////
  // Node Interface creation
  ///////////////////////////

  std::string interfaceName = "Interface_";
  interfaceName.append(name);

  std::cout << "Creating Interface: " << interfaceName << std::endl;

  WifiInterface * n_int = new WifiInterface(interfaceName, n);

  n->netInterface(n_int);

  ///////////////////////////
  // Node Link creation
  ///////////////////////////

  std::string linkName = "Link_";
  linkName.append(name);

  std::cout << "Creating Link: " << linkName << std::endl;

  WifiLink * n_l = new WifiLink(linkName);
  n_int->link(n_l);
}

/*
 * Prepares all the data and starts the experiment
 */
void Experiment::start()
{
  generateLinks();
}
