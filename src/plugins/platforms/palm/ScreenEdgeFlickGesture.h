/* ============================================================
 * Date  : 2011-03-16
 * Copyright 2011 Palm, Inc. All rights reserved.
 * ============================================================ */

#ifndef SCREENEDGEFLICKGESTURE_H
#define SCREENEDGEFLICKGESTURE_H


#include <QGesture>
#include <QPoint>
#include "qnamespace.h"

class ScreenEdgeFlickGesture : public QGesture
{
public:

	enum Edge {
		EdgeUnknown = 0,
		EdgeTop,
		EdgeBottom,
		EdgeLeft,
		EdgeRight
	};

	ScreenEdgeFlickGesture(QObject* parent = 0)
		: QGesture(parent, (Qt::GestureType) SysMgrGestureScreenEdgeFlick)
		, m_edge(EdgeUnknown)
		, m_yDistance(0){
	}

	Edge edge() const { return m_edge; }
	int yDistance() const { return m_yDistance; }

private:

	Edge m_edge;
	int m_yDistance;

private:

	friend class QPAHiddTpHandler;
};

#endif /* SCREENEDGEFLICKGESTURE_H */
