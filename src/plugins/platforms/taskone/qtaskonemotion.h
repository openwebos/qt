/****************************************************************************
**
** Copyright (C) 2012 TaskOne
** All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
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

#ifndef QTASKONE_MOTION_H
#define QTASKONE_MOTION_H

#include <qobject.h>
#include <Qt>

class QSocketNotifier;

class QTaskOneMotionHandlerData;

class QTaskOneMotionHandler : public QObject
{
    Q_OBJECT
public:
    QTaskOneMotionHandler(const QString &specification);
    ~QTaskOneMotionHandler();

private slots:
    void readMotionData();

private:
    void sendMotionEvent(int x, int y, Qt::MouseButtons buttons);
    QSocketNotifier *          m_notify;
    int                        m_fd;
    int                        m_x, m_y;
    int m_prevx, m_prevy;
    int m_xoffset, m_yoffset;
    int m_smoothx, m_smoothy;
    Qt::MouseButtons           m_buttons;
    bool m_compression;
    bool m_smooth;
    int m_jitterLimitSquared;
//    QTaskOneMotionHandlerData *d;
};

#endif /* QTASKONE_MOTION_H */

