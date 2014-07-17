#ifndef TOOLBAR_HPP
#define TOOLBAR_HPP

#include <QToolBar>

class ToolBar : public QToolBar
{
  Q_OBJECT

public:
  explicit ToolBar(QWidget *parent = 0);
  ~ToolBar();
};

#endif // TOOLBAR_HPP
