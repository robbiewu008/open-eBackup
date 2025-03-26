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
#include "device_access/netapp/NetAppNasNFS.h"
#include "define/Types.h"

namespace Module {
    NetAppNasNFS::~NetAppNasNFS()
    {
    }

    int NetAppNasNFS::Bind(HostInfo &host, const std::string &shareId)
    {
        return SUCCESS;
    }

    int NetAppNasNFS::UnBind(HostInfo host, const std::string &shareId)
    {
        return SUCCESS;
    }

    int NetAppNasNFS::QueryNFSShare(std::string volumeName, std::string &sharePath, int &errorCode)
    {
        if (volumeName.empty()) {
            errorCode = -NUMB_ONE;
            HCP_Log(ERR, NETAPP_MODULE) << "no volume to query NFS share" << HCPENDLOG;
            return FAILED;
        }
        // Check if any path exists for this volume
        int ret = SetNasPathFromVolume(volumeName, sharePath, errorCode);
        if (ret == SUCCESS && !sharePath.empty() && errorCode == NUMB_ZERO) {
            HCP_Log(INFO, NETAPP_MODULE) << "SharePath exists for volume"
                << volumeName << HCPENDLOG;
            return SUCCESS;
        } else {
            if (errorCode == -NUMB_ONE)
                return FAILED;
            if (sharePath.empty()) {
                errorCode = NUMB_ZERO;
                return FAILED;
            }
            return FAILED;
        }
    }

    int NetAppNasNFS::Query(DeviceDetails& info)
    {
        std::string resourceName = m_resourceName;
        if (resourceName.at(0) == '/') {
            HCP_Log(ERR, NETAPP_MODULE) << "VolumeName should not start with /" << HCPENDLOG;
            return FAILED;
        }
        int iRet;
        int errorCode = NUMB_ZERO;
        iRet = QueryVolume(resourceName, errorCode);
        if (iRet != SUCCESS && errorCode == NUMB_ZERO) {
            HCP_Log(ERR, NETAPP_MODULE) << "Query Volume not found" << HCPENDLOG;
            return iRet;
        } else if (errorCode == -NUMB_ONE) {
            return FAILED;
        }
        std::string sharePath = "";
        iRet = QueryNFSShare(resourceName, sharePath, errorCode);
        if (iRet == SUCCESS && errorCode == NUMB_ZERO) {
            return SUCCESS;
        }
        return FAILED;
    }

    int NetAppNasNFS::NFSShareAddClient(std::string name, int ID)
    {
        return SUCCESS;
    }

    int NetAppNasNFS::CreateShare()
    {
        std::string resourceName = m_resourceName;
        if (resourceName.at(0) == '/') {
            HCP_Log(ERR, NETAPP_MODULE) << "VolumeName should not start with /" << HCPENDLOG;
            return FAILED;
        }

        int errorCode = NUMB_ZERO;
        int iRet = QueryVolume(resourceName, errorCode);
        if (iRet != SUCCESS && errorCode == NUMB_ZERO) {
            HCP_Log(ERR, NETAPP_MODULE) << "Volume doesn't not exist: "
                << m_resourceName << HCPENDLOG;
            return FAILED;
        } else if (errorCode == -NUMB_ONE) {
            return FAILED;
        }
        std::string sharePath = "/" + m_resourceName;
        return CreateNFSShare(m_resourceName, sharePath);
    }

    int NetAppNasNFS::ValidateCreateNFSShareResponse(Json::Value &data, std::string &sharePath)
    {
        int iRet;
        for (Json::Value::ArrayIndex i = 0; i != data["jobs"].size(); i++) {
            if (!data["jobs"][i].isMember("uuid") || data["jobs"][i]["uuid"].empty()) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "response data format does not"
                    "have uuid or it is empty" << data["jobs"][i] << HCPENDLOG;
                continue;
            }
            if (!data["jobs"][i]["uuid"].empty()) {
                iRet = CheckJobStatus(data["jobs"][i]["uuid"].asString(), "CreateNFSShare");
                if (iRet != SUCCESS) {
                    return FAILED;
                } else {
                    HCP_Log(INFO, NETAPP_MODULE) << "Create NFS Share success: "
                        << sharePath << HCPENDLOG;
                    return SUCCESS;
                }
            }
        }
        HCP_Log(ERR, NETAPP_MODULE) << "Could not Create Share: " << sharePath << HCPENDLOG;
        return FAILED;
    }

    int NetAppNasNFS::CreateNFSShareCheck(const int& iRet, Json::Value& data, std::string& sharePath)
    {
        if (iRet == SUCCESS || iRet == HTTP_202) {
            if (data.size() > 0 && data.isMember("jobs") && data["jobs"].isArray()) {
                return ValidateCreateNFSShareResponse(data, sharePath);
            } else {
                if (!data.isMember("jobs") || !data["jobs"].isArray()) {
                    HCP_Log(ERR, NETAPP_MODULE) << "http rsp does not have jobs"
                        "or it's not array" << HCPENDLOG;
                }
                if (data.isMember("num_records") && data["num_records"].asInt() == 0) {
                    HCP_Log(ERR, NETAPP_MODULE) << "Create Share fail,"
                        " volume doesn't exist" << HCPENDLOG;
                }
            }
        }
        return FAILED;
    }

    int NetAppNasNFS::CreateNFSShare(std::string volumeName, std::string sharePath)
    {
        int errorCode = NUMB_ZERO;
        std::string path {};
        int iRet = QueryNFSShare(volumeName, path, errorCode);
        if (iRet == SUCCESS && errorCode == NUMB_ZERO)
            return SUCCESS;
        if (iRet == FAILED && errorCode == -NUMB_ONE)
            return FAILED;
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["nas"]["path"] = sharePath;
        Json::FastWriter jsonWriter;
        req.body = jsonWriter.write(jsonReq);
        req.method = "PATCH";
        req.url = "/api/storage/volumes/?name=" + volumeName;
        std::string errorDes;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        return CreateNFSShareCheck(iRet, data, sharePath);
    }

    int NetAppNasNFS::Create(unsigned long long size)
    {
        int ret = CreateVolume(size, SECURITY_TYPE::_UNIX);
        if (ret != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Create volume failure!!" << HCPENDLOG;
            return FAILED;
        }
        std::string sharePath = "/" + m_resourceName;
        return CreateNFSShare(m_resourceName, sharePath);
    }

    int NetAppNasNFS::DeleteNFSShare(std::string volumeName)
    {
        int errorCode = NUMB_ZERO;
        std::string sharePath = "";
        int iRet = QueryNFSShare(volumeName, sharePath, errorCode);
        if (iRet == FAILED && errorCode == -NUMB_ONE) {
            return FAILED;
        } else if (iRet == FAILED && errorCode == NUMB_ZERO) {
            return SUCCESS;
        } else if (iRet == SUCCESS && !sharePath.empty()) {
            iRet =  UnmountVolume(volumeName);
            if (iRet == SUCCESS) {
                HCP_Log(INFO, NETAPP_MODULE) << "Delete nfs share success!" << HCPENDLOG;
                return SUCCESS;
            } else {
                HCP_Log(ERR, NETAPP_MODULE) << "Delete nfs share failed!" << HCPENDLOG;
                return FAILED;
            }
        }
        return FAILED;
    }

    int NetAppNasNFS::Delete()
    {
        if (m_resourceName.at(0) == '/') {
            HCP_Log(ERR, NETAPP_MODULE) << "Volume Name starts with /" << HCPENDLOG;
            return FAILED;
        }
        int errorCode = NUMB_ZERO;
        int iRet = QueryVolume(m_resourceName, errorCode);
        if (iRet != SUCCESS && errorCode == NUMB_ZERO) {
            HCP_Log(WARN, NETAPP_MODULE) << "No volume to delete "
                << m_resourceName << HCPENDLOG;
            return SUCCESS;
        } else if (errorCode == -NUMB_ONE) {
            return FAILED;
        }

        iRet = DeleteNFSShare(m_resourceName);
        if (iRet == FAILED) {
            return FAILED;
        }

        if (!isDeleteParentSnapShot) {
            iRet = DeleteVolume(m_resourceName, false);
            if (iRet == FAILED) {
                return FAILED;
            }
        } else {
            std::string parentVolumeName {};
            std::string parentVolumeUuid {};
            std::string parentSnapshot {};
            iRet = QueryParentVolumeDetails(m_resourceName, parentVolumeName, parentVolumeUuid,
                                            parentSnapshot);
            if (iRet == FAILED) {
                HCP_Log(WARN, NETAPP_MODULE) << "Query parent Info not success" << HCPENDLOG;
            }

            iRet = DeleteVolume(m_resourceName, false);
            if (iRet == FAILED) {
                return FAILED;
            }
            // Delete Parent Snapshot only after deleting current current volume
            iRet = DeleteParentSnapshot(parentVolumeName, parentVolumeUuid, parentSnapshot);
            if (iRet == FAILED) {
                HCP_Log(WARN, NETAPP_MODULE) << "Could not Delete Parent Snapshot" << HCPENDLOG;
                return FAILED;
            }
        }
        HCP_Log(INFO, NETAPP_MODULE) << "Delete nfs share and volume success!" << HCPENDLOG;
        return SUCCESS;
    }

    std::unique_ptr<ControlDevice> NetAppNasNFS::CreateClone(std::string cloneName, int &errorCode)
    {
        if (m_resourceName.at(0) == '/') {
            HCP_Log(ERR, NETAPP_MODULE) << "Volume Name starts with /" << HCPENDLOG;
            return nullptr;
        }
        // query source volume
        int ret = QueryVolume(m_resourceName, errorCode);
        if (ret == FAILED && errorCode == NUMB_ZERO) {
            HCP_Log(ERR, NETAPP_MODULE) << "Source volume does not not exist!" << HCPENDLOG;
            return nullptr;
        } else if (errorCode == -NUMB_ONE) {
            return nullptr;
        }

        ret = CreateCloneVolume(cloneName, errorCode);
        if (ret != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Create Clone Volume failed, errorCode: "
                << errorCode << HCPENDLOG;
            return nullptr;
        }
        std::string clonePath = "/" + cloneName;

        ret = CreateNFSShare(cloneName, clonePath);
        if (ret != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Create clone share failed!" << HCPENDLOG;
            return nullptr;
        }

        ControlDeviceInfo deviceInfo = {};
        AssignDeviceInfo(deviceInfo, cloneName);
        std::unique_ptr<NetAppNasNFS> cloneFileSystemObj =  std::make_unique<NetAppNasNFS>(deviceInfo);
        if (cloneFileSystemObj  == nullptr) {
            HCP_Log(ERR, NETAPP_MODULE) <<"Create clone filesystem failed!" << HCPENDLOG;
            return nullptr;
        }
        return cloneFileSystemObj;
    }

    int NetAppNasNFS::QueryNFSShareClient(const std::string shareId, std::vector<std::string> &iPList)
    {
        return SUCCESS;
    }

    int NetAppNasNFS::DeleteNFSShareClient(const std::vector<std::string> &iPList, const std::string shareId)
    {
        return SUCCESS;
    }

    int NetAppNasNFS::DeleteNFSShareClient(std::string shareClientId)
    {
        return SUCCESS;
    }

    int NetAppNasNFS::DeleteSnapshot(std::string SnapshotName)
    {
        if (QueryVolUuidByVolNameAndSvmName(m_vserverName, m_resourceName) != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Query SvmUuid By SvmName Failed" << HCPENDLOG;
            return FAILED;
        }
        return NetAppNas::DeleteSnapshot(SnapshotName);
    }

    int NetAppNasNFS::QueryVolUuidByVolNameAndSvmName(const std::string &svmName, const std::string &volName)
    {
        if (CheckSvmDetails() != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "No SVM found to query volume" << HCPENDLOG;
            return FAILED;
        }

        HttpRequest req;
        req.method = "GET";
        req.url = "/api/storage/volumes?fields=uuid,name,svm&name=" + volName + "&svm=" + svmName;
        std::string errorDes;
        int errorCode = NUMB_ZERO;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS) {
            if (data.size() > 0 && data.isMember("records") &&
                (data["records"].isArray() && !data["records"].empty())) {
                return ValidateQueryVolumeResponse(data, volName);
            } else {
                if (data["records"].isArray() && data["records"].empty() &&
                    data["num_records"].asInt() == 0) {
                    HCP_Log(ERR, NETAPP_MODULE)<<"Given Volume Doesn't exist" << HCPENDLOG;
                }
                HCP_Log(ERR, NETAPP_MODULE) << "No data" << HCPENDLOG;
                return FAILED;
            }
        } else {
            HCP_Log(ERR, NETAPP_MODULE) << "HTTP FAILED" << HCPENDLOG;
            return FAILED;
        }
        return FAILED;
    }
}