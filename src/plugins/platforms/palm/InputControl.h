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


#ifndef _INPUTCONTROL_H_
#define _INPUTCONTROL_H_

#include <nyx/nyx_client.h>

class InputControl
{
public:

    virtual ~InputControl() {};

    virtual bool on() = 0;
    virtual bool off() = 0;
    virtual bool setRate(int rate) =0;

    virtual nyx_device_handle_t getHandle() = 0;
};

#endif /* _INPUTCONTROL_H_ */
