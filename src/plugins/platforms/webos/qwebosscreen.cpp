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

//#include "NWindowBuffer.h"
#include "qwebosscreen.h"

#include "../eglconvenience/qeglconvenience.h"
#include "../eglconvenience/qeglplatformcontext.h"

#include <GLES2/gl2.h>

#include <QDebug>

#define STATUS_BAR_PADDING 28

QT_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif
    EGLDisplay* disp = NULL;
    void setEglSwapInterval(int interval) {
        //printf("Turning vsync %s\n", interval ? "on" : "off");
        eglSwapInterval(disp, interval);
    }
#ifdef __cplusplus
}
#endif

// #define QEGL_EXTRA_DEBUG

#ifdef QEGL_EXTRA_DEBUG
struct AttrInfo { EGLint attr; const char *name; };
static struct AttrInfo attrs[] = {
    {EGL_BUFFER_SIZE, "EGL_BUFFER_SIZE"},
    {EGL_ALPHA_SIZE, "EGL_ALPHA_SIZE"},
    {EGL_BLUE_SIZE, "EGL_BLUE_SIZE"},
    {EGL_GREEN_SIZE, "EGL_GREEN_SIZE"},
    {EGL_RED_SIZE, "EGL_RED_SIZE"},
    {EGL_DEPTH_SIZE, "EGL_DEPTH_SIZE"},
    {EGL_STENCIL_SIZE, "EGL_STENCIL_SIZE"},
    {EGL_CONFIG_CAVEAT, "EGL_CONFIG_CAVEAT"},
    {EGL_CONFIG_ID, "EGL_CONFIG_ID"},
    {EGL_LEVEL, "EGL_LEVEL"},
    {EGL_MAX_PBUFFER_HEIGHT, "EGL_MAX_PBUFFER_HEIGHT"},
    {EGL_MAX_PBUFFER_PIXELS, "EGL_MAX_PBUFFER_PIXELS"},
    {EGL_MAX_PBUFFER_WIDTH, "EGL_MAX_PBUFFER_WIDTH"},
    {EGL_NATIVE_RENDERABLE, "EGL_NATIVE_RENDERABLE"},
    {EGL_NATIVE_VISUAL_ID, "EGL_NATIVE_VISUAL_ID"},
    {EGL_NATIVE_VISUAL_TYPE, "EGL_NATIVE_VISUAL_TYPE"},
    {EGL_SAMPLES, "EGL_SAMPLES"},
    {EGL_SAMPLE_BUFFERS, "EGL_SAMPLE_BUFFERS"},
    {EGL_SURFACE_TYPE, "EGL_SURFACE_TYPE"},
    {EGL_TRANSPARENT_TYPE, "EGL_TRANSPARENT_TYPE"},
    {EGL_TRANSPARENT_BLUE_VALUE, "EGL_TRANSPARENT_BLUE_VALUE"},
    {EGL_TRANSPARENT_GREEN_VALUE, "EGL_TRANSPARENT_GREEN_VALUE"},
    {EGL_TRANSPARENT_RED_VALUE, "EGL_TRANSPARENT_RED_VALUE"},
    {EGL_BIND_TO_TEXTURE_RGB, "EGL_BIND_TO_TEXTURE_RGB"},
    {EGL_BIND_TO_TEXTURE_RGBA, "EGL_BIND_TO_TEXTURE_RGBA"},
    {EGL_MIN_SWAP_INTERVAL, "EGL_MIN_SWAP_INTERVAL"},
    {EGL_MAX_SWAP_INTERVAL, "EGL_MAX_SWAP_INTERVAL"},
    {-1, 0}};
#endif //QEGL_EXTRA_DEBUG

QWebOSScreen::QWebOSScreen(EGLNativeDisplayType display, bool offscreen)
    : m_depth(32)
    , m_format(QImage::Format_Invalid)
    , m_platformContext(0)
    , m_surface(0)
    , m_offscreen(offscreen)
//    , m_nrWindow(0)
    , m_direct(false)
{
#ifdef QEGL_EXTRA_DEBUG
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
#endif

    EGLint major, minor;
#ifdef QEGL_EXTRA_DEBUG
    EGLint index;
#endif
    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        qWarning("Could not bind GL_ES API\n");
        qFatal("EGL error");
    }

    m_dpy = eglGetDisplay(display);
    if (m_dpy == EGL_NO_DISPLAY) {
        qDebug() << "Could not open egl display\n";
    }
    qDebug() << "Opened display" << m_dpy;

    if (!eglInitialize(m_dpy, &major, &minor)) {
        qDebug() << "Could not initialize egl display";
    }

    qDebug("Initialized display %d %d\n", major, minor);

    int swapInterval = 1;
    QByteArray swapIntervalString = qgetenv("QT_QPA_EGLFS_SWAPINTERVAL");
    if (!swapIntervalString.isEmpty()) {
        bool ok;
        swapInterval = swapIntervalString.toInt(&ok);
        if (!ok)
            swapInterval = 1;
    }
    eglSwapInterval(m_dpy, swapInterval);

}

void QWebOSScreen::createAndSetPlatformContext() const {
    const_cast<QWebOSScreen *>(this)->createAndSetPlatformContext();
}

void QWebOSScreen::createAndSetPlatformContext()
{
    QPlatformWindowFormat platformFormat = QPlatformWindowFormat::defaultFormat();

    platformFormat.setWindowApi(QPlatformWindowFormat::OpenGL);

    QByteArray depthString = qgetenv("QT_QPA_EGLFS_DEPTH");
    if (depthString.toInt() == 16) {
        platformFormat.setDepth(16);
        platformFormat.setRedBufferSize(5);
        platformFormat.setGreenBufferSize(6);
        platformFormat.setBlueBufferSize(5);
        m_depth = 16;
        m_format = QImage::Format_RGB16;
    } else {
        platformFormat.setDepth(32);
        platformFormat.setRedBufferSize(8);
        platformFormat.setGreenBufferSize(8);
        platformFormat.setBlueBufferSize(8);
        m_depth = 32;
        m_format = QImage::Format_ARGB32_Premultiplied;
    }
    if (!qgetenv("QT_QPA_EGLFS_MULTISAMPLE").isEmpty()) {
        platformFormat.setSampleBuffers(true);
    }

    EGLConfig config = q_configFromQPlatformWindowFormat(m_dpy, platformFormat);

    EGLNativeWindowType eglWindow = 0;

#ifdef TASKONE
    // It should be fixed to assign by requested window size.
    eglWindow = (fbdev_window *)malloc(sizeof(fbdev_window));
    eglWindow->x = 0;
    eglWindow->y = 0;
    eglWindow->width = 1280;
    eglWindow->height = 720;
    eglWindow->bStretchToDisplay = TRUE;
#endif

    if(m_offscreen) 
    {
//        qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "3   ****************";
//        int width=0, height=0;
//        m_surfaceDirect = eglCreateWindowSurface(m_dpy, config, eglWindow, NULL);
//        if(m_surfaceDirect != EGL_NO_SURFACE)
//        {
//            eglQuerySurface(m_dpy, m_surfaceDirect, EGL_WIDTH, &width);
//            eglQuerySurface(m_dpy, m_surfaceDirect, EGL_HEIGHT, &height);
//            height -= STATUS_BAR_PADDING;
//        }
//        else
//        {
//            qFatal("failed to create direct rendering surface");
//            return;
//        }

//        m_nrWindow = new QWebOSWindowPrivate();
        
//        m_nrWindow->Set(width, height);
//        unsigned int handles[1] = { 0 };
//        if(!m_nrWindow->AttachBuffers(1, handles)) {
//            qFatal("couldn't attach buffers");
//            return;
//        }
//        NPixmap* m_nPixmap = NPixmap::Create(width,height);
//        int pitch = sizeof(uint32_t) * ((width + 31) & ~31);
//        m_nrWindow->SetBuffer(0, NPixmap::Attach(0, width, height, pitch));
//        m_nrWindow->setEglDisplay(m_dpy);

//        m_surface = eglCreateWindowSurface(m_dpy, config, m_nrWindow, NULL);
    }
    else
    {
        m_surface = eglCreateWindowSurface(m_dpy, config, eglWindow, NULL);
    }

    if (m_surface == EGL_NO_SURFACE) {
        qDebug("Could not create the egl surface: error = 0x%x\n", eglGetError());
        eglTerminate(m_dpy);
        qFatal("EGL error");
    }

    qWarning("5. Created surface");

#ifdef QEGL_EXTRA_DEBUG
    qWarning("Configuration %d matches requirements\n", (int)config);

    for (index = 0; attrs[index].attr != -1; ++index) {
        EGLint value;
        if (eglGetConfigAttrib(m_dpy, config, attrs[index].attr, &value)) {
            qWarning("\t%s: %d\n", attrs[index].name, (int)value);
        }
    }
    qWarning("\n");
#endif

    EGLint temp;
    EGLint attribList[32];

    temp = 0;

    attribList[temp++] = EGL_CONTEXT_CLIENT_VERSION;
    attribList[temp++] = 2; // GLES version 2
    attribList[temp++] = EGL_NONE;
    

    //start me in offscreen mode
    m_platformContext = new QEGLPlatformContext(m_dpy,config,attribList,m_surface,EGL_OPENGL_ES_API);

    EGLint w,h;                    // screen size detection
    eglQuerySurface(m_dpy, m_surface, EGL_WIDTH, &w);
    eglQuerySurface(m_dpy, m_surface, EGL_HEIGHT, &h);
    
    m_geometry = QRect(0,0,w,h);

    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

}

//void QWebOSScreen::directRendering(bool enable)
//{
//    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
//    if(enable)
//    {
//        if(m_direct != enable) {
//            glClearColor(0,0,0,0);
//            glClear(GL_COLOR_BUFFER_BIT);
//        }
//        m_platformContext->doneCurrent();
//        m_platformContext->setSurface(m_surfaceDirect);
//        m_platformContext->makeCurrent();
//    }
//    else
//    {
//        m_platformContext->setSurface(m_surface);
//        m_platformContext->makeCurrent();
//    }
//    m_direct = enable;
//}

QRect QWebOSScreen::geometry() const
{
    //qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "Geometry" << m_geometry <<"****************";
    if (m_geometry.isNull()) {
        createAndSetPlatformContext();
    }
    return m_geometry;
}

int QWebOSScreen::depth() const
{
    return m_depth;
}

QImage::Format QWebOSScreen::format() const
{
    if (m_format == QImage::Format_Invalid)
        createAndSetPlatformContext();
    return m_format;
}
QPlatformGLContext *QWebOSScreen::platformContext() const
{
    if (!m_platformContext) {
        QWebOSScreen *that = const_cast<QWebOSScreen *>(this);
        that->createAndSetPlatformContext();
    }

    return m_platformContext;
}

QT_END_NAMESPACE
