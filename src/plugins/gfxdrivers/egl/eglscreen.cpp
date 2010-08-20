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

#include "eglscreen.h"

#include <QRegExp>
#include <qwindowsystem_qws.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fb.h>
// PALM -->
#include <cstdio>
// <-- PALM

#include <QtGui/private/qgraphicssystem_qws_p.h>
#include <QtOpenGL/private/qpixmapdata_gl_p.h>

#include "eglwindowsurface.h"

class EglScreenPrivate : public QWSGraphicsSystem
{
public:

    virtual QPixmapData* createPixmapData(QPixmapData::PixelType type) const {
        return new QGLPixmapData(type);
    }

};


//![0]
EglScreen::EglScreen(int displayId)
    : QGLScreen(displayId)
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    setOptions(NativeWindows);
    setSupportsBlitInClients(true);
    setSurfaceFunctions(new EglScreenSurfaceFunctions(this, displayId));
    parent = 0;

    // Make sure that the EGL layer is initialized and the drivers loaded.
    EGLDisplay dpy = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
    if (!eglInitialize(dpy, 0, 0))
        qWarning("Could not initialize EGL display - are the drivers loaded?");
}

EglScreen::~EglScreen()
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
}

bool EglScreen::initDevice()
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
	
    return true;
}

bool EglScreen::connect(const QString &displaySpec)
{
	TRACE("%s\n", __PRETTY_FUNCTION__);

	int fd = open("/dev/fb0", O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open framebuffer device\n");
        return false;
    }

	struct fb_var_screeninfo varinfo;
	memset(&varinfo, 0, sizeof(varinfo));

    ioctl(fd, FBIOGET_VSCREENINFO, &varinfo);	

	data = 0;
	w = varinfo.xres;
	h = varinfo.yres;
	lstep = varinfo.xres * varinfo.bits_per_pixel / 8;
	dw = w;
	dh = h;
	size = h * lstep;
	mapsize = size;
	d = varinfo.bits_per_pixel;

	close(fd);
	
	setPixelFormat(QImage::Format_ARGB32_Premultiplied);
	
/*	
    // Initialize the QScreen properties.
    data = (uchar *)(pvrQwsDisplay.screens[0].mapped);
    w = pvrQwsDisplay.screens[0].screenRect.width;
    h = pvrQwsDisplay.screens[0].screenRect.height;
    lstep = pvrQwsDisplay.screens[0].screenStride;
    dw = w;
    dh = h;
    size = h * lstep;
    mapsize = size;
    switch (pvrQwsDisplay.screens[0].pixelFormat) {
	case PVR2D_RGB565:
            d = 16;
            setPixelFormat(QImage::Format_RGB16);
            break;
	case PVR2D_ARGB4444:
            d = 16;
            setPixelFormat(QImage::Format_ARGB4444_Premultiplied);
            break;
	case PVR2D_ARGB8888:
            d = 32;
            setPixelFormat(QImage::Format_ARGB32_Premultiplied);
            break;
        default:
            pvrQwsDisplayClose();
            qWarning("EglScreen::connect: unsupported pixel format %d", (int)(pvrQwsDisplay.screens[0].pixelFormat));
            return false;
    }
*/
    // Handle display physical size spec.
    QStringList displayArgs = displaySpec.split(QLatin1Char(':'));
    QRegExp mmWidthRx(QLatin1String("mmWidth=?(\\d+)"));
    int dimIdxW = displayArgs.indexOf(mmWidthRx);
    QRegExp mmHeightRx(QLatin1String("mmHeight=?(\\d+)"));
    int dimIdxH = displayArgs.indexOf(mmHeightRx);
    if (dimIdxW >= 0) {
        mmWidthRx.exactMatch(displayArgs.at(dimIdxW));
        physWidth = mmWidthRx.cap(1).toInt();
        if (dimIdxH < 0)
            physHeight = dh*physWidth/dw;
    }
    if (dimIdxH >= 0) {
        mmHeightRx.exactMatch(displayArgs.at(dimIdxH));
        physHeight = mmHeightRx.cap(1).toInt();
        if (dimIdxW < 0)
            physWidth = dw*physHeight/dh;
    }
    if (dimIdxW < 0 && dimIdxH < 0) {
        const int dpi = 72;
        physWidth = qRound(dw * 25.4 / dpi);
        physHeight = qRound(dh * 25.4 / dpi);
    }

    setGraphicsSystem(new EglScreenPrivate);
    
    // The screen is ready.
    return true;
}

void EglScreen::disconnect()
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
}

void EglScreen::shutdownDevice()
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
}

void EglScreen::blit(const QImage &img, const QPoint &topLeft, const QRegion &region)
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
//    QGLScreen::blit(img, topLeft, region);
//    sync();
}

void EglScreen::solidFill(const QColor &color, const QRegion &region)
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
//    QGLScreen::solidFill(color, region);
//    sync();
}

bool EglScreen::chooseContext
    (QGLContext *context, const QGLContext *shareContext)
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    // We use EglScreenSurfaceFunctions instead.
    Q_UNUSED(context);
    Q_UNUSED(shareContext);
    return false;
}

bool EglScreen::hasOpenGL()
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    return true;
}

//![1]
QWSWindowSurface* EglScreen::createSurface(QWidget *widget) const
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    if (qobject_cast<QGLWidget*>(widget))
        return new EglWindowSurface(widget, (EglScreen *)this, displayId);

    return QScreen::createSurface(widget);
}

QWSWindowSurface* EglScreen::createSurface(const QString &key) const
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    if (key == QLatin1String("Egl"))
        return new EglWindowSurface();

    return QScreen::createSurface(key);
}
//![1]

void EglScreen::sync()
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
    // Put code here to synchronize 2D and 3D operations if necessary.
}

//![2]
bool EglScreenSurfaceFunctions::createNativeWindow(QWidget *widget, EGLNativeWindowType *native)
{
	TRACE("%s\n", __PRETTY_FUNCTION__);
//![2]
    QWSWindowSurface *surface =
        static_cast<QWSWindowSurface *>(widget->windowSurface());
    if (!surface) {
        // The widget does not have a surface yet, so give it one.
        surface = new EglWindowSurface(widget, screen, displayId);
        widget->setWindowSurface(surface);
    } else if (surface->key() != QLatin1String("Egl")) {
        // The application has attached a QGLContext to an ordinary QWidget.
        // Replace the widget's window surface with a new one that can do GL.
        QRect geometry = widget->frameGeometry();
        geometry.moveTo(widget->mapToGlobal(QPoint(0, 0)));
        surface = new EglWindowSurface(widget, screen, displayId);
        surface->setGeometry(geometry);
        widget->setWindowSurface(surface);
        widget->setAttribute(Qt::WA_NoSystemBackground, true);
    }
    EglWindowSurface *nsurface = static_cast<EglWindowSurface*>(surface);
    //*native = (EGLNativeWindowType)(nsurface->nativeDrawable());
	*native = NULL;
    return true;
}
