#ifndef QWEBOSCLIPBOARD_H
#define QWEBOSCLIPBOARD_H

#define QWEBOS_EXPORT __attribute__((visibility("default")))

#include <QPlatformClipboard>
#include <QMimeData>
#include <QSharedMemory>
#include <QStringList>
#include <QDataStream>
#include <QDebug>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QWEBOS_EXPORT QWebOSClipboard : public QPlatformClipboard {
    public:
        QWebOSClipboard();
        virtual ~QWebOSClipboard();
        virtual QMimeData *mimeData(QClipboard::Mode mode = QClipboard::Clipboard);
        virtual void setMimeData(QMimeData *data, QClipboard::Mode mode = QClipboard::Clipboard);
        virtual bool supportsMode(QClipboard::Mode mode) const;
    private:
        QSharedMemory* m_shm;
        QMimeData* m_data;
};

QT_END_NAMESPACE
QT_END_HEADER

#endif
