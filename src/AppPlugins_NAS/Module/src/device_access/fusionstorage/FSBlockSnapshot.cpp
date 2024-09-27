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
#include "device_access/fusionstorage/FSBlockSnapshot.h"
#include "common/JsonUtils.h"
namespace Module {
    int FSBlockSnapshot::Query(DeviceDetails &info) {
        int status;
        const int forward = 2;
        int ret = QuerySnapshotEx(ResourceName, ResourceId, Wwn, status);
        if (ret != SUCCESS) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Query LUN failed. " << HCPENDLOG;
            return FAILED;
        }
        if (Wwn.find("0x") != std::string::npos) {
            Wwn = Wwn.substr(Wwn.find("0x") + forward);
        } else {
            Wwn = Wwn;
        }
        info.deviceId = ResourceId;
        info.deviceName = ResourceName;
        info.deviceUniquePath = Wwn;
        info.status = status;
        return ret;
    }

/*
create link clone volume with special snapshot
Date : 2020/03/03
out params:id -> link clone volume ID
          :WWN -> link clone volume WWN
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.create link clone volume with special snapshot
*/
    std::unique_ptr <ControlDevice> FSBlockSnapshot::CreateClone(std::string volumeName, int &errorCode) {
        unsigned long long size;
        unsigned long long usedSize;
        int id;
        std::string wwn;
        ControlDeviceInfo deviceInfo = {};
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                << "Start create clone from snap " << ResourceName << ",volume " << volumeName << HCPENDLOG;
        int mpRet = QueryLUN(volumeName, id, wwn, size, usedSize);
        if (mpRet == SUCCESS) {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "already exist lun " << volumeName << HCPENDLOG;
        } else {
            HttpRequest req;
            Json::Value jsonReq;
            jsonReq["name"] = volumeName;
            jsonReq["clone_snapshot_name"] = ResourceName;
            Json::FastWriter jsonWriter;
            req.method = "POST";
            req.url = "/api/v2/block_service/clone_volumes";
            req.body = jsonWriter.write(jsonReq);
            Json::Value data;
            std::string errorDes;
            int iRet = SendRequest(req, data, errorDes, errorCode);
            if (iRet == SUCCESS && errorCode == SUCCESS && data["id"].asInt() != 0) {
                QueryLUN(volumeName, id, wwn, size, usedSize);
            } else {
                return nullptr;
            }
        }
        deviceInfo.deviceName = volumeName;
        deviceInfo.url = FusionStorageIP;
        deviceInfo.port = FusionStoragePort;
        deviceInfo.userName = FusionStorageUsername;
        deviceInfo.password = FusionStoragePassword;
        deviceInfo.poolId = FusionStoragePoolId;
        return std::make_unique<FusionStorageBlock>(deviceInfo);
    }

/*
delete snapshot by name
Date : 2020/03/05
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.delete snapshot by name
*/
    int FSBlockSnapshot::Delete() {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start delete snapshot " << ResourceName << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["name"] = ResourceName;
        Json::FastWriter jsonWriter;
        req.method = "DELETE";
        req.url = "/api/v2/block_service/snapshots";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return SUCCESS;
        } else if (iRet == SUCCESS && errorCode == ERRORCODE_NOTEXIST) {
            HCP_Log(WARN, FUSION_STORAGE_MODULE_NAME) << "Snapshot is not exist, return success" << HCPENDLOG;
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

    int FSBlockSnapshot::ExtendSize(unsigned long long size) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "extend failed, snapshot not support." << HCPENDLOG;
        return FAILED;
    }

    int FSBlockSnapshot::Revert(std::string SnapshotName) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "extend failed, snapshot not support." << HCPENDLOG;
        return FAILED;
    }

    int FSBlockSnapshot::QuerySnapshotEx(std::string SnapshotName, int &id, std::string &WWN, int &status) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start query snapshot " << SnapshotName << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "/dsware/service/v1.3/snapshot/queryByName?snapshotName=" + SnapshotName;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            id = data["snapshot"]["snapshotId"].asInt();
            WWN = data["snapshot"]["wwn"].asString();
            status = data["snapshot"]["status"].asInt();
            return SUCCESS;
        } else {
            return FAILED;
        }
    }
}
