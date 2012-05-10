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
