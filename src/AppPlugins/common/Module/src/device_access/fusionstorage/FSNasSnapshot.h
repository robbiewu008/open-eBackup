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
#ifndef FUSIONSTOR_NAS_SNAPSHOT_H
#define FUSIONSTOR_NAS_SNAPSHOT_H

#include "define/Types.h"
#include "log/Log.h"
#include "curl_http/HttpClientInterface.h"
#include "device_access/Const.h"
#include "device_access/fusionstorage/FusionStorageNas.h"

namespace Module {
    class FSNasSnapshot : public FusionStorageNas {
    public:
        explicit FSNasSnapshot(ControlDeviceInfo deviceInfo, std::string id, std::string path)
            : FusionStorageNas(deviceInfo)
        {
            fileSystemId = id;
            uniquePath = path;
        }

        virtual ~FSNasSnapshot() {}

        std::unique_ptr <ControlDevice> CreateClone(std::string cloneName, int &errorCode)
        {
            return nullptr;
        }

    protected:
        std::string uniquePath;
    };
}

#endif
