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
#include "device_access/fusionstorage/FusionStorageNasNFS.h"
#include "device_access/fusionstorage/FSNasSnapshot.h"

namespace Module {
    int FusionStorageNasNFS::Query(DeviceDetails &info)
    {
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Query filesystem failure! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }
        return QueryNFSShare(info, fileSystemId);
    }

    int FusionStorageNasNFS::QueryNFSShare(DeviceDetails &info, std::string fsId)
    {
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/v2/nas_protocol/nfs_share_list";
        std::string errorDes;
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        jsonValue["filter"]["fs_id"] = fsId;
        req.body = jsonWriter.write(jsonValue);
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data[0].size() > 0) {
            info.deviceId = atoi(data[0]["id"].asString().c_str());
            info.deviceUniquePath = data[0]["share_path"].asString();
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Nfs share hase exists!" << HCPENDLOG;
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int FusionStorageNasNFS::QueryDtreeWithShare()
    {
        INFOLOG("QueryDtreeWithShare %s", ResourceName.c_str());
        size_t pos = ResourceName.find("/");
        if (ResourceName.size() > pos + 1) {
            INFOLOG("It's a sharename with dtree");
            size_t pos2 = ResourceName.rfind("/");
            m_dtreeName = ResourceName.substr(pos2 + 1);
            INFOLOG("set dtree path: %s", m_dtreeName.c_str());
        }
        return SUCCESS;
    }

    std::unique_ptr<ControlDevice> FusionStorageNasNFS::CreateSnapshot(std::string snapshotName, int &errorCode)
    {
        std::string id;
        ControlDeviceInfo deviceInfo;
        deviceInfo.deviceName = snapshotName;
        deviceInfo.url = FusionStorageIP;
        deviceInfo.port = FusionStoragePort;
        deviceInfo.userName = FusionStorageUsername;
        deviceInfo.password = FusionStoragePassword;
        deviceInfo.poolId = FusionStoragePoolId;
        deviceInfo.dtreeId = dtreeId;
        deviceInfo.fileSystemId = fileSystemId;

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
        if (dtreeId != "") {
            jsonValue["dtree_id"] = dtreeId;
        }
        req.body = jsonWriter.write(jsonValue);
        INFOLOG("NFS CreateSnapshot check snap param, name %s, namespace_id %s, dtree_id: %s", snapshotName.c_str(),
            fileSystemId.c_str(), dtreeId.c_str());
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS ||
                                errorCode == FusionStorageErrorCode::NAMESPACE_SNAPSHOT_ALREADY_EXIST)) {
            return std::make_unique<FSNasSnapshot>(deviceInfo, fileSystemId, "/" + fileSystemName + "/");
        } else {
            ERRLOG("CreateSnapshot SendRequest fialed, errorCode: %d", errorCode);
            return nullptr;
        }
    }

    int FusionStorageNasNFS::DeleteSnapshot(std::string snapshotName)
    {
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
        if (dtreeId != "") {
            jsonValue["dtree_id"] = dtreeId;
        }
        req.body = jsonWriter.write(jsonValue);
        INFOLOG("NFS DeleteSnapshot check snap param, name %s, namespace_id %s, dtree_id: %s", snapshotName.c_str(),
            fileSystemId.c_str(), dtreeId.c_str());
        int errorCode;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS ||
                                errorCode == FusionStorageErrorCode::NAMESPACE_SNAPSHOT_NOTEXIST)) {
            return SUCCESS;
        }
        ERRLOG("NFS DeleteSnapshot SendRequest fialed, errorCode: %d", errorCode);
        return (errorCode == 0) ? FAILED : errorCode;
    }
}

