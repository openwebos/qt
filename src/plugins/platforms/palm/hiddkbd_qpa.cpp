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

#include <Qt>
#include <QKeyEvent>
#include <QApplication>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <glib.h>
#include "hiddkbd_qpa.h"
#include <hid/IncsPublic/HidLib.h>
#include "InputControl.h"
#include "NyxInputControl.h"
#include "webosDeviceKeymap.h"
#include <dlfcn.h>

#include <QWSKeyboardHandler>
#include <QKbdDriverFactory>


extern "C" {
    void (*setBluetoothKeyboardActive)(bool) = NULL;
    void setBluetoothCallback(void (*fun)(bool)) { setBluetoothKeyboardActive = fun; }
}

QPAHiddKbdHandler::QPAHiddKbdHandler() :
      m_nyxKeysHandle(0)
    , m_shiftKeyDown(false)
    , m_altKeyDown(false)
    , m_optKeyDown(false)
    , m_homeKeyDown(false)
    , m_sendHomeDoubleClick(false)
    , m_curDeviceId (-1)
{
	m_keyMap = NULL;
	void* handle = dlopen("libLunaKeymaps.so", RTLD_LAZY);
	if(handle) {
	    const KeyMapType* (*webosGetDeviceKeymap)(void);
	    *(void**)(&webosGetDeviceKeymap) = dlsym(handle, "webosGetDeviceKeymap");
	    if(dlerror() == NULL) {
		m_keyMap = (*webosGetDeviceKeymap)();
	    }
	}

	nyx_error_t error = NYX_ERROR_NONE;

	InputControl* ic = new NyxInputControl(NYX_DEVICE_KEYS, "Main");
	if (NULL == ic)
	{
		g_critical("Unable to obtain InputControl");
		return;
	}

	m_nyxKeysHandle = ic->getHandle();
	if (NULL == m_nyxKeysHandle)
	{
		g_critical("Unable to obtain m_nyxKeysHandle");
		return;
	}

	error = nyx_device_get_event_source(m_nyxKeysHandle, &m_keyFd);
	if (error != NYX_ERROR_NONE)
	{
		g_critical("Unable to obtain m_nyxKeysHandle event_source");
		return;
	}

	m_keyNotifier = new QSocketNotifier(m_keyFd, QSocketNotifier::Read, this);
	connect(m_keyNotifier, SIGNAL(activated(int)), this, SLOT(readKeyData()));

    // inputdev
    ic = new NyxInputControl(NYX_DEVICE_BLUETOOTH_INPUT_DETECT, "Main");
    if (NULL != ic)
    {
        m_nyxInputDevHandle = ic->getHandle();
        if (m_nyxInputDevHandle)
        {
            int bluetooth_input_detect_source_fd = 0;
            error = nyx_device_get_event_source(m_nyxInputDevHandle,
                    &bluetooth_input_detect_source_fd);

            if (error != NYX_ERROR_NONE)
            {
                g_critical("Unable to obtain fusionHandle event_source");
                return;
            }

	        m_inputDevNotifier = new QSocketNotifier (bluetooth_input_detect_source_fd, 
                    QSocketNotifier::Read, this);
	        connect (m_inputDevNotifier, SIGNAL(activated(int)), this, SLOT(readInputDevData()));
        }
    }
}

QPAHiddKbdHandler::~QPAHiddKbdHandler() {
	if (m_keyFd >= 0)
		close(m_keyFd);
}

void QPAHiddKbdHandler::readInputDevData()
{
	nyx_error_t error = NYX_ERROR_NONE;
	nyx_event_handle_t event_handle = NULL;

	while ((error = nyx_device_get_event(m_nyxInputDevHandle, &event_handle)) == NYX_ERROR_NONE && event_handle != NULL)
	{
        nyx_bluetooth_input_detect_event_item_t data;

        error = nyx_bluetooth_input_detect_event_get_data(event_handle, &data);
        if (error != NYX_ERROR_NONE)
            g_critical("failed to get bluetooth input detect event data");

        switch (data.event_type)
        {
            case NYX_BLUETOOTH_INPUT_DETECT_EVENT_DEVICE_ID_ADD:
                {
				    m_curDeviceId = data.value;
					g_debug("QPAHiddKbdHandler: Added BT input device id: %d", m_curDeviceId);
                }
                break;
             case NYX_BLUETOOTH_INPUT_DETECT_EVENT_DEVICE_ID_REMOVE:
                {
		    
				    DeviceInfo *devInfo = m_deviceIdMap[data.value];
				    if (devInfo) delete devInfo;

					m_deviceIdMap.erase(data.value);
					m_curDeviceId = -1;
					g_debug("QPAHiddKbdHandler: Removed BT input device id: %d", data.value);
					if (m_deviceIdMap.empty()) {
					    //Tell sysmgr we don't have keyboards
					    if(setBluetoothKeyboardActive)
						(*setBluetoothKeyboardActive)(false);
		                        }
				} 
				break;
            case NYX_BLUETOOTH_INPUT_DETECT_EVENT_DEVICE_KEYBOARD_TYPE:
                {
				    // TODO: add qmap option
					// keymap=xx.qmap
					char idBuf[16];
					snprintf(idBuf, sizeof(idBuf), "%d", m_curDeviceId);
					idBuf[sizeof(idBuf)-1] = '\0';

					QString optionStr = "/dev/input/event";
					optionStr += idBuf;
					optionStr += ":keymap=/usr/share/qt4/keymaps/keymap-";

					bool	compose = false;

					// Country codes come directly from the USB HID spec
					// See NOV-93218
					switch (m_curDeviceCountry) {
					    case 33: optionStr += "us"; break;
						case 8: optionStr += "fr"; compose = true; break;
						case 9: optionStr += "de"; compose = true; break;
						case 32: optionStr += "uk"; compose = true; break;
						default: optionStr += "us"; break;
					}

					optionStr += ".qmap";

					if (compose)
						optionStr += ":enable-compose";

					g_message("QPAHiddKbdHandler: BT keyboard handler set to %s", optionStr.toUtf8().data());

					QWSKeyboardHandler* handler = QKbdDriverFactory::create("LinuxInput", optionStr);
					handler->setIsExternalKeyboard(true);
					m_deviceIdMap.insert(std::pair<int, DeviceInfo*>(m_curDeviceId, new DeviceInfo(handler)));
					//tell sysmgr we have keyboards
					printf("bluetooth is active\n");
					if(setBluetoothKeyboardActive)
						(*setBluetoothKeyboardActive)(true);
			    }
				break;

            case NYX_BLUETOOTH_INPUT_DETECT_EVENT_DEVICE_COUNTRY:
				g_debug("QPAHiddKbdHandler: BT keyboard country set to %d", data.value);
				m_curDeviceCountry = data.value;
			    break;
		    default: 
                break;
		}

	    event_handle = NULL;
    }
}

void QPAHiddKbdHandler::readKeyData() {
	nyx_error_t error = NYX_ERROR_NONE;
	nyx_event_handle_t event_handle = NULL;
	while ((error = nyx_device_get_event(m_nyxKeysHandle, &event_handle)) == NYX_ERROR_NONE && event_handle != NULL)
	{
		int key = 0; 
		int keycode = 0;
		nyx_key_type_t key_type;
		bool is_auto_repeat = false;
		bool is_press = false;
		bool consumeKey = false;

		error = nyx_keys_event_get_key(event_handle, &keycode);
		if (error == NYX_ERROR_NONE)
			error = nyx_keys_event_get_key_type(event_handle, &key_type);
		if (error == NYX_ERROR_NONE)
			error = nyx_keys_event_get_key_is_auto_repeat(event_handle, &is_auto_repeat);
		if (error == NYX_ERROR_NONE)
			error = nyx_keys_event_get_key_is_press(event_handle, &is_press);

		if (error != NYX_ERROR_NONE)
		{
			g_critical("Unable to obtain event_handle properties");
			return;
		}

		int keyValue = is_press ? 1 : 0;

		if (key_type == NYX_KEY_TYPE_STANDARD)
		{
			key = lookupKey(keycode, keyValue, &consumeKey);
		}
		else if (key_type == NYX_KEY_TYPE_CUSTOM)
		{
			key = lookupSwitch((nyx_keys_custom_key_t)keycode);
// NOTE: uncomment this block to re-enable the Home double-tap functionality
/*
			if (key == Qt::Key_CoreNavi_Home)
			{
				if (m_homeKeyDoubleClickTimer.running()) 
				{
					if (!m_homeKeyDown) 
					{
						// suppress the home button down since the user is performing a double click
						m_homeKeyDown = (keyValue != 0);
						consumeKey = true; // suppress
						m_homeKeyDoubleClickTimer.stop(); // this is a double home button click
						m_sendHomeDoubleClick = true;
					}
				}
				else 
				{
					if (!m_homeKeyDown) 
					{
						// send a home button down
						m_homeKeyDown = (keyValue != 0);
					}
					else if (m_sendHomeDoubleClick) 
					{
						// send a home button up as double click
						m_homeKeyDown = (keyValue != 0);
						//printf("home key up send (double)");
					}
					else 
					{
						// suppress the first home button up to wait to see if the user will double click
						m_homeKeyDown = (keyValue != 0);
						m_homeKeyDoubleClickTimer.start(Settings::LunaSettings()->homeDoubleClickDuration, true);
						consumeKey = true; // suppress
					}
				}
			}
*/
		}
		else
		{
			error = nyx_device_release_event(m_nyxKeysHandle, event_handle);
			if (error != NYX_ERROR_NONE)
			{
				g_critical("Unable to release m_nyxKeysHandle event");
				return;
			}
			//reset the event handle
			event_handle = NULL;
			continue;
		}

		if (consumeKey)
		{
			error = nyx_device_release_event(m_nyxKeysHandle, event_handle);
			if (error != NYX_ERROR_NONE)
			{
				g_critical("Unable to release m_nyxKeysHandle event");
				return;
			}
			//reset the event handle
			event_handle = NULL;
			continue;
		}

		Qt::KeyboardModifiers modifiers = Qt::NoModifier;
		if (m_shiftKeyDown)
			modifiers |= Qt::ShiftModifier;
		if (m_altKeyDown)
			modifiers |= Qt::AltModifier;
		if (m_optKeyDown)
			modifiers |= Qt::ControlModifier;
		/*if (HostBase::instance()->metaModifier()) 
		{
			 modifiers |= Qt::MetaModifier;
			 g_message("%s: MetaModifier is set", __PRETTY_FUNCTION__);
		}*/
		// force auto repeat for the Home button when we want to signify a double click
		if (key == Qt::Key_CoreNavi_Home && m_sendHomeDoubleClick) 
		{
			is_auto_repeat = true;
			m_sendHomeDoubleClick = false;
		}

		//handler->processKeyEvent(0, (Qt::Key) key, modifiers, is_press, is_auto_repeat);
		QKeyEvent* e = new QKeyEvent((is_press ? QEvent::KeyPress : QEvent::KeyRelease), (Qt::Key)key, modifiers);
		QApplication::postEvent((QObject*)QApplication::activeWindow(), (QEvent*)e);
		error = nyx_device_release_event(m_nyxKeysHandle, event_handle);
		if (error != NYX_ERROR_NONE)
		{
			g_critical("Unable to release m_nyxKeysHandle event");
			return;
		}
		event_handle = NULL;
	}
}

// This is a temporary implementation to use processKeyEvent in the QWSKeyboardHandler.
// TODO: Implement the keymap solution to allow us to translate thus directly using QWSKeyboardHandler::processKeyCode
Qt::Key QPAHiddKbdHandler::lookupKey(int keyCode, int keyValue, bool* consumeKey) {

	int key = 0;

	bool shiftKeyDown = (keyValue != 0);
	bool altKeyDown = (keyValue != 0);
	bool optKeyDown = (keyValue != 0);

	// Check for shift, Option or Alt Key Down
	switch (keyCode) 
	{

		case KEY_LEFTSHIFT: 
		case KEY_RIGHTSHIFT:
			//printf("shift key %s\n", keyValue == 0 ? "up" : "down");
			if (shiftKeyDown && m_shiftKeyDown)
				*consumeKey = true;
			else 
			{
				m_shiftKeyDown = shiftKeyDown;
				key = Qt::Key_Shift;
			}
			break;
		case KEY_LEFTALT: 
			//printf("alt key %s\n", keyValue == 0 ? "up" : "down");
			if (altKeyDown && m_altKeyDown)
				*consumeKey = true;
			else 
			{
				m_altKeyDown = altKeyDown;
				key = Qt::Key_Alt;
			}
			break;
		case KEY_LEFTCTRL:
			//printf("control key %s\n", keyValue == 0 ? "up" : "down");
			if (optKeyDown && m_optKeyDown)
				*consumeKey = true;
			else 
			{
				m_optKeyDown = optKeyDown;
				key = Qt::Key_Control;
			}
			break;
		// home button
		default:
			break;
	}

	if(m_keyMap) {
#ifdef TARGET_EMULATOR
	// this should be the last valid key in luna-keymaps
	if (keyCode > 0 && keyCode <= KEY_PAUSE) {
		if (m_shiftKeyDown)
			key = m_keyMap[keyCode].qtKeyOpt;
		else
			key = m_keyMap[keyCode].qtKey;
	}
#else
	if (keyCode > 0 && keyCode <= KEY_SPACE) {
		if (m_altKeyDown)
			key = m_keyMap[keyCode].qtKeyOpt;
		else
			key = m_keyMap[keyCode].qtKey;
	}
#endif
	}

	return (Qt::Key) key;
}

Qt::Key QPAHiddKbdHandler::lookupSwitch(nyx_keys_custom_key_t code)
{
	int key = 0;

	switch (code) 
	{
		// veng-bug: add? return Qt::Key_HeadsetButton;

		case NYX_KEYS_CUSTOM_KEY_VOL_UP:
			key = Qt::Key_VolumeUp;
			break;
		case NYX_KEYS_CUSTOM_KEY_VOL_DOWN:
			key = Qt::Key_VolumeDown;
			break;
		case NYX_KEYS_CUSTOM_KEY_POWER_ON:
			key = Qt::Key_Power;
			break;
		case NYX_KEYS_CUSTOM_KEY_HOME:
			key = Qt::Key_CoreNavi_Home;
			break;
		case NYX_KEYS_CUSTOM_KEY_RINGER_SW:
			key = Qt::Key_Ringer;
			break;
		case NYX_KEYS_CUSTOM_KEY_SLIDER_SW:
			key = Qt::Key_Slider;
			break;
        case NYX_KEYS_CUSTOM_KEY_HEADSET_BUTTON:
            key = Qt::Key_HeadsetButton;
            break;
		case NYX_KEYS_CUSTOM_KEY_HEADSET_PORT:
			key = Qt::Key_Headset;
			break;
		case NYX_KEYS_CUSTOM_KEY_HEADSET_PORT_MIC:
			key = Qt::Key_HeadsetMic;
			break;
		case NYX_KEYS_CUSTOM_KEY_OPTICAL:
			key = Qt::Key_Optical;
			break;
		case NYX_KEYS_CUSTOM_KEY_MEDIA_PLAY:
			key = Qt::Key_MediaPlay;
			break;
		case NYX_KEYS_CUSTOM_KEY_MEDIA_PAUSE:
			key = Qt::Key_MediaPause;
			break;
		case NYX_KEYS_CUSTOM_KEY_MEDIA_STOP:
			key = Qt::Key_MediaStop;
			break;
		case NYX_KEYS_CUSTOM_KEY_MEDIA_NEXT:
			key = Qt::Key_MediaNext;
			break;
		case NYX_KEYS_CUSTOM_KEY_MEDIA_PREVIOUS:
			key = Qt::Key_MediaPrevious;
			break;
		case NYX_KEYS_CUSTOM_KEY_MEDIA_REPEAT_ALL:
			key = Qt::Key_MediaRepeatAll;
			break;
		case NYX_KEYS_CUSTOM_KEY_MEDIA_REPEAT_TRACK:
			key = Qt::Key_MediaRepeatTrack;
			break;
		case NYX_KEYS_CUSTOM_KEY_MEDIA_REPEAT_NONE:
			key = Qt::Key_MediaRepeatNone;
			break;
		case NYX_KEYS_CUSTOM_KEY_MEDIA_SHUFFLE_ON:
			key = Qt::Key_MediaShuffleOn;
			break;
		case NYX_KEYS_CUSTOM_KEY_MEDIA_SHUFFLE_OFF:
			key = Qt::Key_MediaShuffleOff;
			break;
		case NYX_KEYS_CUSTOM_KEY_UNDEFINED:
		default:
			key = 0;
			break;
	}
	return (Qt::Key)key;
}

bool QPAHiddKbdHandler::homeKeyDoubleClickTimeout()
{
    //printf("home key up send (single)\n");
    // if this timeout gets called, it means we should only process a single Home key release
    Qt::KeyboardModifiers modifiers = (Qt::KeyboardModifiers) ((m_shiftKeyDown ? Qt::ShiftModifier : 0)
        | (m_altKeyDown ? Qt::AltModifier : 0) | (m_optKeyDown ? Qt::ControlModifier : 0));

    //QWSServer::sendKeyEvent(Qt::Key_CoreNavi_Home, Qt::Key_CoreNavi_Home, modifiers, false, false);

    return true;
}

