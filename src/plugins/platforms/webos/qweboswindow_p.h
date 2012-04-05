
#ifndef QWEBOSWINDOW_P_H
#define QWEBOSWINDOW_P_H

#include <NRemoteWindow.h>

struct QWebOSWindowPrivate : public NRemoteWindow
{
    virtual void Post(int Buffer) { Q_UNUSED(Buffer) }
    virtual void Wait(int* Buffer) { Q_UNUSED(Buffer) }
};

#endif

