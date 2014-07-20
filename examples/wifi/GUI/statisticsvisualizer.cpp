#include "statisticsvisualizer.hpp"

#include <QGraphicsLineItem>

#include <QDir>
#include <QDebug>

StatisticsVisualizer::StatisticsVisualizer(QWidget *parent) :
  QDockWidget(parent)
{
  this->setWindowTitle("Statistics Plots");

  scene = new QGraphicsScene(this);
  //populateSceneWithFile(fileName, *scene);

  view = new QGraphicsView(scene);

  this->setWidget(&mainWidget);

  mainWidgetLayout.addWidget(&treewidget);
  generateTreeWidget();

  mainWidgetLayout.addWidget(view);

  mainWidget.setLayout(&mainWidgetLayout);

  connect(&treewidget,
          SIGNAL(itemClicked(QTreeWidgetItem*,int)),
          this,
          SLOT(on_ExperimentChanged(QTreeWidgetItem*,int)));
}

void StatisticsVisualizer::resizeEvent(QResizeEvent * event)
{
  if (treewidget.selectedItems().size() > 0) {
    populateScene(*scene);
  }
}

void StatisticsVisualizer::generateTreeWidget()
{
  treewidget.setColumnCount(1);
  treewidget.setHeaderLabel("View");
  treewidget.setMaximumWidth(150);
  treewidget.setMinimumWidth(150);

  QDir dir;
  dir.cd(QDir::currentPath() + "/stats");

  QStringList filters;
  filters << "*.dat";

  qDebug() << dir.currentPath();
  qDebug() << QDir::currentPath();

  QList<QTreeWidgetItem *> items;

  for (auto o : dir.entryList(filters))
    items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("%1").arg(o))));
  //  qDebug() << o;

  /*
  unsigned int experimentNum = experiment->exp().sideMax - experiment->exp().sideMin + 1;
  for (unsigned int i=0; i<experimentNum; ++i) {
    items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("%1").arg(i))));

    unsigned int counter = 0;
    for (unsigned int j=experiment->exp().periodMin; j<=experiment->exp().periodMax; j+=experiment->exp().periodStep)
      items.last()->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("%1").arg(counter++))));
  }*/

  treewidget.insertTopLevelItems(0, items);
  //treewidget.setCurrentItem(items.first());
}

void StatisticsVisualizer::populateScene(QGraphicsScene &scene)
{
  scene.clear();

  QGraphicsLineItem * xAxis = scene.addLine(0, 0, view->width(), 0);
  xAxis->setPos(0, view->height()-40);

  QGraphicsLineItem * yAxis = scene.addLine(0, 0, 0, view->height());
  yAxis->setPos(40, 0);
}

void StatisticsVisualizer::loadStatistics(const QString &fileName)
{
  QFile schedFile(fileName);

  if (!schedFile.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  unsigned int time;
  double mean;
  double conf_int;

  QTextStream in(&schedFile);
  in.readLine();
  while (!in.atEnd()) {
    in >> time;
    in >> mean;
    in >> conf_int;

    if (in.atEnd())
      break;

    values[time] = std::make_pair(mean, conf_int);
  }
}

void StatisticsVisualizer::on_ExperimentChanged(QTreeWidgetItem * item, int column)
{
  loadStatistics(QDir::currentPath() + "/stats/" + item->text(0));
  populateScene(*scene);
}
