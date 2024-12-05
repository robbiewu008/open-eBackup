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
#ifndef OCEANSTOR_NAS_SNAPSHOT_H
#define OCEANSTOR_NAS_SNAPSHOT_H

#include "define/Types.h"
#include "log/Log.h"
#include "curl_http/HttpClientInterface.h"
#include "device_access/Const.h"
#include "device_access/oceanstor/OceanstorNas.h"

namespace Module {
    class OceanstorNasSnapshot : public OceanstorNas {
    public:
        explicit OceanstorNasSnapshot(ControlDeviceInfo deviceInfo, std::string id, std::string path)
            : OceanstorNas(deviceInfo)
        {
            fileSystemId = id;
            uniquePath = path;
        }

        OceanstorNasSnapshot(ControlDeviceInfo deviceInfo, std::string id, std::string vId, std::string path)
            : OceanstorNas(deviceInfo)
        {
            fileSystemId = id;
            vstoreId = vId;
            uniquePath = path;
        }

        virtual ~OceanstorNasSnapshot() {}

        std::unique_ptr <ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override
        {
            return nullptr;
        }

        mp_int32 Query(DeviceDetails &info) override;

        std::unique_ptr <ControlDevice> CreateClone(std::string cloneName, int &errorCode) override;

    protected:
        std::string uniquePath;

        mp_int32 CreateCloneFromSnapShot(std::string cloneName, std::string &fsId);
    };
}
#endif // OCEANSTOR_NAS_SNAPSHOT_H
