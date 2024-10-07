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
#ifndef FS_BLOCK_SNAPSHOT_H
#define FS_BLOCK_SNAPSHOT_H

#include "define/Types.h"
#include "log/Log.h"
#include "device_access/Const.h"
#include "device_access/fusionstorage/FusionStorageBlock.h"

namespace Module {
    class FSBlockSnapshot : public FusionStorageBlock {
    public:
        FSBlockSnapshot(ControlDeviceInfo deviceInfo, int id, mp_string wwn) : FusionStorageBlock(deviceInfo) {
            ResourceId = id;
            Wwn = wwn;
        }

        virtual ~FSBlockSnapshot() {
            DeleteDeviceSession();
            if (fs_pHttpCLient) {
                fs_pHttpCLient = NULL;
            }
        }

        int Query(DeviceDetails &info) override;

        int Delete() override;

        int ExtendSize(unsigned long long size) override;

        int Revert(mp_string SnapshotName) override;

        std::unique_ptr <ControlDevice> CreateClone(mp_string cloneName, int &errorCode) override;

    private:
        int QuerySnapshotEx(mp_string SnapshotName, int &id, mp_string &WWN, int &status);
    };
}
#endif  // FS_BLOCK_SNAPSHOT_H
