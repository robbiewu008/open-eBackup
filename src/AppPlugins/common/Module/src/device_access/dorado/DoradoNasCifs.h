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
#ifndef DORADO_NAS_CIFS_H
#define DORADO_NAS_CIFS_H

#include "define/Types.h"
#include "log/Log.h"
#include "curl_http/HttpClientInterface.h"
#include "device_access/Const.h"
#include "device_access/dorado/DoradoNas.h"

namespace Module {
    class DoradoNasCIFS : public DoradoNas {
    public:
        explicit DoradoNasCIFS(ControlDeviceInfo deviceInfo, bool readFromK8s = true)
            : DoradoNas(deviceInfo, readFromK8s)
        {
            readK8s = readFromK8s;
            fileSystemName = deviceInfo.deviceName;
        }

        DoradoNasCIFS(ControlDeviceInfo deviceInfo, std::string fsId, bool readFromK8s = true)
            : DoradoNas(deviceInfo, fsId, readFromK8s)
        {
            readK8s = readFromK8s;
        }

        virtual ~DoradoNasCIFS();

        int Bind(HostInfo &host, const std::string &shareId = "") override;

        int AddShareClient(HostInfo &host, const std::string &sharePath, std::string &cifsShareName);

        int UnBind(HostInfo host, const std::string &shareId = "") override;

        int Create(unsigned long long size);

        int Query(DeviceDetails &info) override;

        int QueryServiceHost(std::vector<std::string> &ipList, IP_TYPE ipType = IP_TYPE::IP_V4);

        int DeleteWindowsUser(std::string userName);

        int CreateShare();

        int QueryCifsShare(std::vector<NasSharedInfo> &infos, std::string fsId);

        int QueryCifsShareClient(NasSharedInfo &info, std::string sharedId);

        int Delete() override;

        int DeleteShare(const std::string sharePath, const std::string shareName);

        int LOCAL_USER = 2;
        int AD_USER = 0;

        int FULL_CONTROL = 1;

        std::unique_ptr<ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;

    protected:
        int QueryFileSystem(DeviceDetails &info) override;

        int CIFSShareAddClient(std::string name, int ID, const std::string &domainName = "");

        int DeleteCIFSShare(DeviceDetails info);

        int CreateCifsShare(std::string fileSystemName, std::string FsId);

        int QueryCifsShare(DeviceDetails &info, std::string fsId);

        int QueryCifsShare(const std::string sharePath, DeviceDetails &info, std::string cifsShareName);

        int CreateWindowUser(std::string userName, std::string password);

        std::unique_ptr<ControlDevice> CreateClone(std::string volumeName, int &errorCode);

        void FilterLogicPort(Json::Value data, std::vector<std::string> &cifsIPList, IP_TYPE ipType);

        int GetFsNameFromShareName();

        std::string fileSystemName{};
    };
}

#endif