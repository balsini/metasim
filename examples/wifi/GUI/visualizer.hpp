#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <QDockWidget>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QHBoxLayout>
#include <QEvent>
#include <QResizeEvent>
#include <QTreeWidget>

#include <list>

#include "experimentsetup.hpp"

class Visualizer : public QDockWidget
{
  Q_OBJECT

  QWidget mainWidget;
  QHBoxLayout mainWidgetLayout;
  QTreeWidget treewidget;

  QGraphicsView view;
  QGraphicsScene scene;

  ExperimentSetup * experiment;

  std::list<QGraphicsItem *> sceneItems;

  unsigned int _order;

  void generateTreeWidget();
  void drawNodes(unsigned int experimentIndex);

private slots:
  void on_ExperimentChanged(QTreeWidgetItem * item, int column);

public:
  explicit Visualizer(ExperimentSetup &e, QWidget *parent = 0);
  void populate();

  void resizeEvent(QResizeEvent * e);
};

#endif // VISUALIZER_H
