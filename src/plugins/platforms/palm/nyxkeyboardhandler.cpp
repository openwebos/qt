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

#include <QKbdDriverFactory>
#include <QDebug>
#include <Qt>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>

#include "nyxkeyboardhandler.h"
#include "NyxInputControl.h"

extern "C" {
    void (*setBluetoothKeyboardActive)(bool) = NULL;
    void setBluetoothCallback(void (*fun)(bool)) { setBluetoothKeyboardActive = fun; }
}

NyxKeyboardHandler::~NyxKeyboardHandler() {
	if (m_keyFd >= 0)
		close(m_keyFd);
    delete m_handler;
}

NyxKeyboardHandler::NyxKeyboardHandler() :
    m_nyxKeysHandle(0)
    , m_curDeviceId (-1)
{
	/* initial NYX support */
	/* fine-tuning  support for NYX*/
	nyx_error_t error = NYX_ERROR_NONE;

    InputControl* ic = new NyxInputControl(NYX_DEVICE_KEYS, "Main");
	if (NULL == ic)
	{
		qWarning() << __PRETTY_FUNCTION__ << "Unable to obtain InputControl";
		return;
	}

	m_nyxKeysHandle = ic->getHandle();
	if (NULL == m_nyxKeysHandle)
	{
        qWarning() << __PRETTY_FUNCTION__ <<  "Unable to obtain m_nyxKeysHandle";
		return;
	}

	error = nyx_device_get_event_source(m_nyxKeysHandle, &m_keyFd);
	if (error != NYX_ERROR_NONE)
	{
        qWarning() << __PRETTY_FUNCTION__ << "Unable to obtain m_nyxKeysHandle event_source";
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
                qDebug() << __PRETTY_FUNCTION__ <<  "Unable to obtain fusionHandle event_source";
                return;
            }

	        m_inputDevNotifier = new QSocketNotifier (bluetooth_input_detect_source_fd,
                    QSocketNotifier::Read, this);
	        connect (m_inputDevNotifier, SIGNAL(activated(int)), this, SLOT(readInputDevData()));
        }
    }
    m_handler = new QWSKeyboardHandler("");
}

void NyxKeyboardHandler::readInputDevData()
{
	nyx_error_t error = NYX_ERROR_NONE;
	nyx_event_handle_t event_handle = NULL;

	while ((error = nyx_device_get_event(m_nyxInputDevHandle, &event_handle)) == NYX_ERROR_NONE && event_handle != NULL) {
        nyx_bluetooth_input_detect_event_item_t data;

        error = nyx_bluetooth_input_detect_event_get_data(event_handle, &data);
        if (error != NYX_ERROR_NONE)
            qDebug() << __PRETTY_FUNCTION__ << "failed to get bluetooth input detect event data";

        switch (data.event_type)
        {
            case NYX_BLUETOOTH_INPUT_DETECT_EVENT_DEVICE_ID_ADD:
                {
				    m_curDeviceId = data.value;
					qDebug() << "NyxKeyboardHandlerPrivate: Added BT input device id:" << m_curDeviceId;
                }
                break;
             case NYX_BLUETOOTH_INPUT_DETECT_EVENT_DEVICE_ID_REMOVE:
                {
				    DeviceInfo *devInfo = m_deviceIdMap[data.value];
				    if (devInfo) delete devInfo;

					m_deviceIdMap.erase(data.value);
					m_curDeviceId = -1;
					qDebug() << __PRETTY_FUNCTION__ << "NyxKeyboardHandlerPrivate: Removed BT input device id:" << data.value;
					if (m_deviceIdMap.empty()) {
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

					qDebug() << __PRETTY_FUNCTION__ <<  "NyxKeyboardHandlerPrivate: BT keyboard handler set to" << optionStr;

					QWSKeyboardHandler* handler = QKbdDriverFactory::create("LinuxInput", optionStr);
					handler->setIsExternalKeyboard(true);
					m_deviceIdMap.insert(std::pair<int, DeviceInfo*>(m_curDeviceId, new DeviceInfo(handler)));
					if(setBluetoothKeyboardActive)
						(*setBluetoothKeyboardActive)(true);
			    }
				break;

            case NYX_BLUETOOTH_INPUT_DETECT_EVENT_DEVICE_COUNTRY:
				qDebug() << "NyxKeyboardHandlerPrivate: BT keyboard country set to" << data.value;
				m_curDeviceCountry = data.value;
			    break;
		    default:
                break;
		}

	    event_handle = NULL;
    }
}

void NyxKeyboardHandler::readKeyData() {
	/* initial NYX support */
	/* fine-tuning  support for NYX*/
	nyx_error_t error = NYX_ERROR_NONE;
	nyx_event_handle_t event_handle = NULL;

	while ((error = nyx_device_get_event(m_nyxKeysHandle, &event_handle)) == NYX_ERROR_NONE && event_handle != NULL)
	{
		int keycode = 0;
		nyx_key_type_t key_type;
		bool is_auto_repeat = false;
		bool is_press = false;

		error = nyx_keys_event_get_key(event_handle, &keycode);
		if (error == NYX_ERROR_NONE)
			error = nyx_keys_event_get_key_type(event_handle, &key_type);
		if (error == NYX_ERROR_NONE)
			error = nyx_keys_event_get_key_is_auto_repeat(event_handle, &is_auto_repeat);
		if (error == NYX_ERROR_NONE)
			error = nyx_keys_event_get_key_is_press(event_handle, &is_press);

		if (error != NYX_ERROR_NONE) {
			qWarning() << __PRETTY_FUNCTION__ << "Unable to obtain event_handle properties";
			return;
		}

        if (key_type == NYX_KEY_TYPE_STANDARD) {
            // These are the 'regular keys' and the keycodes should adhere to the linux
            // key codes -> let Qt do the mapping, this way ALT, SHIFT etc are also handled
            // the Qt way
            m_handler->processKeycode(keycode, is_press, is_auto_repeat);
        } else if (key_type == NYX_KEY_TYPE_CUSTOM) {
            Qt::Key qtKey = mapNyxKeyCode((nyx_keys_custom_key_t)keycode);
            // Since these are custom keys and in practice have very little in relation
            // to the modifiers (i.e. PowerKey && Shift or VolumeUp and Alt do not make
            // very much sense) we'll send these as have no modifiers.
            //
            // The first param unicode value is set to zero. Also these events are sent
            // directly to the active application and Qt will not try to map them
            m_handler->processKeyEvent(0, qtKey, Qt::NoModifier, is_press, is_auto_repeat);
        } else if (key_type == NYX_KEY_TYPE_UNDEFINED) {
            qWarning() << __PRETTY_FUNCTION__ << "NYX_KEY_TYPE_UNDEFINED, code:" << keycode << ", type:" << key_type;
		}

		error = nyx_device_release_event(m_nyxKeysHandle, event_handle);
		if (error != NYX_ERROR_NONE) {
			qWarning() << __PRETTY_FUNCTION__ << "Unable to release m_nyxKeysHandle event";
			return;
		}
		event_handle = NULL;
	}
}

Qt::Key NyxKeyboardHandler::mapNyxKeyCode(nyx_keys_custom_key_t code)
{
    Qt::Key key = Qt::Key_unknown;

    switch (code) {
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
        qWarning() << __PRETTY_FUNCTION__ << "nyx_keys_custom_key code" << code << "was not recognized";
        key = Qt::Key_unknown;
        break;
    }
    return key;
}
