#include "qweboswindowevents.h"

#include <QWidget>
#include <QDebug>

bool QWebOSWindowEvents::eventFilter(QObject* obj, QEvent* event)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    switch(event->type())
    {
        case QEvent::ChildAdded:
            ((QChildEvent*)event)->child()->installEventFilter(this);
            qWarning("Added a child");
            break;
        case QEvent::RequestSoftwareInputPanel:
            qWarning("Requested a keyboard");
            Q_EMIT signalInputFocusChanged(true, obj);
            return true;
            break;
        case QEvent::CloseSoftwareInputPanel:
            qWarning("Closed a keyboard");
            Q_EMIT signalInputFocusChanged(false, obj);
            return true;
            break;
        default:
            break;
    }
    return QObject::eventFilter(obj, event);
}
