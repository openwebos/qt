/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
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
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QABSTRACTKINETICSCROLLER_H
#define QABSTRACTKINETICSCROLLER_H

#include <QtCore/qmetatype.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QAbstractKineticScrollerPrivate;
class QMouseEvent;
class QGraphicsSceneMouseEvent;

class Q_GUI_EXPORT QAbstractKineticScroller
{
public:
    ~QAbstractKineticScroller();

    enum Mode
    {
        AutoMode,
        PushMode,
        AccelerationMode
    };

    bool isEnabled() const;
    void setEnabled(bool b);

    Mode mode() const;
    void setMode(Mode mode);

    enum OvershootPolicy
    {
        OvershootWhenScrollable,
        OvershootAlwaysOff,
        OvershootAlwaysOn,
    };

    OvershootPolicy overshootPolicy() const;
    void setOvershootPolicy(OvershootPolicy policy);

    bool isLowFrictionEnabled() const;
    void setLowFrictionEnabled(bool b);

    qreal dragInertia() const;
    void setDragInertia(qreal inertia);

    int directionErrorMargin() const;
    void setDirectionErrorMargin(int errorMargin);

    int panningThreshold() const;
    void setPanningThreshold(int threshold);

    qreal decelerationFactor() const;
    void setDecelerationFactor(qreal f);

    qreal fastVelocityFactor() const;
    void setFastVelocityFactor(qreal f);

    qreal minimumVelocity() const;
    void setMinimumVelocity(qreal v);

    qreal maximumVelocity() const;
    void setMaximumVelocity(qreal v);

    qreal axisLockThreshold() const;
    void setAxisLockThreshold(qreal threshold);

    int scrollsPerSecond() const;
    void setScrollsPerSecond(int sps);

    void scrollTo(const QPoint &pos);
    void ensureVisible(const QPoint &pos, int xmargin = 50, int ymargin = 50);

    enum State
    {
        Inactive,
        MousePressed,
        Pushing,
        AutoScrolling
    };

    State state() const;

    void reset();

protected:
    explicit QAbstractKineticScroller();

    virtual QSize viewportSize() const = 0;
    virtual QPoint maximumScrollPosition() const = 0;
    virtual QPoint scrollPosition() const = 0;
    virtual void setScrollPosition(const QPoint &pos, const QPoint &overshootDelta) = 0;

    virtual void stateChanged(State oldState);
    virtual bool canStartScrollingAt(const QPoint &globalPos) const;
    virtual void cancelLeftMouseButtonPress(const QPoint &globalPressPos);

    bool handleMouseEvent(QMouseEvent *e);
    bool handleMouseEvent(QGraphicsSceneMouseEvent *e);

    QAbstractKineticScroller(QAbstractKineticScrollerPrivate &dd);

    QScopedPointer<QAbstractKineticScrollerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QAbstractKineticScroller)
    Q_DECLARE_PRIVATE(QAbstractKineticScroller)
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QAbstractKineticScroller *)

QT_END_HEADER

#endif // QABSTRACTKINETICSCROLLER_H
