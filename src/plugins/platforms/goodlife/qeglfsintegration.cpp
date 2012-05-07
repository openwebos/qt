/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (C) 2012 GoodLife
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

#include "qeglfsintegration.h"

#include "qeglfswindow.h"
#include "qeglfswindowsurface.h"
#include "qinputdevicescanner.h"

#include "qgenericunixfontdatabase.h"

#include <QtGui/QPlatformWindow>
#include <QtGui/QPlatformWindowFormat>
#include <QtGui/private/qpixmap_raster_p.h>
#include <QtOpenGL/private/qpixmapdata_gl_p.h>
#include <QtOpenGL/private/qpixmapdata_egl_p.h>

#include <QtGlobal>

#include <EGL/egl.h>

QT_BEGIN_NAMESPACE

QEglFSIntegration::QEglFSIntegration(bool soft)
    : mFontDb(new QGenericUnixFontDatabase())
{
#ifdef GOODLIFE_EGL_MODIFIED
    m_primaryScreen = new QEglFSScreen((EGLNativeDisplayType)1);
#else // original code
    m_primaryScreen = new QEglFSScreen(EGL_DEFAULT_DISPLAY);
#endif

    QString keyboardName = qgetenv("QPA_KEYBOARD");
    QString mouseName    = qgetenv("QPA_MOUSE");

    QInputDeviceScanner *scanner = new QInputDeviceScanner();

    if( scanner )
    {
        scanner->scan();
    }

    if( keyboardName.length() > 0 )
    {
        m_keyboards.append(new QLinuxKeyboardHandler(keyboardName));
    }
    else
    {
        // If the environment variables are not set,
        // we append keyboards from scanner.

        for( int i = 0; i < scanner->getNumOfKeyboards(); i++ )
        {
            QString keyboard = scanner->getKeyboardName(i);

            m_keyboards.append(new QLinuxKeyboardHandler(keyboard));
        }
    }

    if( mouseName.length() > 0 )
    {
        m_mouses.append(new QLinuxMouseHandler(mouseName));
    }
    else
    {
        // If the environment variables are not set,
        // we append mouses from scanner.

        for( int i = 0; i < scanner->getNumOfMouses(); i++ )
        {
            QString mouse = scanner->getMouseName(i);

            m_mouses.append(new QLinuxMouseHandler(mouse));
        }
    }

    if( scanner )
    {
        delete scanner;
        scanner = NULL;
    }

    this->soft = soft;

    mScreens.append(m_primaryScreen);
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglIntegration\n");
#endif
}

QEglFSIntegration::~QEglFSIntegration()
{
    qDebug("%s() Called", __func__);

    if( m_mouses.count() )
    {
        m_mouses.clear();
    }

    if( m_keyboards.count() )
    {
        m_keyboards.clear();
    }
}

bool QEglFSIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPixmapData *QEglFSIntegration::createPixmapData(QPixmapData::PixelType type) const
{
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglIntegration::createPixmapData %d\n", type);
#endif
    if( soft )
    {
        return new QRasterPixmapData(type);
    }
    else
    {
        return new QEglGLPixmapData(type);
    }
}

QPlatformWindow *QEglFSIntegration::createPlatformWindow(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglIntegration::createPlatformWindow %p\n",widget);
#endif
    return new QEglFSWindow(widget, m_primaryScreen);
}


QWindowSurface *QEglFSIntegration::createWindowSurface(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);

#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglIntegration::createWindowSurface %p\n",widget);
#endif
    return new QEglFSWindowSurface(m_primaryScreen,widget);
}

QPlatformFontDatabase *QEglFSIntegration::fontDatabase() const
{
    return mFontDb;
}

QT_END_NAMESPACE
