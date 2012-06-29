/****************************************************************************
**
** Copyright (C) 2012 Hewlett-Packard Development Company, L.P.
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


#include "qpixmapdata_egl_p.h"

#include <unistd.h>

#include <private/qimage_p.h>
#include <private/qdrawhelper_p.h>
#include <private/qpaintengineex_opengl2_p.h>
#include <private/qgl_p.h>
#include <private/qegl_p.h>

#include <qdesktopwidget.h>
#include <qfile.h>
#include <qimagereader.h>

QT_BEGIN_NAMESPACE

#if defined(EGLPIXMAP_DEBUG)
#define NI() do { printf("%s not implemented\n", __PRETTY_FUNCTION__); /*exit(-1); */ } while (0)
#define TRACE() printf("%s (%d)\n", __PRETTY_FUNCTION__, __LINE__)
#else
#define NI()
#define TRACE()
#endif

extern QGLWidget* qt_gl_share_widget();

QEglGLPixmapData::QEglGLPixmapData(PixelType type)
    : QPixmapData(type, OpenGLClass)
    , m_engine(0)
    , m_ctx(0)
    , m_dirty(false)
    , m_hasFillColor(false)
    , m_hasAlpha(false)
    , m_fbo(0)
{
    TRACE();
}

QEglGLPixmapData::~QEglGLPixmapData()
{
    TRACE();

    QGLWidget *shareWidget = qt_gl_share_widget();
    if (!shareWidget)
        return;

    delete m_engine;

    if (m_fbo) {
        QGLShareContextScope ctx(shareWidget->context());
        glDeleteFramebuffers(1, &m_fbo);
    }

    if (m_texture.id) {
        QGLShareContextScope ctx(shareWidget->context());
        glDeleteTextures(1, &m_texture.id);
    }    
}

void QEglGLPixmapData::resize(int width, int height)
{
    TRACE();
    if (width == w && height == h)
        return;

    if (width <= 0 || height <= 0) {
        width = 0;
        height = 0;
    }

    w = width;
    h = height;
    is_null = (w <= 0 || h <= 0);
    d = pixelType() == QPixmapData::PixmapType ? 32 : 1;

    if (m_texture.id) {
        QGLShareContextScope ctx(qt_gl_share_widget()->context());
        glDeleteTextures(1, &m_texture.id);
        m_texture.id = 0;
    }

    m_source = QImage();
    m_dirty = isValid();
}

void QEglGLPixmapData::fromImage(const QImage &image, Qt::ImageConversionFlags flags)
{
    TRACE();
    resize(image.width(), image.height());

    if (pixelType() == BitmapType) {
        m_source = image.convertToFormat(QImage::Format_MonoLSB);

    } else {
        QImage::Format format = QImage::Format_RGB32;
        if (qApp->desktop()->depth() == 16)
            format = QImage::Format_RGB16;

        if (image.hasAlphaChannel() && const_cast<QImage &>(image).data_ptr()->checkForAlphaPixels())
            format = QImage::Format_ARGB32_Premultiplied;;

        m_source = image.convertToFormat(format);
    }

    m_dirty = true;
    m_hasFillColor = false;

    m_hasAlpha = m_source.hasAlphaChannel();
    w = image.width();
    h = image.height();
    is_null = (w <= 0 || h <= 0);
    d = m_source.depth();

    if (m_texture.id) {
        QGLShareContextScope ctx(qt_gl_share_widget()->context());
        glDeleteTextures(1, &m_texture.id);
        m_texture.id = 0;
    }    
}

bool QEglGLPixmapData::fromFile(const QString &filename, const char *format,
                                Qt::ImageConversionFlags flags)
{
    TRACE();
    if (pixelType() == QPixmapData::BitmapType)
        return QPixmapData::fromFile(filename, format, flags);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    QByteArray data = file.peek(64);
    bool alpha;
    if (m_texture.canBindCompressedTexture
            (data.constData(), data.size(), format, &alpha)) {
        resize(0, 0);
        data = file.readAll();
        file.close();
        QGLShareContextScope ctx(qt_gl_share_widget()->context());
        QSize size = m_texture.bindCompressedTexture
            (data.constData(), data.size(), format);
        if (!size.isEmpty()) {
            w = size.width();
            h = size.height();
            is_null = false;
            d = 32;
            m_hasAlpha = alpha;
            m_source = QImage();
            m_dirty = isValid();
            return true;
        }
        return false;
    }
    fromImage(QImageReader(&file, format).read(), flags);
    return !isNull();    
}

bool QEglGLPixmapData::fromData(const uchar *buffer, uint len, const char *format,
                                Qt::ImageConversionFlags flags)
{
    TRACE();
    bool alpha;
    const char *buf = reinterpret_cast<const char *>(buffer);
    if (m_texture.canBindCompressedTexture(buf, int(len), format, &alpha)) {
        resize(0, 0);
        QGLShareContextScope ctx(qt_gl_share_widget()->context());
        QSize size = m_texture.bindCompressedTexture(buf, int(len), format);
        if (!size.isEmpty()) {
            w = size.width();
            h = size.height();
            is_null = false;
            d = 32;
            m_hasAlpha = alpha;
            m_source = QImage();
            m_dirty = isValid();
            return true;
        }
    }
    return QPixmapData::fromData(buffer, len, format, flags);    
}

void QEglGLPixmapData::copy(const QPixmapData *data, const QRect &rect)
{
    QPixmapData::copy(data, rect);
}

bool QEglGLPixmapData::scroll(int dx, int dy, const QRect &rect)
{
    NI();
    return false;
}

void QEglGLPixmapData::fill(const QColor &color)
{
    TRACE();
    if (!isValid())
        return;

    bool hasAlpha = color.alpha() != 255;
    if (hasAlpha && !m_hasAlpha) {
        if (m_texture.id) {
            glDeleteTextures(1, &m_texture.id);
            m_texture.id = 0;
            m_dirty = true;
        }
        m_hasAlpha = color.alpha() != 255;
    }

    m_source = QImage();
    m_hasFillColor = true;
    m_fillColor = color;
}

void QEglGLPixmapData::setHasAlpha(bool val)
{
    TRACE();
    m_hasAlpha = val;
}

bool QEglGLPixmapData::hasAlphaChannel() const
{
    TRACE();
    return m_hasAlpha;    
}

extern QImage qt_gl_read_framebuffer(const QSize &size, bool alpha_format, bool include_alpha);

QImage QEglGLPixmapData::toImage() const
{
    TRACE();
    // We should not do so many toImage calls alltogether. This is what is slowing us down right now.
    // QWK should keep a cache of transformed images instead of re-doing it every frame.
    // FIXME make QWebKit not do so many QPixmap(QImage(pixmap.toImage).transform()) style transformations.
    if (!isValid())
        return QImage();

    if (!m_source.isNull()) {
        return m_source;
    } else if (m_dirty || m_hasFillColor) {
        m_source = fillImage(m_fillColor);
        return m_source;
    } else {
        ensureCreated();
    }
    // read the image from the FBO to memory
    if(m_fbo) {
        // we read the data back from the fbo
        m_ctx->makeCurrent();
        QGLShareContextScope ctx(qt_gl_share_widget()->context());
        glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_fbo);
        // this does glReadPixels - not very fast!
        QImage temp=qt_gl_read_framebuffer(QSize(w, h), true, true);
        glBindFramebuffer(GL_FRAMEBUFFER_EXT, ctx->d_ptr->current_fbo);
        m_source=temp;
        m_ctx->doneCurrent();
        return temp;
    }
    else if (m_texture.id) {
        // we read back from the texture by binding its id as a framebuffer
        // is this in the OpenGL standard? It seems to work
        m_ctx->makeCurrent();
        QGLShareContextScope ctx(qt_gl_share_widget()->context());
        glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_texture.id);
        // this does glReadPixels - not very fast
        QImage temp=qt_gl_read_framebuffer(QSize(w, h), true, true);
        glBindFramebuffer(GL_FRAMEBUFFER_EXT, ctx->d_ptr->current_fbo);
        // should we cache the fetched image locally to speed up future access?
        // m_source=temp;
        m_ctx->doneCurrent();
        return temp;
    }
    else {
        QImage temp(w,h, QImage::Format_ARGB32_Premultiplied);
        // FIXME indicating something went wrong, neither of above cases worked - look out for yellow images
        temp.fill(Qt::yellow);
        m_source=temp;
        return temp;
    }

}

QPaintEngine* QEglGLPixmapData::paintEngine() const
{
    TRACE();
    if (!isValid())
        return 0;

    TRACE();
    if (m_fbo)
        return m_engine;

    ensureCreated();

    TRACE();

    QGLContext *ctx = const_cast<QGLContext *>(QGLContext::currentContext());
    glGenFramebuffers(1, &m_fbo);   
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_texture.id, 0);

    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {

        printf("FBO generation failed with error: 0x%08x\n", status);
        return 0;
    }

    TRACE();
    m_engine = new QGL2PaintEngineEx;
    return m_engine;    
}

extern int qt_defaultDpiX();
extern int qt_defaultDpiY();


int QEglGLPixmapData::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    TRACE();
    if (w == 0)
        return 0;

    switch (metric) {
    case QPaintDevice::PdmWidth:
        return w;
    case QPaintDevice::PdmHeight:
        return h;
    case QPaintDevice::PdmNumColors:
        return 0;
    case QPaintDevice::PdmDepth:
        return d;
    case QPaintDevice::PdmWidthMM:
        return qRound(w * 25.4 / qt_defaultDpiX());
    case QPaintDevice::PdmHeightMM:
        return qRound(h * 25.4 / qt_defaultDpiY());
    case QPaintDevice::PdmDpiX:
    case QPaintDevice::PdmPhysicalDpiX:
        return qt_defaultDpiX();
    case QPaintDevice::PdmDpiY:
    case QPaintDevice::PdmPhysicalDpiY:
        return qt_defaultDpiY();
    default:
        qWarning("QGLPixmapData::metric(): Invalid metric");
        return 0;
    }    
}

bool QEglGLPixmapData::isValidContext(const QGLContext *ctx) const
{
    TRACE();
    if (ctx == m_ctx)
        return true;

    const QGLContext *share_ctx = qt_gl_share_widget()->context();
    bool ret = ctx == share_ctx || QGLContext::areSharing(ctx, share_ctx);
    return ret;
}

extern QRgb qt_gl_convertToGLFormat(QRgb src_pixel, GLenum texture_format);

GLuint QEglGLPixmapData::bind() const
{
    TRACE();
    ensureCreated();

    GLuint id = m_texture.id;
    glBindTexture(GL_TEXTURE_2D, id);

    if (m_hasFillColor) {
        m_hasFillColor = false;

        const GLenum format = GL_RGBA;
        QImage tx(w, h, QImage::Format_ARGB32_Premultiplied);
        tx.fill(qt_gl_convertToGLFormat(m_fillColor.rgba(), format));
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, tx.bits());
    }

    return id;
}

QGLTexture * QEglGLPixmapData::texture() const
{
    TRACE();
    return &m_texture;
}

bool QEglGLPixmapData::isValid() const
{
    TRACE();
    return w > 0 && h > 0;    
}

void QEglGLPixmapData::ensureCreated() const
{
    TRACE();
    if (!m_dirty)
        return;

    m_dirty = false;

    QGLShareContextScope ctx(qt_gl_share_widget()->context());
    m_ctx = ctx;

    const GLenum internal_format = m_hasAlpha ? GL_RGBA : GL_RGB;
    const GLenum external_format = internal_format;
    const GLenum target = GL_TEXTURE_2D;

    TRACE();
    
    if (!m_texture.id) {

        TRACE();
        glGenTextures(1, &m_texture.id);
        glBindTexture(target, m_texture.id);
        glTexImage2D(target, 0, internal_format, w, h, 0, external_format, GL_UNSIGNED_BYTE, 0);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            
    }

    if (!m_source.isNull()) {
        TRACE();
        if (external_format == GL_RGB) {
            const QImage tx = m_source.convertToFormat(QImage::Format_RGB888).mirrored(false, true);

            glBindTexture(target, m_texture.id);
            glTexSubImage2D(target, 0, 0, 0, w, h, external_format,
                            GL_UNSIGNED_BYTE, tx.bits());
        } else {
            const QImage tx = m_ctx->d_func()->convertToGLFormat(m_source, true, external_format);

            glBindTexture(target, m_texture.id);
            glTexSubImage2D(target, 0, 0, 0, w, h, external_format,
                            GL_UNSIGNED_BYTE, tx.bits());
        }

        m_source = QImage();
    }

    m_texture.options &= ~QGLContext::MemoryManagedBindOption;    
}

QImage QEglGLPixmapData::fillImage(const QColor &color) const
{
    QImage img;
    if (pixelType() == BitmapType) {
        img = QImage(w, h, QImage::Format_MonoLSB);

        img.setColorCount(2);
        img.setColor(0, QColor(Qt::color0).rgba());
        img.setColor(1, QColor(Qt::color1).rgba());

        if (color == Qt::color1)
            img.fill(1);
        else
            img.fill(0);
    } else {
        img = QImage(w, h,
                m_hasAlpha
                ? QImage::Format_ARGB32_Premultiplied
                : QImage::Format_RGB32);
        img.fill(PREMUL(color.rgba()));
    }
    return img;
}

QGLPaintDevice* QEglGLPixmapData::glDevice() const
{
    return (QGLPaintDevice*)(this);
}

void QEglGLPixmapData::beginPaint()
{
    TRACE();
    if (!isValid())
        return;

    m_thisFBO = m_fbo;
    QGLPaintDevice::beginPaint();

    if (m_hasFillColor) {
        m_hasFillColor = false;
        float alpha = m_fillColor.alphaF();
        glDisable(GL_SCISSOR_TEST);
        glClearColor(m_fillColor.redF() * alpha, m_fillColor.greenF() * alpha, m_fillColor.blueF() * alpha, alpha);
        glClear(GL_COLOR_BUFFER_BIT);
    }           
}

void QEglGLPixmapData::endPaint()
{
    TRACE();
    if (!isValid())
        return;

    QGLPaintDevice::endPaint();
}

QGLContext* QEglGLPixmapData::context() const
{
    ensureCreated();
    return m_ctx;    
}

QSize QEglGLPixmapData::size() const
{
    return QSize(w, h);    
}

QT_END_NAMESPACE
