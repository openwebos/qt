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

#include <QTime>
#include <QPointer>
#include <QObject>
#include <QAbstractKineticScroller>
#include <QEvent>

QT_BEGIN_NAMESPACE

class QMouseEvent;

class QAbstractKineticScrollerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QAbstractKineticScroller)

public:
    QAbstractKineticScrollerPrivate();
    virtual ~QAbstractKineticScrollerPrivate();
    void init();

    bool handleMousePress(QMouseEvent *e);
    bool handleMouseMove(QMouseEvent *e);
    bool handleMouseRelease(QMouseEvent *e);
    bool handleMouseDblClick(QMouseEvent *e);

    void handleIdleTimer();
    void handleScrollTimer();

protected:
    void timerEvent(QTimerEvent *e);

private:
    void checkMove(QMouseEvent *me, QPoint &delta);
    void handleMove(QMouseEvent *me, QPoint &delta);

    QPointF calculateVelocity(const QPointF &dPixel, int dTime);
    void scrollUpdate(const QPoint &delta);

    void changeState(QAbstractKineticScroller::State);

    void setScrollPositionHelper(const QPoint &p);

    enum {
        MaximumVelocityThreshold = 280,
        MinimumAccelerationThreshold = 40,
        FastClick = 125, // ms
        CursorStoppedTimeout = 200, // ms
        MotionEventsPerSecond =  25,
        AccelFactor = 27,
    };

    QAbstractKineticScroller *q_ptr;

    bool enabled;
    QAbstractKineticScroller::Mode mode;

    // state
    QAbstractKineticScroller::State state;
    QEvent::Type lastType;
    QTime lastTime;
    QTime lastPressTime;
    bool moved;
    bool lastIn;
    bool firstDrag;
    bool lowFrictionMode;
    QAbstractKineticScroller::OvershootPolicy overshootPolicy;

    QPoint pos;
    QPoint ipos; // mouse-press position
    QPoint scrollTo; // the target in case somebody called scrollTo
    QPoint motion;

    // overshoot
    int bounceSteps;
    QPoint maxOvershoot;
    int vmaxOvershoot;
    QPoint overshootDist;
    int overshooting; // the overshooting time in idleTimer steps

    // velocity
    QPointF velocity;
    QPointF oldVelocity;
    qreal minVelocity;
    qreal maxVelocity;
    qreal fastVelocityFactor;
    qreal deceleration;
    QPointF accelerationVelocity;
    int scrollsPerSecond;
    int panningThreshold;
    int directionErrorMargin;
    qreal dragInertia;
    int scrollTime;
    qreal axisLockThreshold;

    // timer
    int idleTimerId;
    int scrollTimerId;
};

QT_END_NAMESPACE
