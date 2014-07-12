#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <map>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  //QString fileName = "/home/alessio/git_repo/metasim/examples/wifi/netInterfacesTraceNode.txt";
  //QString fileName = "/home/alessio/git_repo/metasim/examples/wifi/netInterfacesTrace1.txt";
  QString fileName = "/home/alessio/git_repo/metasim/examples/wifi/netInterfacesTrace2.txt";
  //QString fileName = "/home/alessio/git_repo/metasim/examples/wifi/netInterfacesTrace3.txt";
  //QString fileName = "/home/alessio/git_repo/metasim/examples/wifi/netInterfacesTrace4.txt";
  //QString fileName = "/home/alessio/git_repo/metasim/examples/wifi/experimentNetInterfacesTrace.txt";

  std::map<QString, QColor> m;
  m["IDLE"] = Qt::white;
  m["SENDING_MESSAGE"] = Qt::green;
  m["RECEIVING_MESSAGE"] = Qt::red;
  m["SENDING_ACK"] = Qt::yellow;
  m["WAITING_FOR_ACK"] = Qt::gray;
  m["WAITING_FOR_DIFS"] = Qt::darkGreen;
  m["WAITING_FOR_SIFS"] = Qt::darkYellow;
  m["WAITING_FOR_BACKOFF"] = Qt::blue;

  sv = std::unique_ptr<SchedulingVisualizer>(new SchedulingVisualizer(fileName, m, this));
}

MainWindow::~MainWindow()
{
  delete ui;
}
