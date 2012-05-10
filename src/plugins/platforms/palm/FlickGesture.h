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
