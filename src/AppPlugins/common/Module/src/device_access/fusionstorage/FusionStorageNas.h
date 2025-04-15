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
#ifndef FUSION_STORAGE_NAS_H
#define FUSION_STORAGE_NAS_H

#include "device_access/fusionstorage/FusionStorageBlock.h"
#include "common/JsonUtils.h"
#include "system/System.hpp"

namespace Module {
    namespace FusionStorageErrorCode {
        const int NAMESPACE_SNAPSHOT_NOTEXIST = 33656855;
        const int NAMESPACE_SNAPSHOT_ALREADY_EXIST = 33656849;
    };

    class FusionStorageNas : public FusionStorageBlock {
    public:
        explicit FusionStorageNas(ControlDeviceInfo deviceInfo) : FusionStorageBlock(deviceInfo)
        {
            shareId = deviceInfo.shareId;
            fileSystemId = deviceInfo.fileSystemId;
            dtreeId = deviceInfo.dtreeId;
            sharePath = deviceInfo.sharePath;
        }

        FusionStorageNas(ControlDeviceInfo deviceInfo, std::string fsId) : FusionStorageBlock(deviceInfo)
        {
            fileSystemId = fsId;
        }

        virtual ~FusionStorageNas() {}

        int Delete() override;

        int Query(DeviceDetails &info) override;

        std::unique_ptr <ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;

        int DeleteSnapshot(std::string SnapshotName) override;

        int QuerySnapshotRollBackStatus(const std::string &snapshotName, std::string &snapshotId,
                                            std::string &rollbackStatus, std::string &endTime);

        int RollBackBySnapShotName(const std::string &snapshotName);

    protected:
        virtual int QueryFileSystem(DeviceDetails &info);

        int QueryFileSystem(std::string fileName, DeviceDetails &info);

        int QuerySnapshot(std::string SnapshotName, std::string &id);

        virtual int QueryDtreeWithShare();

        std::string fileSystemId;
        std::string dtreeId;
        std::string m_dtreePath;
        std::string m_dtreeName;
        std::string fileSystemName;
        std::string shareId;
        std::string sharePath;
    };
}
#endif  // FUSION_STORAGE_BLOCK_H
