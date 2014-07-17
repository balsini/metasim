#include "centralwidget.hpp"
#include "ui_centralwidget.h"

CentralWidget::CentralWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::CentralWidget)
{
  ui->setupUi(this);
}

CentralWidget::~CentralWidget()
{
  delete ui;
}

void CentralWidget::append(const QString &s)
{
  ui->textBrowser->append(s);
}

void CentralWidget::separator()
{
  append("################");
}
