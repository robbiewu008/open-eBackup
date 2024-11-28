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
#include "device_access/netapp/NetAppNasSnapshot.h"

namespace Module {
    int NetAppNasSnapshot::Create(unsigned long long size)
    {
        return FAILED;
    }

    int NetAppNasSnapshot::Query(DeviceDetails& info)
    {
        int ret = SetVolumeDetails(this->snapVolumeName, this->snapVolumeUuid);
        if (ret != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Could not set Volume Details" << HCPENDLOG;
            return FAILED;
        }

        int errorCode = NUMB_ZERO;
        ret = QuerySnapshot(this->snapshotName, errorCode);
        if (ret == SUCCESS && errorCode == NUMB_ZERO) {
            return SUCCESS;
        }
        return FAILED;
    }

    int NetAppNasSnapshot::Delete()
    {
        int ret = SetVolumeDetails(this->snapVolumeName, this->snapVolumeUuid);
        if (ret != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Could not set Volume Details" << HCPENDLOG;
            return FAILED;
        }

        int errorCode = NUMB_ZERO;
        ret = QuerySnapshot(this->snapshotName, errorCode);
        if (ret == SUCCESS && errorCode == NUMB_ZERO) {
            return DeleteSnapshot(this->snapshotName);
        } else if (ret != SUCCESS && errorCode == NUMB_ZERO) {
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

    int NetAppNasSnapshot::ValidateCloneFromSnapshotResponse(Json::Value &data)
    {
        int iRet;
        if (!data["job"].isMember("uuid") || data["job"]["uuid"].empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "response data format does not"
                "have job uuid or it is empty" << data << HCPENDLOG;
            return FAILED;
        }
        if (!data["job"]["uuid"].empty()) {
            // check if the job is processed successfully
            iRet = CheckJobStatus(data["job"]["uuid"].asString(), "CreateCloneFromSnapshot");
            if (iRet != SUCCESS) {
                return FAILED;
            } else {
                HCP_Log(INFO, NETAPP_MODULE) << "Create Clone from snapshot: "
                    << this->snapshotName << " success" << HCPENDLOG;
                return SUCCESS;
            }
        } else {
            return FAILED;
        }
    }

    int NetAppNasSnapshot::CreateCloneFromSnapshot(std::string cloneName, int &errorCode)
    {
        if (this->snapshotName.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "No snapshot to create clone from, "
                "caller can call SetResourceName() to set explicitly" << HCPENDLOG;
            return FAILED;
        }

        int iRet;
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["name"] = cloneName;
        jsonReq["svm"]["name"] = m_vserverName;
        jsonReq["clone"]["is_flexclone"] = true;
        jsonReq["clone"]["parent_snapshot"]["name"] = this->snapshotName;
        jsonReq["clone"]["parent_svm"]["name"] = m_vserverName;
        jsonReq["clone"]["parent_svm"]["uuid"] = m_vserverUuid;
        jsonReq["clone"]["parent_volume"]["name"] = this->snapVolumeName;
        jsonReq["clone"]["parent_volume"]["uuid"] = this->snapVolumeUuid;
        jsonReq["guarantee"]["type"] = "none";
        Json::FastWriter jsonWriter;
        req.body = jsonWriter.write(jsonReq);
        req.method = "POST";
        req.url = "/api/storage/volumes";

        std::string errorDes;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS || iRet == HTTP_202) {
            if (data.size() > 0 && data.isMember("job")) {
                return ValidateCloneFromSnapshotResponse(data);
            } else {
                if (!data.isMember("job"))
                    HCP_Log(ERR, NETAPP_MODULE) << "rsp does not have job field" << HCPENDLOG;
                return FAILED;
            }
        } else {
            HCP_Log(ERR, NETAPP_MODULE) << "errorDes: " << errorDes << HCPENDLOG;
            return FAILED;
        }
    }

    std::unique_ptr<ControlDevice> NetAppNasSnapshot::CreateClone(std::string cloneName, int &errorCode)
    {
        ControlDeviceInfo deviceInfo = {};
        int ret = QueryVolume(cloneName, errorCode);
        if (ret == SUCCESS && errorCode == NUMB_ZERO) {
            HCP_Log(ERR, NETAPP_MODULE) << "Clone volume name already exists!" << HCPENDLOG;
            AssignDeviceInfo(deviceInfo, cloneName);
            return std::make_unique<NetAppNas>(deviceInfo, false);
        } else if (errorCode == -NUMB_ONE) {
            return nullptr;
        }

        ret = CreateCloneFromSnapshot(cloneName, errorCode);
        if (ret != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "create clone failed, errorCode: "
                << errorCode << HCPENDLOG;
            return nullptr;
        }
        AssignDeviceInfo(deviceInfo, cloneName);
        return std::make_unique<NetAppNas>(deviceInfo, false);
    }

    int NetAppNasSnapshot::Bind(HostInfo &host, const std::string &shareId)
    {
        return FAILED;
    }

    int NetAppNasSnapshot::UnBind(HostInfo host, const std::string &shareId)
    {
        return FAILED;
    }

    std::unique_ptr<ControlDevice> NetAppNasSnapshot::CreateSnapshot(std::string snapshotName, int &errorCode)
    {
        return nullptr;
    }
}