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
#ifndef __ADD_DISK_RESPONSE__
#define __ADD_DISK_RESPONSE__

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {

class AddDiskResponse : public VirtPlugin::ResponseModel {
public:
    AddDiskResponse() {};
    ~AddDiskResponse() {};

    std::string GetTaskId()
    {
        return m_taskMsg.taskId;
    }

    virtual bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_taskMsg)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }

private:
    TaskMsg m_taskMsg;
};

};

#endif