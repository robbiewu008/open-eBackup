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
#ifndef NETAPP_NAS_CIFS_H
#define NETAPP_NAS_CIFS_H

#include <sstream>
#include "log/Log.h"
#include "common/Path.h"
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
    class NetAppNasCIFS : public NetAppNas {
    public:
        explicit NetAppNasCIFS(ControlDeviceInfo deviceInfo, bool querySvm = true)
            : NetAppNas(deviceInfo, querySvm)
        {m_userOrGroup = deviceInfo.nasUserGroup; }

        virtual ~NetAppNasCIFS();

        int Bind(HostInfo &host, const std::string &shareId="") override;

        int UnBind(HostInfo host, const std::string &shareId="") override;

        int Create(unsigned long long size);

        int Query(DeviceDetails& info) override;

        int CreateShare();

        int Delete() override;

        int SetIsDeleteParentSnapShotFlag(bool flag)
        {
            isDeleteParentSnapShot = flag;
        }

        // following function not used currently
        int DeleteWindowsUser(std::string userName);

    protected:
        int DeleteCIFSShare(std::string volumeName);

        int CreateCifsShare(std::string volumeName);

        int ValidateCreateNasShareRsp(Json::Value &data, std::string sharePath);

        int CreateNasShare(std::string volumeName, std::string sharePath);

        int ValidateQueryCifsShareResponse(Json::Value &data, std::string volumeName,
                                                std::string &sharePath, int& errorCode);

        int QueryCifsShare(std::string volumeName, std::string &sharePath, int& errorCode);

        std::unique_ptr<ControlDevice> CreateClone(std::string cloneName, int &errorCode);

        // following functions not used currently
        int CIFSShareAddClient(std::string name, int ID);

        int CreateWindowUser(std::string userName, std::string password);

        bool isDeleteParentSnapShot = false;
        std::string m_userOrGroup {};
    };
}
#endif