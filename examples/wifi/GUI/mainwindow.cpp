#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QDebug>
#include <QFile>

#include <string>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  updateTitle();

  this->showMaximized();

  centralwidget = nullptr;
  visualizerwindow = nullptr;
  schedulingvisualizerwindow = nullptr;
  aboutwindow = nullptr;
  toolbar = nullptr;
  addnodewindow = nullptr;

  aboutwindow = new AboutWindow(this);

  toolbar = new ToolBar(this);
  this->addToolBar(Qt::LeftToolBarArea, toolbar);

  centralwidget = new CentralWidget(this);
  this->setCentralWidget(centralwidget);
  centralwidget->separator();
  centralwidget->append("Experiment GUI Ready");
  centralwidget->separator();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::updateTitle()
{
  QString t = "Wifi Simulator GUI";
  if (filename.length() > 0) {
    t.append(" - ");
    t.append(filename);
  }
  this->setWindowTitle(t);
}

void MainWindow::on_actionGraphics_Visualizer_triggered()
{
  visualizerwindow->show();
}

void MainWindow::on_actionAbout_triggered()
{
  aboutwindow->show();
}

void MainWindow::on_actionHelp_triggered()
{
  QDesktopServices::openUrl(QUrl("https://github.com/balsini/metasim"));
}

void MainWindow::on_actionQuit_triggered()
{
  this->close();
}

void MainWindow::on_actionOpen_triggered()
{
  QString tmpfilename = QFileDialog::getOpenFileName(
                          this,
                          tr("Open File"),
                          "./",
                          "Experiment File (*.xpr)"
                          );

  QFile openFile(tmpfilename);
  if (openFile.open(QIODevice::ReadOnly)) {
    experimentsetup.parseParameters(openFile.readLine().data());

    openFile.close();

    on_actionNew_triggered();
    filename = tmpfilename;
    updateTitle();

    centralwidget->append("Opened: " + filename);

    on_experimentChanged();
  } else {
    qDebug() << "Error while opening file";
  }
}


void MainWindow::saveTo(const QString &tmpfilename) {
  QString name = tmpfilename;

  if (name.mid(name.length() - 4, 4) != ".xpr")
    name.append(".xpr");

  QFile openFile(name);
  if (openFile.open(QIODevice::WriteOnly)) {
    filename = name;

    qDebug() << "Writing to file: " << experimentsetup.getParameters().c_str();

    openFile.write(experimentsetup.getParameters().c_str());

    updateTitle();
    openFile.close();

    centralwidget->append("Saved in: " + filename);
  } else {
    qDebug() << "Error while opening file";
  }
}

void MainWindow::on_actionSave_As_triggered()
{
  QString tmpfilename = QFileDialog::getSaveFileName(
                          this,
                          tr("Save File"),
                          "./",
                          "Experiment File (*.xpr)"
                          );
  saveTo(tmpfilename);
}

void MainWindow::on_actionSave_triggered()
{
  QString tmpfilename = filename;
  if (filename.length() == 0) {
    tmpfilename = QFileDialog::getSaveFileName(
                    this,
                    tr("Save File"),
                    "./",
                    "Experiment File (*.xpr)"
                    );
  }

  saveTo(tmpfilename);
}

void MainWindow::on_actionAdd_Nodes_triggered()
{
  addnodewindow->show();
  //ui->actionAdd_Nodes->setEnabled(false);
}

void MainWindow::on_experimentChanged()
{
  qDebug() << "Experiment changed, updating visualizer";

  centralwidget->append("Experiment configuration generated");

  visualizerwindow = new Visualizer(experimentsetup, this);
  this->addDockWidget(Qt::RightDockWidgetArea/*Qt::AllDockWidgetAreas*/, visualizerwindow);

  visualizerwindow->populate();

  ui->actionRun->setEnabled(true);

  ui->actionSave->setEnabled(true);
  ui->actionSave_As->setEnabled(true);
}

void MainWindow::on_actionNew_triggered()
{
  filename = "";
  updateTitle();

  if (visualizerwindow != nullptr) {
    delete visualizerwindow;
    visualizerwindow = nullptr;
  }
  if (addnodewindow != nullptr) {
    delete addnodewindow;
    addnodewindow = nullptr;
  }
  if (schedulingvisualizerwindow != nullptr) {
    delete schedulingvisualizerwindow;
    schedulingvisualizerwindow = nullptr;
  }

  //experiment = std::make_shared<Experiment>();
  addnodewindow = new AddNodeWindow(experimentsetup, this);

  //ui->actionNew->setEnabled(false);

  ui->actionAdd_Nodes->setEnabled(true);

  centralwidget->append("New Experiment Created");
}

QAction ** MainWindow::getActionOpen()
{
  return &(ui->actionOpen);
}

QAction ** MainWindow::getActionNew()
{
  return &(ui->actionNew);
}

QAction ** MainWindow::getActionSave()
{
  return &(ui->actionSave);
}

QAction ** MainWindow::getActionSaveAs()
{
  return &(ui->actionSave_As);
}

QAction ** MainWindow::getActionRunExperiment()
{
  return &(ui->actionRun);
}

QAction ** MainWindow::getActionAddNodes()
{
  return &(ui->actionAdd_Nodes);
}

QAction ** MainWindow::getActionShowGraphics()
{
  return &(ui->actionGraphics_Visualizer);
}

QAction ** MainWindow::getActionShowTraces()
{
  return &(ui->actionNetInterfaces_Trace);
}

void MainWindow::on_actionRun_triggered()
{
  QStringList filters;
  filters << "*.txt";
  QDir dir;
  dir.cd(dir.currentPath() + "/traces");

  for (auto o : dir.entryList(filters)) {
    //qDebug() << "Removing file: " << dir.path() + "/" + o;
    QFile::remove(dir.path() + "/" + o);
  }

  unsigned int numberOfExperiments = experimentsetup.exp().sideMax -
                                     experimentsetup.exp().sideMin +
                                     1;

  for (unsigned int i=0; i<numberOfExperiments; ++i) {
    Experiment experiment;

    experimentmanager.generateExperiment(experimentsetup, i, &experiment);
    experiment.start(experimentsetup.exp().periodMin, experimentsetup.exp().periodStep, experimentsetup.exp().periodMax);
  }

  if (schedulingvisualizerwindow != nullptr)
    delete schedulingvisualizerwindow;

  std::map<QString, QColor> m;
  m["IDLE"] = Qt::white;
  m["SENDING_MESSAGE"] = Qt::green;
  m["RECEIVING_MESSAGE"] = Qt::red;
  m["SENDING_ACK"] = Qt::yellow;
  m["WAITING_FOR_ACK"] = Qt::gray;
  m["WAITING_FOR_DIFS"] = Qt::darkGreen;
  m["WAITING_FOR_SIFS"] = Qt::darkYellow;
  m["WAITING_FOR_BACKOFF"] = Qt::blue;

  schedulingvisualizerwindow = new SchedulingVisualizer(experimentsetup, m, this);
  this->addDockWidget(Qt::TopDockWidgetArea, schedulingvisualizerwindow);

  ui->actionGraphics_Visualizer->setEnabled(true);
  ui->actionNetInterfaces_Trace->setEnabled(true);
}
