/****************************************************************************
**
** Copyright (C) 2012 TaskOne
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

#include "qinputdevicescanner.h"

#include <Qt>
#include <QFile>
#include <QDebug>

#include <linux/input.h>

#define MAX_INPUT_DEVICES   20

#define BITS_PER_LONG           (sizeof(long) * 8)
#define NBITS(x)                ((((x)-1)/BITS_PER_LONG)+1)
#define OFFSET(x)               ((x)%BITS_PER_LONG)
#define LONG(x)                 ((x)/BITS_PER_LONG)
#define IS_BIT_SET(bit, array)  ((array[LONG(bit)] >> OFFSET(bit)) & 1)

QInputDeviceScanner::QInputDeviceScanner()
{
    setObjectName(QLatin1String("Input Device Scanner"));
}

QInputDeviceScanner::~QInputDeviceScanner()
{
    m_listOfKeyboard.clear();
    m_listOfMouse.clear();
}

void QInputDeviceScanner::scan()
{
    for( int index = 0; index < MAX_INPUT_DEVICES; index++ )
    {
        QString devInput = QString("/dev/input");

        QString devEvent = devInput + "/event" + QString::number(index);

        QFile fileEvent;

        fileEvent.setFileName(devEvent);

        if( fileEvent.open(QIODevice::ReadOnly) )
        {
            char name[256] = "Unknown";
            char phys[256] = "";

            if( ::ioctl(fileEvent.handle(), EVIOCGNAME(sizeof(name)), name) < 0 )
            {
//                qWarning("Cannot get the name of device");
            }

            if( ::ioctl(fileEvent.handle(), EVIOCGPHYS(sizeof(phys)), phys) < 0 )
            {
//                qWarning("Cannot get the physical location");
            }

            QString deviceName = QString(name);
            QString devicePhys = QString(phys);

            if( devicePhys.startsWith("usb-dev") )
            {
                unsigned long evbit[NBITS(EV_MAX + 1)];

                if( ::ioctl(fileEvent.handle(), EVIOCGBIT(0, sizeof(evbit)), evbit) >= 0 )
                {
                    if( IS_BIT_SET(EV_REL, evbit) )
                    {
                        qDebug() << QString("Found Mouse: ") << devEvent;

                        m_listOfMouse.append(devEvent);
                    }
                    else if( IS_BIT_SET(EV_KEY, evbit) &&
                             ! IS_BIT_SET(EV_REL, evbit) &&
                             ! IS_BIT_SET(EV_ABS, evbit) )
                    {
                        qDebug() << QString("Found Keyboard: ") << devEvent;

                        m_listOfKeyboard.append(devEvent);
                    }
                }
            }

            fileEvent.close();
        }
    }
}

int QInputDeviceScanner::getNumOfMouses()
{
    return m_listOfMouse.count();
}

int QInputDeviceScanner::getNumOfKeyboards()
{
    return m_listOfKeyboard.count();
}

const QString & QInputDeviceScanner::getMouseName(int idx)
{
    return m_listOfMouse.at(idx);
}

const QString & QInputDeviceScanner::getKeyboardName(int idx)
{
    return m_listOfKeyboard.at(idx);
}


