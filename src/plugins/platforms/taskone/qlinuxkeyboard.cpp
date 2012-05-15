/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include "qlinuxkeyboard.h"

#include <Qt>
#include <QKeyEvent>
#include <QApplication>
#include <QSocketNotifier>
#include <QStringList>
#include <QWindowSystemInterface>

#include <qkbd_qws.h>

#include <qplatformdefs.h>
#include <private/qcore_unix_p.h>

#include <errno.h>
#include <linux/input.h>

#include <qdebug.h>

#define LINUX_KEY_PRESSED 	1
#define LINUX_KEY_RELEASED	0

class QWSLinuxInputKeyboardHandler : public QWSKeyboardHandler
{
public:
    QWSLinuxInputKeyboardHandler(const QString&);
    ~QWSLinuxInputKeyboardHandler();

    virtual bool filterInputEvent(quint16 &input_code, qint32 &input_value);

};

QWSLinuxInputKeyboardHandler::QWSLinuxInputKeyboardHandler(const QString &device)
    : QWSKeyboardHandler(device)
{
}

QWSLinuxInputKeyboardHandler::~QWSLinuxInputKeyboardHandler()
{
}

bool QWSLinuxInputKeyboardHandler::filterInputEvent(quint16 &, qint32 &)
{
    return false;
}


QLinuxKeyboardHandler::QLinuxKeyboardHandler(const QString &specification)
{
    QString dev = QLatin1String("/dev/input/event1");
    int repeat_delay = -1;
    int repeat_rate = -1;
    bool ttymode = false;

    QStringList args = specification.split(QLatin1Char(':'));
    foreach( const QString &arg, args )
    {
        if( arg.startsWith(QLatin1String("repeat-delay=")) )
        {
            repeat_delay = arg.mid(13).toInt();
        }
        else if( arg.startsWith(QLatin1String("repeat-rate=")) )
        {
            repeat_rate = arg.mid(12).toInt();
        }
        else if( arg.startsWith(QLatin1String("ttymode")) )
        {
            ttymode = true;
        }
        else if( arg.startsWith(QLatin1String("/dev/")) )
        {
            dev = arg;
        }
    }

    m_handler = new QWSLinuxInputKeyboardHandler(dev);

    m_fd = QT_OPEN(dev.toLocal8Bit().constData(), O_RDWR, 0);
    if( m_fd >= 0 )
    {
        if( repeat_delay > 0 && repeat_rate > 0 )
        {
            int kbdrep[2] = { repeat_delay, repeat_rate };
            ::ioctl(m_fd, EVIOCSREP, kbdrep);
        }

        QSocketNotifier *notifier;
        notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)), this, SLOT(readKeycode()));

        if( ttymode )
        {
            // To do if required.
        }
    }
    else
    {
        qWarning("Cannot open keyboard input device '%s': %s", qPrintable(dev), strerror(errno));
        return;
    }
}

QLinuxKeyboardHandler::~QLinuxKeyboardHandler() 
{
    if( m_fd >= 0 )
    {
        QT_CLOSE(m_fd);
    }

    delete m_handler;
}

void QLinuxKeyboardHandler::readKeycode() 
{
    struct ::input_event buffer[32];
    int n = 0;

    forever 
    {
        n = QT_READ(m_fd, reinterpret_cast<char *>(buffer) + n, sizeof(buffer) - n);

        if (n == 0) 
        {
            qWarning("Got EOF from the input device.");
            return;
        } 
        else if (n < 0 && (errno != EINTR && errno != EAGAIN)) 
        {
            qWarning("Could not read from input device: %s", strerror(errno));
            return;
        } 
        else if (n % sizeof(buffer[0]) == 0) 
        {
            break;
        }
    }

    n /= sizeof(buffer[0]);

    for( int i = 0; i < n; ++i ) 
    {
        if (buffer[i].type != EV_KEY)
            continue;

        quint16 code = buffer[i].code;
        qint32 value = buffer[i].value;

        if( m_handler->filterInputEvent(code, value) )
        {
            continue;
        }

        QWSKeyboardHandler::KeycodeAction ka;
        ka = m_handler->processKeycode(code, value != 0, value == 2);

        // To Do: Process for KeycodeAction
    }
}
