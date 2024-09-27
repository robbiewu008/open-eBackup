/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#ifndef PLUGIN_SERVICE_IMP_H
#define PLUGIN_SERVICE_IMP_H

#include "PluginService.h"

#ifdef WIN32
#include "define/Defines.h"

class AGENT_API PluginServiceImp;
#endif

class PluginServiceImp : public AppProtect::PluginServiceIf {
public:
    PluginServiceImp();
    ~PluginServiceImp();

    /**
     * query the application plugin information
    */
    virtual void QueryPlugin(ApplicationPlugin& returnValue) override;
};

#endif // _PLUGIN_SERVICE_IMP_H_