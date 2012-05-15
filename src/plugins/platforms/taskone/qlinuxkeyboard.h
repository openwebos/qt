/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (C) 2012 TaskOne
** All rights reserved.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Palm gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QLINUX_KEYBOARD_H
#define QLINUX_KEYBOARD_H

#include <qobject.h>
#include <QApplication>
#include <QEvent>

class QWSLinuxInputKeyboardHandler;

class QLinuxKeyboardHandler: public QObject
{
Q_OBJECT
public:
    QLinuxKeyboardHandler(const QString &specification);
    ~QLinuxKeyboardHandler();

private:
    int                           m_fd;
    QWSLinuxInputKeyboardHandler *m_handler;

private Q_SLOTS:
    void readKeycode();
};


#endif /* QLINUX_KEYBOARD_H */
