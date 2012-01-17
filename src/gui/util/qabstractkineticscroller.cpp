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
#include <qabstractkineticscroller.h>
#include "qabstractkineticscroller_p.h"


#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QScrollBar>
#include <QAbstractItemView>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>

#include <QtDebug>

QT_BEGIN_NAMESPACE

//#define KINETIC_SCROLLER_DEBUG

#ifdef KINETIC_SCROLLER_DEBUG
#  define qKSDebug  qDebug
#else
#  define qKSDebug  while (false) qDebug
#endif


/*!
    \class QAbstractKineticScroller
    \brief The QAbstractKineticScroller class enables kinetic scrolling for any scrolling widget or graphics item.
    \ingroup qtmaemo5
    \since 4.6
    \preliminary

    With kinetic scrolling, the user can push the widget in a given
    direction and it will continue to scroll in this direction until it is
    stopped either by the user or by friction.  Aspects of inertia, friction
    and other physical concepts can be changed in order to fine-tune an
    intuitive user experience.

    To enable kinetic scrolling for a widget or graphics item, you need to
    derive from this class and implement at least all the pure-virtual
    functions.

    Qt for Maemo 5 already comes with two implementations for
    QAbstractScrollArea and QWebView, and those kinetic scrollers are
    automatically instantiated and attached to these widgets on creation.
    In the QAbstractScrollArea case, the kinetic scroller is initially
    disabled. However, for QAbstractItemView and QScrollArea derived classes
    it is enabled by default.  You can obtain these automatically created
    objects via a dynamic property:

    \code
    // disable the kinetic scroller on scrollArea
    QAbstractKineticScroller *scroller = scrollArea->property("kineticScroller")
                                             .value<QAbstractKineticScroller *>();
    if (scroller)
        scroller->setEnabled(false);
    \endcode

    In addition there is also an example on how you would add kinetic
    scrolling to a QGraphicsView based application in \c maemobrowser
    examples in the \c maemo5 examples directory.

    The kinetic scroller installs an event filter on the widget to handle mouse
    presses and moves on the widget \mdash presses and moves on a device's touch screen
    are also handled by this mechanism. These events will be interpreted as scroll actions
    depending on the current state() of the scroller.

    Even though this kinetic scroller has a huge number of settings, we
    recommend that you leave them all at their default values.  In case you
    really want to change them you can try out the \c kineticscroller
    example in the \c maemo5 examples directory.

    \sa QWidget
*/


/*!
    Constructs a new kinetic scroller.
*/
QAbstractKineticScroller::QAbstractKineticScroller()
    : d_ptr(new QAbstractKineticScrollerPrivate())
{
    Q_D(QAbstractKineticScroller);
    d->q_ptr = this;
    d->init();
}

/*! \internal
*/
QAbstractKineticScroller::QAbstractKineticScroller(QAbstractKineticScrollerPrivate &dd)
    : d_ptr(&dd)
{
    Q_D(QAbstractKineticScroller);
    d->q_ptr = this;
    d->init();
}

/*!
    Destroys the scroller.
*/
QAbstractKineticScroller::~QAbstractKineticScroller()
{
}

/*!
    Resets the internal state of the kinetic scroller. This function is not
    needed for normal use.  This function only needs to be called if the
    kinetic scroller is being re-attached to a different widget.
*/
void QAbstractKineticScroller::reset()
{
    Q_D(QAbstractKineticScroller);

    d->changeState(Inactive);
    if (d->idleTimerId)
        d->killTimer(d->idleTimerId);
    d->idleTimerId = 0;
    if (d->scrollTimerId)
        d->killTimer(d->scrollTimerId);
    d->scrollTimerId = 0;
    d->velocity = d->oldVelocity = QPointF(0, 0);
    d->overshootDist = QPoint(0, 0);
    d->overshooting = 0;
}

/*!
    \enum QAbstractKineticScroller::State

    This enum describes the possible states the kinetic scroller can be in.

    \value Inactive The scroller is inactive. It may also have been disabled.
    \value MousePressed The user has pressed the mouse button (or pressed the
                        the touch screen).
    \value Pushing      The user is dragging the mouse cursor (or other input
                        point) over the scroll area.
    \value AutoScrolling Scrolling is occurring without direct user input.
*/

/*!
    Returns the current state of the kinetic scroller.
*/
QAbstractKineticScroller::State QAbstractKineticScroller::state() const
{
    Q_D(const QAbstractKineticScroller);
    return d->state;
}

/*!
    \fn bool QAbstractKineticScroller::handleMouseEvent(QMouseEvent *event)

    This filter function changes, blocks or creates mouse events passed as \a event
    for the attached widget depending on the detected kinetic scrolling state.

    Subclass implementations need to call this function for every mouse
    event.

    The return value of this function is analogous to QObject::eventFilter().
    Returns true if the event should be filtered out (not handled further);
    otherwise returns false.
*/
bool QAbstractKineticScroller::handleMouseEvent(QMouseEvent *e)
{
    Q_D(QAbstractKineticScroller);
    if (!e || !d->enabled)
        return false;

    bool swallow = false;

    switch (e->type()) {
    case QEvent::MouseButtonPress:
        swallow = d->handleMousePress(e);
        break;
    case QEvent::MouseMove:
        swallow = d->handleMouseMove(e);
        break;
    case QEvent::MouseButtonRelease:
        swallow = d->handleMouseRelease(e);
        break;
    case QEvent::MouseButtonDblClick:
        break;

    default:
        qWarning("QAbstractKineticScroller::handleMouseEvent() received a non-mouse event (type=%d)", e->type());
        break;
    }
    if (swallow)
        e->setAccepted(swallow);
    return swallow;
}

/*!
    \fn bool QAbstractKineticScroller::handleMouseEvent(QGraphicsSceneMouseEvent *event)
    \overload

    This filter function changes, blocks or creates mouse events passed as \a event
    for the attached graphics item depending on the detected kinetic scrolling state.

    \sa handleMouseEvent()
*/
bool QAbstractKineticScroller::handleMouseEvent(QGraphicsSceneMouseEvent *e)
{
    if (!e)
        return false;

    QEvent::Type mapped = QEvent::None;

    switch (e->type()) {
    case QEvent::GraphicsSceneMousePress:
        mapped = QEvent::MouseButtonPress;
        break;
    case QEvent::GraphicsSceneMouseMove:
        mapped = QEvent::MouseMove;
        break;
    case QEvent::GraphicsSceneMouseRelease:
        mapped = QEvent::MouseButtonRelease;
        break;
    case QEvent::GraphicsSceneMouseDoubleClick:
        mapped = QEvent::MouseButtonDblClick;
        break;
    default:
        break;
    }
    if (mapped != QEvent::None) {
        QMouseEvent me(mapped, e->pos().toPoint(), e->screenPos(),
                       e->button(), e->buttons(), e->modifiers());
        bool swallow = handleMouseEvent(&me);
        e->setAccepted(me.isAccepted());
        return swallow;
    } else {
        qWarning("QAbstractKineticScroller::handleMouseEvent() received a non-mouse event (type=%d)", e->type());
        return false;
    }
}

/*! \internal
    The timer event will be triggered by the scroll and idle timer.
*/
void QAbstractKineticScrollerPrivate::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == idleTimerId)
        handleIdleTimer();
    else if (te->timerId() == scrollTimerId)
        handleScrollTimer();
}

QAbstractKineticScrollerPrivate::QAbstractKineticScrollerPrivate()
    : enabled(true), mode(QAbstractKineticScroller::AutoMode), state(QAbstractKineticScroller::Inactive),
    lastType(QEvent::User), moved(false), lastIn(true),
    firstDrag(true), lowFrictionMode(false), overshootPolicy(QAbstractKineticScroller::OvershootWhenScrollable),
    scrollTo(-1, -1), bounceSteps(3), maxOvershoot(150, 150), vmaxOvershoot(130),
    overshootDist(0, 0), overshooting(0),
    minVelocity(10), maxVelocity(3500), fastVelocityFactor(0.01), deceleration(0.85),
    scrollsPerSecond(20), panningThreshold(25), directionErrorMargin(10),
    dragInertia(0.85), scrollTime(1000), axisLockThreshold(0),
    idleTimerId(0), scrollTimerId(0)
{ }

QAbstractKineticScrollerPrivate::~QAbstractKineticScrollerPrivate()
{ }

void QAbstractKineticScrollerPrivate::init()
{ }

void QAbstractKineticScrollerPrivate::changeState(QAbstractKineticScroller::State newState)
{
    Q_Q(QAbstractKineticScroller);

    if (newState != state) {
        QAbstractKineticScroller::State oldState = state;
        state = newState;
        q->stateChanged(oldState);
    }
}

void QAbstractKineticScrollerPrivate::handleScrollTimer()
{
    Q_Q(QAbstractKineticScroller);

    qKSDebug("handleScrollTimer: %d/%d", motion.x(), motion.y());

    if (!motion.isNull())
        setScrollPositionHelper(q->scrollPosition() - overshootDist - motion);
    killTimer(scrollTimerId);
    scrollTimerId = 0;
}

void QAbstractKineticScrollerPrivate::handleIdleTimer()
{
    Q_Q(QAbstractKineticScroller);

    if (mode == QAbstractKineticScroller::PushMode && overshootDist.isNull()) {
        killTimer(idleTimerId);
        idleTimerId = 0;
        changeState(QAbstractKineticScroller::Inactive);
        return;
    }
    qKSDebug() << "idle timer - velocity: " << velocity << " overshoot: " << overshootDist;

    setScrollPositionHelper(q->scrollPosition() - overshootDist - velocity.toPoint());

    if (!overshootDist.isNull()) {
        if (moved)
            return;

        overshooting++;
        scrollTo = QPoint(-1, -1);

        /* When the overshoot has started we continue for
         * PROP_BOUNCE_STEPS more steps into the overshoot before we
         * reverse direction. The deceleration factor is calculated
         * based on the percentage distance from the first item with
         * each iteration, therefore always returning us to the
         * top/bottom most element
         */
        if (overshooting < bounceSteps) {
            velocity.setX( overshootDist.x() / maxOvershoot.x() * velocity.x() );
            velocity.setY( overshootDist.y() / maxOvershoot.y() * velocity.y() );
        } else {
            velocity.setX( -overshootDist.x() * 0.8 );
            velocity.setY( -overshootDist.y() * 0.8 );

            // ensure a minimum speed when scrolling back or else we might never return
            if (velocity.x() > -1.0 && velocity.x() < 0.0)
                velocity.setX(-1.0);
            if (velocity.x() <  1.0 && velocity.x() > 0.0)
                velocity.setX( 1.0);
            if (velocity.y() > -1.0 && velocity.y() < 0.0)
                velocity.setY(-1.0);
            if (velocity.y() <  1.0 && velocity.y() > 0.0)
                velocity.setY( 1.0);
        }

        velocity.setX( qBound((qreal)-vmaxOvershoot, velocity.x(), (qreal)vmaxOvershoot));
        velocity.setY( qBound((qreal)-vmaxOvershoot, velocity.y(), (qreal)vmaxOvershoot));

    } else if (state == QAbstractKineticScroller::AutoScrolling) {
        // Decelerate gradually when pointer is raised
        overshooting = 0;

        // in case we move to a specific point do not decelerate when arriving
        if (scrollTo.x() != -1 || scrollTo.y() != -1) {

            // -- check if target was reached
            QPoint pos = q->scrollPosition();
            QPointF  dist = QPointF(scrollTo - pos);

            qKSDebug() << "handleIdleTimer dist:" << dist << " scrollTo:" << scrollTo;

            // -- break if target was reached
            if ((velocity.x() < 0.0 && dist.x() <= 0.0) ||
                (velocity.x() > 0.0 && dist.x() >= 0.0) )
                velocity.setX(0.0);

            if ((velocity.y() < 0.0 && dist.y() <= 0.0) ||
                (velocity.y() > 0.0 && dist.y() >= 0.0) )
                velocity.setY(0.0);

            // -- break if we reached the borders
            /*
            QPoint maxPos = q->maximumScrollPosition();
            if ((0 > pos.x()-velocity.x() && velocity.x() > 0.0) ||
                (maxPos.x() < pos.x()-velocity.x() && velocity.x() < 0.0))
                velocity.setX(0.0);

            if ((0 > pos.y()-velocity.y() && velocity.y() > 0.0) ||
                (maxPos.y() < pos.y()-velocity.y() && velocity.y() < 0.0))
                velocity.setY(0.0);
            */

            if (velocity.x() == 0.0 && velocity.y() == 0.0) {
                killTimer(idleTimerId);
                idleTimerId = 0;
                changeState(QAbstractKineticScroller::Inactive);
                return;
            }

            // -- don't get too slow if target was not yet reached
            if (qAbs(velocity.x()) >= qreal(1.5))
                velocity.rx() *= deceleration;
            if (qAbs(velocity.y()) >= qreal(1.5))
                velocity.ry() *= deceleration;

        } else {
            if (!lowFrictionMode || (qAbs(velocity.x()) < qreal(0.8) * maxVelocity))
                velocity.rx() *= deceleration;
            if (!lowFrictionMode || (qAbs(velocity.y()) < qreal(0.8) * maxVelocity))
                velocity.ry() *= deceleration;

            if ((qAbs(velocity.x()) < qreal(1.0)) && (qAbs(velocity.y()) < qreal(1.0))) {
                velocity = QPointF(0, 0);
                killTimer(idleTimerId);
                idleTimerId = 0;
                changeState(QAbstractKineticScroller::Inactive);
            }
        }
    } else if (mode == QAbstractKineticScroller::AutoMode) {
        killTimer(idleTimerId);
        idleTimerId = 0;
        changeState(QAbstractKineticScroller::Inactive);
    }
}

bool QAbstractKineticScrollerPrivate::handleMousePress(QMouseEvent *e)
{
    Q_Q(QAbstractKineticScroller);
    qKSDebug("MP: start");
    if (e->button() != Qt::LeftButton)
        return false;

    QPoint maxPos = q->maximumScrollPosition();
    bool canScrollX = (maxPos.x() > 0);
    bool canScrollY = (maxPos.y() > 0);

    if ((!canScrollX || !canScrollY) && (overshootPolicy == QAbstractKineticScroller::OvershootAlwaysOn))
        canScrollX = canScrollY = true;

    scrollTo = QPoint(-1, -1);

    pos = e->globalPos();
    ipos = pos;

    bool swallow = true;

    // don't allow a click if we're still moving fast
    if ((qAbs(velocity.x()) <= (maxVelocity * fastVelocityFactor)) &&
        (qAbs(velocity.y()) <= (maxVelocity * fastVelocityFactor))) {
        swallow = false;

        if (!q->canStartScrollingAt(e->globalPos()))
            canScrollX = canScrollY = false;
    }

    // stop scrolling on mouse press (so you can flick, then hold to stop)
    oldVelocity = velocity;
    velocity = QPointF(0, 0);

    if (idleTimerId) {
        killTimer(idleTimerId);
        idleTimerId = 0;
        changeState(QAbstractKineticScroller::Inactive);
    }

    if (canScrollX || canScrollY) {
        lastTime.start();
        lastPressTime.start();
        lastType = e->type();

        changeState(QAbstractKineticScroller::MousePressed);
    }

    qKSDebug("MP: end %d", swallow);
    e->setAccepted(true);
    return swallow;
}

bool QAbstractKineticScrollerPrivate::handleMouseMove(QMouseEvent *e)
{
    Q_Q(QAbstractKineticScroller);
    qKSDebug() << "MM: pos: " << e->globalPos() << " - time: " << QTime::currentTime().msec();
    if (!(e->buttons() & Qt::LeftButton))
        return false;
    if ((state != QAbstractKineticScroller::MousePressed) &&
        (state != QAbstractKineticScroller::Pushing) &&
        (state != QAbstractKineticScroller::AutoScrolling))
        return false;
    if (moved && !lastTime.elapsed())
        return true;
    if (lastType == QEvent::MouseButtonPress)
        firstDrag = true;

    QPoint delta = e->globalPos() - pos;

    if (!moved) {
        checkMove(e, delta);
    }

    if (moved) {
        if (state != QAbstractKineticScroller::Pushing) {
            q->cancelLeftMouseButtonPress(ipos);

            changeState(QAbstractKineticScroller::Pushing);
        }
        handleMove(e, delta);
    }
    lastTime.restart();
    lastType = e->type();

    qKSDebug("MM: end %d", moved);
    return moved;
}

bool QAbstractKineticScrollerPrivate::handleMouseRelease(QMouseEvent *e)
{
    qKSDebug() << "MR: pos: " << e->globalPos() << " - time: " << QTime::currentTime().msec();
    Q_Q(QAbstractKineticScroller);

    if (e->button() != Qt::LeftButton)
        return false;
    if ((state != QAbstractKineticScroller::MousePressed) && (state != QAbstractKineticScroller::Pushing))
        return false;

    // if last event was a motion-notify we have to check the
    // movement and launch the animation
    if (lastType == QEvent::MouseMove) {
        if (moved) {
            QPoint delta = e->globalPos() - pos;
            handleMove(e, delta);

            // move all the way to the last position now
            if (scrollTimerId) {
                killTimer(scrollTimerId);
                scrollTimerId = 0;
                setScrollPositionHelper(q->scrollPosition() - overshootDist - motion);
                motion = QPoint(0, 0);
            }
        }
    }
    // If overshoot has been initiated with a finger down,
    // on release set max speed
    if (overshootDist.x()) {
        overshooting = bounceSteps; // Hack to stop a bounce in the finger down case
        velocity.setX(-overshootDist.x() * qreal(0.8));

    }
    if (overshootDist.y()) {
        overshooting = bounceSteps; // Hack to stop a bounce in the finger down case
        velocity.setY(-overshootDist.y() * qreal(0.8));
    }

    bool forceFast = true;

    // if widget was moving fast in the panning, increase speed even more
    if ((lastPressTime.elapsed() < FastClick) &&
        ((qAbs(oldVelocity.x()) > minVelocity) ||
         (qAbs(oldVelocity.y()) > minVelocity)) &&
        ((qAbs(oldVelocity.x()) > MinimumAccelerationThreshold) ||
         (qAbs(oldVelocity.y()) > MinimumAccelerationThreshold))) {

        qKSDebug() << "FAST CLICK - using oldVelocity " << oldVelocity;
        int signX = 0, signY = 0;
        if (velocity.x())
            signX = (velocity.x() > 0) == (oldVelocity.x() > 0) ? 1 : -1;
        if (velocity.y())
            signY = (velocity.y() > 0) == (oldVelocity.y() > 0) ? 1 : -1;

        // calculate the acceleration velocity
        QPoint maxPos = q->maximumScrollPosition();
        accelerationVelocity  = QPoint(0, 0);
        QSize size = q->viewportSize();
        if (size.width())
            accelerationVelocity.setX(qMin(int(q->maximumVelocity()), maxPos.x() / size.width() * AccelFactor));
        if (size.height())
            accelerationVelocity.setY(qMin(int(q->maximumVelocity()), maxPos.y() / size.height() * AccelFactor));

        velocity.setX(signX * (oldVelocity.x() + (oldVelocity.x() > 0 ? accelerationVelocity.x() : -accelerationVelocity.x())));
        velocity.setY(signY * (oldVelocity.y() + (oldVelocity.y() > 0 ? accelerationVelocity.y() : -accelerationVelocity.y())));
        forceFast = false;
    }

    if ((qAbs(velocity.x()) >= minVelocity) ||
        (qAbs(velocity.y()) >= minVelocity)) {

        qKSDebug() << "over min velocity: " << velocity;
        // we have to move because we are in overshooting position
        if (!moved) {
            // (opt) emit panningStarted()
        }
        // (must) enable scrollbars

        if (forceFast) {
            if ((qAbs(velocity.x()) > MaximumVelocityThreshold) &&
                (accelerationVelocity.x() > MaximumVelocityThreshold)) {
                velocity.setX(velocity.x() > 0 ? accelerationVelocity.x() : -accelerationVelocity.x());
            }
            if ((qAbs(velocity.y()) > MaximumVelocityThreshold) &&
                (accelerationVelocity.y() > MaximumVelocityThreshold)) {
                velocity.setY(velocity.y() > 0 ? accelerationVelocity.y() : -accelerationVelocity.y());
            }
            qKSDebug() << "Force fast is on - velocity: " << velocity;

        }
        changeState(QAbstractKineticScroller::AutoScrolling);

    } else {
        if (moved) {
            // (opt) emit panningFinished()
        }
        changeState(QAbstractKineticScroller::Inactive);
    }

    // -- create the idle timer if we are auto scrolling or overshooting.
    if (!idleTimerId
            && ((qAbs(velocity.x()) >= minVelocity)
                || (qAbs(velocity.y()) >= minVelocity)
                || overshootDist.x()
                || overshootDist.y()) ) {
        idleTimerId = startTimer(1000 / scrollsPerSecond);
    }

    lastTime.restart();
    lastType = e->type();

    bool wasMoved = moved;
    moved = false;
    qKSDebug("MR: end %d", wasMoved);
    return wasMoved; // do not swallow the mouse release, if we didn't move at all
}

void QAbstractKineticScrollerPrivate::checkMove(QMouseEvent *me, QPoint &delta)
{
    Q_Q(QAbstractKineticScroller);

    if (firstDrag && !moved && ((qAbs(delta.x()) > panningThreshold) || (qAbs(delta.y()) > panningThreshold))) {
        moved = true;
        // ignore the panning distance
        if (delta.x() > panningThreshold)
            delta.rx() -= panningThreshold;
        else if (delta.x() < -panningThreshold)
            delta.rx() += panningThreshold;
        else
            delta.setX(0);
        if (delta.y() > panningThreshold)
            delta.ry() -= panningThreshold;
        else if (delta.y() < -panningThreshold)
            delta.ry() += panningThreshold;
        else
            delta.setY(0);

        if (firstDrag) {
            int deltaXtoY = qAbs(ipos.x() - me->globalPos().x()) - qAbs(ipos.y() - me->globalPos().y());

            qKSDebug() << "First Drag with delta " << delta << ", greater than " << panningThreshold << " -- deltaXtoY: " << deltaXtoY;

            QPoint maxPos = q->maximumScrollPosition();
            bool canScrollX = (maxPos.x() > 0);
            bool canScrollY = (maxPos.y() > 0);

            if ((!canScrollX || !canScrollY) && (overshootPolicy == QAbstractKineticScroller::OvershootAlwaysOn))
                canScrollX = canScrollY = true;

            if (deltaXtoY < 0) {
                if (!canScrollY && (!canScrollX || (-deltaXtoY >= directionErrorMargin)))
                    moved = false;
            } else {
                if (!canScrollX && (!canScrollY || (deltaXtoY >= directionErrorMargin)))
                    moved = false;
            }
        }
        firstDrag = false;

        if (moved && (mode == QAbstractKineticScroller::AccelerationMode)) {

            if (!idleTimerId) {
                changeState(QAbstractKineticScroller::AutoScrolling);
                idleTimerId = startTimer(1000 / scrollsPerSecond);
            }
        }
    }
}

void QAbstractKineticScrollerPrivate::handleMove(QMouseEvent *me, QPoint &delta)
{
    Q_Q(QAbstractKineticScroller);

    if (mode == QAbstractKineticScroller::AccelerationMode) {
        // we need delta to be the delta to ipos, not pos in this case
        delta = me->globalPos() - ipos;
    }

    if (axisLockThreshold) {
        int dx = qAbs(delta.x());
        int dy = qAbs(delta.y());
        if (dx || dy) {
            bool vertical = (dy > dx);
            qreal alpha = qreal(vertical ? dx : dy) / qreal(vertical ? dy : dx);
            qKSDebug() << "axis lock: " << alpha << " / " << axisLockThreshold << " - isvertical: " << vertical << " - dx: " << dx << " - dy: " << dy;
            if (alpha <= axisLockThreshold) {
                if (vertical)
                    delta.setX(0);
                else
                    delta.setY(0);
            }
        }
    }

    switch (mode) {
    case QAbstractKineticScroller::PushMode:
        // Scroll by the amount of pixels the cursor has moved
        // since the last motion event.
        scrollUpdate(delta);
        pos = me->globalPos();
        break;

    case QAbstractKineticScroller::AccelerationMode: {
        // Set acceleration relative to the initial click
        QSize size = q->viewportSize();
        qreal signX = 0, signY = 0;
        if (delta.x() < 0)
            signX = -1;
        else if (delta.x() > 0)
            signX = 1;
        if (delta.y() < 0)
            signY = -1;
        else if (delta.y() > 0)
            signY = 1;

        velocity.setX(signX * ((qreal(qAbs(delta.x())) / qreal(size.width()) * (maxVelocity - minVelocity)) + minVelocity));
        velocity.setY(signY * ((qreal(qAbs(delta.y())) / qreal(size.height()) * (maxVelocity - minVelocity)) + minVelocity));
        break;
    }
    case QAbstractKineticScroller::AutoMode:
        QPointF newVelocity = calculateVelocity(delta, lastTime.elapsed());
        QPoint maxPos = q->maximumScrollPosition();

        bool alwaysOvershoot = (overshootPolicy == QAbstractKineticScroller::OvershootAlwaysOn);

        if (!maxPos.x() && !alwaysOvershoot) {
            delta.setX(0);
            newVelocity.setX(0);
        }
        if (!maxPos.y() && !alwaysOvershoot) {
            delta.setY(0);
            newVelocity.setY(0);
        }
        velocity = newVelocity;

        scrollUpdate(delta);

        if (maxPos.x() || alwaysOvershoot)
            pos.setX(me->globalPos().x());
        if (maxPos.y() || alwaysOvershoot)
            pos.setY(me->globalPos().y());
        break;
    }
}

QPointF QAbstractKineticScrollerPrivate::calculateVelocity(const QPointF &dPixelRaw, int dTime)
{
    qKSDebug() << "calculateVelocity(dP = " << dPixelRaw << ", dT = " << dTime << ") -- velocity: " << velocity;

    QPointF newv = velocity;
    QPointF dPixel = dPixelRaw;

    // faster than 25 pix / ms seems bogus (that's a screen height in ~20 ms)
    if ((dPixelRaw / qreal(dTime)).manhattanLength() > 25)
        dPixel = dPixelRaw * qreal(25) / (dPixelRaw / qreal(dTime)).manhattanLength();

    QPointF rawv = dPixel / qreal(dTime) * qreal(1000) / qreal(scrollsPerSecond);
    newv = newv * (qreal(1) - dragInertia) + rawv * dragInertia;

    qKSDebug() << " --> " << newv << " (before clamping)";

    newv.setX(dPixel.x() ? qBound(-maxVelocity, newv.x(), maxVelocity) : velocity.x());
    newv.setY(dPixel.y() ? qBound(-maxVelocity, newv.y(), maxVelocity) : velocity.y());
    return newv;
}

void QAbstractKineticScrollerPrivate::scrollUpdate(const QPoint &delta)
{
    Q_Q(QAbstractKineticScroller);

    if (scrollTimerId) {
        motion += delta;
    } else {
        // we do not delay the first event but the next ones
        setScrollPositionHelper(q->scrollPosition() - overshootDist - delta);
        motion = QPoint(0, 0);
        scrollTimerId = startTimer(1000 / MotionEventsPerSecond);
    }
}


/*!
    If \a enable is true, enables the kinetic scroller; otherwise disables it.
*/
void QAbstractKineticScroller::setEnabled(bool enable)
{
    Q_D(QAbstractKineticScroller);
    d->enabled = enable;
}

/*!
    Returns wether this kinetic scroller is enabled or not.
*/
bool QAbstractKineticScroller::isEnabled() const
{
    Q_D(const QAbstractKineticScroller);
    return d->enabled;
}


/*!
    Returns the scrolling mode.

    \sa setMode(), Mode
*/
QAbstractKineticScroller::Mode QAbstractKineticScroller::mode() const
{
    Q_D(const QAbstractKineticScroller);
    return d->mode;
}

/*!
    Set the scrolling mode to the given \a mode.

    \sa mode(), Mode
*/
void QAbstractKineticScroller::setMode(Mode mode)
{
    Q_D(QAbstractKineticScroller);
    d->mode = mode;
}


/*!
    Returns the value of the low-friction mode.

    Low friction means that scrolling will not be slowed down by the current
    deceleration factor.

    \sa decelerationFactor(), setLowFrictionEnabled()
*/
bool QAbstractKineticScroller::isLowFrictionEnabled() const
{
    Q_D(const QAbstractKineticScroller);
    return d->lowFrictionMode;
}

/*!
    \fn void QAbstractKineticScroller::setLowFrictionEnabled(bool enable)

    If \a enable is true, enables low-friction mode; otherwise disables it.

    \sa decelerationFactor(), isLowFrictionEnabled()
*/
void QAbstractKineticScroller::setLowFrictionEnabled(bool b)
{
    Q_D(QAbstractKineticScroller);
    d->lowFrictionMode = b;
}


/*!
    Returns the overshooting policy.

    The default policy is OvershootWhenScrollable.

    \sa setOvershootPolicy()
*/
QAbstractKineticScroller::OvershootPolicy QAbstractKineticScroller::overshootPolicy() const
{
    Q_D(const QAbstractKineticScroller);
    return d->overshootPolicy;
}

/*!
    Sets the overshooting policy to \a policy.

    \sa overshootPolicy()
*/
void QAbstractKineticScroller::setOvershootPolicy(OvershootPolicy policy)
{
    Q_D(QAbstractKineticScroller);
    d->overshootPolicy = policy;
}

/*!
    Returns the value of the drag inertia.

    This value is a value between 0.0 and 1.0 that describes the effect of dragging
    on a scrolling widget that may already be scrolling. This value is used to smoothen
    the user input from the dragging motion.

    A value of 0.0 causes no scrolling to occur at all if the widget is not scrolling.
    A value of 1.0 causes the dragging motion to dominate, but may result in
    oversensitive scrolling behavior.

    By default, the drag inertia is set to 0.85. This smoothens the input, reducing the
    sensitivity of the widget to the dragging motion, while keeping it responsive.

    \sa setDragInertia()
*/
qreal QAbstractKineticScroller::dragInertia() const
{
    Q_D(const QAbstractKineticScroller);
    return d->dragInertia;
}

/*!
    Sets the value of the drag inertia to the given \a inertia value.

    \sa dragInertia()
*/
void QAbstractKineticScroller::setDragInertia(qreal inertia)
{
    Q_D(QAbstractKineticScroller);
    d->dragInertia = inertia;
}


/*!
    Returns the margin of error that controls how close to horizontal and vertical motion
    the user's input must be to cause scrolling to occur.

    This value influences whether a scroll is recognized as such if the point of input moved
    diagonally.

    \sa setDirectionErrorMargin()
*/
int QAbstractKineticScroller::directionErrorMargin() const
{
    Q_D(const QAbstractKineticScroller);
    return d->directionErrorMargin;
}

/*!
    Sets the margin of error for the direction to the given \a errorMargin value.

    \sa directionErrorMargin()
*/
void QAbstractKineticScroller::setDirectionErrorMargin(int errorMargin)
{
    Q_D(QAbstractKineticScroller);
    d->directionErrorMargin = errorMargin;
}


/*!
    Returns the amount in pixels the point of input must move before scrolling is started.

    \sa setPanningThreshold()
*/
int QAbstractKineticScroller::panningThreshold() const
{
    Q_D(const QAbstractKineticScroller);
    return d->panningThreshold;
}

/*!
    Sets the panning threshold to the given \a threshold value.

    \sa panningThreshold()
*/
void QAbstractKineticScroller::setPanningThreshold(int threshold)
{
    Q_D(QAbstractKineticScroller);
    d->panningThreshold = threshold;
}


/*!
    Returns the deceleration factor used to slow down scrolling over time.

    This value is the percentage (expressed as a value between 0.0 and 1.0) of
    the scrolling velocity remaining after every scroll step.

    \sa setDecelerationFactor()
*/
qreal QAbstractKineticScroller::decelerationFactor() const
{
    Q_D(const QAbstractKineticScroller);
    return d->deceleration;
}

/*!
    Sets the deceleration factor to \a f.

    \sa decelerationFactor()
*/
void QAbstractKineticScroller::setDecelerationFactor(qreal f)
{
    Q_D(QAbstractKineticScroller);
    d->deceleration = f;
}


/*!
    Returns the fast velocity factor. This value, between 0.0 and 1.0, is the factor
    that determines if the scrolling speed is too fast for a mouse click.

    If the current velocity is greater then maximumVelocity()*fastVelocityFactor()
    then the mouse click (or press on a touch screen) will just stop the scrolling,
    and the scroller will not send the click event to the scroll area.

    If the velocity is less than the calculated value then a click (or press) will
    stop the scrolling, and the scroller will send the click event to the scroll area
    for further processing.

    \sa setFastVelocityFactor()
*/
qreal QAbstractKineticScroller::fastVelocityFactor() const
{
    Q_D(const QAbstractKineticScroller);
    return d->fastVelocityFactor;
}

/*!
    Sets the fast velocity factor to \a f.

    \sa fastVelocityFactor()
*/
void QAbstractKineticScroller::setFastVelocityFactor(qreal f)
{
    Q_D(QAbstractKineticScroller);
    d->fastVelocityFactor = f;
}


/*!
    Returns the minimum scrolling velocity.

    The minimum velocity is the slowest speed used in the acceleration mode.

    \sa setMinimumVelocity()
*/
qreal QAbstractKineticScroller::minimumVelocity() const
{
    Q_D(const QAbstractKineticScroller);
    return d->minVelocity;
}

/*!
    Sets the minimum scrolling velocity to \a v.

    \sa minimumVelocity()
*/
void QAbstractKineticScroller::setMinimumVelocity(qreal v)
{
    Q_D(QAbstractKineticScroller);
    d->minVelocity = v;
}


/*!
    Returns the maximum scrolling velocity.

    The maximum velocity is the slowest speed used in the acceleration mode.

    \sa setMaximumVelocity()
*/
qreal QAbstractKineticScroller::maximumVelocity() const
{
    Q_D(const QAbstractKineticScroller);
    return d->maxVelocity;
}

/*!
    Sets the maximum scrolling velocity to \a v.

    \sa maximumVelocity()
*/
void QAbstractKineticScroller::setMaximumVelocity(qreal v)
{
    Q_D(QAbstractKineticScroller);
    d->maxVelocity = v;
}


/*!
    Returns the axis lock threshold.

    On every mouse move, the scroller computes the angle between the vector
    of the mouse movement the and the nearest X or Y axis.

    If the tangens of this angle is less than the axis lock threshold, the
    scroll direction is restricted to the nearest axis.

    The threshold is a floating point value in the range \c 0.0 to \c 1.0.

    The default value of \c 0.0 will not restrict the scroll direction at
    all, while a value of \c 1.0 (wich corresponds to a 45 degree angle)
    restricts the scroll direction to either the X or Y axis.

    \sa setAxisLockThreshold()
*/
qreal QAbstractKineticScroller::axisLockThreshold() const
{
    Q_D(const QAbstractKineticScroller);
    return d->axisLockThreshold;
}

/*!
    Sets the axis lock \a threshold.

    \sa axisLockThreshold()
*/
void QAbstractKineticScroller::setAxisLockThreshold(qreal threshold)
{
    Q_D(QAbstractKineticScroller);
    d->axisLockThreshold = qBound(qreal(0), threshold, qreal(1));
}


/*!
    Returns the number of scrolls (frames) per second.

    This is the frame rate which will be used when the scroller is in the
    AutoScrolling state.

    The default value for Maemo 5 is \c 20.

    \sa setScrollsPerSecond()
*/
int QAbstractKineticScroller::scrollsPerSecond() const
{
    Q_D(const QAbstractKineticScroller);
    return d->scrollsPerSecond;
}

/*!
    Sets the number of scrolls (frames) per second to \a sps.

    \sa scrollsPerSecond()
*/
void QAbstractKineticScroller::setScrollsPerSecond(int sps)
{
    Q_D(QAbstractKineticScroller);
    d->scrollsPerSecond = qBound(1, sps, 100);
}


/*!
    Starts scrolling the widget so that the point \a pos is visible inside
    the viewport.

    If the specified point cannot be reached, the contents are scrolled to the
    nearest valid position.

    The scrolling speed will be calculated so that the given position will
    be reached after a platform-defined time span (1 second for Maemo 5).
    The final speed at the end position is not guaranteed to be zero.

    \sa ensureVisible(), maximumScrollPosition()
*/
void QAbstractKineticScroller::scrollTo(const QPoint &pos)
{
    Q_D(QAbstractKineticScroller);

    if ((pos == scrollPosition()) ||
        (d->state == QAbstractKineticScroller::MousePressed) ||
        (d->state == QAbstractKineticScroller::Pushing)) {
        return;
    }

    // --- calculate the initial velocity to get to the point
    // -- calc the number of steps we have:
    int steps = d->scrollTime * d->scrollsPerSecond / 1000;

    // -- calc the distance we will move with a starting velocity of 1.0
    float dist = 0.0;
    float curVel = 1.0;
    for (int i = 0; i<steps; i++) {
        dist += curVel;
        curVel *= d->deceleration;
    }

    // --- start the scrolling
    d->scrollTo = pos;
    d->velocity = - QPointF(pos-scrollPosition()) / dist;
    d->mode = QAbstractKineticScroller::AutoMode;

    qKSDebug() << "QAbstractKineticScroller::scrollTo new pos:" << pos << " velocity:"<<d->velocity;

    if (!d->idleTimerId) {
        d->changeState(QAbstractKineticScroller::AutoScrolling);
        d->idleTimerId = d->startTimer(1000 / d->scrollsPerSecond);
    }
}

/*!
    Starts scrolling the widget so that the point \a pos is visible inside the
    viewport with margins specified in pixels by \a xmargin and \a ymargin.

    If the specified point cannot be reached, the contents are scrolled to the
    nearest valid position.  The default value for both margins is 50 pixels.

    This function performs the actual scrolling by calling scrollTo().

    \sa maximumScrollPosition()
*/
void QAbstractKineticScroller::ensureVisible(const QPoint &pos, int xmargin, int ymargin)
{
    QSize visible = viewportSize();
    QPoint currentPos = scrollPosition();

    qKSDebug() << "QAbstractKineticScroller::ensureVisible(" << pos << ", " << xmargin << ", " << ymargin << ") - position: " << scrollPosition();

    QRect posRect(pos.x() - xmargin, pos.y() - ymargin, 2 * xmargin, 2 * ymargin);
    QRect visibleRect(currentPos, visible);

    if (visibleRect.contains(posRect))
        return;

    QPoint newPos = currentPos;
    if (posRect.top() < visibleRect.top())
        newPos.setY(posRect.top());
    else if (posRect.bottom() > visibleRect.bottom())
        newPos.setY(posRect.bottom() - visible.height());
    if (posRect.left() < visibleRect.left())
        newPos.setX(posRect.left());
    else if (posRect.right() > visibleRect.right())
        newPos.setX(posRect.right() - visible.width());

    scrollTo(newPos);
}

/*
    Decomposes the position into a scroll and an overshoot part.
    Also keeps track of the current over-shooting value in overshootDist.
*/
void QAbstractKineticScrollerPrivate::setScrollPositionHelper(const QPoint &pos)
{
    Q_Q(QAbstractKineticScroller);

    QPoint maxPos = q->maximumScrollPosition();

    QPoint clampedPos;
    clampedPos.setX(qBound(0, pos.x(), maxPos.x()));
    clampedPos.setY(qBound(0, pos.y(), maxPos.y()));

    bool alwaysOvershoot = (overshootPolicy == QAbstractKineticScroller::OvershootAlwaysOn);
    int overshootX = (maxPos.x() || alwaysOvershoot) ? clampedPos.x() - pos.x() : 0;
    int overshootY = (maxPos.y() || alwaysOvershoot) ? clampedPos.y() - pos.y() : 0;

    overshootDist.setX(qBound(-maxOvershoot.x(), overshootX, maxOvershoot.x()));
    overshootDist.setY(qBound(-maxOvershoot.y(), overshootY, maxOvershoot.y()));

    qKSDebug() << "setPosition raw: " << pos << ", clamped: " << clampedPos << ", overshoot: " << overshootDist;
    q->setScrollPosition(clampedPos, overshootPolicy == QAbstractKineticScroller::OvershootAlwaysOff ? QPoint() : overshootDist);
}

/*!
    If kinetic scrolling can be started at the global screen coordinate \a
    globalPos within the widget or graphics object, this function needs to
    return true; otherwise it needs to return false.

    The default value is true, regardless of \a globalPos.
*/
bool QAbstractKineticScroller::canStartScrollingAt(const QPoint &globalPos) const
{
    Q_UNUSED(globalPos);
    return true;
}

/*!
    Since a mouse press is always delivered normally when the scroller is in
    the Inactive state, we may need to cancel it as soon as the user has moved
    the mouse far enough to actually start a kinetic scroll operation.

    The \a globalPressPos parameter can be used to find out which widget
    received the mouse press in the first place.

    Subclasses may choose to simulate a fake mouse release event for that
    widget, preferably \bold not within its boundaries.  The default
    implementation does nothing.
*/
void QAbstractKineticScroller::cancelLeftMouseButtonPress(const QPoint &globalPressPos)
{
    Q_UNUSED(globalPressPos);
}

/*!
    \enum QAbstractKineticScroller::OvershootPolicy

    This enum describes the various modes of overshooting.

    \value OvershootWhenScrollable Overshooting is when the content is scrollable. This is the default.

    \value OvershootAlwaysOff Overshooting is never enabled (even when the content is scrollable).

    \value OvershootAlwaysOn Overshooting is always enabled (even when the content is not scrollable).
*/

/*!
    \enum QAbstractKineticScroller::Mode

    This enum contains the different modes for the QAbstractKineticScroller.

    \value AutoMode The mode will allow both pushing and acceleration.

    \value PushMode The area will be scrolled as long as the user drags it around with pressed mouse button.

    \value AccelerationMode The area will continue scrolling after the user releases the mouse button.
*/


/*!
    This function get called whenever the state of the kinetic scroller changes.
    The old state is supplied as \a oldState, while the new state is returned by
    calling state().

    The default implementation does nothing.

    \sa state()
*/
void QAbstractKineticScroller::stateChanged(State oldState)
{
    Q_UNUSED(oldState);
}

/*!
    \fn QPoint QAbstractKineticScroller::maximumScrollPosition() const

    Returns the maximum valid scroll position. The minimum is always \c (0,0).

    \sa scrollTo()
*/

/*!
    \fn QSize QAbstractKineticScroller::viewportSize() const

    Returns the size of the currently visible scroll positions.
    In the case where an QAbstractScrollArea is used, this is equivalent
    to the viewport() size.

    \sa scrollTo()
*/

/*!
    \fn QPoint QAbstractKineticScroller::scrollPosition() const

    \brief Returns the current scroll position.
    Note that overshooting is not considered to be "real" scrolling so the position might be (0,0)
    even if the user is currently dragging the widget outside the "normal" maximumScrollPosition().

    \sa maximumScrollPosition()
*/


/*!
    \fn void QAbstractKineticScroller::setScrollPosition(const QPoint &pos, const QPoint &overshoot)

    Sets the scroll position of the widget to \a pos. This
    parameter will always be in the valid range returned by maximumScrollPosition().

    In the case where overshooting is required, the \a overshoot parameter
    will give the direction and the absolute pixel distance to overshoot.

    \sa maximumScrollPosition()
*/

QT_END_NAMESPACE
