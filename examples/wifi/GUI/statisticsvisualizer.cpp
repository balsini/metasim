#include "statisticsvisualizer.hpp"

#include <QGraphicsLineItem>
#include <QGraphicsTextItem>

#include <QDir>
#include <QDebug>

double max_value(std::map<unsigned int, std::pair<double, double>>::iterator first,
                 std::map<unsigned int, std::pair<double, double>>::iterator last,
                 const bool firstElement)
{
  if (firstElement) {
    if (first == last)
      return last->first;
    double largest = first->first;

    while (++first != last)
      if (largest < first->first)    // or: if (comp(*largest,*first)) for version (2)
        largest = first->first;
    return largest;
  }
  if (first == last)
    return std::get<0>(last->second);
  double largest = std::get<0>(first->second);

  while (++first != last)
    if (largest < std::get<0>(first->second))    // or: if (comp(*largest,*first)) for version (2)
      largest = std::get<0>(first->second);
  return largest;

}

StatisticsVisualizer::StatisticsVisualizer(const QString & directory, QWidget *parent) :
  QDockWidget(parent),
  _directory(directory)
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

  QBrush sceneBGBrush(Qt::yellow, Qt::CrossPattern);
  scene->setBackgroundBrush(sceneBGBrush);

  connect(&treewidget,
          SIGNAL(itemClicked(QTreeWidgetItem*,int)),
          this,
          SLOT(on_ExperimentChanged(QTreeWidgetItem*,int)));
}

void StatisticsVisualizer::resizeEvent(QResizeEvent * event)
{
  if (treewidget.selectedItems().size() > 0)
    populateScene(*scene);
}

void StatisticsVisualizer::generateTreeWidget()
{
  treewidget.setColumnCount(1);
  treewidget.setHeaderLabel("View");
  treewidget.setMaximumWidth(150);
  treewidget.setMinimumWidth(150);

  QDir dir(_directory);

  QStringList filters;
  filters << "*.dat";

  qDebug() << dir.currentPath();
  qDebug() << QDir::currentPath();

  QList<QTreeWidgetItem *> items;

  for (auto o : dir.entryList(filters))
    items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("%1").arg(o))));

  treewidget.insertTopLevelItems(0, items);
}

void StatisticsVisualizer::populateScene(QGraphicsScene &scene)
{
  scene.clear();

  const unsigned int scrollBarDisplacement = 30;
  const unsigned int axisDisplacement = 60;
  const unsigned int xTextDisplacement = 20;
  const unsigned int yTextDisplacement = 20;
  const unsigned int textHeight = 12;

  auto xAxis = scene.addLine(0, 0, view->width() - scrollBarDisplacement, 0);
  xAxis->setPos(0,
                view->height() - axisDisplacement);

  auto yAxis = scene.addLine(0, 0, 0, view->height() - scrollBarDisplacement);
  yAxis->setPos(axisDisplacement - scrollBarDisplacement,
                0);

  double maxY = max_value(values.begin(), values.end(), false);
  //qDebug() << "maxY: " << maxY;

  double maxX = max_value(values.begin(), values.end(), true);
  //qDebug() << "maxX: " << maxX;

  // Y axis numbers
  for (unsigned int i=0; i<=(view->height() - axisDisplacement); i+=yTextDisplacement) {
    unsigned int textValue = (double)i/(view->height() - axisDisplacement) * maxY;
    //qDebug() << "i: " << textValue;
    auto yNumber = scene.addSimpleText(QString::number(textValue));
    yNumber->setPos(0, (view->height() - axisDisplacement) - i - textHeight / 2);
  }

  // X axis numbers
  for (unsigned int i=0; i<=(view->width() - axisDisplacement); i+=xTextDisplacement) {
    unsigned int textValue = (double)i/(view->width() - axisDisplacement) * maxX;
    //qDebug() << "i: " << textValue;
    auto xNumber = scene.addSimpleText(QString::number(textValue));
    xNumber->setPos(i + axisDisplacement - scrollBarDisplacement + textHeight / 2,
                    view->height() - axisDisplacement);
    xNumber->setRotation(45);
  }

  QPen pointPen(Qt::SolidLine);
  QBrush pointBrush(Qt::green, Qt::SolidPattern);
  const double pointRadius = 3;

  for (auto o : values) {
    double yPos = view->height() - axisDisplacement - std::get<0>(o.second) / maxY * (view->height() - axisDisplacement);
    auto it = scene.addEllipse(0, 0, pointRadius * 2, pointRadius * 2, pointPen, pointBrush);
    //qDebug() << yPos;
    it->setPos(o.first / maxX * (view->width() - axisDisplacement) + axisDisplacement - scrollBarDisplacement,
               yPos - pointRadius);
  }
}

void StatisticsVisualizer::loadStatistics(const QString &fileName)
{
  QFile schedFile(fileName);

  if (!schedFile.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  unsigned int time;
  double mean;
  double conf_int;

  values.clear();

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
  loadStatistics(_directory + "/" + item->text(0));
  populateScene(*scene);
}
