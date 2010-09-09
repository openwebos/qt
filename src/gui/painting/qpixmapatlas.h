/****************************************************************************
**
** Copyright (C) 2010 Palm Inc and/or its subsidiary(-ies).
** All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QPIXMAPATLAS_H
#define QPIXMAPATLAS_H

#include <QtGui/QPixmap>
#include <QtCore/QSize>
#include <QtCore/QExplicitlySharedDataPointer>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE
class QAreaAllocator;
class QPixmapAtlasPrivate;
class QPixmap;

QT_MODULE(Gui)

class Q_GUI_EXPORT QPixmapAtlas
{
public:
    typedef QExplicitlySharedDataPointer<QPixmapAtlasPrivate> DataPtr;

    struct SpriteEntry {
        qint64 key;
        QPixmap pixmap;
        QSize preferredSize;
    };

    struct SpriteDrawInfo {
        qint64 key;
        QRect dest;
        qreal opacity;
    };

    static void setMaximumAtlasSize(int width, int height);
    static void setMaximumAtlasSize(const QSize& size);
    static QSize maximumAtlasSize();

    QPixmapAtlas(const QSize& initialSize = QSize(), const QSize& margin = QSize());
    QPixmapAtlas(QAreaAllocator *allocator);
    QPixmapAtlas(const QPixmapAtlas& atlas);
    ~QPixmapAtlas();

    const QAreaAllocator* allocator() const;

    void detach();
    bool isDetached() const;

    QPixmapAtlas& operator=(const QPixmapAtlas& other);

    qint64 cachePixmap(const SpriteEntry& sprite);
    QList<qint64> cachePixmaps(const QList<SpriteEntry>& sprites);

    bool uncachePixmap(qint64 key);
    QList<qint64> uncachePixmaps(const QList<qint64> &keys);

    bool updatePixmap(qint64 oldKey, qint64 newKey, const QPixmap& icon);

    int overhead() const;

    bool saveDbg(const QString& path) const;
    bool save(const QString& path) const;
    bool save(QDataStream& output) const;
    bool load(const QString& path);
    bool load(QDataStream& input);

	QRect lookupSprite(qint64 key, bool *ok = NULL) const;
	int numSprites() const;
	QSize size() const;

    QList<qint64> paint(QPainter *painter, const QList<SpriteDrawInfo>& sprites);

    inline DataPtr& data_ptr() { return d_ptr; }
protected:
    DataPtr d_ptr;

private:
    Q_DECLARE_PRIVATE(QPixmapAtlas);
};

Q_DECLARE_SHARED(QPixmapAtlas);

QT_END_NAMESPACE

QT_END_HEADER

#endif
