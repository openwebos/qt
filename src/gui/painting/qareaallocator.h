/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Copyright (C) 2010 Palm Inc
** All rights reserved.
**
** This file is part of the QtGui module of the Palm-modified  Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
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

#ifndef QAREAALLOCATOR_H
#define QAREAALLOCATOR_H

#include <QtCore/qsize.h>
#include <QtCore/qrect.h>
#include <QtCore/qlist.h>
#include <QtCore/QScopedPointer>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

// Palm -->
class QAreaAllocatorPrivate;
// <-- Palm

QT_MODULE(Gui)

// Palm -->
class Q_GUI_EXPORT QAreaAllocator
// <-- Palm
{
public:
// Palm -->
    typedef QScopedPointer<QAreaAllocatorPrivate> DataPtr;
    inline DataPtr& data_ptr() { return d_ptr; }
// <-- Palm

    QAreaAllocator(const QSize &size);
    virtual ~QAreaAllocator();

// Palm -->
    virtual bool clear();
    virtual QAreaAllocator* create(const QSize &size) = 0;

    QSize size() const;

    QSize minimumAllocation() const;
    void setMinimumAllocation(const QSize &size);

    QSize margin() const;
    void setMargin(const QSize &margin);
// <-- Palm

    virtual void expand(const QSize &size);
    void expandBy(const QSize &size);

// Palm -->
    virtual QRect allocate(const QSize &size, bool *allocated = NULL) = 0;
// <-- Palm
    virtual QList<QRect> allocate(const QList<QSize> &sizes);
    virtual void release(const QRect &rect);
    virtual void release(const QList<QRect> &rects);

    virtual int overhead() const;

protected:
// Palm -->
    void setSize(const QSize newSize);

    DataPtr d_ptr;
// <-- Palm

    QSize roundAllocation(const QSize &size) const;

private:
// Palm -->
    Q_DECLARE_PRIVATE(QAreaAllocator);
// <-- Palm
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
