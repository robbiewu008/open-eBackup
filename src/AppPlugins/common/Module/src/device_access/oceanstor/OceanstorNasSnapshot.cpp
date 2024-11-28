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
#include "device_access/oceanstor/OceanstorNasSnapshot.h"
#include "common/JsonUtils.h"
namespace Module {
    mp_int32 OceanstorNasSnapshot::Query(DeviceDetails &info)
    {
        std::string id;
        mp_int32 ret = QuerySnapshot(ResourceName, id);
        if (ret == SUCCESS) {
            info.deviceName = ResourceName;
            info.deviceId = std::stoi(fileSystemId);
            info.deviceUniquePath = "/" + ResourceName + "/";
            return SUCCESS;
        }
        return ret;
    }

    std::unique_ptr <ControlDevice> OceanstorNasSnapshot::CreateClone(std::string cloneName, int &errorCode)
    {
        mp_int32 id;
        DeviceDetails info;
        ControlDeviceInfo deviceInfo = {};
        std::string fsId;
        mp_int32 ret = QueryFileSystem(cloneName, info);
        if (ret == SUCCESS) {
            errorCode = ret;
            HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "Clone hase been exist!" << HCPENDLOG;
            AssignDeviceInfo(deviceInfo, cloneName);
            return std::make_unique<OceanstorNas>(deviceInfo, std::to_string(info.deviceId));
        }
        ret = CreateCloneFromSnapShot(cloneName, fsId);
        if (ret != SUCCESS) {
            errorCode = ret;
            auto clones = std::make_unique<OceanstorNas>(deviceInfo, cloneName);
            clones->SetErrorCode(GetErrorCode());
            clones->SetExtendInfo(GetExtendInfo());
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "create clone failed, errorCode: "
                                                << ret << " GetExtendInfo: "<< clones->GetExtendInfo() << HCPENDLOG;
            return clones;
        }
        AssignDeviceInfo(deviceInfo, cloneName);
        return std::make_unique<OceanstorNas>(deviceInfo, fsId);
    }

    mp_int32 OceanstorNasSnapshot::CreateCloneFromSnapShot(std::string cloneName, std::string &fsId)
    {
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
        jsonValue["PARENTSNAPSHOTID"] = fileSystemId + "@" + ResourceName;
        jsonValue["PARENTFILESYSTEMID"] = fileSystemId;
        if (!vstoreId.empty() && vstoreId != "0") {
            jsonValue["vstoreId"] = vstoreId;
        }
        req.body = jsonWriter.write(jsonValue);
        HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "PARENTSNAPSHOTNAME: " << ResourceName << " PARENTFILESYSTEMID: "
                                             << fileSystemId << " NAME: " << cloneName << HCPENDLOG;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS &&
            (errorCode == SUCCESS || errorCode == OceanstorErrorCode::FILESYSTEMALREADYEXIST)) {
            fsId = data["ID"].asString();
            HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "create clone success, file system id: " << fsId << HCPENDLOG;
            return SUCCESS;
        }
        HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "create clone failed, errorCode: " << errorCode << " errorDes: "
            << errorDes << "getExtendInfo: " <<  GetExtendInfo() << HCPENDLOG;
        return errorCode;
    }
}