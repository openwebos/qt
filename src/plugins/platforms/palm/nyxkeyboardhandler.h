/* @@@LICENSE
*
*      Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */
#ifndef __NYXKEYBOARDHANDLER_H__
#define __NYXKEYBOARDHANDLER_H__

#include <QWSKeyboardHandler>
#include <QWSMouseHandler>
#include <qsocketnotifier.h>
#include <map>

#include <nyx/nyx_client.h>

#define MAX_HIDD_EVENTS 100
#define CENTER_BUTTON   232
#define EV_GESTURE 0x06

// Same as default kernel values
#define REPEAT_DELAY  250
#define REPEAT_PERIOD 33

// These are defined here for the time being.
// The correct place should be some where within nyx
// as they are coming from the late hiddlib.
#define DEV_TYPE_KEYBOARD 0x00
#define DEV_TYPE_MOUSE    0x01

class NyxKeyboardHandler: public QObject
{
Q_OBJECT
public:
	NyxKeyboardHandler();
	~NyxKeyboardHandler();

private:
	class DeviceInfo {
	public:
        DeviceInfo(QWSKeyboardHandler* handler)
            : m_type(DEV_TYPE_KEYBOARD)
              , m_kbdHandler(handler)
        {}

        DeviceInfo(QWSMouseHandler* handler)
            : m_type(DEV_TYPE_MOUSE)
              , m_mouseHandler(handler)
        {}

        ~DeviceInfo()
        {
            switch (m_type) {
            case DEV_TYPE_KEYBOARD: delete m_kbdHandler; break;
            case DEV_TYPE_MOUSE: delete m_mouseHandler; break;
            }
        }

    private:
        int m_type;
        union {
            QWSKeyboardHandler* m_kbdHandler;
            QWSMouseHandler* m_mouseHandler;
        };
    };

    std::map<int, DeviceInfo*> m_deviceIdMap;
    /*!
        \brief Maps the NYX special keycodes to Qt Key codes

        WEB OS adds new keycodes to Qt's corelib/global/qnamespace.h. This function will
        map keycodes devlivered by NYX to those new codes.
    */
    Qt::Key mapNyxKeyCode(nyx_keys_custom_key_t code);

    int m_keyFd;
    QSocketNotifier *m_keyNotifier;
    QWSKeyboardHandler* m_handler;
    nyx_device_handle_t m_nyxKeysHandle;

    int m_inputDevFd;
    QSocketNotifier *m_inputDevNotifier;
    nyx_device_handle_t m_nyxInputDevHandle;


    int m_curDeviceId;
    int m_curDeviceCountry;

private Q_SLOTS:
    void readKeyData();
    void readInputDevData();
};

#endif /* __NYXKEYBOARDHANDLER_H__ */
