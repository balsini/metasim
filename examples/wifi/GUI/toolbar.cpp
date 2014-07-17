#include "toolbar.hpp"

#include <QAction>
#include <QIcon>
#include <QPixmap>
#include "mainwindow.hpp"

ToolBar::ToolBar(QWidget *parent) :
  QToolBar(parent)
{
  setIconSize(QSize(40,40));

  addAction(*((dynamic_cast<MainWindow *>(parent))->getActionNew()));

  addSeparator();

  addAction(*((dynamic_cast<MainWindow *>(parent))->getActionOpen()));
  addAction(*((dynamic_cast<MainWindow *>(parent))->getActionSave()));
  addAction(*((dynamic_cast<MainWindow *>(parent))->getActionSaveAs()));

  addSeparator();

  addAction(*((dynamic_cast<MainWindow *>(parent))->getActionAddNodes()));

  addSeparator();

  addAction(*((dynamic_cast<MainWindow *>(parent))->getActionRunExperiment()));

  addSeparator();

  addAction(*((dynamic_cast<MainWindow *>(parent))->getActionShowGraphics()));
  addAction(*((dynamic_cast<MainWindow *>(parent))->getActionShowTraces()));
}

ToolBar::~ToolBar()
{}
