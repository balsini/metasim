#ifndef CENTRALWIDGET_HPP
#define CENTRALWIDGET_HPP

#include <QWidget>

namespace Ui {
  class CentralWidget;
}

class CentralWidget : public QWidget
{
  Q_OBJECT

  Ui::CentralWidget *ui;

public:
  explicit CentralWidget(QWidget *parent = 0);
  ~CentralWidget();

  void append(const QString &s);
  void separator();
};

#endif // CENTRALWIDGET_HPP
