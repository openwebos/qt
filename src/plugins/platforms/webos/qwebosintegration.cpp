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

#include "qwebosintegration.h"

#include "qweboswindow.h"
#include "qweboswindowsurface.h"

#include <QtGui/QPlatformWindow>
#include "qwebosclipboard.h"
#include "qwebosfontdatabase.h"
#include <QtGui/QPlatformWindowFormat>
#include <QtGui/private/qpixmap_raster_p.h>
#include <QtOpenGL/private/qpixmapdata_egl_p.h>
#include <EGL/egl.h>

#include <glib.h>
#include <QApplication>

QT_BEGIN_NAMESPACE

QPlatformClipboard* QWebOSIntegration::clipboard() const {
    return (QWebOSClipboard*)m_clipboard;
}

QWebOSIntegration::QWebOSIntegration(bool offscreen)
    : mFontDb(new QWebOSFontDatabase())
      ,m_offscreen(offscreen)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
//QApplication::setStyle(new QWebOSStyle);
    m_clipboard = new QWebOSClipboard();
    m_primaryScreen = new QWebOSScreen((EGLNativeDisplayType) 1/*EGL_DEFAULT_DISPLAY*/, offscreen);
    if(!offscreen) {
//        m_tpHandler = new QPAHiddTpHandler(m_primaryScreen);
//        m_kbdHandler = new QPAHiddKbdHandler;
    }
    
    mScreens.append(m_primaryScreen);
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglIntegration\n");
#endif
}

bool QWebOSIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPixmapData *QWebOSIntegration::createPixmapData(QPixmapData::PixelType type) const
{
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglIntegration::createPixmapData %d\n", type);
#endif
    return new QRasterPixmapData(type);
}

QPlatformWindow *QWebOSIntegration::createPlatformWindow(QWidget *widget, WId winId) const
{
//    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << winId << "****************";
    Q_UNUSED(winId);
    return new QWebOSWindow(widget, m_primaryScreen);
}


QWindowSurface *QWebOSIntegration::createWindowSurface(QWidget *widget, WId winId) const
{
//    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    Q_UNUSED(winId);

    GMainContext* context = g_main_context_default();
    GMainLoop* loop = g_main_loop_new(context, TRUE);
    return new QWebOSWindowSurface(m_primaryScreen, widget, loop);
}

QPlatformFontDatabase *QWebOSIntegration::fontDatabase() const
{
    return mFontDb;
}

QT_END_NAMESPACE
