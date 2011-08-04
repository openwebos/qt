/* ============================================================
 * Date  : 2010-02-04
 * Copyright 2010 Palm, Inc. All rights reserved.
 * ============================================================ */

#ifndef FLICKGESTURE_H
#define FLICKGESTURE_H


#include <QGesture>
#include <QPoint>

enum SysMgrGesture {
	SysMgrGestureFlick = 0x0100 + 1, // this is also defined in qnamespace.h since QTapGestureRecognizer needs to know about it.
	SysMgrGestureSingleClick,
	SysMgrGestureScreenEdgeFlick,
	SysMgrGestureLast = 0xFFFFFFFF
};

class FlickGesture : public QGesture
{
public:

	FlickGesture(QObject* parent = 0) : QGesture(parent, (Qt::GestureType) SysMgrGestureFlick) {}
	QPoint velocity() const { return m_velocity; }
	QPoint startPos() const { return m_startPos; }
	QPoint endPos() const { return m_endPos; }

private:

	QPoint m_velocity;
	QPoint m_endPos;
	QPoint m_startPos;

private:

	friend class QPAHiddTpHandler;
};

#endif /* FLICKGESTURE_H */
