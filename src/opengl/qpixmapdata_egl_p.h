/* ============================================================
 * Date  : 2010-05-01
 * Copyright 2010 Palm, Inc. All rights reserved.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl.html.
 *
 * ============================================================ */

#ifndef QPIXMAPDATA_EGL_P_H
#define QPIXMAPDATA_EGL_P_H

#include "qgl_p.h"
#include "qgl.h"

#include "private/qpixmapdata_p.h"
#include "private/qglpaintdevice_p.h"

QT_BEGIN_NAMESPACE

class QEglGLPixmapData : public QPixmapData, public QGLPaintDevice
{
public:

	QEglGLPixmapData(PixelType type);
	virtual ~QEglGLPixmapData();

    // Re-implemented from QPixmapData:
    void resize(int width, int height);
    void fromImage(const QImage &image, Qt::ImageConversionFlags flags);
    bool fromFile(const QString &filename, const char *format,
                  Qt::ImageConversionFlags flags);
    bool fromData(const uchar *buffer, uint len, const char *format,
                  Qt::ImageConversionFlags flags);
    void copy(const QPixmapData *data, const QRect &rect);
    bool scroll(int dx, int dy, const QRect &rect);
    void fill(const QColor &color);
    bool hasAlphaChannel() const;
    QImage toImage() const;
    QPaintEngine *paintEngine() const;
    int metric(QPaintDevice::PaintDeviceMetric metric) const;

    // For accessing as a source:
    bool isValidContext(const QGLContext *ctx) const;
    GLuint bind() const;
    QGLTexture *texture() const;

    // Re-implemented from QGLPaintDevice
    virtual void beginPaint();
    virtual void endPaint();

	QGLContext* context() const;
    QSize size() const;
	
private:

	bool isValid() const;
	void ensureCreated() const;
	QImage fillImage(const QColor &color) const;
	static QGLContext* sharedContext();

    mutable QPaintEngine *m_engine;
    mutable QGLContext *m_ctx;
	mutable QImage m_source;
	mutable QGLTexture m_texture;

    // the texture is not in sync with the source image
    mutable bool m_dirty;

    // fill has been called and no painting has been done, so the pixmap is
    // represented by a single fill color
    mutable QColor m_fillColor;
    mutable bool m_hasFillColor;

    mutable bool m_hasAlpha;

	mutable unsigned int m_fbo;
};


#endif /* QPIXMAPDATA_EGL_P_H */
