#ifndef EXPERIMENTMANAGER_HPP
#define EXPERIMENTMANAGER_HPP

#include "experimentsetup.hpp"
#include <experiment.hpp>

class ExperimentManager
{
public:
  ExperimentManager();
  void generateExperiment(ExperimentSetup e_setup, unsigned int experimentIndex, Experiment * experiment);
};

#endif // EXPERIMENTMANAGER_HPP
