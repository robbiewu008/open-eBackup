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
#include "device_access/dorado/DoradoBlockSnapshot.h"
#include "common/JsonUtils.h"
namespace Module {
    int DoradoBlockSnapshot::Delete() {
        if (ResourceName.length() > MAX_LENGTH) {
            ResourceName = ResourceName.substr(0, MAX_LENGTH);
        }
        int ret = QuerySnapshot(ResourceName, ResourceId, Wwn);
        if (ret != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query snapshot failed. " << HCPENDLOG;
            return FAILED;
        }
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start delete snapshot " << ResourceName << HCPENDLOG;
        HttpRequest req;
        req.method = "DELETE";
        req.url = "snapshot/" + std::to_string(ResourceId);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

    int DoradoBlockSnapshot::Query(DeviceDetails &info) {
        int status;
        const int forward = 2;
        unsigned long long size;
        unsigned long long usedSize;
        std::string Wwn;
        int ret = QueryLUN(ResourceName, ResourceId, Wwn, size, usedSize);
        if (ret != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query snapshot failed. " << HCPENDLOG;
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
        return ret;
    }

    int DoradoBlockSnapshot::QuerySnapshotEx(std::string SnapshotName, int &id, std::string &WWN) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query snapshot " << SnapshotName << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "SNAPSHOT?filter=NAME::" + SnapshotName;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data.size() > 0) {
            id = std::stoi(data[0]["ID"].asString());
            WWN = data[0]["WWN"].asString();
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

    std::unique_ptr<ControlDevice> DoradoBlockSnapshot::CreateClone(std::string volumeName, int &errorCode) {
        unsigned long long size;
        unsigned long long usedSize;
        int id;
        std::string wwn;
        std::string originalName;
        if (ResourceName.length() > MAX_LENGTH) {
            ResourceName = ResourceName.substr(0, MAX_LENGTH);
        }

        originalName = volumeName;
        if (volumeName.length() > MAX_LENGTH) {
            volumeName = volumeName.substr(0, MAX_LENGTH);
        }
        int ret = QuerySnapshotEx(ResourceName, ResourceId, wwn);
        if (ret != SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Snapshot " << ResourceName << " does not exist" << HCPENDLOG;
            return nullptr;
        }
        ControlDeviceInfo deviceInfo = {};
        std::string description;
        QueryLunDescription(volumeName, description);
        if (originalName == description) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "lun " << volumeName << " already exists." << HCPENDLOG;
            return nullptr;
        }
        HCP_Log(INFO, DORADO_MODULE_NAME)
                << "Start create clone " << volumeName << " from volume " << ResourceName << HCPENDLOG;
        int mpRet = QueryLUN(volumeName, id, wwn, size, usedSize);
        if (mpRet == SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "lun " << volumeName << " already exists." << HCPENDLOG;
            return nullptr;
        } else {
            int iRet;
            CreateCloneSendRequest(volumeName, originalName, iRet, errorCode);
            if (iRet == SUCCESS && errorCode == SUCCESS) {
                QueryLUN(volumeName, id, wwn, size, usedSize);
            } else {
                return nullptr;
            }
        }
        AssignDeviceInfo(deviceInfo, volumeName);
        return std::make_unique<DoradoBlock>(deviceInfo);
    }

    void DoradoBlockSnapshot::CreateCloneSendRequest(
            std::string volumeName, std::string originalName, int &iRet, int &errorCode) {
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["ID"] = ResourceId;
        jsonReq["NAME"] = volumeName;
        jsonReq["DESCRIPTION"] = originalName;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "snapshot/createCopy";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        iRet = SendRequest(req, data, errorDes, errorCode);
    }

    void DoradoBlockSnapshot::AssignDeviceInfo(ControlDeviceInfo &deviceInfo, std::string volumeName) {
        deviceInfo.deviceName = volumeName;
        deviceInfo.url = DoradoIP;
        deviceInfo.port = DoradoPort;
        deviceInfo.userName = DoradoUsername;
        deviceInfo.password = DoradoPassword;
        deviceInfo.poolId = DoradoPoolId;
    }

    int DoradoBlockSnapshot::ExtendSize(unsigned long long size) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "extend failed, snapshot not support." << HCPENDLOG;
        return FAILED;
    }

    int DoradoBlockSnapshot::Revert(std::string SnapshotName) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "extend failed, snapshot not support." << HCPENDLOG;
        return FAILED;
    }

    int DoradoBlockSnapshot::GetLunIDBySnapshot(int snapshotId, int &lunId) {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Start get snapshot " << snapshotId << " origin lunId. " << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "snapshot/" + std::to_string(lunId);
        Json::Value data;
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            lunId = std::stoi(data["SOURCELUNID"].asString());
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "SnapShot info: " << data["NAME"].asString() << " created from lun "
                                               << data["SOURCELUNNAME"].asString() << HCPENDLOG;
            return SUCCESS;
        } else {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "GetLunIDBySnapshot failed " << snapshotId << " failed! " << HCPENDLOG;
            return errorCode;
        }
    }

    int DoradoBlockSnapshot::CreateReplication(
            int snapshotId, int rLunId, std::string rDevId, int bandwidth, std::string &repId) {
        int lunId;
        int ret = GetLunIDBySnapshot(snapshotId, lunId);
        if (ret != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "CreateReplication failed " << snapshotId << " failed! " << HCPENDLOG;
            return FAILED;
        }
        return DoradoBlock::CreateReplication(lunId, rLunId, rDevId, bandwidth, repId);
    }

    int DoradoBlockSnapshot::ActiveReplication(std::string repId) {
        return DoradoBlock::ActiveReplication(repId);
    }

    int DoradoBlockSnapshot::QueryReplication(
            ReplicationPairInfo &replicationPairInfo) {
        return DoradoBlock::QueryReplication(replicationPairInfo);
    }

    int DoradoBlock::GetSnapShotInfo(const std::string &snapShotId, SnapshotInfo &info, std::string &errDes) {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Start get snapshot info: " << snapShotId << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "snapshot/" + snapShotId;
        Json::Value data;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data.size() > 0) {
            info.id = std::stoi(data[0]["ID"].asString());
            info.name = data[0]["NAME"].asString();
            info.wwn = data[0]["WWN"].asString();
            info.snapShotType = 1;
            info.userCapacity = std::stoi(data[0]["USERCAPACITY"].asString());
            info.userCapacity = info.userCapacity * CAPACITY_COEFFICIENT;
            info.consumedCapacity = std::stoi(data[0]["CONSUMEDCAPACITY"].asString());
            info.timeStamp = std::stoi(data[0]["TIMESTAMP"].asString());
            info.lunId = std::stoi(data[0]["SOURCELUNID"].asString());
            HCP_Log(DEBUG, DORADO_MODULE_NAME)
                    << "SnapShot info: " << DBG(info.name) << DBG(info.wwn) << DBG(info.lunId) << DBG(info.userCapacity)
                    << DBG(info.consumedCapacity) << DBG(info.timeStamp) << HCPENDLOG;
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

    int DoradoBlock::GetSnapShotInfoByID(DeviceDetails &info, std::string &errDes) {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Start get snapshot info: " << info.deviceId << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "snapshot/" + std::to_string(info.deviceId);
        Json::Value data;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data.size() > 0) {
            info.deviceName = data["NAME"].asString();
            info.deviceUniquePath = data["WWN"].asString();
            info.totalCapacity = std::stoi(data["USERCAPACITY"].asString());
            info.totalCapacity = info.totalCapacity * CAPACITY_COEFFICIENT;
            info.usedCapacity = std::stoi(data["CONSUMEDCAPACITY"].asString());
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "SnapShot info: " << DBG(info.deviceName)
                                               << DBG(info.deviceUniquePath)
                                               << DBG(info.totalCapacity) << DBG(info.usedCapacity) << HCPENDLOG;
            return SUCCESS;
        } else {
            return errorCode;
        }
    }

    int DoradoBlock::GetSnapShotInfoByName(DeviceDetails &info, std::string &errDes) {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Start get snapshot info: " << info.deviceName << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "SNAPSHOT?filter=NAME::" + info.deviceName;
        Json::Value data;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data.size() > 0) {
            info.deviceId = std::stoi(data[0]["ID"].asString());
            info.deviceUniquePath = data[0]["WWN"].asString();
            info.totalCapacity = std::stoi(data[0]["USERCAPACITY"].asString());
            info.totalCapacity = info.totalCapacity * CAPACITY_COEFFICIENT;
            info.usedCapacity = std::stoi(data[0]["CONSUMEDCAPACITY"].asString());
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "SnapShot info: " << DBG(info.deviceId) << DBG(info.deviceUniquePath)
                                               << DBG(info.totalCapacity) << DBG(info.usedCapacity) << HCPENDLOG;
            return SUCCESS;
        } else {
            return FAILED;
        }
    }
}
