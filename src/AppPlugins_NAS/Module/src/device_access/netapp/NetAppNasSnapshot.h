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
#ifndef NETAPP_NAS_SNAPSHOT_H
#define NETAPP_NAS_SNAPSHOT_H

#include "device_access/netapp/NetAppNas.h"
#include "log/Log.h"
#include "define/Types.h"
#include "common/JsonUtils.h"
#include "curl_http/HttpClientInterface.h"
// #include "framework/MessageProcess.h"
#include "device_access/Const.h"

namespace Module {
    class NetAppNasSnapshot : public NetAppNas {
    public:
        explicit  NetAppNasSnapshot(ControlDeviceInfo deviceInfo, std::string volumeUuid,
                                    std::string volumeName, bool querySvm = true) : NetAppNas(deviceInfo, querySvm)
        {
            this->snapshotName = deviceInfo.deviceName;
            this->snapVolumeName = volumeName;
            this->snapVolumeUuid = volumeUuid;
        };

        explicit  NetAppNasSnapshot()
        {};

        virtual ~NetAppNasSnapshot()
        {};

        int Bind(HostInfo &host, const std::string &shareId="") override;
        int UnBind(HostInfo host, const std::string &shareId="") override;
        int Create(unsigned long long size) override;
        int Delete() override;
        int Query(DeviceDetails& info) override;
        std::unique_ptr<ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;
        std::unique_ptr<ControlDevice> CreateClone(std::string cloneName, int &errorCode) override;

    protected:
        std::string snapshotName {};
        std::string snapVolumeName {};
        std::string snapVolumeUuid {};
        int CreateCloneFromSnapshot(std::string cloneName, int &errorCode);
        int ValidateCloneFromSnapshotResponse(Json::Value &data);
    };
}
#endif