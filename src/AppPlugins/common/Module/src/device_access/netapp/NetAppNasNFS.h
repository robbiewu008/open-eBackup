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
#ifndef NETAPP_NAS_NFS_H
#define NETAPP_NAS_NFS_H

#include <sstream>
#include "common/Path.h"
#include "log/Log.h"
#include "define/Types.h"
#include "system/System.hpp"
#include "common/JsonUtils.h"
// #include "framework/MessageProcess.h"
#include "curl_http/HttpClientInterface.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "device_access/Const.h"
#include "device_access/netapp/NetAppNas.h"

namespace Module {
    class NetAppNasNFS : public NetAppNas {
    public:
        explicit NetAppNasNFS(ControlDeviceInfo deviceInfo, bool querySvm = true)
            : NetAppNas(deviceInfo, querySvm)
        {}

        virtual ~NetAppNasNFS();

        int Bind(HostInfo &host, const std::string &shareId="") override;

        int UnBind(HostInfo host, const std::string &shareId="") override;

        int Create(unsigned long long size) override;

        int Query(DeviceDetails& info) override;

        int Delete() override;

        int CreateShare();

        // following functions are currently unused
        int QueryNFSShareClient(const std::string shareId, std::vector<std::string> &nasShareIPList);

        int SetIsDeleteParentSnapShotFlag(bool flag)
        {
            isDeleteParentSnapShot = flag;
        }

    protected:


        int QueryNFSShare(std::string volumeName, std::string &sharePath, int& errorCode);

        int DeleteNFSShare(std::string volumeName);

        int ValidateCreateNFSShareResponse(Json::Value &data, std::string &sharePath);

        int CreateNFSShareCheck(const int& iRet, Json::Value& data, std::string& sharePath);

        int CreateNFSShare(std::string volumeName, std::string sharePath);

        std::unique_ptr<ControlDevice> CreateClone(std::string cloneName, int &errorCode);

        // following functions currently unused
        int NFSShareAddClient(std::string name, int ID);

        int DeleteNFSShareClient(std::string shareClientId);

        int DeleteNFSShareClient(const std::vector<std::string> &iPList, const std::string shareId);

        std::unordered_map<std::string, std::string> NFSShareIDList;

        bool isDeleteParentSnapShot = false;
    };
}
#endif
