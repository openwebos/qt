/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (C) 2012 Hewlett-Packard Development Company, L.P.
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtOpenGL module of the Qt Toolkit.
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

#include <private/qglpaintdevice_p.h>
#include <private/qgl_p.h>
#include <private/qglpixelbuffer_p.h>
#include <private/qglframebufferobject_p.h>
#ifdef Q_WS_X11
#include <private/qpixmapdata_x11gl_p.h>
#endif

#if !defined(QT_OPENGL_ES_1)
#include <private/qpixmapdata_gl_p.h>
#include <private/qwindowsurface_gl_p.h>
#include <private/qpixmapdata_egl_p.h>
#endif

QT_BEGIN_NAMESPACE

QGLPaintDevice::QGLPaintDevice()
    : m_thisFBO(0)
{
}

QGLPaintDevice::~QGLPaintDevice()
{
}

int QGLPaintDevice::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    switch(metric) {
    case PdmWidth:
        return size().width();
    case PdmHeight:
        return size().height();
    case PdmDepth: {
        const QGLFormat f = format();
        return f.redBufferSize() + f.greenBufferSize() + f.blueBufferSize() + f.alphaBufferSize();
    }
    default:
        qWarning("QGLPaintDevice::metric() - metric %d not known", metric);
        return 0;
    }
}

void QGLPaintDevice::beginPaint()
{
    // Make sure our context is the current one:
    QGLContext *ctx = context();
    if (ctx != QGLContext::currentContext())
        ctx->makeCurrent();

    // Record the currently bound FBO so we can restore it again
    // in endPaint() and bind this device's FBO
    //
    // Note: m_thisFBO could be zero if the paint device is not
    // backed by an FBO (e.g. window back buffer).  But there could
    // be a previous FBO bound to the context which we need to
    // explicitly unbind.  Otherwise the painting will go into
    // the previous FBO instead of to the window.
    m_previousFBO = ctx->d_func()->current_fbo;

    if (m_previousFBO != m_thisFBO) {
        ctx->d_ptr->current_fbo = m_thisFBO;
        glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_thisFBO);
    }

    // Set the default fbo for the context to m_thisFBO so that
    // if some raw GL code between beginNativePainting() and
    // endNativePainting() calls QGLFramebufferObject::release(),
    // painting will revert to the window surface's fbo.
    ctx->d_ptr->default_fbo = m_thisFBO;
}

void QGLPaintDevice::ensureActiveTarget()
{
    QGLContext* ctx = context();
    if (ctx != QGLContext::currentContext())
        ctx->makeCurrent();

    if (ctx->d_ptr->current_fbo != m_thisFBO) {
        ctx->d_ptr->current_fbo = m_thisFBO;
        glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_thisFBO);
    }

    ctx->d_ptr->default_fbo = m_thisFBO;
}

void QGLPaintDevice::endPaint()
{
    // Make sure the FBO bound at beginPaint is re-bound again here:
    QGLContext *ctx = context();
    if (m_previousFBO != ctx->d_func()->current_fbo) {
        ctx->d_ptr->current_fbo = m_previousFBO;
        glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_previousFBO);
    }

    ctx->d_ptr->default_fbo = 0;
}

QGLFormat QGLPaintDevice::format() const
{
    return context()->format();
}

bool QGLPaintDevice::alphaRequested() const
{
    return context()->d_func()->reqFormat.alpha();
}

bool QGLPaintDevice::isFlipped() const
{
    return false;
}

////////////////// QGLWidgetGLPaintDevice //////////////////

QGLWidgetGLPaintDevice::QGLWidgetGLPaintDevice()
{
}

QPaintEngine* QGLWidgetGLPaintDevice::paintEngine() const
{
    return glWidget->paintEngine();
}

void QGLWidgetGLPaintDevice::setWidget(QGLWidget* w)
{
    glWidget = w;
}

void QGLWidgetGLPaintDevice::beginPaint()
{
    QGLPaintDevice::beginPaint();
    if (!glWidget->d_func()->disable_clear_on_painter_begin && glWidget->autoFillBackground()) {
        if (glWidget->testAttribute(Qt::WA_TranslucentBackground))
            glClearColor(0.0, 0.0, 0.0, 0.0);
        else {
            const QColor &c = glWidget->palette().brush(glWidget->backgroundRole()).color();
            float alpha = c.alphaF();
            glClearColor(c.redF() * alpha, c.greenF() * alpha, c.blueF() * alpha, alpha);
        }
        if (context()->d_func()->workaround_needsFullClearOnEveryFrame)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        else
            glClear(GL_COLOR_BUFFER_BIT);
    }
}

void QGLWidgetGLPaintDevice::endPaint()
{
    if (glWidget->autoBufferSwap())
        glWidget->swapBuffers();
    QGLPaintDevice::endPaint();
}


QSize QGLWidgetGLPaintDevice::size() const
{
    return glWidget->size();
}

QGLContext* QGLWidgetGLPaintDevice::context() const
{
    return const_cast<QGLContext*>(glWidget->context());
}

// returns the QGLPaintDevice for the given QPaintDevice
QGLPaintDevice* QGLPaintDevice::getDevice(QPaintDevice* pd)
{
    QGLPaintDevice* glpd = 0;

    switch(pd->devType()) {
        case QInternal::Widget:
            // Should not be called on a non-gl widget:
            Q_ASSERT(qobject_cast<QGLWidget*>(static_cast<QWidget*>(pd)));
            glpd = &(static_cast<QGLWidget*>(pd)->d_func()->glDevice);
            break;
        case QInternal::Pbuffer:
            glpd = &(static_cast<QGLPixelBuffer*>(pd)->d_func()->glDevice);
            break;
        case QInternal::FramebufferObject:
            glpd = &(static_cast<QGLFramebufferObject*>(pd)->d_func()->glDevice);
            break;
        case QInternal::Pixmap: {
#if !defined(QT_OPENGL_ES_1)
            QPixmapData* pmd = static_cast<QPixmap*>(pd)->pixmapData();
            if (pmd->classId() == QPixmapData::OpenGLClass)
                glpd = static_cast<QGLPixmapData*>(pmd)->glDevice();
#ifdef Q_WS_X11
            else if (pmd->classId() == QPixmapData::X11Class)
                glpd = static_cast<QX11GLPixmapData*>(pmd);
#endif
            else
                qWarning("Pixmap type not supported for GL rendering");
#else
            qWarning("Pixmap render targets not supported on OpenGL ES 1.x");
#endif
            break;
        }
        default:
            qWarning("QGLPaintDevice::getDevice() - Unknown device type %d", pd->devType());
            break;
    }

    return glpd;
}

QT_END_NAMESPACE
