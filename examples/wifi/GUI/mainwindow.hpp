#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "visualizer.hpp"
#include "schedulingvisualizer.hpp"
#include "aboutwindow.hpp"
#include "toolbar.hpp"
#include "centralwidget.hpp"
#include "addnodewindow.hpp"
#include "experimentsetup.hpp"
#include "experimentmanager.hpp"
#include "statisticsvisualizer.hpp"

#include <experiment.hpp>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

  Ui::MainWindow * ui;

  CentralWidget * centralwidget;
  Visualizer * visualizerwindow;
  SchedulingVisualizer * schedulingvisualizerwindow;
  StatisticsVisualizer * statisticsvisualizerwindow;
  AboutWindow * aboutwindow;
  ToolBar * toolbar;
  AddNodeWindow * addnodewindow;

  QString filename;

  ExperimentSetup experimentsetup;

  //std::shared_ptr<Experiment> experiment;

  void updateTitle();
  void saveTo(const QString &tmpfilename);

private slots:
  void on_actionGraphics_Visualizer_triggered();
  void on_actionAbout_triggered();
  void on_actionHelp_triggered();
  void on_actionQuit_triggered();
  void on_actionNew_triggered();

  void on_actionRun_triggered();

  void on_actionNetInterfaces_Trace_triggered();

public:
  explicit MainWindow(QWidget *parent = 0);
  QAction ** getActionNew();

  QAction ** getActionOpen();
  QAction ** getActionSave();
  QAction ** getActionSaveAs();

  QAction ** getActionAddNodes();

  QAction ** getActionRunExperiment();

  QAction ** getActionShowGraphics();
  QAction ** getActionShowTraces();

  ~MainWindow();

public slots:
  void on_actionOpen_triggered();
  void on_actionSave_triggered();
  void on_actionSave_As_triggered();
  void on_actionAdd_Nodes_triggered();
  void on_experimentChanged();
};

#endif // MAINWINDOW_H
