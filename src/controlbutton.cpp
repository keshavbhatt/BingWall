#include "controlbutton.h"
#include <QEvent>
#include <QMouseEvent>
#include <QToolTip>

controlButton::controlButton(QWidget *parent) : QPushButton(parent) {
  setMouseTracking(true);
}

bool controlButton::eventFilter(QObject *obj, QEvent *event) {
  Q_UNUSED(obj);
  if (event->type() == QEvent::ToolTip) {
    return true;
  }
  return false;
}

void controlButton::mouseMoveEvent(QMouseEvent *e) {

  QPushButton::mouseMoveEvent(e);
}
