#ifndef SCHEDULINKVISUALIZER_HPP
#define SCHEDULINKVISUALIZER_HPP

#include <QDockWidget>
#include <QGraphicsScene>
#include <QString>
#include <QHBoxLayout>
#include <QTreeWidget>

#include "experimentsetup.hpp"

class SchedulingVisualizer : public QDockWidget
{
  Q_OBJECT

  QWidget mainWidget;
  QHBoxLayout mainWidgetLayout;
  QTreeWidget treewidget;

  QString _directory;

  std::map<QString, QColor> colorMap;

  QGraphicsScene * scene;
  QGraphicsView * view;

  QColor event2color(const QString &event);
  void populateSceneWithFile(const QString &fileName, QGraphicsScene &scene);
  void generateTreeWidget();
  void generateContent();

private slots:
  void on_ExperimentChanged(QTreeWidgetItem*,int);

public:
  SchedulingVisualizer(const QString &directory, QWidget *parent = 0);
  SchedulingVisualizer(const QString &directory, const std::map<QString, QColor> &m, QWidget *parent = 0);
};

#endif // SCHEDULINKVISUALIZER_HPP
