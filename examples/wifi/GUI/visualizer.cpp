#include <QDebug>

#include <QBrush>
#include <QString>
#include <QGraphicsTextItem>

#include "visualizer.hpp"

Visualizer::Visualizer(ExperimentSetup & e, QWidget *parent) :
  QDockWidget(parent),
  experiment(&e)
{
  this->setWindowTitle("Node Distribution");

  this->setWidget(&mainWidget);

  mainWidgetLayout.addWidget(&treewidget);
  generateTreeWidget();

  mainWidgetLayout.addWidget(&view);

  mainWidget.setLayout(&mainWidgetLayout);

  view.setScene(&scene);

  connect(&treewidget,
          SIGNAL(itemClicked(QTreeWidgetItem*,int)),
          this,
          SLOT(on_ExperimentChanged(QTreeWidgetItem*,int)));
}

void Visualizer::generateTreeWidget()
{
  treewidget.setColumnCount(1);
  treewidget.setHeaderLabel("Experiment View");
  treewidget.setMaximumWidth(110);
  treewidget.setMinimumWidth(110);
  /*
  QList<QTreeWidgetItem *> items;
  for (int i = 0; i < 10; ++i)
      items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("item: %1").arg(i))));
  treewidget.insertTopLevelItems(0, items);
  */
}

void Visualizer::drawNodes(unsigned int experimentIndex)
{
  //for (auto o : sceneItems)
  //  scene.removeItem(o);
  sceneItems.clear();
  scene.clear();

  QBrush nodebrush(Qt::yellow);
  QBrush sourcebrush(Qt::red);

  QPen nodepen, rangepen;

  nodepen.setStyle(Qt::SolidLine);
  nodepen.setWidth(1);
  nodepen.setBrush(Qt::black);
  nodepen.setCapStyle(Qt::RoundCap);
  nodepen.setJoinStyle(Qt::RoundJoin);

  rangepen.setStyle(Qt::DotLine);
  rangepen.setWidth(1);
  rangepen.setBrush(Qt::black);
  rangepen.setCapStyle(Qt::RoundCap);
  rangepen.setJoinStyle(Qt::RoundJoin);

  const double sizeMultiplier = 80.0;

  for (auto &n : experiment->nodes(experimentIndex)) {
    double posX = n.posX;
    double posY = n.posY;
    double radius = n.range;

    QRadialGradient radialgradient(posX * sizeMultiplier + radius * sizeMultiplier,
                                   posY * sizeMultiplier + radius * sizeMultiplier,
                                   radius * sizeMultiplier);

    radialgradient.setColorAt(0, QColor::fromRgbF(0, 1, 0, 1));
    radialgradient.setColorAt(0.4, QColor::fromRgbF(0, 1, 0, 0.5));
    radialgradient.setColorAt(0.9, QColor::fromRgbF(0, 0, 0, 0));

    QBrush rangebrush(radialgradient);

    // Draw transmission range
    sceneItems.push_back(scene.addEllipse(posX * sizeMultiplier,
                                          posY * sizeMultiplier,
                                          radius * 2 * sizeMultiplier,
                                          radius * 2 * sizeMultiplier,
                                          rangepen,
                                          rangebrush));
  }

  for (auto &n : experiment->nodes(experimentIndex)) {
    double posX = n.posX;
    double posY = n.posY;
    double radius = n.range;

    // Draw Node
    sceneItems.push_back(scene.addEllipse(posX * sizeMultiplier + radius * sizeMultiplier - 1 * sizeMultiplier / 6 / 2,
                                          posY * sizeMultiplier + radius * sizeMultiplier - 1 * sizeMultiplier / 6 / 2,
                                          1 * sizeMultiplier / 6,
                                          1 * sizeMultiplier / 6,
                                          nodepen,
                                          n.kind == SOURCE ? sourcebrush : nodebrush));

    auto positionText = scene.addSimpleText("(" + QString::number(posX) + "," + QString::number(posY) + ")");
    positionText->setPos(posX * sizeMultiplier + radius * sizeMultiplier - 1 * sizeMultiplier / 6 / 2,
                         posY * sizeMultiplier + radius * sizeMultiplier - 1 * sizeMultiplier / 6 / 2 + sizeMultiplier / 6);
    sceneItems.push_back(positionText);

    auto radiusText = scene.addSimpleText(">" + QString::number(radius) + "<");
    radiusText->setPos(posX * sizeMultiplier + radius * 0.29289321881 * sizeMultiplier,
                       posY * sizeMultiplier + radius * 0.29289321881 * sizeMultiplier);
    radiusText->setRotation(45);
    sceneItems.push_back(radiusText);

  }
}

void Visualizer::populate()
{
  treewidget.clear();
  QList<QTreeWidgetItem *> items;
  for (unsigned int i=0; i<experiment->size(); ++i)
    items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("%1").arg(i))));
  treewidget.insertTopLevelItems(0, items);

  treewidget.setCurrentItem(items.first());

  drawNodes(0);
}

void Visualizer::resizeEvent(QResizeEvent * e)
{
  /*
  e->accept();

  //view.fitInView(scene.sceneRect(), Qt::KeepAspectRatio);
  populate(_order);
  */
}

void Visualizer::on_ExperimentChanged(QTreeWidgetItem * item, int column)
{
  drawNodes(item->text(0).toInt());
}
