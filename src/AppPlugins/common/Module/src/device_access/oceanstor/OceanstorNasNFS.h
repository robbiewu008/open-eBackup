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
#ifndef OCEANSTOR_NAS_NFS_H
#define OCEANSTOR_NAS_NFS_H

#include "define/Types.h"
#include "log/Log.h"
#include "curl_http/HttpClientInterface.h"
#include "device_access/Const.h"
#include "device_access/oceanstor/OceanstorNas.h"

namespace Module {
    class OceanstorNasNFS : public OceanstorNas {
    public:
        explicit OceanstorNasNFS(ControlDeviceInfo deviceInfo) : OceanstorNas(deviceInfo) {}

        OceanstorNasNFS(ControlDeviceInfo deviceInfo, std::string fsId) : OceanstorNas(deviceInfo, fsId) {}

        virtual ~OceanstorNasNFS();

        int Bind(HostInfo &host, const std::string &shareId = "") override;

        int UnBind(HostInfo host, const std::string &shareId = "") override;

        int Query(DeviceDetails &info) override;

        int Delete() override;

        int DeleteShare();

        int CreateShare();

        int QueryNFSShareClient(const std::string shareId, std::vector<std::string> &nasShareIPList);

        std::unique_ptr <ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;

    protected:
        int QueryFileSystem(DeviceDetails &info) override;

        int NFSShareAddClient(std::string name, int ID);

        int QueryNFSShare(DeviceDetails &info, std::string fsId);

        int DeleteNFSShare(DeviceDetails info);

        int CreateNFSShare(std::string fileSystemName, std::string FsId);

        int DeleteNFSShareClient(std::string shareClientId);

        int DeleteNFSShareClient(const std::vector<std::string> &iPList, const std::string shareId);

        std::unique_ptr <ControlDevice> CreateClone(std::string volumeName, int &errorCode);

        std::unordered_map <std::string, std::string> NFSShareIDList;

        int GetFsNameFromShareName();

        void ModifySpecialCharForURL(std::string &stringName);

        void ModifySpecialCharForFSNameCheck(std::string &stringName);

        std::string fileSystemName{};
    };
}

#endif
