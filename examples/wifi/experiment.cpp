#include "experiment.hpp"

Experiment::Experiment() :
  nodeId(0)
{}

void Experiment::generateLinks()
{
  double d; // Node distance

  std::cout << "Generating Links ... " << std::endl;

  for (auto n : _nodes) {
    //std::cout << "Node: " << n->getName() << std::endl;
    for (auto other : _nodes) {
      if (other != n) {
        d = sqrt(pow(std::get<0>(n->position()) - std::get<0>(other->position()), 2)
                 + pow(std::get<1>(n->position()) - std::get<1>(other->position()), 2));

        if (d < n->netInterface()->radius()) {
          //std::cout << "\t" << other->getName() << " -- distance: " << d << std::endl;
          //std::cout << "\t\tIN RANGE" << std::endl;
          std::shared_ptr<WifiInterface> interf(other->netInterface());
          n->netInterface()->link()->addInterface(interf);
        }
      }
    }
  }

  std::cout << "DONE" << std::endl;
}

std::shared_ptr<Node> Experiment::createNode(std::pair <int,int> p, double radius, unsigned int nodeId, const std::string &name)
{
  ///////////////////////////
  // Node Creation
  ///////////////////////////

  //std::cout << "Creating node:\t\t" << name << std::endl;
  //std::cout << "\tPosition:\t[ " << p.first << " , " << p.second << " ]" << std::endl;
  //std::cout << "\tRadius:\t\t" << radius << std::endl;

  auto n = std::make_shared<Node>(name, p);
  _nodes.push_back(n);

  return n;
}

std::shared_ptr<Source> Experiment::createNode(std::pair <int,int> p,
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

  auto n = std::make_shared<Source>(name, p, a);
  _nodes.push_back(n);

  return n;
}

std::shared_ptr<WifiInterface> Experiment::createInterface(std::shared_ptr<Node> &n,
                                                           const std::string &name,
                                                           double radius)
{
  ///////////////////////////
  // Node Interface creation
  ///////////////////////////

  std::string interfaceName = "Interface_";
  interfaceName.append(name);

  //std::cout << "Creating Interface: " << interfaceName << std::endl;

  auto n_int = std::make_shared<WifiInterface>(interfaceName, radius, n);

  n->netInterface(n_int);

  return n_int;
}

void Experiment::createLink(const std::string &name, std::shared_ptr<WifiInterface> &n_int)
{
  ///////////////////////////
  // Node Link creation
  ///////////////////////////

  std::string linkName = "Link_";
  linkName.append(name);

  //std::cout << "Creating Link: " << linkName << std::endl;

  auto n_l = std::make_shared<WifiLink>(linkName);
  n_int->link(n_l);
}

std::shared_ptr<Node> Experiment::addNode(std::pair <int,int> p, double radius)
{
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

std::shared_ptr<Source> Experiment::addNode(std::pair <int,int> p, double radius, const std::shared_ptr<RandomVar> &a)
{
  std::string name = "Node_";

  name.append(std::to_string(++nodeId));
  name.append("_[" +
              std::to_string(std::get<0>(p)) +
              "," +
              std::to_string(std::get<1>(p)) +
              "]");

  auto n = createNode(p, radius, nodeId, name, a);
  auto nn = static_pointer_cast<Node>(n);
  auto n_int = createInterface(nn, name, radius);
  createLink(name, n_int);

  return n;
}

void Experiment::start(double UMIN,
                       double UMAX,
                       double USTEP,
                       int AVG_LEN,
                       Tick SIM_LEN)
{
  generateLinks();

  //CollisionStat stat("coll.txt");
  //stat.attach(&link);

  GnuPlotOutput output;
  output.init();

  std::cout << "Simulating ...\n" << std::endl;

  for (double u=UMIN; u<=UMAX; u+=USTEP) {

    double l = 6 * AVG_LEN / u;

    for (auto o : _nodes)
      o->setInterval(std::shared_ptr<RandomVar>(new UniformVar(1,l)));

    //SIMUL.dbg.setStream("log.txt");
    //SIMUL.dbg.enable(_ETHLINK_DBG);
    //SIMUL.dbg.enable(_ETHINTER_DBG);
    //SIMUL.dbg.enable(_NODE_DBG);

    try {
      cout << "U = " << u << endl;
      SIMUL.run(SIM_LEN, 5);
      output.write(u);
    } catch (BaseExc &e) {
      cout << e.what() << endl;
    }
  }
}
