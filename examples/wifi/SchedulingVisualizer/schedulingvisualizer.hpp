#ifndef SCHEDULINKVISUALIZER_HPP
#define SCHEDULINKVISUALIZER_HPP

#include <QWidget>
#include <QGraphicsScene>
#include <QString>

class SchedulingVisualizer : public QWidget
{
  Q_OBJECT

  std::map<QString, QColor> colorMap;

  QColor event2color(const QString &event);
  void populateSceneWithFile(const QString &fileName, QGraphicsScene &scene);

public:
  SchedulingVisualizer(const QString &fileName, QWidget *parent = 0);
  SchedulingVisualizer(const QString &fileName, const std::map<QString, QColor> &m, QWidget *parent = 0);
};

#endif // SCHEDULINKVISUALIZER_HPP
