
#ifndef QWEBOSWINDOWSURFACE_H
#define QWEBOSWINDOWSURFACE_H

#include "qwebosintegration.h"
#include "qweboswindow.h"
#include "qeglplatformcontext.h"

#include <QtGui/private/qwindowsurface_p.h>
#include "glib.h"

QT_BEGIN_NAMESPACE

class PIpcChannel;
struct QWebOSWindowPrivate;

class QWebOSWindowSurface : public QObject,
                            public QWindowSurface
{
    Q_OBJECT

public:
    QWebOSWindowSurface(QWebOSScreen *screen, QWidget *window, GMainLoop* loop);
    virtual ~QWebOSWindowSurface();

    QPaintDevice *paintDevice() { return m_paintDevice; }
    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);
    void resize(const QSize &size);

private:

    QPaintDevice *m_paintDevice;
    QWebOSWindowPrivate* m_nrWindow;
    QWebOSScreen* m_screen;
    QTransform m_trans;

private Q_SLOTS:
    void slotSwapBuffers();
    void slotInputFocusChanged(bool, QObject*);

};

QT_END_NAMESPACE

#endif // QWEBOSWINDOWSURFACE_H
