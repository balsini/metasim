#include "addnodewindow.hpp"
#include "ui_addnodewindow.h"

#include <sstream>

#include <QPushButton>
#include <QDialogButtonBox>

#include <QDebug>

enum {
  SINGLE_NODE = 1,
  SET_OF_NODES = 0,
  SQUARE_ARRANGE = 0,
  LINE_ARRANGE = 1,
  ELLIPSE_ARRANGE = 2
};

AddNodeWindow::AddNodeWindow(ExperimentSetup & e, QWidget * parent) :
  QDialog(parent),
  ui(new Ui::AddNodeWindow),
  experiment(&e)
{
  ui->setupUi(this);
  setWindowTitle("Add Nodes");

  connect(this, SIGNAL(nodeAdded_signal()), this->parent(), SLOT(on_experimentChanged()));

  connect(ui->tabNodeNumber, SIGNAL(currentChanged(int)), this, SLOT(on_nodeNumberChanged(int)));
  connect(ui->tabNodeSetArrange, SIGNAL(currentChanged(int)), this, SLOT(on_nodeNumberChanged(int)));

  qDebug() << "on_buttonBox_created";
}

AddNodeWindow::~AddNodeWindow()
{
  delete ui;
}

void AddNodeWindow::generateSquareNodes()
{
  std::stringstream experimentParameters;

  experimentParameters
      << "SQUARE "
      << ui->setNodeSquareSideMin->value() << ' '
      << ui->setNodeSquareSideMax->value() << ' '
      << ui->setNodeSquareDistance->value() << ' '
      << ui->setNodeRadius->value() << ' '
      << ui->setNodePeriodMin->value() << ' '
      << ui->setNodePeriodStep->value() << ' '
      << ui->setNodePeriodMax->value() << ' '
      << ui->setNodeRuns->value() << ' ';

  if (ui->setNodeTraces->isChecked())
    experimentParameters << "true";
  else
    experimentParameters << "false";
  experimentParameters << '\n';

  experiment->parseParameters(experimentParameters.str());
}

void AddNodeWindow::on_buttonBox_accepted()
{
  qDebug() << "on_buttonBox_accepted";

  switch (ui->tabNodeNumber->currentIndex()) {
    case SET_OF_NODES:
      qDebug() << "ui->tabNodeSet->isActiveWindow()";

      switch (ui->tabNodeSetArrange->currentIndex()) {
        case SQUARE_ARRANGE:

          generateSquareNodes();

          break;
        case LINE_ARRANGE:
          // TODO in future improvements
          break;
        case ELLIPSE_ARRANGE:
          // TODO in future improvements
          break;
        default: break;
      }
      break;
    case SINGLE_NODE: // Single node
      qDebug() << "ui->tabNodeSingle->isActiveWindow()";

      // TODO in future improvements
      break;
    default: break;
  }
  emit nodeAdded_signal();
}

void AddNodeWindow::on_buttonBox_rejected()
{}

void AddNodeWindow::on_nodeNumberChanged(int n)
{
  switch (n) {
    case SET_OF_NODES:
      switch (ui->tabNodeSetArrange->currentIndex()) {
        case SQUARE_ARRANGE:
          ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
          break;
        default:
          ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
          break;
      }
      break;
    default:
      ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
      break;
  }
}
