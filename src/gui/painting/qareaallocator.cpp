/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Copyright (C) 2010 Palm Inc
** All rights reserved.
**
** This file is part of the QtGui module of the Palm-modified Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// Palm -->
#include "qareaallocator.h"
// <-- Palm
#include "qareaallocator_p.h"

QT_BEGIN_NAMESPACE

// Palm -->
class QAreaAllocatorPrivate
{
public:
    QAreaAllocatorPrivate(const QSize &size)
        : m_size(size)
        , m_minAlloc(1, 1)
        , m_margin(0, 0)
    {
    }

    QSize size() const
    {
        return m_size;
    }

    QSize minimumAllocation() const
    {
        return m_minAlloc;
    }

    QSize margin() const
    {
        return m_margin;
    }

    void setSize(const QSize& size)
    {
        m_size = size;
    }

    void setMinimumAllocation(const QSize& size)
    {
        m_minAlloc = size;
    }

    void setMargin(const QSize& margin)
    {
        m_margin = margin;
    }

private:
    QSize m_size;
    QSize m_minAlloc;
    QSize m_margin;
};

// <-- Palm

/*!
    \class QAreaAllocator
    \brief The QAreaAllocator class provides facilities for allocating sub-regions from a rectangular image.
    \since 4.7
    \ingroup qt3d
    \ingroup qt3d::enablers
    \internal

    Performance on a system can sometimes be improved by storing
    multiple small images in a single large image.  This reduces
    memory allocation overhead and GPU state switching costs.

    QAreaAllocator and its subclasses implement standard strategies
    for sub-region allocation in images without tying those strategies
    to specific technologies such as raster, OpenGL, etc.

    Allocations are managed in a virtual two-dimensional space.
    The caller performs the actual texture upload based on the sub-region
    that allocate() returns.

    The caller can return a sub-region to the allocation pool with
    release().  Note that not all strategies support release().

    \sa QSimpleAreaAllocator, QGeneralAreaAllocator
*/

/*!
    \class QSimpleAreaAllocator
    \brief The QSimpleAreaAllocator class provides a simple allocation policy for simple-sized sub-allocations.
    \since 4.7
    \ingroup qt3d
    \ingroup qt3d::enablers
    \internal

    QSimpleAreaAllocator uses a trivial allocation strategy whereby
    sub-regions are allocated in rows, with a new row is started each
    time the previous row fills up.  Space is never reclaimed by
    release().

    This allocator is suitable for use when the allocations will all
    be of a similar size and all regions will be discarded at
    the same time when the allocator is destroyed.  An example would
    be a font glyph manager.

    \sa QAreaAllocator, QGeneralAreaAllocator
*/

/*!
    \class QGeneralAreaAllocator
    \brief The QGeneralAreaAllocator class provides a general allocation policy for sub-regions in an image.
    \since 4.7
    \ingroup qt3d
    \ingroup qt3d::enablers
    \internal

    QGeneralAreaAllocator can handle arbitrary-sized allocations up to
    size(), in any order, and can release() previously allocated regions.
    It uses a binary subdivision algorithm on the image, which may result
    in fragmentation under heavy load.

    While technically any size sub-region up to size() can be allocated,
    once subdivision begins, the sizes that can be allocated will reduce
    substantially.  It is recommended that incoming requests be size() / 4
    or less for best performance.

    If the sub-region sizes to be allocated are very similar, and release()
    is not necessary, then QSimpleAreaAllocator may work better than
    QGeneralAreaAllocator.

    \sa QAreaAllocator, QSimpleAreaAllocator
*/

/*!
    \internal

    Constructs a new area allocator that is initially \a size pixels
    in size.

    \sa expand()
*/
QAreaAllocator::QAreaAllocator(const QSize &size)
// Palm -->
    : d_ptr(new QAreaAllocatorPrivate(size))
// <-- Palm
{
}

/*!
    \internal

    Destroys this area allocator.
*/
QAreaAllocator::~QAreaAllocator()
{
}

// Palm -->
/*!
    \internal

    Attempts to clear all allocated space.
    All implementations may not support this.
*/
bool QAreaAllocator::clear()
{
    return false;
}

QSize QAreaAllocator::size() const
{
    Q_D(const QAreaAllocator);

    return d->size();
}

QSize QAreaAllocator::minimumAllocation() const
{
    Q_D(const QAreaAllocator);

    return d->minimumAllocation();
}

void QAreaAllocator::setMinimumAllocation(const QSize &size)
{
    Q_D(QAreaAllocator);

    d->setMinimumAllocation(size);
}

QSize QAreaAllocator::margin() const
{
    Q_D(const QAreaAllocator);

    return d->margin();
}

void QAreaAllocator::setMargin(const QSize &margin)
{
    Q_D(QAreaAllocator);

    d->setMargin(margin);
}
// <-- Palm

/*!
    \fn QSize QAreaAllocator::size() const
    \internal

    Returns the current size of the area being used by this allocator.
*/

/*!
    \fn QSize QAreaAllocator::minimumAllocation() const
    \internal

    Returns the minimum allocation size in the x and y directions
    for areas returned by allocate().  The default is (1, 1).

    \sa setMinimumAllocation()
*/

/*!
    \fn void QAreaAllocator::setMinimumAllocation(const QSize &size)
    \internal

    Sets the minimum allocation \a size in the x and y directions
    for areas returned by allocate().

    For example, setting the minimum allocation to (8, 8) will force
    all allocations to be aligned on an 8-pixel boundary.

    \sa minimumAllocation()
*/

/*!
    \fn QSize QAreaAllocator::margin() const
    \internal

    Returns the margin that should be left between allocated items
    in the x and y directions.  The default is (0, 0).

    This may be needed when using OpenGL textures because of
    rounding errors in the floating-point representation of
    texture co-ordinates.  Leaving a small margin between allocations
    can help avoid adjacent images from bleeding into each other.

    \sa setMargin()
*/

/*!
    \fn void QAreaAllocator::setMargin(const QSize &margin)
    \internal

    Sets the \a margin that should be left between allocated items
    in the x and y directions.

    \sa margin()
*/

/*!
    \internal

    Expands this allocator to encompass the width and height of \a size.
    If the area is already larger, this function does nothing.

    The rectangles that were returned for previous allocations will
    remain valid.

    \sa expandBy()
*/
void QAreaAllocator::expand(const QSize &size)
{
// Palm -->
    Q_D(QAreaAllocator);

    int newWidth = qMax(d->size().width(), size.width());
    int newHeight = qMax(d->size().height(), size.height());
    d->setSize(QSize(newWidth, newHeight));
// <-- Palm
}

/*!
    \internal

    Expands this allocator by \a size pixels in the x and y directions.

    For example, expanding by (0, 128) will add 128 additional pixels
    of height but will leave the width unchanged.

    \sa expand()
*/
void QAreaAllocator::expandBy(const QSize &size)
{
// Palm -->
    Q_D(QAreaAllocator);

    expand(d->size() + size);
// <-- Palm
}

/*!
    \fn QRect QAreaAllocator::allocate(const QSize &size)
    \internal

    Allocates \a size pixels from this allocator and returns the rectangle
    that should be used by the caller.  Returns a null rectangle if
    this allocator does not have sufficient space to accomodate \a size.

    \sa release()
*/

/*!
    \internal

    Allocates and returns a list of rectangles corresponding to the
    elements of \a sizes.  The returned list will have less elements
    than \a sizes if there is insufficient space to accomodate
    all of the allocation requests.  The values that are in the returned
    list will be allocated and need to be passed to release() to
    deallocate them.

    The default implementation will call the subclass allocate() once
    for each size until all \a sizes have been allocated or an
    allocation fails.  Subclasses may override this method to
    reorder the allocations for best-fit.

    \sa release()
*/
QList<QRect> QAreaAllocator::allocate(const QList<QSize> &sizes)
{
    QList<QRect> rects;
    QRect rect;
    for (int index = 0; index < sizes.count(); ++index) {
        rect = allocate(sizes[index]);
        if (rect.isNull())
            break;
        rects.append(rect);
    }
    return rects;
}

/*!
    \internal

    Releases the space occupied by \a rect back to the allocator.
    The default implementation does nothing.

    The \a rect must have been returned by a previous call to allocate().
    Otherwise the behaviour is undefined.

    \sa allocate()
*/
void QAreaAllocator::release(const QRect &rect)
{
    Q_UNUSED(rect);
}

/*!
    \internal

    Releases the space occupied by the members of \a rects back to
    the allocator.  The default implementation calls release() for
    each rectangle in the list.

    The members of \a rects must have been returned by previous calls
    to allocate().  Otherwise the behaviour is undefined.

    \sa allocate()
*/
void QAreaAllocator::release(const QList<QRect> &rects)
{
    for (int index = 0; index < rects.count(); ++index)
        release(rects[index]);
}

/*!
    \internal

    Returns a rough estimate of the number of bytes of overhead that
    are currently in use to store the house-keeping data structures
    for this area allocator.  The default implementation returns zero.
*/
int QAreaAllocator::overhead() const
{
    return 0;
}

/*!
    \internal

    Returns \a size, after rounding it up to account for
    minimumAllocation() and margin().

    This is a convenience function, provided for subclass overrides
    of allocate().

    \sa allocate()
*/
QSize QAreaAllocator::roundAllocation(const QSize &size) const
{
// Palm -->
    Q_D(const QAreaAllocator);

    int width = size.width() + d->margin().width();
    int height = size.height() + d->margin().height();
    int extra = width % d->minimumAllocation().width();
    if (extra)
        width += d->minimumAllocation().width() - extra;
    extra = height % d->minimumAllocation().height();
    if (extra)
        height += d->minimumAllocation().height() - extra;
    return QSize(width, height);
// <-- Palm
}

/*!
    \internal

    Constructs a simple area allocator that is initially \a size pixels
    in size.
*/
QSimpleAreaAllocator::QSimpleAreaAllocator(const QSize &size)
    : QAreaAllocator(size)
    , m_row(0)
    , m_column(0)
    , m_rowHeight(0)
{
}

/*!
    \internal

    Destroys this simple area allocator.
*/
QSimpleAreaAllocator::~QSimpleAreaAllocator()
{
}

// Palm -->
/*!
    \internal
*/
bool QSimpleAreaAllocator::clear()
{
    m_row = 0;
    m_column = 0;
    m_rowHeight = 0;
    return true;
}
// <-- Palm

/*!
    \internal
*/
// Palm -->
QRect QSimpleAreaAllocator::allocate(const QSize &size, bool *allocated)
// <-- Palm
{
    // Round up the allocation size to account for the margin and
    // minimum allocation parameters.
    QSize rounded = roundAllocation(size);
    int width = rounded.width();
    int height = rounded.height();

    // Bail out if the size is obviously too small or too big.
// Palm -->
    if (width == 0 && height == 0) {
        // 0-length allocations don't take any space
        // we can satisfy as many of them as needed
        if (allocated) *allocated = true;
        return QRect();
    }
// Palm -->
    if (width <= 0 || width > this->size().width()) {
// <-- Palm
        if (allocated) *allocated = false;
        return QRect();
    }
// Palm -->
    if (height <= 0 || height > (this->size().height() - m_row)) {
// <-- Palm
        if (allocated) *allocated = false;
        return QRect();
    }
// <-- Palm

    // Do we need to place this allocation on a new row?
    int row = m_row;
    int column = m_column;
    int rowHeight = m_rowHeight;
// Palm -->
    if ((column + width) > this->size().width()) {
// <-- Palm
        row += m_rowHeight;
        column = 0;
        rowHeight = 0;
// Palm -->
        if (height > (this->size().height() - row)) {
            if (allocated) *allocated = false;
            return QRect();
        }
// <-- Palm
    }

    // Update the current allocation position.
    m_row = row;
    m_column = column + width;
    m_rowHeight = qMax(rowHeight, height);

// Palm -->
    if (allocated) *allocated = true;
// <-- Palm
    // Return the allocation, using the original size without rounding.
    return QRect(column, row, size.width(), size.height());
}

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

/*!
    \internal

    Constructs a general area allocator that is initially \a size pixels
    in size.  The \a size will be rounded up to the next power of two,
    to simplify the internal allocation policy.

    This constructor sets minimumAllocation() to (8, 8) to reduce the
    housekeeping overhead of the internal data structures.
*/
QGeneralAreaAllocator::QGeneralAreaAllocator(const QSize &size)
    : QAreaAllocator(nextPowerOfTwo(size))
{
    m_root = new Node();
// Palm -->
    m_root->rect = QRect(0, 0, this->size().width(), this->size().height());
    m_root->largestFree = this->size();
// <-- Palm
    m_root->parent = 0;
    m_root->left = 0;
    m_root->right = 0;
    m_nodeCount = 1;
    setMinimumAllocation(QSize(8, 8));
}

/*!
    \internal

    Destroys this general area allocator.
*/
QGeneralAreaAllocator::~QGeneralAreaAllocator()
{
    freeNode(m_root);
}

/*!
    \internal
*/
void QGeneralAreaAllocator::freeNode(Node *node)
{
    if (node) {
        freeNode(node->left);
        freeNode(node->right);
    }
    delete node;
}

/*!
    \internal

    The \a size will be rounded up to the next power of two.
    Use size() to determine the actual size after expansion.
*/
void QGeneralAreaAllocator::expand(const QSize &size)
{
    QAreaAllocator::expand(nextPowerOfTwo(size));

// Palm -->
    if (m_root->rect.size() == this->size())
// <-- Palm
        return;     // No change.
    if (!m_root->left && m_root->largestFree.width() > 0) {
        // No allocations have occurred, so just adjust the root size.
// Palm -->
        m_root->rect = QRect(0, 0, this->size().width(), this->size().height());
        m_root->largestFree = this->size();
// <-- Palm
        return;
    }

    // Add extra nodes above the current root to expand the tree.
    Node *oldRoot = m_root;
    Split split;
// Palm -->
    if (this->size().width() >= this->size().height())
// <-- Palm
        split = SplitOnX;
    else
        split = SplitOnY;
// Palm -->
    while (m_root->rect.size() != this->size()) {
        if (m_root->rect.width() == this->size().width())
// <-- Palm
            split = SplitOnY;
// Palm -->
        else if (m_root->rect.height() == this->size().height())
// <-- Palm
            split = SplitOnX;
        Node *parent = new Node();
        Node *right = new Node();
        m_nodeCount += 2;
        m_root->parent = parent;
        parent->parent = 0;
        parent->left = m_root;
        parent->right = right;
        parent->largestFree = m_root->rect.size();
        right->parent = parent;
        right->left = 0;
        right->right = 0;
        right->largestFree = m_root->rect.size();
        if (split == SplitOnX) {
            parent->rect = QRect(m_root->rect.x(), m_root->rect.y(),
                                 m_root->rect.width() * 2,
                                 m_root->rect.height());
            right->rect = QRect(m_root->rect.x() + m_root->rect.width(),
                                m_root->rect.y(),
                                m_root->rect.width(), m_root->rect.height());
        } else {
            parent->rect = QRect(m_root->rect.x(), m_root->rect.y(),
                                 m_root->rect.width(),
                                 m_root->rect.height() * 2);
            right->rect = QRect(m_root->rect.x(),
                                m_root->rect.y() + m_root->rect.width(),
                                m_root->rect.width(), m_root->rect.height());
        }
        split = (split == SplitOnX ? SplitOnY : SplitOnX);
        m_root = parent;
    }
    updateLargestFree(oldRoot);
}

static inline bool fitsWithin(const QSize &size1, const QSize &size2)
{
    return size1.width() <= size2.width() && size1.height() <= size2.height();
}

/*!
    \internal
*/
// Palm -->
QRect QGeneralAreaAllocator::allocate(const QSize &size, bool *allocated)
// <-- Palm
{
    QSize rounded = roundAllocation(size);
    rounded = QSize(nextPowerOfTwo(rounded.width()),
                    nextPowerOfTwo(rounded.height()));
// Palm -->
    if (rounded.width() == 0 && rounded.height() == 0) {
        if (allocated) *allocated = true;
        return QRect();
    }
    if (rounded.width() <= 0 || rounded.width() > this->size().width() ||
            rounded.height() <= 0 || rounded.height() > this->size().height()) {
// <-- Palm
        if (allocated) *allocated = false;
        return QRect();
    }
    QPoint point = allocateFromNode(rounded, m_root);
// Palm -->
    if (point.x() >= 0) {
        if (allocated) *allocated = true;
        return QRect(point, size);
    } else {
        if (allocated) *allocated = false;
        return QRect();
    }
// <-- Palm
}

/*!
    \internal
*/
QPoint QGeneralAreaAllocator::allocateFromNode(const QSize &size, Node *node)
{
    // Find the best node to insert into, which should be
    // a node with the least amount of unused space that is
    // big enough to contain the requested size.
    while (node != 0) {
        // Go down a level and determine if the left or right
        // sub-tree contains the best chance of allocation.
        Node *left = node->left;
        Node *right = node->right;
        if (left && fitsWithin(size, left->largestFree)) {
            if (right && fitsWithin(size, right->largestFree)) {
                if (left->largestFree.width() < right->largestFree.width() ||
                    left->largestFree.height() < right->largestFree.height()) {
                    // The largestFree values may be a little oversized,
                    // so try the left sub-tree and then the right sub-tree.
                    QPoint point = allocateFromNode(size, left);
                    if (point.x() >= 0)
                        return point;
                    else
                        return allocateFromNode(size, right);
                } else {
                    node = right;
                }
            } else {
                node = left;
            }
        } else if (right && fitsWithin(size, right->largestFree)) {
            node = right;
        } else if (left || right) {
            // Neither sub-node has enough space to allocate from.
            return QPoint(-1, -1);
        } else if (fitsWithin(size, node->largestFree)) {
            // Do we need to split this node into smaller pieces?
            Split split;
            if (fitsWithin(QSize(size.width() * 2, size.height() * 2),
                           node->largestFree)) {
                // Split in either direction: choose the inverse of
                // the parent node's split direction to try to balance
                // out the wasted space as further subdivisions happen.
                if (node->parent &&
                        node->parent->left->rect.x() ==
                            node->parent->right->rect.x())
                    split = SplitOnX;
                else if (node->parent)
                    split = SplitOnY;
                else if (node->rect.width() >= node->rect.height())
                    split = SplitOnX;
                else
                    split = SplitOnY;
            } else if (fitsWithin(QSize(size.width() * 2, size.height()),
                                  node->largestFree)) {
                // Split along the X direction.
                split = SplitOnX;
            } else if (fitsWithin(QSize(size.width(), size.height() * 2),
                                  node->largestFree)) {
                // Split along the Y direction.
                split = SplitOnY;
            } else {
                // Cannot split further - allocate this node.
                node->largestFree = QSize(0, 0);
                updateLargestFree(node);
                return node->rect.topLeft();
            }

            // Split the node, then go around again using the left sub-tree.
            node = splitNode(node, split);
        } else {
            // Cannot possibly fit into this node.
            break;
        }
    }
    return QPoint(-1, -1);
}

/*!
    \internal
*/
QGeneralAreaAllocator::Node *QGeneralAreaAllocator::splitNode
    (Node *node, Split split)
{
    Node *left = new Node();
    Node *right = new Node();
    m_nodeCount += 2;
    left->parent = node;
    left->left = 0;
    left->right = 0;
    right->parent = node;
    right->left = 0;
    right->right = 0;
    node->left = left;
    node->right = right;
    if (split == SplitOnX) {
        left->rect = QRect(node->rect.x(), node->rect.y(),
                           node->rect.width() / 2,
                           node->rect.height());
        right->rect = QRect(left->rect.right() + 1, node->rect.y(),
                            node->rect.width() / 2,
                            node->rect.height());
    } else {
        left->rect = QRect(node->rect.x(), node->rect.y(),
                           node->rect.width(),
                           node->rect.height() / 2);
        right->rect = QRect(node->rect.x(), left->rect.bottom() + 1,
                            node->rect.width(),
                            node->rect.height() / 2);
    }
    left->largestFree = left->rect.size();
    right->largestFree = right->rect.size();
    node->largestFree = right->largestFree;
    return left;
}

/*!
    \internal
*/
void QGeneralAreaAllocator::updateLargestFree(Node *node)
{
    while ((node = node->parent) != 0) {
        node->largestFree =
            QSize(qMax(node->left->largestFree.width(),
                       node->right->largestFree.width()),
                  qMax(node->left->largestFree.height(),
                       node->right->largestFree.height()));
    }
}

/*!
    \internal
*/
void QGeneralAreaAllocator::release(const QRect &rect)
{
    // Locate the node that contains the allocated region.
    Node *node = m_root;
    QPoint point = rect.topLeft();
    while (node != 0) {
        if (node->left && node->left->rect.contains(point))
            node = node->left;
        else if (node->right && node->right->rect.contains(point))
            node = node->right;
        else if (node->rect.contains(point))
            break;
        else
            return;     // Point is completely outside the tree.
    }
    if (!node)
        return;

    // Mark the node as free and then work upwards through the tree
    // recombining and deleting nodes until we reach a sibling
    // that is still allocated.
    node->largestFree = node->rect.size();
    while (node->parent) {
        if (node->parent->left == node) {
            if (node->parent->right->largestFree !=
                    node->parent->right->rect.size())
                break;
        } else {
            if (node->parent->left->largestFree !=
                    node->parent->left->rect.size())
                break;
        }
        node = node->parent;
        freeNode(node->left);
        freeNode(node->right);
        m_nodeCount -= 2;
        node->left = 0;
        node->right = 0;
        node->largestFree = node->rect.size();
    }

    // Make the rest of our ancestors have the correct "largest free size".
    updateLargestFree(node);
}

/*!
    \internal
*/
int QGeneralAreaAllocator::overhead() const
{
    return m_nodeCount * sizeof(Node);
}

QT_END_NAMESPACE
