
#ifndef WEBOSINTEGRATION_H
#define WEBOSINTEGRATION_H

#include "qwebosscreen.h"

#include <QtGui/QPlatformIntegration>
#include <QtGui/QPlatformScreen>
#include "qwebosclipboard.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QWebOSClipboard;
class QWebOSIntegration : public QPlatformIntegration
{
public:
    QWebOSIntegration(bool offscreen);
    bool hasCapability(QPlatformIntegration::Capability cap) const;
    QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId) const;
    QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const;

    QList<QPlatformScreen *> screens() const { return mScreens; }

    QPlatformFontDatabase *fontDatabase() const;
    virtual QPlatformClipboard *clipboard() const;
private:
    QPlatformFontDatabase *mFontDb;
    QList<QPlatformScreen *> mScreens;
    QWebOSScreen *m_primaryScreen;
    bool m_offscreen;
    QWebOSClipboard* m_clipboard;
};

QT_END_NAMESPACE
QT_END_HEADER

#endif
