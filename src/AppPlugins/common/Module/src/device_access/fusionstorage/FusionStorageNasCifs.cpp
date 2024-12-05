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

    int FusionStorageNasCIFS::Query(DeviceDetails &info)
    {
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Query filesystem failure! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }
        return QueryCIFSShare(info, fileSystemId);
    }

    int FusionStorageNasCIFS::QueryFileSystem(DeviceDetails &info)
    {
        if (fileSystemName.empty()) {
            if (GetFsNameFromShareName() != SUCCESS) {
                HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Get FS name from Sharename Failed" << HCPENDLOG;
                return FAILED;
            }
            if (QueryDtreeId() != SUCCESS) {
                HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Get dtree id from FS Failed" << HCPENDLOG;
                return FAILED;
            }
        }
        INFOLOG("QueryFileSystem fileSystemName:%s", fileSystemName.c_str());
        int ret = FusionStorageNas::QueryFileSystem(fileSystemName, info);
        if (ret != SUCCESS) {
            return ret;
        }
        fileSystemId = std::to_string(info.deviceId);
        return SUCCESS;
    }
    int FusionStorageNasCIFS::QueryCIFSShare(DeviceDetails &info, std::string fsId)
    {
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

    int FusionStorageNasCIFS::GetFsNameFromShareName()
    {
        HttpRequest req;
        req.method = "GET";
        std::string errorDes;
        Json::Value data;
        int iRet;
        int errorCode = FAILED;
        Json::Value jsonValue;
        int offset = 0;
        Json::FastWriter jsonWriter;
        req.url = "/api/v2/file_service/cifs_share_list";
        jsonValue["range"]["limit"] = HUNDRED;
        jsonValue["filter"][0]["name"] = ResourceName;
        int count = 0;
        if (GetCifsShareCount(count) != SUCCESS) {
            return FAILED;
        }
        while (offset < count) {
            jsonValue["range"]["offset"] = offset;
            req.body = jsonWriter.write(jsonValue);
            iRet = SendRequest(req, data, errorDes, errorCode);
            if (iRet != SUCCESS || data.size() <= 0 || errorCode != SUCCESS) {
                return FAILED;
            }
            if (GetFsNameFromShareListResponse(data) == SUCCESS) {
                return SUCCESS;
            }
            offset += HUNDRED;
        }
        INFOLOG("Cannot find fs, ShareName: %s", ResourceName.c_str());
        return FAILED;
    }
 
    int FusionStorageNasCIFS::GetFsNameFromShareListResponse(Json::Value &data)
    {
        std::string shareName = ResourceName;
        for (auto dataTraverse : data) {
            if (shareName.compare(dataTraverse["name"].asString()) != 0) {
                DBGLOG("GetFsNameFromShareName path:%s", dataTraverse["share_path"].asString().c_str());
                continue;
            }
            std::string path = dataTraverse["share_path"].asString();
            INFOLOG("GetFsNameFromShareName path:%s", path.c_str());
            // path示例：
            // dtree: /fileSystemName/dir
            // 非dtree: /fileSystemName/
            std::size_t pos = path.find('/', 1);
            if (pos == std::string::npos) {
                fileSystemName = path.substr(1, pos);
                INFOLOG("set filesystem name: %s", fileSystemName.c_str());
                return SUCCESS;
            }
            fileSystemName = path.substr(1, pos - 1);
            INFOLOG("set filesystem name: %s", fileSystemName.c_str());
            if (path.size() > pos + 1) {
                m_dtreePath = path.substr(pos + 1);
            }
            INFOLOG("set m_dtreePath name: %s", m_dtreePath.c_str());
            return SUCCESS;
        }
        INFOLOG("Cannot find fs, ShareName: %s", ResourceName.c_str());
        return FAILED;
    }
 
    int FusionStorageNasCIFS::GetCifsShareCount(int &count)
    {
        HttpRequest req;
        req.method = "GET";
        std::string errorDes;
        Json::Value data;
        int iRet;
        int errorCode = FAILED;
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        req.url = "/api/v2/file_service/cifs_share_count";
        jsonValue["filter"][0]["name"] = ResourceName;
        req.body = jsonWriter.write(jsonValue);
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            ERRLOG("Get cifs share count failed! ret: %d, error code: %d", iRet, errorCode);
            return FAILED;
        }
        if (!data.isMember("count")) {
            ERRLOG("Count is not response member");
            return FAILED;
        }
        count = data["count"].asInt();
        INFOLOG("Cifs share count is %d", count);
        return SUCCESS;
    }

    std::unique_ptr<ControlDevice> FusionStorageNasCIFS::CreateSnapshot(std::string snapshotName, int &errorCode)
    {
        std::string id;
        ControlDeviceInfo deviceInfo;
        deviceInfo.deviceName = snapshotName;
        deviceInfo.url = FusionStorageIP;
        deviceInfo.port = FusionStoragePort;
        deviceInfo.userName = FusionStorageUsername;
        deviceInfo.password = FusionStoragePassword;
        deviceInfo.poolId = FusionStoragePoolId;
        deviceInfo.sharePath = sharePath;
        deviceInfo.fileSystemId = fileSystemId;
 
        if (QueryDtreeIdByPath(deviceInfo.sharePath) != SUCCESS) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Get dtree id from FS Failed"
                                                     << "sharePath: " << sharePath << HCPENDLOG;
            return nullptr;
        }

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
        INFOLOG("CIFS CreateSnapshot check snap param, name %s, namespace_id %s, dtree_id: %s", snapshotName.c_str(),
            fileSystemId.c_str(), dtreeId.c_str());
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS ||
                                errorCode == FusionStorageErrorCode::NAMESPACE_SNAPSHOT_ALREADY_EXIST)) {
            return std::make_unique<FSNasSnapshot>(deviceInfo, fileSystemId, "/" + fileSystemName + "/");
        } else {
            return nullptr;
        }
    }
 
    int FusionStorageNasCIFS::DeleteSnapshot(std::string snapshotName)
    {
        DeviceDetails info;
        if (fileSystemId.empty()) {
            if (QueryFileSystem(info) != SUCCESS) {
                return FAILED;
            }
        }
           
        if (QueryDtreeIdByPath(sharePath) != SUCCESS) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Get dtree id from FS Failed"
                                                     << "sharePath: " << sharePath << HCPENDLOG;
            return FAILED;
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
        INFOLOG("CIFS DeleteSnapshot check snap param, name %s, namespace_id %s, dtree_id: %s", snapshotName.c_str(),
            fileSystemId.c_str(), dtreeId.c_str());
        int errorCode;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS ||
                                errorCode == FusionStorageErrorCode::NAMESPACE_SNAPSHOT_NOTEXIST)) {
            return SUCCESS;
        }
        ERRLOG("CIFS DeleteSnapshot SendRequest fialed, errorCode: %d", errorCode);
        return (errorCode == 0) ? FAILED : errorCode;
    }
 
    int FusionStorageNasCIFS::QueryDtreeId()
    {
        INFOLOG("Enter QueryDtreeId: %s, %s", m_dtreePath.c_str(), fileSystemName.c_str());
        if (m_dtreePath == "") {
            DBGLOG("QueryDtreeId this share is not in a dtree!");
            return SUCCESS;
        }
        size_t pos = m_dtreePath.find('/');
        if (pos != std::string::npos) {
            m_dtreePath = m_dtreePath.substr(0, pos);
            INFOLOG("set m_dtreePath name: %s", m_dtreePath.c_str());
        }
        HttpRequest req;
        req.method = "GET";
        std::string errorDes;
        Json::Value data;
        int iRet;
        int errorCode = FAILED;
        Json::Value jsonValue;
        req.url = "/api/v2/file_service/dtrees?file_system_name=" + fileSystemName;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS || data.size() <= 0 || errorCode != SUCCESS) {
            return FAILED;
        }
        for (auto dataTraverse : data) {
            if (m_dtreePath != dataTraverse["name"].asString()) {
                continue;
            }
            dtreeId = dataTraverse["id"].asString();
            return SUCCESS;
        }
        INFOLOG("Cannot find dtree: %s", m_dtreePath.c_str());
        return FAILED;
    }

    int FusionStorageNasCIFS::QueryDtreeIdByPath(const std::string& sharePath)
    {
        size_t pos = sharePath.find_last_of("/");
        if (pos != std::string::npos) {
            m_dtreePath = sharePath.substr(pos + 1);
        } else {
            dtreeId = "";
            ERRLOG("Not found m_dtreePath");
            return FAILED;
        }
 
        if (m_dtreePath == "") {
            return SUCCESS;
        }
        HttpRequest req;
        req.method = "GET";
        std::string errorDes;
        Json::Value data;
        int iRet;
        int errorCode = FAILED;
        Json::Value jsonValue;
        req.url = "/api/v2/file_service/dtrees?file_system_id=" + fileSystemId;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS || data.size() <= 0 || errorCode != SUCCESS) {
            ERRLOG("QueryDtreeIdByPath SendRequest fialed, errorCode: %d", errorCode);
            return FAILED;
        }
        for (auto dataTraverse : data) {
            if (m_dtreePath != dataTraverse["dir_name"].asString()) {
                continue;
            }
            dtreeId = dataTraverse["id"].asString();
            return SUCCESS;
        }
        ERRLOG("Cannot find dtree: %s", m_dtreePath.c_str());
        return FAILED;
    }
}

