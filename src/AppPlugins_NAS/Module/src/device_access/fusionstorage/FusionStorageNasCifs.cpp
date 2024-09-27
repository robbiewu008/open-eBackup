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
#include "common/JsonUtils.h"
#include "system/System.hpp"
#include "device_access/fusionstorage/FSNasSnapshot.h"

#include "device_access/fusionstorage/FusionStorageNasCifs.h"

namespace Module {
    namespace {
        constexpr int HUNDRED = 100;
    } // namespace

    int FusionStorageNasCIFS::Query(DeviceDetails &info) {
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Query filesystem failure! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }
        return QueryCIFSShare(info, fileSystemId);
    }

    int FusionStorageNasCIFS::QueryFileSystem(DeviceDetails &info) {
        if (fileSystemName.empty() == true) {
            if (GetFsNameFromShareName() != SUCCESS) {
                HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Get FS name from Sharename Failed" << HCPENDLOG;
                return FAILED;
            }
        }
        int ret = FusionStorageNas::QueryFileSystem(fileSystemName, info);
        if (ret != SUCCESS) {
            return ret;
        }
        fileSystemId = std::to_string(info.deviceId);
        return SUCCESS;
    }

    int FusionStorageNasCIFS::QueryCIFSShare(DeviceDetails &info, std::string fsId) {
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/v2/nas_protocol/cifs_share_list";
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        jsonValue["range"]["offset"] = 0;
        jsonValue["range"]["limit"] = HUNDRED;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data[0].size() > 0) {
            info.deviceId = atoi(data[0]["id"].asString().c_str());
            info.deviceUniquePath = data[0]["name"].asString();
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Cifs share has exists!" << HCPENDLOG;
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int FusionStorageNasCIFS::GetFsNameFromShareName() {
        HttpRequest req;
        req.method = "GET";
        std::string errorDes;
        Json::Value data;
        int iRet;
        int errorCode = FAILED;
        Json::Value jsonValue;
        req.url = "/api/v2/file_service/cifs_share_list";
        jsonValue["range"]["offset"] = 0;
        jsonValue["range"]["limit"] = HUNDRED;
        jsonValue["filter"][0]["name"] = ResourceName;
        iRet = SendRequest(req, data, errorDes, errorCode);
        std::string shareName = ResourceName;
        if (iRet == SUCCESS && data.size() > 0 && errorCode == SUCCESS) {
            for (auto dataTraverse : data) {
                if (shareName.compare(dataTraverse["name"].asString()) == 0) {
                    std::string path = dataTraverse["share_path"].asString();
                    fileSystemName = path.substr(1, path.size() - FUSIONSTORE_NUMBER_TWO);
                    return SUCCESS;
                }
            }
        }
        return FAILED;
    }

    std::unique_ptr <ControlDevice> FusionStorageNasCIFS::CreateSnapshot(std::string snapshotName, int &errorCode) {
        std::string id;
        ControlDeviceInfo deviceInfo;
        deviceInfo.deviceName = snapshotName;
        deviceInfo.url = FusionStorageIP;
        deviceInfo.port = FusionStoragePort;
        deviceInfo.userName = FusionStorageUsername;
        deviceInfo.password = FusionStoragePassword;
        deviceInfo.poolId = FusionStoragePoolId;

        int ret = QuerySnapshot(snapshotName, id);
        if (ret == SUCCESS) {
            HCP_Log(DEBUG, "FUSION_MODULE_NAME") << "snapshotName: " << snapshotName << HCPENDLOG;
            return std::make_unique<FSNasSnapshot>(deviceInfo, fileSystemId, "/" + fileSystemName + "/");
        }

        HttpRequest req;
        req.method = "POST";
        req.url = "/api/v2/converged_service/snapshots";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        jsonValue["name"] = snapshotName;
        jsonValue["namespace_id"] = fileSystemId;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS ||
                                errorCode == FusionStorageErrorCode::NAMESPACE_SNAPSHOT_ALREADY_EXIST)) {
            return std::make_unique<FSNasSnapshot>(deviceInfo, fileSystemId, "/" + fileSystemName + "/");
        } else {
            return nullptr;
        }
    }
}

