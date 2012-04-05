
#ifndef QWEBOSWINDOW_H
#define QWEBOSWINDOW_H

#include "qwebosintegration.h"
#include "qwebosscreen.h"

#include <QPlatformWindow>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QWebOSWindow : public QPlatformWindow
{
public:
    QWebOSWindow(QWidget *w, QWebOSScreen *screen);

    void setGeometry(const QRect &);
    WId winId() const;

    QPlatformGLContext *glContext() const;

private:
    QWebOSScreen *m_screen;
    WId m_winid;
};
QT_END_NAMESPACE
#endif // QWEBOSWINDOW_H
