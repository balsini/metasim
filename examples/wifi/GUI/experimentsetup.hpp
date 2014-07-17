#ifndef EXPERIMENTSETUPGUI_HPP
#define EXPERIMENTSETUPGUI_HPP

#include <vector>
#include <string>
#include <sstream>

#include <QDebug>

enum ExperimentKind_t {
  SQUARE
};
enum NodeKind_t {
  SOURCE,
  NODE
};

class ExperimentSetup
{
  struct Experiment_t {
    ExperimentKind_t kind;
    unsigned int size;

    unsigned int sideMin;
    unsigned int sideMax;

    double distance;
    double range;

    unsigned int periodMin;
    unsigned int periodStep;
    unsigned int periodMax;
  };

  struct Node_t {
    NodeKind_t kind;
    double posX;
    double posY;
    double range;
  };

  Experiment_t experiment;

public:
  ExperimentSetup();
  /*
  void addNode(NodeKind_t kind, double posX, double posY, double range)
  {
    Node_t n;
    n.kind = kind;
    n.posX = posX;
    n.posY = posY;
    n.range = range;
    nodevector.push_back(n);
  }
  */

  void parseParameters(const std::string &s)
  {
    qDebug() << "Parsing: " << s.c_str();

    std::stringstream ss(s);
    std::string expkind;

    ss >> expkind;

    //qDebug() << "Kind: " << expkind.c_str();

    if (expkind == "SQUARE") {
      experiment.kind = SQUARE;
      ss >> experiment.sideMin;
      ss >> experiment.sideMax;
      ss >> experiment.distance;
      ss >> experiment.range;
      ss >> experiment.periodMin;
      ss >> experiment.periodStep;
      ss >> experiment.periodMax;
      experiment.size = experiment.sideMax - experiment.sideMin + 1;
    }
  }

  std::string getParameters()
  {
    //qDebug() << "Parsing: " << s.c_str();

    std::stringstream ss;
    //qDebug() << "Kind: " << expkind.c_str();

    if (experiment.kind == SQUARE) {
      ss << "SQUARE" << ' '
         << experiment.sideMin << ' '
         << experiment.sideMax << ' '
         << experiment.distance << ' '
         << experiment.range << ' '
         << experiment.periodMin << ' '
         << experiment.periodStep << ' '
         << experiment.periodMax << '\n';
    }

    qDebug() << "Writing to file: " << ss.str().c_str();

    return ss.str();
  }

  unsigned int size()
  {
    return experiment.size;
  }

  std::vector<Node_t> nodes(unsigned int i)
  {
    std::vector<Node_t> nodevector;

    if (experiment.kind == SQUARE) {
      int COLUMNS_MIN = experiment.sideMin;
      //int ROWS_MIN = COLUMNS_MIN;

      //int COLUMNS_MAX = experiment.sideMax;
      //int ROWS_MAX = COLUMNS_MAX;

      double distance = experiment.distance;
      double radius = experiment.range;


      unsigned int COLUMNS = COLUMNS_MIN + i;
      unsigned int ROWS = COLUMNS;

      for (unsigned int i=0; i<COLUMNS; ++i) {
        for (unsigned int j=0; j<ROWS; ++j) {
          Node_t n;
          if (i==0 or j==0)
            n.kind = SOURCE;
          else
            n.kind = NODE;
          n.posX = i*distance;
          n.posY = j*distance;
          n.range = radius;
          nodevector.push_back(n);
          //qDebug() << i*distance << j*distance << radius;
        }
      }
    }

    return nodevector;
  }

  Experiment_t exp()
  {
    return experiment;
  }
};

#endif // EXPERIMENTSETUP_HPP
