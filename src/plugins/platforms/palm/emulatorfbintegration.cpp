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

#include "emulatorfbintegration.h"
#include "hiddtp_qpa.h"
#include "nyxkeyboardhandler.h"
#include <QDebug>

EmulatorFbIntegration::EmulatorFbIntegration() : QLinuxFbIntegration()
{

    QList<QPlatformScreen*> screenList = screens();
    if (screenList.size() > 0) {
        // The first one is the primary screen
        QRect screenGeom = screenList.first()->geometry();
        qDebug() << __PRETTY_FUNCTION__ << "using geometry" << screenGeom;
        mouse = new QPAHiddTpHandler(screenGeom.width(), screenGeom.height());
        keyboard = new NyxKeyboardHandler;
    } else {
        qCritical() << __PRETTY_FUNCTION__ << "Could not get a handle to the primary screen";
    }
}

EmulatorFbIntegration::~EmulatorFbIntegration()
{
    delete keyboard;
    delete mouse;
}
