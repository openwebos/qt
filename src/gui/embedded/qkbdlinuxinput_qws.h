/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (C) 2012 Hewlett-Packard Development Company, L.P.
** Copyright (C) 2012 TaskOne
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QKBDLINUXINPUT_QWS_H
#define QKBDLINUXINPUT_QWS_H

#include <QtGui/qkbd_qws.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_QWS_KEYBOARD

#ifndef QT_NO_QWS_KBD_LINUXINPUT

class QWSLinuxInputKbPrivate;

#if !defined(QT_WEBOS) || defined(TASKONE)
class QWSLinuxInputKeyboardHandler : public QWSKeyboardHandler
#else // QT_WEBOS
class Q_GUI_EXPORT QWSLinuxInputKeyboardHandler : public QWSKeyboardHandler
#endif // QT_WEBOS
{
public:
    QWSLinuxInputKeyboardHandler(const QString&);
    virtual ~QWSLinuxInputKeyboardHandler();

    virtual bool filterInputEvent(quint16 &input_code, qint32 &input_value);

private:
    QWSLinuxInputKbPrivate *d;
};

#endif // QT_NO_QWS_KBD_LINUXINPUT

#endif // QT_NO_QWS_KEYBOARD

QT_END_NAMESPACE

QT_END_HEADER

#endif // QKBDLINUXINPUT_QWS_H
