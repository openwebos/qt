/**
 *******************************************************************************
 * @file InputControl.h
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

#ifndef _INPUTCONTROL_H_
#define _INPUTCONTROL_H_

#include <hal/hal.h>

class InputControl
{
public:

    virtual ~InputControl() {};

    virtual bool on() = 0;
    virtual bool off() = 0;
    virtual bool setRate(int rate) =0;

    virtual hal_device_handle_t getHandle() = 0;
};

#endif /* _INPUTCONTROL_H_ */
