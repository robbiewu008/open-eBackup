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

namespace Module {
    int FusionStorageNasNFS::Query(DeviceDetails &info) {
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Query filesystem failure! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }
        return QueryNFSShare(info, fileSystemId);
    }

    int FusionStorageNasNFS::QueryNFSShare(DeviceDetails &info, std::string fsId) {
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
}

