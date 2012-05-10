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


#ifndef _HALINPUTCONTROL_H_
#define _HALINPUTCONTROL_H_

#include "InputControl.h"

class HalInputControl: public InputControl {
public:
    HalInputControl(hal_device_type_t type, hal_device_id_t id);
    virtual ~HalInputControl();

    virtual bool on();
    virtual bool off();
    virtual bool setRate(int rate);

    virtual hal_device_handle_t getHandle() { return m_handle; }

private:
    hal_device_handle_t m_handle;
};

#endif /* _HALINPUTCONTROL_H_ */
