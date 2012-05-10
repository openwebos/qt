/****************************************************************************
**
** Copyright (C) 2012 Hewlett-Packard Development Company, L.P.
** All rights reserved.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Palm gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

