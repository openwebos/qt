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

#ifndef QHIDDTP_QPA_H
#define QHIDDTP_QPA_H

#include "hidd_qpa.h"
#include <QList>
#include <QTouchEvent>
#include <QElapsedTimer>
#include <stdint.h>
#include <stdio.h>
#include <glib.h>

#include <nyx/nyx_client.h>

#include "FlickGesture.h"
#include "ScreenEdgeFlickGesture.h"
#include "InputControl.h"

#define EV_GESTURE 0x06

class QPAHiddTpHandler: public QObject {
Q_OBJECT
public:
	QPAHiddTpHandler(int width, int height);
	~QPAHiddTpHandler();

	void suspend();
	void resume();
	int setupHiddSocket(const char* path);
	void parseHiddData(struct input_event* inputEvents, int numEvents);
private:
	Qt::Key lookupGesture(uint16_t value);

	enum State {
	    FingerDown, FingerUp, FingerMove, FingerNoMove, Invalid = -1
	};

	struct HiddTouch {
		uint64_t hiddId;
		int id;
		struct timeval time;
		int x;
		int y;
		int xDown;
		int yDown;
		int xVelocity;
		int yVelocity;
		int gestureKey;

		State state;
		bool isPrimary;
		bool isMetaTouch;
		bool seenInScan;

		HiddTouch() {
		    reset();
		}

		void reset() {
		    hiddId = 0;
		    id = -1;
		    x = 0;
		    y = 0;
			xDown = 0;
			yDown = 0;
		    time.tv_sec = 0;
		    time.tv_usec = 0;
		    state = Invalid;
		    isPrimary = false;
		    gestureKey = Qt::Key_unknown;
		    xVelocity = 0;
		    yVelocity = 0;
		    isMetaTouch = false;
			seenInScan = false;
		}
	};

	QList<HiddTouch> m_touches;
	QPoint m_lastTouchDown;
	QPoint m_mousePress;
	QElapsedTimer m_touchTimer;
	qint64 m_mousePressTime;
	
	nyx_device_handle_t m_nyxPenHandle;
	int m_penFd;

	int m_metaActiveTouchesCount;
	bool m_sendPenCancel;
	QPoint m_penCancelPoint;

	bool m_isSuspended;

	bool updateTouchEvents(QList<HiddTouch>& hiddTouches);

	void addNewTouch(HiddTouch& touch);
	bool updateOldTouch(HiddTouch& touch);
	void generateTouchEvent();
	void removeReleasedTouches();

	inline int squareDistance (const HiddTouch& p1, const HiddTouch& p2) {
	    int dx = p1.x - p2.x;
	    int dy = p1.y - p2.y;
	    
	    return dx*dx + dy*dy;
	}

	bool updateTouch (struct Touch touch);

	static gboolean ioCallback(GIOChannel* channel, GIOCondition condition, gpointer arg);
	FlickGesture* flickGesture;
	ScreenEdgeFlickGesture* m_screenEdgeFlickGesture;
	int m_deviceWidth;
	int m_deviceHeight;
private Q_SLOTS:
	void readHiddData();
};


#endif /* QHIDDTP_QPA_H */
