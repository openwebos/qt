/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "eglwindowsurface.h"

#include <QScreen>
#include <QDebug>
#include <QWSDisplay>

#include "eglscreen.h"

#

EglWindowSurface::EglWindowSurface
        (QWidget *widget, EglScreen *screen, int screenNum)
    : QWSGLWindowSurface(widget)
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    setSurfaceFlags(QWSWindowSurface::Opaque);

    this->widget = widget;
    this->screen = screen;
    this->pdevice = 0;

    QPoint pos = offset(widget);
    QSize size = widget->size();
}

EglWindowSurface::EglWindowSurface()
    : QWSGLWindowSurface()
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    setSurfaceFlags(QWSWindowSurface::Opaque);
    widget = 0;
    screen = 0;
    pdevice = 0;
}

EglWindowSurface::~EglWindowSurface()
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    delete pdevice;
}

bool EglWindowSurface::isValid() const
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    return (widget != 0);
}

void EglWindowSurface::setGeometry(const QRect &rect)
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    QWSGLWindowSurface::setGeometry(rect);
}

bool EglWindowSurface::move(const QPoint &offset)
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    return QWSGLWindowSurface::move(offset);
}

QByteArray EglWindowSurface::permanentState() const
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    // Nothing interesting to pass to the server just yet.
    return QByteArray();
}

void EglWindowSurface::setPermanentState(const QByteArray &state)
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    Q_UNUSED(state);
}

void EglWindowSurface::flush
        (QWidget *widget, const QRegion &region, const QPoint &offset)
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    // The GL paint engine is responsible for the swapBuffers() call.
    // If we were to call the base class's implementation of flush()
    // then it would fetch the image() and manually blit it to the
    // screeen instead of using the fast PVR2D blit.
    Q_UNUSED(widget);
    Q_UNUSED(region);
    Q_UNUSED(offset);
}

QImage EglWindowSurface::image() const
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    return QImage(16, 16, screen->pixelFormat());
}

QPaintDevice *EglWindowSurface::paintDevice()
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    return widget;
}

void EglWindowSurface::setDirectRegion(const QRegion &r, int id)
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    QWSGLWindowSurface::setDirectRegion(r, id);
}
