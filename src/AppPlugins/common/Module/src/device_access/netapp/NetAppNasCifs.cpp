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
#include "device_access/netapp/NetAppNasCifs.h"


namespace Module {
    NetAppNasCIFS::~NetAppNasCIFS()
    {
    }

    int NetAppNasCIFS::Bind(HostInfo &host, const std::string &shareId)
    {
        HCP_Log(INFO, NETAPP_MODULE) << "do not need to bind!" << HCPENDLOG;
        return SUCCESS;
    }

    int NetAppNasCIFS::UnBind(HostInfo host, const std::string &shareId)
    {
        HCP_Log(INFO, NETAPP_MODULE) << "do not need to unbind!" << HCPENDLOG;
        return SUCCESS;
    }

    int NetAppNasCIFS::CreateShare()
    {
        int errorCode = NUMB_ZERO;
        int ret = QueryVolume(m_resourceName, errorCode);
        if (ret != SUCCESS && errorCode == NUMB_ZERO) {
            HCP_Log(ERR, NETAPP_MODULE) << "Volume doesnot not exist:"
                << m_resourceName << HCPENDLOG;
            return FAILED;
        } else if (errorCode == -NUMB_ONE) {
            return FAILED;
        }

        std::string nasSharePath = "/" + m_resourceName;
        ret = CreateNasShare(m_resourceName, nasSharePath);
        if (ret != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Create NAS share failed, can't create"
                " CIFS share also!" << HCPENDLOG;
            return FAILED;
        }

        return CreateCifsShare(m_resourceName);
    }

    int NetAppNasCIFS::ValidateCreateNasShareRsp(Json::Value &data, std::string sharePath)
    {
        int iRet;
        for (Json::Value::ArrayIndex i = 0; i != data["jobs"].size(); i++) {
            if (!data["jobs"][i].isMember("uuid") || data["jobs"][i]["uuid"].empty()) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "response data format does not"
                    "have uuid or it is empty" << data["jobs"][i] << HCPENDLOG;
                continue;
            }
            if (!data["jobs"][i]["uuid"].empty()) {
                iRet = CheckJobStatus(data["jobs"][i]["uuid"].asString(), "Create Nas Share");
                if (iRet != SUCCESS) {
                    return FAILED;
                } else {
                    HCP_Log(INFO, NETAPP_MODULE) << "Create NasShare success:"
                        << sharePath << HCPENDLOG;
                    return SUCCESS;
                }
            }
        }
        HCP_Log(ERR, NETAPP_MODULE) << "Could not Create Nas Share: " << sharePath << HCPENDLOG;
        return FAILED;
    }

    int NetAppNasCIFS::CreateNasShare(std::string volumeName, std::string sharePath)
    {
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["nas"]["path"] = sharePath;
        Json::FastWriter jsonWriter;
        req.body = jsonWriter.write(jsonReq);
        req.method = "PATCH";
        req.url = "/api/storage/volumes/?name=" + volumeName;
        std::string errorDes;
        Json::Value data;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS || iRet == HTTP_202) {
            if (data.size() > 0 && data.isMember("jobs") && data["jobs"].isArray()) {
                return ValidateCreateNasShareRsp(data, sharePath);
            } else {
                if (!data.isMember("jobs") || !data["jobs"].isArray()) {
                    HCP_Log(ERR, NETAPP_MODULE) << "response does not have jobs field"
                        "or it is not an array" << HCPENDLOG;
                }
                if (data.isMember("num_records") && data["jobs"].asInt() == 0)
                    HCP_Log(ERR, NETAPP_MODULE) <<" Create Nas share fail,"
                        " volume doesn't exist" << HCPENDLOG;
                return FAILED;
            }
        } else {
            return FAILED;
        }
    }

    int NetAppNasCIFS::CreateCifsShare(std::string volumeName)
    {
        int errorCode = NUMB_ZERO;
        std::string path {};
        int iRet = QueryCifsShare(volumeName, path, errorCode);
        if (iRet == SUCCESS && errorCode == NUMB_ZERO)
            return SUCCESS;
        if (iRet == FAILED && errorCode == -NUMB_ONE)
            return FAILED;
        std::string tempPath = "";
        iRet = SetNasPathFromVolume(volumeName, tempPath, errorCode);
        if (errorCode == -NUMB_ONE) {
            return FAILED;
        }
        if (iRet == FAILED || tempPath.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "Create Cifs share failed because no NAS path"
                "exists for " << volumeName << HCPENDLOG;
            return FAILED;
        }
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["home_directory"] = "false";
        jsonReq["name"] = volumeName; // cifs share name is given same as volumeName
        jsonReq["path"] = tempPath;
        jsonReq["encryption"] = (m_shareParam.encryption == true) ? "true" : "false";
        jsonReq["svm"]["name"] = m_vserverName;
        jsonReq["svm"]["uuid"] = m_vserverUuid;
        jsonReq["acls"][0]["type"] = "windows";
        jsonReq["acls"][0]["permission"] = "full_control";
        jsonReq["acls"][0]["user_or_group"] = m_userOrGroup;
        Json::FastWriter jsonWriter;
        req.body = jsonWriter.write(jsonReq);
        req.method = "POST";
        req.url = "/api/protocols/cifs/shares/";
        std::string errorDes;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS || iRet == HTTP_202) {
            if (data.isObject() && data.size() == 0) {
                HCP_Log(INFO, NETAPP_MODULE) << "Create Cifs share success for: "
                    << volumeName << HCPENDLOG;
                return SUCCESS;
            } else {
                HCP_Log(ERR, NETAPP_MODULE) << "Create Cifs share failed for: "
                    << volumeName << HCPENDLOG;
                return FAILED;
            }
        } else
            return FAILED;
    }

    int NetAppNasCIFS::Create(unsigned long long size)
    {
        int ret = CreateVolume(size, SECURITY_TYPE::_NTFS);
        if (ret != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Create volume failure!!" << HCPENDLOG;
            return FAILED;
        }
        return CreateCifsShare(m_resourceName);
    }

    std::unique_ptr<ControlDevice> NetAppNasCIFS::CreateClone(std::string cloneName, int &errorCode)
    {
        if (m_resourceName.at(0) == '/') {
            HCP_Log(ERR, NETAPP_MODULE) << "Volume Name starts with /" << HCPENDLOG;
            return nullptr;
        }
        // query source volume
        int ret = QueryVolume(m_resourceName, errorCode);
        if (ret == FAILED && errorCode == NUMB_ZERO) {
            HCP_Log(ERR, NETAPP_MODULE) << "Source volume doesn't not exist" << HCPENDLOG;
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

        ret = CreateNasShare(cloneName, clonePath);
        if (ret != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Create clone nfs share failed!" << HCPENDLOG;
            return nullptr;
        }

        ret = CreateCifsShare(cloneName);
        if (ret != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Create clone cifs share failed!" << HCPENDLOG;
            return nullptr;
        }

        ControlDeviceInfo deviceInfo = {};
        AssignDeviceInfo(deviceInfo, cloneName);
        return std::make_unique<NetAppNasCIFS>(deviceInfo);
    }

    int NetAppNasCIFS::ValidateQueryCifsShareResponse(Json::Value &data, std::string volumeName,
                                                      std::string &sharePath, int& errorCode)
    {
        for (Json::Value::ArrayIndex i = 0; i != data["records"].size(); i++) {
            if (!data["records"][i].isMember("name") || !data["records"][i].isMember("path") ||
                !data["records"][i].isMember("volume") || !data["records"][i].isMember("svm") ||
                !data["records"][i]["volume"].isMember("name")) {
                continue;
            }
            if (!data["records"][i]["name"].empty() && !data["records"][i]["path"].empty() &&
                !data["records"][i]["svm"]["name"].empty() &&
                !data["records"][i]["volume"]["name"].empty() &&
                data["records"][i]["volume"]["name"].asString() == volumeName &&
                data["records"][i]["svm"]["name"].asString() == m_vserverName &&
                data["records"][i]["svm"]["uuid"].asString() == m_vserverUuid) {
                sharePath = data["records"][i]["path"].asString();
                errorCode = NUMB_ZERO;
                HCP_Log(INFO, NETAPP_MODULE) << "Cifs share: " << sharePath
                    << " exists for volumeName: " << volumeName << HCPENDLOG;
                return SUCCESS;
            }
        }
        HCP_Log(ERR, NETAPP_MODULE) << "No Cifs share found for "<<volumeName<<HCPENDLOG;
        sharePath = "";
        errorCode = NUMB_ZERO;
        return FAILED;
    }

    int NetAppNasCIFS::QueryCifsShare(std::string volumeName, std::string &sharePath, int& errorCode)
    {
        int iRet;
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/protocols/cifs/shares?fields=name,path,svm,volume&volume=" + volumeName;
        std::string errorDes;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS || iRet == HTTP_202) {
            if (data.size() > 0 && data.isMember("records") &&
                (data["records"].isArray() && !data["records"].empty())) {
                return ValidateQueryCifsShareResponse(data, volumeName, sharePath, errorCode);
            } else {
                if (data["records"].isArray() && data["records"].empty())
                    HCP_Log(ERR, NETAPP_MODULE) << "No Cifs records found for volume:"
                        << volumeName << HCPENDLOG;
                errorCode = NUMB_ZERO;
                return FAILED;
            }
        } else {
            return iRet;
        }
    }

    int NetAppNasCIFS::Query(DeviceDetails& info)
    {
        // info is empty
        std::string resourceName = m_resourceName;
        if (resourceName.at(0) == '/') {
            HCP_Log(ERR, NETAPP_MODULE) << "VolumeName starts with /" << HCPENDLOG;
            return FAILED;
        }
        int iRet;
        int errorCode = NUMB_ZERO;
        iRet = QueryVolume(resourceName, errorCode);
        if (iRet != SUCCESS && errorCode == NUMB_ZERO) {
            HCP_Log(ERR, NETAPP_MODULE) << "Volume Not found" << HCPENDLOG;
            return iRet;
        } else if (errorCode == -NUMB_ONE) {
            return FAILED;
        }

        std::string sharePath = "";
        iRet = QueryCifsShare(resourceName, sharePath, errorCode);
        if (iRet == SUCCESS && errorCode == NUMB_ZERO && !sharePath.empty()) {
            return SUCCESS;
        }
        return FAILED;
    }

    int NetAppNasCIFS::DeleteCIFSShare(std::string volumeName)
    {
        int errorCode = NUMB_ZERO;
        std::string sharePath = "";
        int iRet = QueryCifsShare(volumeName, sharePath, errorCode);
        if (iRet == FAILED && errorCode == NUMB_ZERO) {
            return SUCCESS;
        }
        if (iRet == FAILED && errorCode == -NUMB_ONE) {
            return FAILED;
        }
        if (sharePath.empty())
            return FAILED;

        HttpRequest req;
        req.method = "DELETE";
        req.url = "/api/protocols/cifs/shares/" + m_vserverUuid + "/?path=" + sharePath;
        std::string errorDes;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        // HTTP_202 represents the request is accepted for processing, but not processed yet
        if (iRet == SUCCESS || iRet == HTTP_202) {
            if (data.size() > 0 && data.isMember("num_records")) {
                if (data["num_records"].asInt() != 0) {
                    HCP_Log(INFO, NETAPP_MODULE) << "Delete Cifs shares success for "
                        << volumeName << HCPENDLOG;
                    return SUCCESS;
                } else {
                    HCP_Log(ERR, NETAPP_MODULE) << "Delete Cifs share failed for "
                        << volumeName << HCPENDLOG;
                    return FAILED;
                }
            } else {
                HCP_Log(ERR, NETAPP_MODULE) << "Delete Cifs share failed for: "
                    << volumeName << HCPENDLOG;
                return FAILED;
            }
        } else {
            return FAILED;
        }
    }

    int NetAppNasCIFS::Delete()
    {
        if (m_resourceName.at(0) == '/') {
            HCP_Log(ERR, NETAPP_MODULE) << "VolumeName starts with /" << HCPENDLOG;
            return FAILED;
        }
        int errorCode = NUMB_ZERO;
        int iRet = QueryVolume(m_resourceName, errorCode);
        if (iRet != SUCCESS && errorCode == NUMB_ZERO) {
            HCP_Log(ERR, NETAPP_MODULE) << "No volume to delete "<<m_resourceName<<HCPENDLOG;
            return SUCCESS;
        } else if (errorCode == -NUMB_ONE) {
            return FAILED;
        }

        iRet = DeleteCIFSShare(m_resourceName);
        if (iRet == FAILED) {
            return FAILED;
        }

        iRet =  UnmountVolume(m_resourceName);
        if (iRet != SUCCESS) {
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
                HCP_Log(ERR, NETAPP_MODULE) << "Query parent Info not success" << HCPENDLOG;
            }

            iRet = DeleteVolume(m_resourceName, false);
            if (iRet == FAILED) {
                return FAILED;
            }
            // Delete Parent Snapshot only after deleting current current volume
            iRet = DeleteParentSnapshot(parentVolumeName, parentVolumeUuid, parentSnapshot);
            if (iRet == FAILED) {
                HCP_Log(ERR, NETAPP_MODULE) << "Could not Delete Parent Snapshot" << HCPENDLOG;
                return FAILED;
            }
        }
        HCP_Log(INFO, NETAPP_MODULE) <<"Delete cifs share and volume success!" << HCPENDLOG;
        return SUCCESS;
    }


    int NetAppNasCIFS::DeleteWindowsUser(std::string userName)
    {
        return SUCCESS;
    }

    int NetAppNasCIFS::CreateWindowUser(std::string userName, std::string password)
    {
        return SUCCESS;
    }

    int NetAppNasCIFS::CIFSShareAddClient(std::string name, int ID)
    {
        return SUCCESS;
    }
}