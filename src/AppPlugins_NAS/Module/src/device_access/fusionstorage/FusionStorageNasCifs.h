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
#ifndef FUSION_STORAGE_NAS_CIFS_H
#define FUSION_STORAGE_NAS_CIFS_H

#include "device_access/fusionstorage/FusionStorageNas.h"

namespace Module {
    class FusionStorageNasCIFS : public FusionStorageNas {
    public:
        explicit FusionStorageNasCIFS(ControlDeviceInfo deviceInfo) : FusionStorageNas(deviceInfo) {}

        FusionStorageNasCIFS(ControlDeviceInfo deviceInfo, std::string fsId) : FusionStorageNas(deviceInfo, fsId) {}

        FusionStorageNasCIFS(ControlDeviceInfo deviceInfo, std::string id, std::string path)
                : FusionStorageNas(deviceInfo) {
            fileSystemId = id;
            uniquePath = path;
        }

        virtual ~FusionStorageNasCIFS() {}

        int Query(DeviceDetails &info) override;

        std::unique_ptr <ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;

    protected:
        int QueryFileSystem(DeviceDetails &info);

        int QueryCIFSShare(DeviceDetails &info, std::string fsId);

        int GetFsNameFromShareName();

        std::string uniquePath;

        std::string fileSystemName{};
    };
}

#endif