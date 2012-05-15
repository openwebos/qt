/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (C) 2012 TaskOne
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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

#include "qtaskonecursor.h"
#include "qtaskonecursordraw.h"

#include "qtaskoneintegration.h"
#include "qtaskonescreen.h"
#include "qtaskonewindow.h"

#include <QtGui/QBitmap>

#define POSD_CURSOR_ID  LX_FBDEV_ID_CSR0

QT_BEGIN_NAMESPACE

QTaskOneCursor::QTaskOneCursor(QTaskOneScreen *screen)
    : QPlatformCursor(screen)
{
    qDebug("[whatsub] %s() Called", __func__);

    graphic = new QPlatformCursorImage(0, 0, 0, 0, 0, 0);

    drawer = new QTaskOneCursorDraw();

    setCursor(Qt::ArrowCursor);
}

void QTaskOneCursor::setCursor(Qt::CursorShape shape)
{
	qDebug("JCY : %s[%d] ", __FUNCTION__,__LINE__);

    //graphic->set(shape);
    drawer->setCursorImg(0);
	setVisible();
}

void QTaskOneCursor::drawCursor(const QPoint &p)
{

	if(drawer->setPosition(p.x(), p.y(), 0, 0, 0, 0, 0, 1,1) != 0)
	{
		qDebug("JCY : %s[%d] - setPosition Error", __FUNCTION__,__LINE__);
	}
}

int QTaskOneCursor::setVisible(void)
{
	qDebug("JCY : %s[%d] ", __FUNCTION__,__LINE__);
	drawer->setVisible(TRUE);
}
int QTaskOneCursor::setInVisible(void)
{
	qDebug("JCY : %s[%d] ", __FUNCTION__,__LINE__);
	drawer->setVisible(FALSE);
}

void QTaskOneCursor::pointerEvent(const QMouseEvent &e)
{
    drawCursor(e.pos());
}

void QTaskOneCursor::changeCursor(QCursor *cursor, QWidget *widget)
{
    if (!widget)
        return;

    int id = cursor->handle();
}

QT_END_NAMESPACE
