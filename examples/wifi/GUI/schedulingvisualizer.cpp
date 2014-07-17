#include "schedulingvisualizer.hpp"

#include <QGraphicsView>

#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QHeaderView>

#include <QFile>
#include <QDir>

#include <map>

#include <QDebug>

QColor SchedulingVisualizer::event2color(const QString &event)
{
  static unsigned int counter = 0;

  if (colorMap.find(event) == colorMap.end()) {
    QColor c;
    if (event != "IDLE") {
      counter = (counter + 60) % 255;
      c.setHsv(counter, 255, 255);
    }
    colorMap[event] = c;
  }

  //qDebug() << "Returning color: " << colorVector.at(eventVector.indexOf(event));
  return colorMap[event];
}

void SchedulingVisualizer::populateSceneWithFile(const QString &fileName, QGraphicsScene &scene)
{
  scene.clear();

  const int verticalSpace = 180;
  const int initialHorizontalSpace = 150;
  const int rectHeight = 20;
  const int maxTextDisalign = 5;
  const int textHeight = 13;

  QFile schedFile(fileName);

  if (!schedFile.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  QPen rectPen(Qt::SolidLine);
  QBrush sceneBGBrush(Qt::yellow, Qt::CrossPattern);
  scene.setBackgroundBrush(sceneBGBrush);

  unsigned int time;
  QString who;
  QString event;

  QVector<QString> nodeVector;
  QVector<QGraphicsRectItem *> lastRectVector;
  QVector<unsigned int> textDisalignVector;

  QTextStream in(&schedFile);
  while (!in.atEnd()) {
    in >> time;
    in >> who;
    in >> event;

    if (time == 0 and event == "" and who == "")
      break;

    if (nodeVector.indexOf(who) == -1) {
      nodeVector.append(who);
      lastRectVector.append(nullptr);
      textDisalignVector.append(0);
      auto whoText = scene.addSimpleText(who);
      whoText->setPos(0 , (nodeVector.indexOf(who) + 1) * verticalSpace);
      whoText->setRotation(20);
    }

    scene.addSimpleText(event)->setPos(time + initialHorizontalSpace,
                                       (nodeVector.indexOf(who) + 1) * verticalSpace - rectHeight - textHeight * textDisalignVector.at(nodeVector.indexOf(who)));
    scene.addSimpleText(QString::number(time))->setPos(time + initialHorizontalSpace,
                                                       (nodeVector.indexOf(who) + 1)*verticalSpace + rectHeight + textHeight * textDisalignVector.at(nodeVector.indexOf(who)));
    textDisalignVector.replace(nodeVector.indexOf(who), (textDisalignVector.at(nodeVector.indexOf(who)) + 1) % maxTextDisalign);

    QGraphicsRectItem * newRect = scene.addRect(0, 0, 0, 0, rectPen, event2color(event));
    newRect->setPos(time + initialHorizontalSpace, (nodeVector.indexOf(who) + 1) * verticalSpace);

    QGraphicsRectItem * lastRect = lastRectVector.at(nodeVector.indexOf(who));
    if (lastRect != nullptr) {
      lastRect->setRect(0,
                        0,
                        time - lastRect->pos().x() + initialHorizontalSpace,
                        rectHeight);
    }

    lastRectVector.replace(nodeVector.indexOf(who), newRect);

    //qDebug() << "At " << time << ", node " << who << " passed to " << event;
  }

  for (auto lastRect : lastRectVector) {
    lastRect->setRect(0,
                      0,
                      50,
                      rectHeight);
  }

  schedFile.close();
}

SchedulingVisualizer::SchedulingVisualizer(ExperimentSetup &e, QWidget *parent) :
  QDockWidget(parent),
  experiment(&e)
{
  generateContent();
}

SchedulingVisualizer::SchedulingVisualizer(ExperimentSetup &e,
                                           const std::map<QString, QColor> &m,
                                           QWidget *parent) :
  QDockWidget(parent),
  experiment(&e)
{
  colorMap = m;
  generateContent();
}

void SchedulingVisualizer::generateContent()
{
  this->setWindowTitle("Nodes Traces");

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

void SchedulingVisualizer::on_ExperimentChanged(QTreeWidgetItem * item, int column)
{
  populateSceneWithFile(QDir::currentPath() + "/traces/" + item->text(0),
                        *scene);
}

void SchedulingVisualizer::generateTreeWidget()
{
  treewidget.setColumnCount(1);
  treewidget.setHeaderLabel("View");
  treewidget.setMaximumWidth(400);
  treewidget.setMinimumWidth(400);

  QDir dir;
  dir.cd(QDir::currentPath() + "/traces");

  QStringList filters;
  filters << "*.txt";

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
