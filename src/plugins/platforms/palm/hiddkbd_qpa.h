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

#ifndef QHIDDKBD_QPA_H
#define QHIDDKBD_QPA_H

#include "hidd_qpa.h"

#include <qsocketnotifier.h>
#include <map>
#include "webosDeviceKeymap.h"
#include <hid/IncsPublic/HidLib.h>

#include <hal/hal.h>
#include <QWSKeyboardHandler>

#define MAX_HIDD_EVENTS 100
#define CENTER_BUTTON 	232
#define EV_GESTURE 0x06

// Same as default kernel values
#define REPEAT_DELAY	250
#define REPEAT_PERIOD	33



enum {
  F1    = 0x276C,         /* Function keys */
  F2    = 0x276D,
  F3    = 0x276E,
  F4    = 0x276F,
  F5    = 0x2770,
  F6    = 0x2771,
  F7    = 0x2772,
  F8    = 0x2773,
  F9    = 0x2774,
  F10   = 0x2775,
  KEY_SYM    = 0xf6,
  KEY_ORANGE = 0x64
};



class QPAHiddKbdHandler;


class QPAHiddKbdHandler: public QObject
{
Q_OBJECT
public:
	QPAHiddKbdHandler();
	~QPAHiddKbdHandler();

	int setupHiddSocket(const char* path);

private:

	class DeviceInfo {
	public:
		DeviceInfo(QWSKeyboardHandler* handler)
			: m_type(DEV_TYPE_KEYBOARD)
			, m_kbdHandler(handler)
		{}
/*
		DeviceInfo(QWSMouseHandler* handler)
			: m_type(DEV_TYPE_MOUSE)
			, m_mouseHandler(handler)
		{}
*/
		~DeviceInfo()
		{
			switch (m_type) {
			case DEV_TYPE_KEYBOARD: delete m_kbdHandler; break;
//			case DEV_TYPE_MOUSE: delete m_mouseHandler; break;
			}
		}

	private:
		int m_type;
		union {
			QWSKeyboardHandler* m_kbdHandler;
		//	QWSMouseHandler* m_mouseHandler;
		};
	};

	Qt::Key lookupKey(int code, int value, bool* consume);
	Qt::Key lookupSwitch(hal_keys_custom_key_t code);

    bool homeKeyDoubleClickTimeout();

	int m_keyFd;
	QSocketNotifier *m_keyNotifier;
	hal_device_handle_t m_halKeysHandle;

	bool m_shiftKeyDown;
	bool m_altKeyDown;
	bool m_optKeyDown;

	void initKeymap();
	const KeyMapType* m_keyMap;

	int m_inputDevFd;
	QSocketNotifier *m_inputDevNotifier;
	hal_device_handle_t m_halInputDevHandle;

	std::map<int, DeviceInfo*> m_deviceIdMap;

	bool m_homeKeyDown;
    bool m_sendHomeDoubleClick;

	int m_curDeviceId;
	int m_curDeviceCountry;

private Q_SLOTS:
	void readKeyData();
	void readInputDevData();
};


#endif /* QHIDDKBD_QPA_H */
