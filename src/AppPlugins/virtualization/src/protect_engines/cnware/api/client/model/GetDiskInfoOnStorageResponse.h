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
#ifndef CNWARE_GET_DISK_INFO_ON_STORAGE_RESPONSE_H
#define CNWARE_GET_DISK_INFO_ON_STORAGE_RESPONSE_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {
class GetDiskInfoOnStorageResponse : public VirtPlugin::ResponseModel {
public:
    GetDiskInfoOnStorageResponse() {}
    ~GetDiskInfoOnStorageResponse() {}

    bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_diskInfo)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }

    CNwareDiskData GetInfo()
    {
        return m_diskInfo;
    }

private:
    CNwareDiskData m_diskInfo {};
};
};

#endif