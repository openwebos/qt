
#include "qweboswindowsurface.h"

#include <QtGui/QPlatformGLContext>

#include <QtOpenGL/private/qgl_p.h>
#include <QtOpenGL/private/qglpaintdevice_p.h>
#include <QWindowSystemInterface>
#include <QDebug>
#include <PIpcChannel.h>
#include <palmimedefines.h>
#include "qweboswindow_p.h"
#include "qweboswindowevents.h"

#define MESSAGES_INTERNAL_FILE "SysMgrMessagesInternal.h"
#include <PIpcMessageMacros.h>

QT_BEGIN_NAMESPACE

class QWebOSPaintDevice : public QGLPaintDevice
{
public:
    QWebOSPaintDevice(QWebOSScreen *screen, QWidget *widget)
        :QGLPaintDevice(), m_screen(screen)
    {
    #ifdef QEGL_EXTRA_DEBUG
        qWarning("QEglPaintDevice %p, %p, %p",this, screen, widget);
    #else
        Q_UNUSED(widget)
    #endif
    }

    QSize size() const { return m_screen->geometry().size(); }
    QGLContext* context() const { return QGLContext::fromPlatformGLContext(m_screen->platformContext());}

    QPaintEngine *paintEngine() const { qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************"; return qt_qgl_paint_engine(); }

    void  beginPaint(){
        qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
        QGLPaintDevice::beginPaint();
    }
private:
    QWebOSScreen *m_screen;
    QGLContext *m_context;
};


//void QWebOSWindowSurface::slotSwapBuffers()
//{
//    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
//    if (m_channel && !(m_screen->isDirectRendering())) {
//        m_channel->sendAsyncMessage(new ViewHost_UpdateFullWindow(routingId()));
//    }
//}

QWebOSWindowSurface::QWebOSWindowSurface(QWebOSScreen *screen, QWidget *window, GMainLoop* loop)
    : QWindowSurface(window)
{
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglWindowSurface %p, %p", window, screen);
#endif
    m_paintDevice = new QWebOSPaintDevice(screen, window);
    m_screen = screen;
    //connect((QEGLPlatformContext*)(window->platformWindow()->glContext()), SIGNAL(signalSwapBuffers()), SLOT(slotSwapBuffers()));

//    QWebOSWindowEvents* filter = new QWebOSWindowEvents;
//    connect(filter, SIGNAL(signalInputFocusChanged(bool,QObject*)), SLOT(slotInputFocusChanged(bool,QObject*)));
//    window->installEventFilter(filter);
}

void QWebOSWindowSurface::flush(QWidget *widget, const QRegion &region, const QPoint &offset)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    Q_UNUSED(widget);
    Q_UNUSED(region);
    Q_UNUSED(offset);
    
    widget->platformWindow()->glContext()->swapBuffers();
}

//void QWebOSWindowSurface::slotInputFocusChanged(bool focus, QObject* obj)
//{
//    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";

//    if (m_channel) {
//        PalmIME::EditorState state;
//        if(QWidget* widget = qobject_cast<QWidget*>(obj))
//        {
//            Qt::InputMethodHints hints = widget->inputMethodHints();
//            if((hints & Qt::ImhUrlCharactersOnly) == Qt::ImhUrlCharactersOnly)
//                state = PalmIME::EditorState(PalmIME::FieldType_URL);
//            else if((hints & Qt::ImhEmailCharactersOnly) == Qt::ImhEmailCharactersOnly)
//                state = PalmIME::EditorState(PalmIME::FieldType_Email);
//            else if((hints & Qt::ImhDialableCharactersOnly) == Qt::ImhDialableCharactersOnly)
//                state = PalmIME::EditorState(PalmIME::FieldType_Phone);
//        }
//        m_channel->sendAsyncMessage(new ViewHost_EditorFocusChanged(routingId(), focus, state));
//    }
//}

QWebOSWindowSurface::~QWebOSWindowSurface()
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
}

void QWebOSWindowSurface::resize(const QSize &size)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    Q_UNUSED(size);
}

#if 0
int QWebOSWindowSurface::routingId() const
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    if (m_nrWindow)
        return m_nrWindow->GetBuffer(0)->Handle();
    return 0;
}

void QWebOSWindowSurface::onDisconnected()
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    qWarning("%s", __PRETTY_FUNCTION__);
    exit(-1);
}

void QWebOSWindowSurface::serverConnected(PIpcChannel* channel)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    channel->setListener(this);

    m_nrWindow = m_screen->remoteWindow();

    setChannel(channel);

    QRect r = window()->platformWindow()->geometry();
    m_channel->sendAsyncMessage(new ViewHost_PrepareAddWindow(routingId(), 8, r.width(), r.height()));

    std::string winProps = "{ "
                           "  'fullScreen': false, "                     // defaults to false
                           "  'overlayNotificationsPosition': 'bottom', " // options are left, right, top, bottom
                           "  'subtleLightbar': true, "                 // defaults to false
                           "  'blockScreenTimeout': true, "             // defaults to false
                           "  'fastAccelerometer': true, "              // defaults to false
                           "  'suppressBannerMessages': false, "        // defaults to false
                           "  'hasPauseUi': true "                     // defaults to false
                           " }";
    m_channel->sendAsyncMessage(new ViewHost_SetWindowProperties(routingId(), winProps));

    m_channel->sendAsyncMessage(new ViewHost_AddWindow(routingId()));
}

void QWebOSWindowSurface::serverDisconnected()
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    qWarning("%s", __PRETTY_FUNCTION__);
    exit(-1);
}


//Message handling
void QWebOSWindowSurface::onMessageReceived(const PIpcMessage& msg)
{
    bool msgIsOk;

    IPC_BEGIN_MESSAGE_MAP(QWebOSWindowSurface, msg, msgIsOk)
        IPC_MESSAGE_HANDLER(View_InputEvent, onInputEvent)
        IPC_MESSAGE_HANDLER(View_KeyEvent, onKeyEvent)
        IPC_MESSAGE_HANDLER(View_TouchEvent, onTouchEvent)
        IPC_MESSAGE_HANDLER(View_RemoveInputFocus, onRemoveInputFocus)
        IPC_MESSAGE_HANDLER(View_FullScreenEnabled, onFullScreenEnabled)
        IPC_MESSAGE_HANDLER(View_FullScreenDisabled, onFullScreenDisabled)
        IPC_MESSAGE_HANDLER(View_CommitText, onCommitText)
    IPC_END_MESSAGE_MAP()
}

void QWebOSWindowSurface::onCommitText(std::string s)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    QWidget* focus = QApplication::focusWidget();
    if(focus)
    {
        QString str(s.c_str());
        for(QString::iterator iter = str.begin(); iter != str.end(); iter++)
        {
            QWindowSystemInterface::handleKeyEvent(window(), QEvent::KeyPress, 0, 0, *iter);
            QWindowSystemInterface::handleKeyEvent(window(), QEvent::KeyRelease, 0, 0, *iter);
        }
    }
}

void QWebOSWindowSurface::onRemoveInputFocus()
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    qApp->postEvent(window(), new QEvent(QEvent::CloseSoftwareInputPanel));
}

void QWebOSWindowSurface::onFullScreenEnabled()
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    qWarning("Full screen enabled");
    m_screen->directRendering(true);
    window()->update();
}
void QWebOSWindowSurface::onFullScreenDisabled()
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    qWarning("Full screen disabled");
    m_screen->directRendering(false);
    window()->update();
}

void QWebOSWindowSurface::onKeyEvent(const SysMgrKeyEvent& e)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    QKeyEvent key = e.qtEvent();
    QWindowSystemInterface::handleKeyEvent(window(), key.type(), key.key(), key.modifiers(), key.text());
}

void QWebOSWindowSurface::onTouchEvent(const SysMgrTouchEvent& te)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    QList<QWindowSystemInterface::TouchPoint> touchPoints;
    for (int i = 0; i < te.numTouchPoints; i++) {

        QWindowSystemInterface::TouchPoint touchPoint;
        QPoint pt(te.touchPoints[i].x, te.touchPoints[i].y);
        QRect rc = window()->platformWindow()->geometry();

        touchPoint.id = te.touchPoints[i].id;
        touchPoint.isPrimary = te.touchPoints[i].isPrimary;
        touchPoint.normalPosition = QPointF((qreal)pt.x() / (qreal)rc.width(), (qreal)pt.y() / (qreal)rc.height());
        touchPoint.area = QRectF(pt, QSize(1,1));
        touchPoint.pressure = 1;
        touchPoint.state = static_cast<Qt::TouchPointState>(te.touchPoints[i].state);

        if (touchPoint.isPrimary) {

            QWindowSystemInterface::handleMouseEvent(window(), pt, pt, (touchPoint.state != Qt::TouchPointReleased ? Qt::LeftButton : Qt::NoButton));
        }

        touchPoints.append(touchPoint);
    }

    QWindowSystemInterface::handleTouchEvent(window(), static_cast<QEvent::Type>(te.type), QTouchEvent::TouchScreen, touchPoints);
}

void QWebOSWindowSurface::onInputEvent(const SysMgrEventWrapper& wrapper) 
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    SysMgrEvent* e = wrapper.event;
    QPoint mousePos;
    switch(e->type) 
    {
        case SysMgrEvent::Accelerometer:
            m_trans.reset();
            switch(e->orientation) 
            {
                case SysMgrEvent::Orientation_Up:
                    m_trans.rotate(0);
                    break;
                case SysMgrEvent::Orientation_Down:
                    m_trans.rotate(180);
                    break;
                case SysMgrEvent::Orientation_Left:
                    m_trans.rotate(90);
                    break;
                case SysMgrEvent::Orientation_Right:
                    m_trans.rotate(270);
                    break;
                default:
                    //qWarning("Unhandled orientation");
                    break;
            }
            break;
        case SysMgrEvent::PenDown:
#ifdef DEBUG_INPUT
            printf("Pen Down\n");
#endif
            mousePos = QPoint(e->x, e->y);
            QWindowSystemInterface::handleMouseEvent(window(), mousePos, mousePos, Qt::LeftButton);
            break;
        case SysMgrEvent::PenUp:
#ifdef DEBUG_INPUT
            printf("Pen Up\n");
#endif
            mousePos = QPoint(e->x, e->y);
            QWindowSystemInterface::handleMouseEvent(window(), mousePos, mousePos, Qt::NoButton);
            break;
        case SysMgrEvent::PenMove:
#ifdef DEBUG_INPUT
            printf("Pen Move\n");
#endif
            mousePos = QPoint(e->x, e->y);
            QWindowSystemInterface::handleMouseEvent(window(), mousePos, mousePos, Qt::LeftButton);
            break;
        case SysMgrEvent::GestureStart:
#ifdef DEBUG_INPUT
            printf("Gesture start\n");
#endif
            break;
        case SysMgrEvent::GestureEnd:
#ifdef DEBUG_INPUT
            printf("Gesture end\n");
#endif
            break;
        case SysMgrEvent::GestureCancel:
#ifdef DEBUG_INPUT
            printf("Gesture cancel\n");
#endif
            break;
        default:
            printf("Unhandled event type\n");
            break;
    }
}
#endif
QT_END_NAMESPACE
