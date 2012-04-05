#include "qwebosclipboard.h"

#define CLIPBOARD_SIZE 16*1024

QWebOSClipboard::QWebOSClipboard() {
    m_shm = new QSharedMemory("webosclipboard");
    if(!m_shm->create(CLIPBOARD_SIZE)) {
        switch(m_shm->error()) {
            case QSharedMemory::AlreadyExists:
                if(!m_shm->attach())
                    qCritical() << "Couldn't attach clipboard!";
                break;
            default:
                qCritical() << "Couldn't create or attach clipboard!";
        }
    }
    m_data = new QMimeData();
}

QWebOSClipboard::~QWebOSClipboard() {
    delete m_shm;
    delete m_data;
}

QMimeData *QWebOSClipboard::mimeData(QClipboard::Mode mode) {
    Q_UNUSED(mode);
    m_shm->lock();
    QDataStream data(QByteArray::fromRawData((const char*)m_shm->data(), CLIPBOARD_SIZE));
    int count;
    data >> count;
    for(int i=0; i<count; i++)
    {
        QString format;
        int size;
        data >> format;
        data >> size;
        char buffer[size];
        data.readRawData(buffer, size);
        QByteArray bBuffer(buffer, size);
        m_data->setData(format, bBuffer);
    }
    m_shm->unlock();
    return m_data;
}

void QWebOSClipboard::setMimeData(QMimeData *data, QClipboard::Mode mode) {
    Q_UNUSED(mode);
    m_shm->lock();
    QByteArray buffer(CLIPBOARD_SIZE, 0);
    QDataStream newData(&buffer, QIODevice::ReadWrite);
    newData << data->formats().count();
    Q_FOREACH(QString format, data->formats()) {
        newData << format;
        newData << data->data(format).size();
        newData.writeRawData(data->data(format).constData(), data->data(format).size());
    }
    memcpy(m_shm->data(), buffer.constData(), CLIPBOARD_SIZE);
    m_shm->unlock();
}
bool QWebOSClipboard::supportsMode(QClipboard::Mode mode) const {
    return (mode == QClipboard::Clipboard) ? true : false;
}

