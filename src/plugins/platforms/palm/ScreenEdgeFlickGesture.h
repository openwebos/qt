/****************************************************************************
**
** Copyright (C) 2012 Hewlett-Packard Development Company, L.P.
** All rights reserved.
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
