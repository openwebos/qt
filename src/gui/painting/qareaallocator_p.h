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

#ifndef QAREAALLOCATOR_P_H
#define QAREAALLOCATOR_P_H

#include "qareaallocator.h"

#include <QtCore/qsize.h>
#include <QtCore/qrect.h>
#include <QtCore/qlist.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

// Palm -->
class QSimpleAreaAllocator : public QAreaAllocator
// <-- Palm
{
public:
    QSimpleAreaAllocator(const QSize &size);
    virtual ~QSimpleAreaAllocator();

// Palm -->
    bool clear();
// <-- Palm

// Palm -->
    QRect allocate(const QSize &size, bool *allocated = NULL);
    virtual QAreaAllocator* create(const QSize &size) { return new QSimpleAreaAllocator(size); }
// <-- Palm

private:
    int m_row;
    int m_column;
    int m_rowHeight;
};

// Palm -->
class QGeneralAreaAllocator : public QAreaAllocator
// <-- Palm
{
public:
    QGeneralAreaAllocator(const QSize &size);
    virtual ~QGeneralAreaAllocator();

    void expand(const QSize &size);
// Palm -->
    QRect allocate(const QSize &size, bool *allocated = NULL);
    virtual QAreaAllocator* create(const QSize &size) { return new QGeneralAreaAllocator(size); }
// <-- Palm
    void release(const QRect &rect);
    int overhead() const;

private:
    enum Split { SplitOnX, SplitOnY };

    struct Node
    {
        QRect rect;
        QSize largestFree;
        Node *parent;
        Node *left;
        Node *right;
    };

    Node *m_root;
    int m_nodeCount;

    static void freeNode(Node *node);
    QPoint allocateFromNode(const QSize &size, Node *node);
    Node *splitNode(Node *node, Split split);
    static void updateLargestFree(Node *node);
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
