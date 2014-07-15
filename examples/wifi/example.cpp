#include <unistd.h>
#include <string>
#include <sstream>

#include <simul.hpp>

#include "experiment.hpp"

#include "link.hpp"
#include "message.hpp"
#include "netinterface.hpp"
#include "node.hpp"

using namespace MetaSim;



const unsigned AVG_LEN = 800;
const Tick SIM_LEN = (int) (AVG_LEN * 10000);

int main(int argc, char *argv[])
{
  std::string arrangement;
  std::string nodes;
  std::string periodInit;
  std::string periodEnd;
  std::string periodStep;
  int c;

  opterr = 0;
  while ((c = getopt(argc, argv, "a:n:i:e:s:")) != -1) {
    switch (c)
    {
      case 'a': // Arrangement
        arrangement = optarg;
        std::cout << "arrangement: " << arrangement << std::endl;
        break;
      case 'i':
        periodInit = optarg;
        std::cout << "period initial: " << periodInit << std::endl;
        break;
      case 'e':
        periodEnd = optarg;
        std::cout << "period end: " << periodEnd << std::endl;
        break;
      case 's':
        periodStep = optarg;
        std::cout << "period step: " << periodStep << std::endl;
        break;
      case 'n':
        nodes = optarg;
        std::cout << "nodes: " << nodes << std::endl;
        break;
      default:
        abort();
    }
  }

  if (arrangement != "SQUARE"
      or periodInit.length() == 0
      or periodEnd.length() == 0
      or periodStep.length() == 0) {
    std::cerr << "Wrong arguments. Exiting" << std::endl;
    abort();
  }

  int ROWS, COLUMNS;
  int P_STEP, P_MIN, P_MAX;

  istringstream(nodes) >> ROWS;
  istringstream(nodes) >> COLUMNS;

  istringstream(periodEnd) >> P_MAX;
  istringstream(periodInit) >> P_MIN;
  istringstream(periodStep) >> P_STEP;

  Experiment experiment;

  std::cout << std::endl;
  std::cout << "\\¯¯\\    /¯¯/ (  )  |¯¯¯¯¯¯| (  )\n";
  std::cout << " \\  \\/\\/  / |¯¯¯¯| |   ¯¯| |¯¯¯¯|\n";
  std::cout << "  \\__/\\__/  |____| |__|¯¯  |____| MetaSim";
  std::cout << std::endl;

  if (ROWS < 2 || COLUMNS < 2) {
    std::cout << "ROWS should be greater than 1" << std::endl;
    std::cout << "COLUMNS should be greater than 1" << std::endl;
    exit(1);
  }

  /***********************************
   *                                 *
   *        Generating nodes         *
   *                                 *
   ***********************************/
  std::cout << "\nCreating Nodes ..." << std::endl;

  //auto at = std::make_shared<ExponentialVar>(1000);
  auto at = std::make_shared<UniformVar>(1,500);

  // First, generates the nodes inside the matrix

  /************
   *
   * o o o o o
   * o x x x o
   * o x x x o
   * o x x x o
   * o o o o o
   *
   ************/

  for (int c=1; c<COLUMNS-1; ++c) {
    for (int r=1; r<ROWS-1; ++r) {
      //std::cout << "Generated node [" << r << " , " << c << "]" << std::endl;
      experiment.addNode(std::make_pair(r, c), 1.1);
    }
  }

  // Now, generates the sinks and associates them to
  // the sources

  /************
   *
   * o x x x o
   * x o o o x
   * x o o o x
   * x o o o x
   * o x x x o
   *
   ************/

  for (int c=1; c<COLUMNS-1; ++c) {
    //std::cout << "Generated node [" << ROWS-1 << " , " << c << "]" << std::endl;
    auto n = experiment.addNode(std::make_pair(ROWS-1, c),
                                1.1);

    //std::cout << "Generated source [" << 0 << " , " << c << "]" << std::endl;
    auto s = experiment.addNode(std::make_pair(0, c),
                                1.1,
                                at);
    s->addDest(n);
  }
  for (int r=1; r<ROWS-1; ++r) {

    //std::cout << "Generated node [" << r << " , " << COLUMNS-1 << "]" << std::endl;
    auto n = experiment.addNode(std::make_pair(r, COLUMNS-1),
                                1.1);

    //std::cout << "Generated source [" << r << " , " << 0 << "]" << std::endl;
    auto s = experiment.addNode(std::make_pair(r, 0),
                                1.1,
                                at);
    s->addDest(n);
  }

  // Now the corners

  /************
   *
   * x o o o x
   * o o o o o
   * o o o o o
   * o o o o o
   * x o o o x
   *
   ************/

  //std::cout << "Generated node [" << ROWS-1 << " , " << COLUMNS-1 << "]" << std::endl;
  auto nbr = experiment.addNode(std::make_pair(ROWS-1, COLUMNS-1),
                                1.1);

  //std::cout << "Generated source [" << 0 << " , " << COLUMNS-1 << "]" << std::endl;
  auto str = experiment.addNode(std::make_pair(0, COLUMNS-1),
                                1.1,
                                at);

  //std::cout << "Generated source [" << ROWS-1 << " , " << 0 << "]" << std::endl;
  auto sbl = experiment.addNode(std::make_pair(ROWS-1, 0),
                                1.1,
                                at);

  //std::cout << "Generated source [" << 0 << " , " << 0 << "]" << std::endl;
  auto stl = experiment.addNode(std::make_pair(0, 0),
                                1.1,
                                at);
  str->addDest(nbr);
  sbl->addDest(nbr);
  stl->addDest(str);
  stl->addDest(sbl);

  std::cout << "DONE" << std::endl;

  /***********************************
   *                                 *
   *     Running the experiment      *
   *                                 *
   ***********************************/

  experiment.start(P_MIN, P_STEP, P_MAX, SIM_LEN);
}
