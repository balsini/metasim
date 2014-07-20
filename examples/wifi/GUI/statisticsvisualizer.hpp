#ifndef STATISTICSVISUALIZER_HPP
#define STATISTICSVISUALIZER_HPP

#include <QWidget>
#include <QDockWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QTreeWidget>

#include <utility>
#include <map>

class StatisticsVisualizer : public QDockWidget
{
  Q_OBJECT

  std::map<unsigned int, std::pair<double, double>> values;

  QWidget mainWidget;
  QHBoxLayout mainWidgetLayout;
  QTreeWidget treewidget;

  QGraphicsScene * scene;
  QGraphicsView * view;

  void loadStatistics(const QString &fileName);
  void populateScene(QGraphicsScene &scene);
  void generateTreeWidget();
  void resizeEvent(QResizeEvent * event);

private slots:
  void on_ExperimentChanged(QTreeWidgetItem*,int);

public:
  StatisticsVisualizer(QWidget *parent = 0);
};

#endif // STATISTICSVISUALIZER_HPP
