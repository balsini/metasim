#include "aboutwindow.hpp"
#include "ui_aboutwindow.h"

AboutWindow::AboutWindow(QWidget *parent) :
  QWidget(parent, Qt::Window),
  ui(new Ui::AboutWindow)
{
  ui->setupUi(this);
}

AboutWindow::~AboutWindow()
{
  delete ui;
}
