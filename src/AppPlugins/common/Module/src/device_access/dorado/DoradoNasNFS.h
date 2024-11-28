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
#ifndef DORADO_NAS_NFS_H
#define DORADO_NAS_NFS_H

#include "define/Types.h"
#include "log/Log.h"
#include "curl_http/HttpClientInterface.h"
#include "device_access/Const.h"
#include "device_access/dorado/DoradoNas.h"

namespace Module {
    namespace Syncmode {
        const int SYNC = 0;
        const int ASYNC = 1;
    };

    class DoradoNasNFS : public DoradoNas {
    public:
        explicit DoradoNasNFS(ControlDeviceInfo deviceInfo, bool readFromK8s = true)
            : DoradoNas(deviceInfo, readFromK8s)
        {
            readK8s = readFromK8s;
        }

        DoradoNasNFS(ControlDeviceInfo deviceInfo, std::string fsId, bool readFromK8s = true)
            : DoradoNas(deviceInfo, fsId, readFromK8s)
        {
            readK8s = readFromK8s;
        }

        virtual ~DoradoNasNFS();

        int Bind(HostInfo &host, const std::string &shareId = "") override;

        int UnBind(HostInfo host, const std::string &shareId = "") override;

        int Create(unsigned long long size) override;

        int Query(DeviceDetails &info) override;

        int Delete() override;

        int QueryServiceHost(std::vector<std::string> &ipList, IP_TYPE ipType = IP_TYPE::IP_V4) override;

        int CreateShare();

        int QueryNFSShareClient(const std::string shareId, std::vector<std::string> &nasShareIPList);

        int QueryNFSShare(std::vector<NasSharedInfo> &info, std::string fsId);

        int QueryNFSShareClient(NasSharedInfo &info, std::string sharedId);

        int SetIsDeleteParentSnapShotFlag(bool flag)
        {
            isDeleteParentSnapShot = flag;
        }

        int QueryNFSShare(DeviceDetails &info, std::string fsId);

        int DeleteNFSShare(DeviceDetails info);

        // 此接口由调用者确定文件系统没有共享时调用，为文件系统创建共享，CreateShare()接口在文件系统下存在其他目录共享时不会创建
        int CreateNFSShare();

        int CreateNFSShareExact(std::string fileSystemName, std::string FsId);

    protected:
        int NFSShareAddClient(std::string name, int ID);

        int CreateNFSShare(std::string fileSystemName, std::string FsId);

        int DeleteNFSShareClient(std::string shareClientId);

        int DeleteNFSShareClient(const std::vector<std::string> &iPList, const std::string shareId);

        std::unique_ptr<ControlDevice> CreateClone(std::string volumeName, int &errorCode);

        void FilterLogicPort(Json::Value data, std::vector<std::string> &nfsIPList, IP_TYPE ipType);

        bool CheckNFSShareClientExist(std::string deviceId, std::string clientIp);

        std::unordered_map <std::string, std::string> NFSShareIDList;

        bool isDeleteParentSnapShot = false;  // 基于文件系统创建的克隆文件系统，删除需要删除对应的快照
    };
}

#endif
