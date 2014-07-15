#include "schedulingvisualizer.hpp"

#include <QGraphicsView>

#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>

#include <QFile>

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

    if (time == 0 and event == "" && who == "")
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


SchedulingVisualizer::SchedulingVisualizer(const QString& fileName, QWidget *parent) :
  QWidget(parent)
{
  QGraphicsScene * scene = new QGraphicsScene(this);
  populateSceneWithFile(fileName, *scene);

  QGraphicsView * view = new QGraphicsView(scene);
  view->show();
}

SchedulingVisualizer::SchedulingVisualizer(const QString &fileName, const std::map<QString, QColor> &m, QWidget *parent) :
  QWidget(parent)
{

  colorMap = m;

  QGraphicsScene * scene = new QGraphicsScene(this);
  populateSceneWithFile(fileName, *scene);

  QGraphicsView * view = new QGraphicsView(scene);
  view->show();
}
