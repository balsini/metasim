#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QGraphicsScene>
#include <QGraphicsView>

#include <QGraphicsTextItem>
#include <QGraphicsRectItem>

#include <QFile>

#include <QDebug>

QColor event2color(const QString &event)
{
  static unsigned int counter = 0;
  static QVector<QString> eventVector;
  static QVector<QColor> colorVector;
  if (eventVector.indexOf(event) == -1) {
    eventVector.append(event);
    QColor c;
    if (event != "IDLE") {
    counter = (counter + 55) % 255;

    c.setHsv(counter, 255, 255);
    } else {
      c.setRgb(255,255,255);
    }
    colorVector.append(c);
  }
  //qDebug() << "Returning color: " << colorVector.at(eventVector.indexOf(event));
  return colorVector.at(eventVector.indexOf(event));
}

void populateSceneWithFile(const QString &fileName, QGraphicsScene &scene)
{
  const int verticalSpace = 180;
  const int initialHorizontalSpace = 50;
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
      scene.addSimpleText(who)->setPos(0 , (nodeVector.indexOf(who) + 1) * verticalSpace);
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

    qDebug() << "At " << time << ", node " << who << " passed to " << event;
  }

  for (auto lastRect : lastRectVector) {
    lastRect->setRect(0,
                      0,
                      50,
                      rectHeight);
  }

  schedFile.close();
}

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  QGraphicsScene * scene = new QGraphicsScene(this);
  populateSceneWithFile("/home/alessio/git_repo/metasim/examples/wifi/netInterfacesTrace2.txt", *scene);

  QGraphicsView * view = new QGraphicsView(scene);
  view->show();
}

MainWindow::~MainWindow()
{
  delete ui;
}
