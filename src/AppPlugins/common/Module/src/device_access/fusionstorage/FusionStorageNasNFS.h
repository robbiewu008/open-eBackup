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
#ifndef FUSION_STORAGE_NAS_NFS_H
#define FUSION_STORAGE_NAS_NFS_H

#include "device_access/fusionstorage/FusionStorageNas.h"

namespace Module {
    class FusionStorageNasNFS : public FusionStorageNas {
    public:
        explicit FusionStorageNasNFS(ControlDeviceInfo deviceInfo) : FusionStorageNas(deviceInfo) {}

        ~FusionStorageNasNFS() {}

        int Query(DeviceDetails &info) override;
        
        std::unique_ptr <ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;

        int DeleteSnapshot(std::string SnapshotName) override;

    protected:
        int QueryNFSShare(DeviceDetails &info, std::string fsId);

        int QueryDtreeWithShare() override;
    };
}

#endif
