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

#include "qpixmapatlas.h"
#include "qareaallocator_p.h"
#include "qdrawutil.h"

#include <QtCore/QHash>
#include <QtCore/QLinkedList>
#include <QtCore/QFile>
#include <QtCore/QScopedPointer>
#include <QtGui/QPainter>
#include <QtCore/QTemporaryFile>

#include <QDebug>

QT_BEGIN_NAMESPACE

#define ATLAS_MAGIC_KEY "qpixmapatlas"

static inline int nextPowerOfTwo(int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    ++v;
    return v;
}

static inline QSize nextPowerOfTwo(const QSize &v)
{
    return QSize(nextPowerOfTwo(v.width()), nextPowerOfTwo(v.height()));
}

class QPixmapAtlasPrivate
{
public:
    typedef QPixmapAtlas::SpriteEntry Sprite;
    typedef QPixmapAtlas::SpriteDrawInfo SpriteDrawInfo;

    static QSize maximumAtlasSize()
    {
        return maximumSize;
    }

    static void setMaximumAtlasSize(const QSize& newSize)
    {
        maximumSize = newSize;
    }

    QAtomicInt ref;

    QPixmapAtlasPrivate(QAreaAllocator *allocator)
        : alloc(allocator)
        , usedAtlasSize(0)
        , expandWidth(false)
    {
        Q_ASSERT(!alloc.isNull());
    }

    QPixmapAtlasPrivate(const QSize initialSize)
        : alloc(new QSimpleAreaAllocator(nextPowerOfTwo(initialSize)))
        , usedAtlasSize(0)
        , expandWidth(false)
    {
        Q_ASSERT(atlas.isNull());
        Q_ASSERT(atlas.width() == 0);
        Q_ASSERT(atlas.height() == 0);
    }

    QPixmapAtlasPrivate(const QPixmapAtlasPrivate& copy)
        : alloc(copy.alloc->create(copy.alloc->size()))
        , usedAtlasSize(0)
        , expandWidth(false)
    {
        QList<Sprite> sprites;
        for (SpriteOrder::const_iterator i = copy.insertionOrder.begin(), ni = copy.insertionOrder.end(); i != ni; i++) {
            Sprite sprite;
            sprite.key = *i;
            sprite.preferredSize = copy.lookup[sprite.key].size();
            // three ways to do this that I can see:
            // 1. modify the atlas to support populating from another atlas
            // 2. extract the pixmap from the atlas and use a copy (memory overhead & cpu)
            // 3. use a dummy pixmap & then paint into the new atlas using the fast-atlas
            //    painting technique
            // 3 seems like it requires less work than 1 & works the fastest, especially with FBO pixmaps
            // the danger is if addPixmaps uses the information within the pixmap itself for anything,
            // but that's a simple audit
            sprites.append(sprite);
        }
        addPixmaps(sprites);
        atlas = copy.atlas;
        atlas.detach();
        Q_ASSERT(atlas.isNull() || atlas.width() > 0);
        Q_ASSERT(atlas.isNull() || atlas.height() > 0);
        Q_ASSERT(alloc->size() == copy.alloc->size());
    }

    ~QPixmapAtlasPrivate()
    {
    }

    qint64 addPixmap(const Sprite& sprite)
    {
        QList<Sprite> sprites;
        sprites.append(sprite);
        QList<qint64> added = addPixmaps(sprites);
        if (added.empty())
            return -1;
        Q_ASSERT(added.size() == 1);
        return added.first();
    }

    /**
      * Expand to the requested size if possible.  This will not expand to a width or height
      * above the maximum atlas size.  If the amount of space requested is less than the available
      * space, nothing happens either.  If it requests more space, then the requested shape is granted
      * if it can at least fit the largest sprite in the atlas (this may change the fragmentation level
      * & cause the expansion to fail for this reason).
      * Most likely, a square layout for the texture (w = h) is probably best for fragmentation, but
      * this is an untested hypothesis.  At the very least, try to simple scale up (don't change the shape
      * of the atlas) as this may cause unexpected failure due to incrased fragmentation.
      */
    bool expand(QSize size)
    {
        size = nextPowerOfTwo(size);
        if (size.width() > maximumAtlasSize().width() || size.height() > maximumAtlasSize().height()) {
            // the maximum atlas size contains the largest width & height a texture may be
            // qWarning() << "Requested atlas size" << size << "exceeds maximum texture size" << maximumAtlasSize() << "supported - cannot expand";
            return false;
        }

        if (size.width() <= alloc->size().width() && size.height() <= alloc->size().height()) {
            // nothing to do - trying to expand to a subset.
#ifdef QT_DEBUG
            qDebug() << "No point expanding atlas - requested size" << size << "can already be satisfied";
#endif
            return true;
        }

        QPixmapAtlasPrivate copy(alloc->create(size));
        QList<Sprite> sprites;
        for (SpriteOrder::const_iterator i = insertionOrder.begin(), ni = insertionOrder.end(); i != ni; i++) {
            Sprite sprite;
            sprite.key = *i;
            sprite.preferredSize = lookup[sprite.key].size();
            Q_ASSERT(sprite.pixmap.isNull());
            // three ways to do this that I can see:
            // 1. modify the atlas to support populating from another atlas
            // 2. extract the pixmap from the atlas and use a copy (memory overhead & cpu)
            // 3. use a dummy pixmap & then paint into the new atlas using the fast-atlas
            //    painting technique
            // 3 seems like it requires less work than 1 & works the fastest, especially with FBO pixmaps
            // the danger is if addPixmaps uses the information within the pixmap itself for anything,
            // but that's a simple audit
            sprites.append(sprite);
        }
        QList<qint64> added = copy.addPixmaps(sprites);
        if (added.size() != sprites.size()) {
            qWarning() << "Expanded atlas of size" << size << "is unable to contain contents of current atlas" << atlas.size();
            return false;
        }

        QList<SpriteDrawInfo> commandBuffer;
        Q_FOREACH(qint64 spriteId, added) {
            SpriteDrawInfo command;
            command.key = spriteId;
            command.dest = copy.lookupSprite(spriteId);
            command.opacity = 1;
            commandBuffer.append(command);
        }

        if (!atlas.isNull()) {
            if (copy.atlas.isNull()) {
                copy.atlas = QPixmap(copy.alloc->size());
                copy.atlas.fill(Qt::transparent);
            }
            QPainter expandedAtlas(&copy.atlas);
            expandedAtlas.setCompositionMode(QPainter::CompositionMode_Source);
            QList<qint64> paintErrors = paint(&expandedAtlas, commandBuffer);
            expandedAtlas.end();

            if (!paintErrors.isEmpty()) {
#ifdef QT_DEBUG
                qWarning() << "Failed to paint expanded atlas" << copy.atlas.size() << "- missing" << paintErrors.size() << "sprites";
#endif
                return false;
            }
        } else
            Q_ASSERT(usedAtlasSize == 0);

        atlas = copy.atlas;
        alloc.reset(copy.alloc.take());
        lookup = copy.lookup;
        usedAtlasSize = copy.usedAtlasSize;

        return true;
    }

    QList<qint64> addPixmaps(const QList<Sprite>& sprites)
    {
        QList<QSize> spriteSizes;
        QList<qint64> boundSprites;

        // how many pixels the requests will cover
        int requestedSize = 0;

        for (int i = 0, ni = sprites.size(); i < ni; i++) {
            const Sprite& sprite = sprites.at(i);
            if (sprite.key < 0) {
                qCritical() << "Atlas given an invalid key for sprite" << i;
                return boundSprites;
            }

            QSize preferredSize;

            if (lookup.contains(sprite.key)) {
                // no need to have the allocater allocate space for this sprite
                preferredSize = QSize(0, 0);
            } else if (!sprite.preferredSize.isNull())
                // we were given a size
                preferredSize = sprite.preferredSize;
            else
                // otherwise try to use the pixmap size
                preferredSize = sprite.pixmap.size();

            spriteSizes << preferredSize;
            requestedSize += preferredSize.width() * preferredSize.height();
        }

        // need a better heuristic since this doesn't account for fragmentation caused by the new
        // sprites
        int newSizeGuess = usedAtlasSize + requestedSize;
        if (newSizeGuess > alloc->size().width() * alloc->size().height()) {
            QSize newSize = QSize(alloc->size().width(), alloc->size().height());
            do {
                newSize = nextPowerOfTwo(newSize + (expandWidth ? QSize(1, 0) : QSize(0, 1)));
                expandWidth = !expandWidth;
            } while(newSizeGuess > newSize.width() * newSize.height());
            // expanding by 1px bumps width/height to the next power of two.
            if (expand(newSize))
                Q_ASSERT(alloc->size() == newSize || alloc->size().width() * alloc->size().height() > newSize.width() * newSize.height());
        }

        QList<QRect> allocated = alloc->allocate(spriteSizes);
        Q_ASSERT(allocated.size() <= sprites.size());

        Q_ASSERT(atlas.isNull() || atlas.width() > 0);
        Q_ASSERT(atlas.isNull() || atlas.height() > 0);

        if (!atlas.isNull() && (atlas.width() != alloc->size().width() || atlas.height() != alloc->size().height())) {
            qWarning() << "Atlas pixmap is" << atlas.size() << "while allocator is" << alloc->size();
            Q_ASSERT(false);
        }

        if (!allocated.isEmpty()) {
            QPainter *spriteAdder = NULL;

            for (int i = 0, ni = allocated.size(); i < ni; i++) {
                QRect spriteRegion = allocated.at(i);
                qint64 spriteId = sprites.at(i).key;
                Q_ASSERT(!spriteRegion.isNull() || lookup.contains(spriteId));
                if (spriteRegion.isNull()) {
                    spriteRegion = lookup[spriteId];
                } else {
                    lookup.insert(spriteId, spriteRegion);
                    insertionOrder.append(spriteId);
                }
                boundSprites.append(spriteId);

                if (!sprites.at(i).pixmap.isNull()) {
                    if (atlas.isNull() && alloc->size().width() > 0 && alloc->size().height() > 0) {
                        atlas = QPixmap(alloc->size());
                        atlas.fill(Qt::transparent);
                    }
                    if (!spriteAdder) {
                        spriteAdder = new QPainter(&atlas);
                        spriteAdder->setCompositionMode(QPainter::CompositionMode_Source);
                        spriteAdder->setRenderHint(QPainter::SmoothPixmapTransform);
                    }
                    spriteAdder->drawPixmap(spriteRegion, sprites.at(i).pixmap);
                } else if (spriteAdder) {
//                    spriteAdder.fillRect(spriteRegion, QColor(qrand() %255, qrand()%255, qrand()%255, qrand()%255));
                }

                usedAtlasSize += spriteRegion.width() * spriteRegion.height();
            }

            if (spriteAdder)
                spriteAdder->end();
        }

        Q_ASSERT(boundSprites.size() == allocated.size());

        while (boundSprites.size() != sprites.size()) {
            QSize newSize = nextPowerOfTwo(alloc->size() + (expandWidth ? QSize(1, 0) : QSize(0, 1)));
            expandWidth = !expandWidth;
            if (expand(newSize))
                boundSprites.append(addPixmaps(sprites.mid(boundSprites.size())));
            else {
                if (maximumAtlasSize() != atlas.size()) {
#ifdef QT_DEBUG
                    qDebug() << "Atlas expansion stopped prematurly @" << atlas.size();
#endif
                }
                break;
            }
        }

        return boundSprites;
    }

    bool updatePixmap(qint64 oldSprite, qint64 newSprite, const QPixmap& icon)
    {
        SpriteIndex::iterator i = lookup.find(oldSprite);
        if (i == lookup.end())
            return false;

        QRect oldRegion = *i;

        QPainter repainter(&atlas);
        repainter.setCompositionMode(QPainter::CompositionMode_Source);
        repainter.setRenderHint(QPainter::SmoothPixmapTransform);
        repainter.drawPixmap(oldRegion, icon);
        repainter.end();

        lookup.erase(i);

        lookup.insert(newSprite, oldRegion);

        return true;
    }

    bool removePixmap(qint64 sprite)
    {
        SpriteIndex::iterator i = lookup.find(sprite);
        if (i == lookup.end())
            return false;

        QRect spriteRegion = i.value();
        insertionOrder.removeOne(i.key());
        lookup.erase(i);

        alloc->release(spriteRegion);
        usedAtlasSize -= spriteRegion.width() * spriteRegion.height();

        return true;
    }

    QList<qint64> removePixmaps(const QList<qint64>& spriteIds)
    {
        QList<qint64> errors;

        Q_FOREACH(qint64 spriteId, spriteIds) {
            if (!removePixmap(spriteId))
                errors.append(spriteId);
        }

        return errors;
    }

    QList<qint64> paint(QPainter *painter, const QList<SpriteDrawInfo>& sprites)
    {
        QList<qint64> errors;
        if (atlas.isNull()) {
            qCritical() << "Failed to paint because sprite atlas isn't initialized";
            Q_FOREACH(SpriteDrawInfo sprite, sprites) {
                errors << sprite.key;
            }
            return errors;
        }

        QVector<QDrawPixmaps::Data> drawInfo;
        drawInfo.reserve(sprites.size());

        Q_FOREACH(SpriteDrawInfo sprite, sprites) {
            SpriteIndex::const_iterator i = lookup.find(sprite.key);
            if (i == lookup.end()) {
                errors << sprite.key;
                continue;
            }

            const QRect &spriteSrc = *i;

            QDrawPixmaps::Data drawCommand;
            drawCommand.point = QRectF(sprite.dest).center();
            drawCommand.source = spriteSrc;
            drawCommand.scaleX = sprite.dest.width() / (qreal)spriteSrc.width();
            drawCommand.scaleY = sprite.dest.height() / (qreal)spriteSrc.height();
            drawCommand.rotation = 0;
            drawCommand.opacity = sprite.opacity;

            drawInfo.append(drawCommand);
        }

        qDrawPixmaps(painter, drawInfo.constData(), drawInfo.size(), atlas);

        if (!errors.isEmpty()) {
            qWarning() << "Attempt to paint" << errors.size() << "sprite(s) not in atlas";
        }
        return errors;
    }

    int overhead() const
    {
        int allocatedSize = atlas.width() * atlas.height();
        return alloc->overhead() + (allocatedSize - usedAtlasSize) * (atlas.depth() / 8);
    }

    bool saveDbg(const QString& path) const
    {
        if (QFile::exists(path) && !QFile::remove(path))
            return false;

        QString tmpPath = path + QLatin1String("XXXXXX.tmp");
        QTemporaryFile destination(tmpPath);
        destination.setAutoRemove(false);

        if (!destination.open())
            goto save_fail;

        if (!atlas.save(&destination, "PNG"))
            goto save_fail;

        fsync(destination.handle());

        if (!destination.rename(path))
            goto save_fail;

        return true;

save_fail:
        destination.remove();
        return false;
    }

    bool save(const QString& path) const
    {
        if (QFile::exists(path))
            return false;

        QString tmpPath = path + QLatin1String("XXXXXX.tmp");
        QTemporaryFile destination(tmpPath);
        destination.setAutoRemove(false);

        if (!destination.open())
            return false;

        QDataStream writer(&destination);
        if (!save(writer))
            goto save_fail;

        fsync(destination.handle());

        if (!destination.rename(path))
            goto save_fail;

        return true;

save_fail:
        destination.remove();
        return false;
    }

    bool save(QDataStream& writer) const
    {
        int currentVersion = writer.version();
        writer.setVersion(QDataStream::Qt_4_6);
        writer << ATLAS_MAGIC_KEY;
        writer << (qint32)currentVersion;
        writer.setVersion(currentVersion);

        writer << usedAtlasSize;
        writer << lookup.size();

        for (SpriteOrder::const_iterator i = insertionOrder.begin(), ni = insertionOrder.end(); i != ni; i++) {
          writer << *i << lookup[*i];
        }

        writer << atlas;

        return writer.status() == QDataStream::Ok;
    }

    bool load(const QString& path)
    {
        QFile source(path);
        if (!source.open(QIODevice::ReadOnly))
            return false;

        QDataStream reader(&source);
        return load(reader);
    }

    bool load(QDataStream& reader)
    {
        char *keyBuf;
        uint keyLen;
        qint32 version;
        int currentVersion = reader.version();
        int numSprites;

        qint64 newUsedAtlasSize;
        SpriteOrder newInsertionOrder;
        SpriteIndex newLookup;
        QPixmap newAtlas;


        reader.readBytes(keyBuf, keyLen);

        bool validMagic = (keyBuf != NULL) &&
                          (keyLen == sizeof(ATLAS_MAGIC_KEY) - 1) &&
                          (0 == strncmp(ATLAS_MAGIC_KEY, keyBuf, keyLen));
        delete [] keyBuf;
        if (!validMagic)
            return false;

        reader >> version;
        reader.setVersion(version);

        reader >> newUsedAtlasSize;

        reader >> numSprites;
        for (int i = 0; i < numSprites; i++) {
          qint64 key;
          QRect position;
          reader >> key >> position;
          newInsertionOrder.append(key);
          newLookup.insert(key, position);
        }

        reader >> newAtlas;

        reader.setVersion(currentVersion);

        if (reader.status() == QDataStream::Ok) {
          insertionOrder = newInsertionOrder;
          lookup = newLookup;
          atlas = newAtlas;
          return true;
        }

        return false;
    }

    QRect lookupSprite(qint64 key, bool *found = NULL) const
    {
        SpriteIndex::const_iterator i = lookup.find(key);
        if (i == lookup.end()) {
            if (found) *found = false;
            return QRect();
        }

        if (found) *found = true;
        return *i;
    }

    int numSprites() const
    {
        return lookup.size();
    }

    QSize size() const
    {
        return atlas.size();
    }

    const QAreaAllocator* allocator() const
    {
        return alloc.data();
    }

private:
    bool containsSprite(qint64 key)
    {
        return lookup.contains(key);
    }

    typedef QLinkedList<qint64> SpriteOrder;
    typedef QHash<qint64, QRect> SpriteIndex;
    static QSize maximumSize;

    QPixmap atlas;
    QScopedPointer<QAreaAllocator> alloc;
    SpriteOrder insertionOrder;
    SpriteIndex lookup;
    qint64 usedAtlasSize;
    bool expandWidth;
};

QSize QPixmapAtlasPrivate::maximumSize = QSize(1024, 1024);

void QPixmapAtlas::setMaximumAtlasSize(int width, int height)
{
    setMaximumAtlasSize(QSize(width, height));
}

void QPixmapAtlas::setMaximumAtlasSize(const QSize &size)
{
    QPixmapAtlasPrivate::setMaximumAtlasSize(nextPowerOfTwo(size));
}

QSize QPixmapAtlas::maximumAtlasSize()
{
    return QPixmapAtlasPrivate::maximumAtlasSize();
}

QPixmapAtlas::QPixmapAtlas(const QSize &initialSize)
    : d_ptr(new QPixmapAtlasPrivate(initialSize))
{
}

QPixmapAtlas::QPixmapAtlas(QAreaAllocator *allocator)
    : d_ptr(new QPixmapAtlasPrivate(allocator))
{
}

QPixmapAtlas::QPixmapAtlas(const QPixmapAtlas& other)
	: d_ptr(other.d_ptr)
{
}

QPixmapAtlas::~QPixmapAtlas()
{
}

QPixmapAtlas& QPixmapAtlas::operator=(const QPixmapAtlas& other)
{
    if (d_ptr != other.d_ptr)
        d_ptr = other.d_ptr;
    return *this;
}

void QPixmapAtlas::detach()
{
    d_ptr.detach();
}

bool QPixmapAtlas::isDetached() const
{
    return d_func()->ref == 1;
}

qint64 QPixmapAtlas::cachePixmap(const SpriteEntry &sprite)
{
    Q_D(QPixmapAtlas);

    return d->addPixmap(sprite);
}

QList<qint64> QPixmapAtlas::cachePixmaps(const QList<SpriteEntry> &sprites)
{
    Q_D(QPixmapAtlas);

    return d->addPixmaps(sprites);
}

bool QPixmapAtlas::uncachePixmap(qint64 sprite)
{
    Q_D(QPixmapAtlas);

    return d->removePixmap(sprite);
}

QList<qint64> QPixmapAtlas::uncachePixmaps(const QList<qint64> &sprites)
{
    Q_D(QPixmapAtlas);

    return d->removePixmaps(sprites);
}

bool QPixmapAtlas::updatePixmap(qint64 oldKey, qint64 newKey, const QPixmap& icon)
{
    Q_D(QPixmapAtlas);

    return d->updatePixmap(oldKey, newKey, icon);
}

int QPixmapAtlas::overhead() const
{
    Q_D(const QPixmapAtlas);

    return d->overhead();
}

bool QPixmapAtlas::saveDbg(const QString &path) const
{
    Q_D(const QPixmapAtlas);

    return d->saveDbg(path);
}

bool QPixmapAtlas::save(const QString &path) const
{
    Q_D(const QPixmapAtlas);

    return d->save(path);
}

bool QPixmapAtlas::save(QDataStream& output) const
{
    Q_D(const QPixmapAtlas);

    return d->save(output);
}

bool QPixmapAtlas::load(const QString &path)
{
    Q_D(QPixmapAtlas);

    return d->load(path);
}

bool QPixmapAtlas::load(QDataStream& input)
{
    Q_D(QPixmapAtlas);

    return d->load(input);
}

QList<qint64> QPixmapAtlas::paint(QPainter *painter, const QList<SpriteDrawInfo>& sprites)
{
    Q_D(QPixmapAtlas);

    return d->paint(painter, sprites);
}

QRect QPixmapAtlas::lookupSprite(qint64 key, bool *ok) const
{
    Q_D(const QPixmapAtlas);

    return d->lookupSprite(key, ok);
}

int QPixmapAtlas::numSprites() const
{
    Q_D(const QPixmapAtlas);

    return d->numSprites();
}

QSize QPixmapAtlas::size() const
{
    Q_D(const QPixmapAtlas);

    return d->size();
}

const QAreaAllocator* QPixmapAtlas::allocator() const
{
    Q_D(const QPixmapAtlas);

    return d->allocator();
}
