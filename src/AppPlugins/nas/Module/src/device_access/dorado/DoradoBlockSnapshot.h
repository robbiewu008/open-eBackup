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
#ifndef DORADO_BLOCK_SNAPSHOT_H
#define DORADO_BLOCK_SNAPSHOT_H

#include "define/Types.h"
#include "log/Log.h"
#include "device_access/Const.h"
#include "device_access/dorado/DoradoBlock.h"
#include "curl_http/HttpClientInterface.h"
namespace Module {
    class DoradoBlockSnapshot : public DoradoBlock {
    public:
        explicit DoradoBlockSnapshot(ControlDeviceInfo deviceInfo, int id, std::string wwn, bool readFromK8s = true)
                : DoradoBlock(deviceInfo, readFromK8s) {
            ResourceId = id;
            Wwn = wwn;
            if (readFromK8s) {
            GetConnectedIP();
            } else {
                Login();
            }
        }

        ~DoradoBlockSnapshot() {
            if (fs_pHttpCLient) {
                fs_pHttpCLient = NULL;
            }
        }

        int Query(DeviceDetails &info) override;

        int Delete() override;

        int ExtendSize(unsigned long long size) override;

        int Revert(std::string SnapshotName) override;

        std::unique_ptr<ControlDevice> CreateClone(std::string cloneName, int &errorCode) override;

        int CreateReplication(
                int localResId, int rResId, std::string rDevId, int bandwidth, std::string &repId) override;

        int ActiveReplication(std::string repId) override;

        int QueryReplication(ReplicationPairInfo &replicationPairInfo) override;

    protected:
        int QuerySnapshotEx(std::string SnapshotName, int &id, std::string &WWN);

        inline void AssignDeviceInfo(ControlDeviceInfo &deviceInfo, std::string volumeName);

        void CreateCloneSendRequest(std::string volumeName, std::string originalName, int &iRet, int &errorCode);

        int GetLunIDBySnapshot(int snapshotId, int &lunId);
    };
}
#endif
