#include "experimentmanager.hpp"

void generateExperiment(ExperimentSetup e_setup, unsigned int experimentIndex, Experiment * experiment)
{
  if (e_setup.exp().kind == SQUARE) {
    //auto at = std::make_shared<ExponentialVar>(1000);
    auto at = std::make_shared<UniformVar>(1,2000);

    int COLUMNS = e_setup.exp().sideMin + experimentIndex;
    int ROWS = COLUMNS;

    qDebug() << "Experiment COLUMNS: " << COLUMNS;

    double distance = e_setup.exp().distance;
    double radius = e_setup.exp().range;

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
        experiment->addNode(std::make_pair(r*distance, c*distance), radius);
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
      auto n = experiment->addNode(std::make_pair((ROWS-1)*distance, c*distance),
                                   radius);

      //std::cout << "Generated source [" << 0 << " , " << c << "]" << std::endl;
      auto s = experiment->addNode(std::make_pair(0, c*distance),
                                   radius,
                                   at);
      s->addDest(n);
    }
    for (int r=1; r<ROWS-1; ++r) {

      //std::cout << "Generated node [" << r << " , " << COLUMNS-1 << "]" << std::endl;
      auto n = experiment->addNode(std::make_pair(r*distance, (COLUMNS-1)*distance),
                                   radius);

      //std::cout << "Generated source [" << r << " , " << 0 << "]" << std::endl;
      auto s = experiment->addNode(std::make_pair(r*distance, 0),
                                   radius,
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
    auto nbr = experiment->addNode(std::make_pair((ROWS-1)*distance, (COLUMNS-1)*distance),
                                   radius);

    //std::cout << "Generated source [" << 0 << " , " << COLUMNS-1 << "]" << std::endl;
    auto str = experiment->addNode(std::make_pair(0, (COLUMNS-1)*distance),
                                   radius,
                                   at);

    //std::cout << "Generated source [" << ROWS-1 << " , " << 0 << "]" << std::endl;
    auto sbl = experiment->addNode(std::make_pair((ROWS-1)*distance, 0),
                                   radius,
                                   at);

    //std::cout << "Generated source [" << 0 << " , " << 0 << "]" << std::endl;
    auto stl = experiment->addNode(std::make_pair(0, 0),
                                   radius,
                                   at);
    str->addDest(nbr);
    sbl->addDest(nbr);
    stl->addDest(str);
    stl->addDest(sbl);
  }
}
