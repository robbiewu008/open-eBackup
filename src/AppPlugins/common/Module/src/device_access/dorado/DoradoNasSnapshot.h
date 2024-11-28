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
#ifndef DORADO_NAS_SNAPSHOT_H
#define DORADO_NAS_SNAPSHOT_H

#include "define/Types.h"
#include "log/Log.h"
#include "curl_http/HttpClientInterface.h"
#include "device_access/Const.h"
#include "device_access/dorado/DoradoNas.h"
namespace Module {
    class DoradoNasSnapshot : public DoradoNas {
    public:
        explicit DoradoNasSnapshot(ControlDeviceInfo deviceInfo, std::string id, std::string path,
                                   bool readFromK8s = true)
            : DoradoNas(deviceInfo, readFromK8s)
        {
            fileSystemId = id;
            uniquePath = path;
            readK8s = readFromK8s;
        }

        virtual ~DoradoNasSnapshot() {}

        int Bind(HostInfo &host, const std::string &shareId = "") override;

        int UnBind(HostInfo host, const std::string &shareId = "") override;

        int Create(unsigned long long size) override;

        int Delete() override;

        int Query(DeviceDetails &info) override;

        std::unique_ptr<ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;

        std::unique_ptr<ControlDevice> CreateClone(std::string cloneName, int &errorCode) override;

        void AssignDeviceInfo(ControlDeviceInfo &deviceInfo, std::string volumeName);

        int QueryServiceHost(std::vector<std::string> &iscsiList, IP_TYPE ipType = IP_TYPE::IP_V4) override
        {
            return FAILED;
        }

        void SetFileSystemId(std::string &systemId)
        {
            fileSystemId = systemId;
        }

    protected:
        int CreateCloneFromSnapShot(std::string cloneName, std::string &fsId);

        std::string uniquePath;
    };
}
#endif  // DORADO_NAS_SNAPSHOT_H
