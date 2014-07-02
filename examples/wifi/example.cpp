#include <simul.hpp>

#include "experiment.hpp"

#include "link.hpp"
#include "message.hpp"
#include "netinterface.hpp"
#include "node.hpp"

using namespace MetaSim;

const int ROWS = 10;
const int COLUMNS = 10;

const double UMIN = 0.5;
const double UMAX = 0.5;
const double USTEP = 0.1;

const unsigned AVG_LEN = 800;
const Tick SIM_LEN = (int) (AVG_LEN * 1000);

int main()
{
  Experiment experiment;

  std::cout << std::endl;
  std::cout << "\\¯¯\\    /¯¯/ (O)   |¯¯¯¯¯¯| (O)\n";
  std::cout << " \\  \\/\\/  / |¯¯¯¯| |   ¯¯| |¯¯¯¯|\n";
  std::cout << "  \\__/\\__/ '|____| |__|¯¯  |____| MetaSim";
  std::cout << std::endl;

  /***********************************
   *                                 *
   *        Generating nodes         *
   *                                 *
   ***********************************/
  std::cout << "\nCreating Nodes ..." << std::endl;

  ExponentialVar at(20);

  // First, generates the nodes inside the matrix

  for (int c=1; c<COLUMNS-1; ++c) {
    for (int r=1; r<ROWS-1; ++r)
      experiment.addNode(std::make_pair(r, c), 1.1);
  }

  // Now, generates the sinks and associates them to
  // the sources

  for (int c=1; c<COLUMNS-1; ++c) {
    Node * n = experiment.addNode(std::make_pair(ROWS-1, c),
                                  1.1);
    Source * s = experiment.addNode(std::make_pair(0, c),
                       1.1,
                       &at);
    s->addDest(n);
  }
  for (int r=1; r<ROWS-1; ++r) {
    Node * n = experiment.addNode(std::make_pair(r, COLUMNS-1),
                                  1.1);
    Source * s = experiment.addNode(std::make_pair(r, 0),
                       1.1,
                       &at);
    s->addDest(n);
  }


  Node * n1 = experiment.addNode(std::make_pair(COLUMNS-1, 0),
                                1.1);
  Node * n2 = experiment.addNode(std::make_pair(ROWS-1, 0),
                                1.1);
  Source * s = experiment.addNode(std::make_pair(0, 0),
                     1.1,
                     &at);
  s->addDest(n1);
  s->addDest(n2);

  Node * nrc = experiment.addNode(std::make_pair(ROWS-1, COLUMNS-1),
                                1.1);

  Source * s1 = experiment.addNode(std::make_pair(ROWS-1, 0),
                     1.1,
                     &at);
  Source * s2 = experiment.addNode(std::make_pair(0, COLUMNS-1),
                     1.1,
                     &at);

  s1->addDest(nrc);
  s2->addDest(nrc);

  std::cout << "DONE" << std::endl;

  /***********************************
   *                                 *
   *     Running the experiment      *
   *                                 *
   ***********************************/

  experiment.start(UMIN, UMAX, USTEP, AVG_LEN, SIM_LEN);
}
