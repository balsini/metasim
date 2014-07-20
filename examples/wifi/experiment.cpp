#include "experiment.hpp"

#include <sstream>

std::string int2string(int number)
{
  std::stringstream ss;
  ss << number;

  return ss.str();
}

Experiment::Experiment(bool traces) :
  nodeId(0)
{
  _traces = traces;
  std::cout << "Experiment created" << std::endl;
}

Experiment::~Experiment()
{
  std::cout << "Experiment deleted" << std::endl;
}

void Experiment::generateLinks()
{
  double d; // Node distance

  std::cout << "Generating Links ... " << std::endl;

  for (auto &n : _nodes) {
    //std::cout << "Node: " << n->getName() << std::endl;
    for (auto &other : _nodes) {
      if (other != n) {
        d = sqrt(pow(std::get<0>(n->position()) - std::get<0>(other->position()), 2)
                 + pow(std::get<1>(n->position()) - std::get<1>(other->position()), 2));

        if (d < n->netInterface()->radius()) {
          //std::cout << "\t" << other->getName() << " -- distance: " << d << std::endl;
          //std::cout << "\t\tIN RANGE" << std::endl;
          n->netInterface()->link()->addInterface(other->netInterface());
        }
      }
    }
  }

  std::cout << "DONE" << std::endl;
}

Node * Experiment::createNode(std::pair <double,double> p, double radius, unsigned int nodeId, const std::string &name)
{
  ///////////////////////////
  // Node Creation
  ///////////////////////////

  //std::cout << "Creating node:\t\t" << name << std::endl;
  //std::cout << "\tPosition:\t[ " << p.first << " , " << p.second << " ]" << std::endl;
  //std::cout << "\tRadius:\t\t" << radius << std::endl;

  auto n = new Node(name, p);
  _nodes.push_back(std::move(std::unique_ptr<Node>(n)));

  return n;
}

Source * Experiment::createNode(std::pair<double, double> p,
                                double radius,
                                unsigned int nodeId,
                                const std::string &name,
                                const std::shared_ptr<RandomVar> &a)
{
  ///////////////////////////
  // Node Creation
  ///////////////////////////

  //std::cout << "Creating node:\t\t" << name << std::endl;
  //std::cout << "\tPosition:\t[ " << p.first << " , " << p.second << " ]" << std::endl;
  //std::cout << "\tRadius:\t\t" << radius << std::endl;

  auto n = new Source(name, p, a, 100);
  _nodes.push_back(std::move(std::unique_ptr<Node>(n)));

  return n;
}

WifiInterface * Experiment::createInterface(Node * n,
                                            const std::string &name,
                                            double radius)
{
  ///////////////////////////
  // Node Interface creation
  ///////////////////////////

  std::string interfaceName = "Interface_";
  interfaceName.append(name);

  //std::cout << "Creating Interface: " << interfaceName << std::endl;

  auto n_int = new WifiInterface(interfaceName, radius, n);

  n->netInterface(std::move(std::unique_ptr<WifiInterface>(n_int)));

  return n_int;
}

void Experiment::createLink(const std::string &name, WifiInterface * n_int)
{
  ///////////////////////////
  // Node Link creation
  ///////////////////////////

  std::string linkName = "Link_";
  linkName.append(name);

  //std::cout << "Creating Link: " << linkName << std::endl;

  auto n_l = new WifiLink(linkName);
  n_int->link(std::move(std::unique_ptr<WifiLink>(n_l)));
}

Node * Experiment::addNode(std::pair<double, double> p, double radius)
{
  std::cout << "Node added" << std::endl;

  std::string name = "Node_";

  name.append(std::to_string(++nodeId));
  name.append("_[" +
              std::to_string(std::get<0>(p)) +
              "," +
              std::to_string(std::get<1>(p)) +
              "]");

  auto n = createNode(p, radius, nodeId, name);
  auto n_int = createInterface(n, name, radius);
  createLink(name, n_int);

  return n;
}

Source * Experiment::addNode(std::pair<double, double> p, double radius, const std::shared_ptr<RandomVar> &a)
{
  std::cout << "Source added" << std::endl;

  std::string name = "Node_";

  name.append(std::to_string(++nodeId));
  name.append("_[" +
              std::to_string(std::get<0>(p)) +
              "," +
              std::to_string(std::get<1>(p)) +
              "]");

  auto n = createNode(p, radius, nodeId, name, a);
  auto n_int = createInterface(n, name, radius);
  createLink(name, n_int);

  return n;
}

void Experiment::start(unsigned int id,
                       unsigned int period_i,
                       unsigned int period_s,
                       unsigned int period_e,
                       unsigned int runs,
                       Tick SIM_LEN)
{
  generateLinks();

  std::cout << "Simulating ...\n" << std::endl;

  //std::string statName = ;
  std::stringstream ss;
  ss << "stats/collisions_"
        << id
        << ".dat";
  CollisionStat stat(ss.str().c_str());

  GnuPlotOutput output;
  output.init();

  for (auto &o : _nodes)
    o->netInterface()->addStat(&stat);

  for (unsigned int p = period_i; p <= period_e; p += period_s) {

    for (auto &o : _nodes)
      o->setInterval(std::shared_ptr<RandomVar>(new DeltaVar(p)));

    //SIMUL.dbg.setStream("log.txt");
    //SIMUL.dbg.enable(_ETHLINK_DBG);
    //SIMUL.dbg.enable(_ETHINTER_DBG);
    //SIMUL.dbg.enable(_NODE_DBG);

    try {
      cout << "Period = " << p << endl;

      if (_traces) {
        std::string traceName = "traces/experimentNetInterfacesTrace";
        traceName.append("_-aSQUARE_-n" + int2string(_nodes.size()) + "_-p" + int2string(p));

        WifiTrace interfacesTrace(traceName.c_str());

        for (auto &o : _nodes)
          o->netInterface()->addTrace(&interfacesTrace);

        SIMUL.run(SIM_LEN, runs);
        output.write(p);

        interfacesTrace.close();
      } else {
        SIMUL.run(SIM_LEN, runs);
        output.write(p);
      }
    } catch (BaseExc &e) {
      cout << e.what() << endl;
    }
  }


}
