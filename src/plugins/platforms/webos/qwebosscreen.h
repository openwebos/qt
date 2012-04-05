
#ifndef QWEBOSSCREEN_H
#define QWEBOSSCREEN_H

#include <QPlatformScreen>

#include <QtCore/QTextStream>

#include <EGL/egl.h>


QT_BEGIN_NAMESPACE

class QPlatformGLContext;
class QEGLPlatformContext;
//struct QWebOSWindowPrivate;

class QWebOSScreen : public QPlatformScreen
{
public:
    QWebOSScreen(EGLNativeDisplayType display, bool offscreen);
    ~QWebOSScreen() {}

    QRect geometry() const;
    int depth() const;
    QImage::Format format() const;

    QPlatformGLContext *platformContext() const;

//    QWebOSWindowPrivate* remoteWindow() const { return m_nrWindow; }
    void directRendering(bool enable);
    bool isDirectRendering() { return m_direct; }
private:
    void createAndSetPlatformContext() const;
    void createAndSetPlatformContext();

    QRect m_geometry;
    int m_depth;
    QImage::Format m_format;
    QEGLPlatformContext *m_platformContext;
    EGLDisplay m_dpy;
    EGLSurface m_surface;
    EGLSurface m_surfaceDirect;
    bool m_offscreen;
//    QWebOSWindowPrivate* m_nrWindow;
    bool m_direct;
};

QT_END_NAMESPACE
#endif // QWEBOSSCREEN_H
