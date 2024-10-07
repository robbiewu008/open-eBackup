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
#include "device_access/dorado/DoradoNasSnapshot.h"
#include "common/JsonUtils.h"
namespace Module {
    int DoradoNasSnapshot::Create(unsigned long long size) {
        return FAILED;
    }

    int DoradoNasSnapshot::Query(DeviceDetails &info) {
        std::string id;
        int ret = QuerySnapshot(ResourceName, id);
        if (ret == SUCCESS) {
            info.deviceName = ResourceName;
            info.deviceId = std::stoi(fileSystemId);
            info.deviceUniquePath = "/" + ResourceName + "/";
            return SUCCESS;
        }
        return ret;
    }

    std::unique_ptr<ControlDevice> DoradoNasSnapshot::CreateClone(std::string cloneName, int &errorCode) {
        int id;
        DeviceDetails info;
        ControlDeviceInfo deviceInfo = {};
        std::string fsId;
        int ret = QueryFileSystem(cloneName, info);
        if (ret == SUCCESS) {
            errorCode = ret;
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Clone hase been exist!" << HCPENDLOG;
            AssignDeviceInfo(deviceInfo, cloneName);
            return std::make_unique<DoradoNas>(deviceInfo, std::to_string(info.deviceId), readK8s);
        }
        ret = CreateCloneFromSnapShot(cloneName, fsId);
        if (ret != SUCCESS) {
            errorCode = ret;
            HCP_Log(ERR, DORADO_MODULE_NAME) << "create clone failed, errorCode: "
                                             << ret << HCPENDLOG;
            return nullptr;
        }
        AssignDeviceInfo(deviceInfo, cloneName);
        return std::make_unique<DoradoNas>(deviceInfo, fsId, readK8s);
    }

    int DoradoNasSnapshot::CreateCloneFromSnapShot(std::string cloneName, std::string &fsId) {
        HttpRequest req;
        DeviceDetails info;
        req.method = "POST";
        req.url = "filesystem";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["NAME"] = cloneName;
        jsonValue["ALLOCTYPE"] = 1;
        jsonValue["PARENTSNAPSHOTNAME"] = ResourceName;
        jsonValue["PARENTFILESYSTEMID"] = fileSystemId;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS || errorCode == DoradoErrorCode::FILESYSTEMALREADYEXIST)) {
            fsId = data["ID"].asString();
            HCP_Log(INFO, DORADO_MODULE_NAME) << "create clone success, file system id: " << fsId << HCPENDLOG;
            return SUCCESS;
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "create clone failed, errorCode: " << errorCode << HCPENDLOG;
        return errorCode;
    }

    void DoradoNasSnapshot::AssignDeviceInfo(ControlDeviceInfo &deviceInfo, std::string volumeName) {
        deviceInfo.deviceName = volumeName;
        deviceInfo.url = DoradoIP;
        deviceInfo.port = DoradoPort;
        deviceInfo.userName = DoradoUsername;
        deviceInfo.password = DoradoPassword;
        deviceInfo.poolId = DoradoPoolId;
    }

    int DoradoNasSnapshot::Delete() {
        std::string id;
        int ret = QuerySnapshot(ResourceName, id);
        if (ret == SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Query Snapshot Success. " << HCPENDLOG;
            ret = DeleteSnapshot(ResourceName, id);
            if (ret != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Delete Snapshot Failed, errorCode: " << ret << HCPENDLOG;
                return ret;
            }
        } else {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Snapshot Is Not Exist. " << HCPENDLOG;
        }

        return SUCCESS;
    }

    int DoradoNasSnapshot::Bind(HostInfo &host, const std::string &shareId) {
        return FAILED;
    }

    int DoradoNasSnapshot::UnBind(HostInfo host, const std::string &shareId) {
        return FAILED;
    }

    std::unique_ptr<ControlDevice> DoradoNasSnapshot::CreateSnapshot(std::string snapshotName, int &errorCode) {
        return nullptr;
    }
}
