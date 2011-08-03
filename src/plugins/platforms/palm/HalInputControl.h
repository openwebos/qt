/**
 *******************************************************************************
 * @file HalInputControl.h
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
