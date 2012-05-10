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


#include "HalInputControl.h"
#include <glib.h>

HalInputControl::HalInputControl(hal_device_type_t type, hal_device_id_t id) : m_handle(0)
{
    hal_error_t error = hal_device_open(type, id, &m_handle);
    if ((error != HAL_ERROR_SUCCESS) || (m_handle == NULL))
    {
        g_critical("Unable to obtain m_handle");
    }
}

HalInputControl::~HalInputControl()
{
    if (m_handle)
    {
        hal_error_t error = hal_device_close(m_handle);
        g_critical("Unable to release m_handle");
    }
}

bool HalInputControl::on()
{
    if (m_handle)
    {
        hal_error_t error = HAL_ERROR_SUCCESS;
        error = hal_device_set_operating_mode(m_handle, HAL_OPERATING_MODE_ON);
        return (error == HAL_ERROR_SUCCESS || error == HAL_ERROR_NOT_IMPLEMENTED);
    }
    return true;
}

bool HalInputControl::off()
{
    if (m_handle)
    {
        hal_error_t error = HAL_ERROR_SUCCESS;
        error = hal_device_set_operating_mode(m_handle, HAL_OPERATING_MODE_OFF);
        return (error == HAL_ERROR_SUCCESS || error == HAL_ERROR_NOT_IMPLEMENTED);
    }
    return true;
}

bool HalInputControl::setRate(int rate)
{
    if (m_handle)
    {
        hal_error_t error = HAL_ERROR_SUCCESS;
        error = hal_device_set_report_rate(m_handle, rate);
        return (error == HAL_ERROR_SUCCESS || error == HAL_ERROR_NOT_IMPLEMENTED);
    }
    return true;
}
