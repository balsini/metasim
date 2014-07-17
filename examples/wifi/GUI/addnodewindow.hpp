#ifndef EXPERIMENTSETUP_HPP
#define EXPERIMENTSETUP_HPP

#include <QDialog>

#include "experimentsetup.hpp"

namespace Ui {
  class AddNodeWindow;
}

class AddNodeWindow : public QDialog
{
  Q_OBJECT

  Ui::AddNodeWindow *ui;

  ExperimentSetup * experiment;

  void generateSquareNodes();

private slots:
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();
  void on_nodeNumberChanged(int n);

public:
  AddNodeWindow(ExperimentSetup & e, QWidget *parent = 0);
  ~AddNodeWindow();

signals:
  void nodeAdded_signal();
};

#endif // EXPERIMENTSETUP_HPP
