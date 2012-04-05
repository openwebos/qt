/**
 *  Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "qweboswindow.h"

#include <QtGui/QWindowSystemInterface>
#include <QApplication>
QT_BEGIN_NAMESPACE

QWebOSWindow::QWebOSWindow(QWidget *w, QWebOSScreen *screen)
    : QPlatformWindow(w), m_screen(screen)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    static int serialNo = 0;
    m_winid  = ++serialNo;

    qWarning("QEglWindow %p: %p %p 0x%x\n", this, w, screen, uint(m_winid));

}

void QWebOSWindow::setGeometry(const QRect &)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    QRect rect(m_screen->availableGeometry());
    QWindowSystemInterface::handleGeometryChange(this->widget(), rect);

    // Since toplevels are fullscreen, propegate the screen size back to the widget
    widget()->setGeometry(rect);

    QPlatformWindow::setGeometry(rect);
}

WId QWebOSWindow::winId() const
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    return m_winid;
}

QPlatformGLContext *QWebOSWindow::glContext() const
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";

    Q_ASSERT(m_screen);
    if (m_screen)
    {
        qDebug() << "++++++++++++++++++++QEglWindow::glContext +++++++++++++++++++++++++++++++++" ;
        return m_screen->platformContext();
    }

    qDebug() << "++++++++++++++++++++QEglWindow::glContext : NOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO+++++++++++++++++++++++++++++++++" ;
    return 0;
}

QT_END_NAMESPACE
