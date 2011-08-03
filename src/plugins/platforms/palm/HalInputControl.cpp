/**
 *******************************************************************************
 * @file HalInputControl.cpp
 *
 * Copyright (c) 2010 Hewlett-Packard Development Company, L.P.
 * All rights reserved.
 *
 * @brief
 *
 * Created on Jan 11, 2011
 *
 *******************************************************************************
 */

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
