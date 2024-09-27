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
#include "device_access/fusionstorage/FusionStorageNas.h"

namespace Module {
    namespace {
        const auto MODULE = "FusionStorageNas";
        const int NUM_3 = 3;
    }

    int FusionStorageNas::Query(DeviceDetails &info) {
        return QueryFileSystem(info);
    }

    int FusionStorageNas::QueryFileSystem(DeviceDetails &info) {
        int ret = QueryFileSystem(ResourceName, info);
        if (ret != SUCCESS) {
            return ret;
        }
        fileSystemId = std::to_string(info.deviceId);
        return SUCCESS;
    }

    int FusionStorageNas::QueryFileSystem(std::string fileName, DeviceDetails &info) {
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/v2/converged_service/namespaces?name=" + fileName;
        int errorCode;
        std::string errorDes;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            if (data.size() > 0) {
                std::string id = data["id"].asString();
                info.deviceId = std::atoi(id.c_str());
                info.deviceName = ResourceName;
                info.Compress = (data["enable_compress"].asString() == "true") ? true : false;
                info.Dedup = (data["enable_dedup"].asString() == "true") ? true : false;
                std::istringstream capa(data["space_hard_quota"].asString());
                capa >> info.totalCapacity;
                return iRet;
            } else {
                return FAILED;
            }
        } else {
            return iRet;
        }
    }

    int FusionStorageNas::Delete() {
        return FAILED;
    }

    std::unique_ptr <ControlDevice> FusionStorageNas::CreateSnapshot(std::string snapshotName, int &errorCode) {
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
            return std::make_unique<FSNasSnapshot>(deviceInfo, fileSystemId, "/" + ResourceName + "/");
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
            return std::make_unique<FSNasSnapshot>(deviceInfo, fileSystemId, "/" + ResourceName + "/");
        } else {
            return nullptr;
        }
    }

    int FusionStorageNas::QuerySnapshot(std::string snapshotName, std::string &id) {
        int iRet;
        DeviceDetails info;
        if (fileSystemId.empty()) {
            iRet = QueryFileSystem(info);
            if (iRet != SUCCESS) {
                return FAILED;
            }
        }
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/v2/converged_service/snapshots?name=" + snapshotName + "&namespace_id=" + fileSystemId;
        std::string errorDes;
        int errorCode;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        HCP_Log(INFO, MODULE) << "Return data size " << data.size() << HCPENDLOG;
        if (iRet == SUCCESS && data.size() != 0 && errorCode == SUCCESS) {
            id = data["id"].asString();
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int FusionStorageNas::DeleteSnapshot(std::string snapshotName) {
        DeviceDetails info;
        if (fileSystemId.empty()) {
            if (QueryFileSystem(info) != SUCCESS) {
                return FAILED;
            }
        }
        HttpRequest req;
        req.method = "DELETE";
        req.url = "/api/v2/converged_service/snapshots";
        std::string errorDes;
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        jsonValue["name"] = snapshotName;
        jsonValue["namespace_id"] = fileSystemId;
        req.body = jsonWriter.write(jsonValue);
        int errorCode;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS ||
                                errorCode == FusionStorageErrorCode::NAMESPACE_SNAPSHOT_NOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int FusionStorageNas::QuerySnapshotRollBackStatus(const std::string &snapshotName, std::string &snapshotId,
                                                      std::string &rollbackStatus, std::string &endTime) {
        HCP_Log(INFO, MODULE) << "Query snapshot rollback status by fileSystemId" << HCPENDLOG;

        DeviceDetails info;
        if (fileSystemId.empty()) {
            if (QueryFileSystem(info) != SUCCESS) {
                return FAILED;
            }
        }

        HttpRequest req;
        req.method = "GET";
        req.url = "/api/v2/converged_service/snapshots?name=" + snapshotName + "&namespace_id=" + fileSystemId;
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && data.size() != 0 && errorCode == SUCCESS) {
            snapshotId = data["id"].asString();
            rollbackStatus = data["status"].asString();
            endTime = data["rollback_end_time"].asString();
            return SUCCESS;
        }
        
        return errorCode;
    }

    int FusionStorageNas::RollBackBySnapShotName(const std::string &snapshotName)
    {
        HCP_Log(INFO, MODULE) << "Rollback snapshot by snapShotId" << HCPENDLOG;

        DeviceDetails info;
        if (fileSystemId.empty()) {
            if (QueryFileSystem(info) != SUCCESS) {
                return FAILED;
            }
        }

        HttpRequest req;
        req.method = "PUT";
        req.url = "/api/v2/converged_service/rollback_snapshot";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        jsonValue["name"] = snapshotName;
        jsonValue["namespace_id"] = fileSystemId;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return SUCCESS;
        }
    }
}

