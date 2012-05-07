/****************************************************************************
**
** Copyright (C) 2012 GoodLife
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

#ifndef QINPUT_DEVICE_SCANNER_H
#define QINPUT_DEVICE_SCANNER_H

#include <qobject.h>
#include <Qt>

class QInputDeviceScanner : public QObject
{
    Q_OBJECT
public:
    QInputDeviceScanner();
    ~QInputDeviceScanner();

    void scan();

    QString getMouseEventName() const;
    QString getKeyboardEventName() const;

    int getNumOfMouses();
    int getNumOfKeyboards();

    const QString & getMouseName(int idx);
    const QString & getKeyboardName(int idx);

private:
    QString m_mouseEvent;
    QString m_keyboardEvent;

    QList<QString> m_listOfMouse;
    QList<QString> m_listOfKeyboard;
};

#endif /* QINPUT_DEVICE_SCANNER_H */
