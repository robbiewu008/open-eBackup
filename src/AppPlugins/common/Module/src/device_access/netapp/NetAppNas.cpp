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
#include "log/Log.h"
#include "define/Types.h"
#include "system/System.hpp"
#include "device_access/oceanstor/DataMoverUtility.h"
#include "config_reader/ConfigIniReaderImpl.h"
#include "device_access/netapp/NetAppNas.h"

namespace Module {

    int NetAppNas::TestDeviceConnection()
    {
        HCP_Logger_noid(INFO, NETAPP_MODULE) << "Testing Connection..." << HCPENDLOG;
        int iRet;
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/cluster/nodes?fields=name,serial_number,state,management_interfaces";
        std::string errorDes;
        int errorCode;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS) {
            HCP_Logger_noid(ERR, NETAPP_MODULE) << "Connection failed" << HCPENDLOG;
            return FAILED;
        }
        HCP_Logger_noid(INFO, NETAPP_MODULE) << "Connection success" << HCPENDLOG;
        return SUCCESS;
    }

    void NetAppNas::Init(bool querySvm)
    {
        int ret = SUCCESS;
        if (querySvm) {
            ret = Base64Encryption(NetAppUsername, NetAppPassword);
            if (ret != SUCCESS || m_encryptedKey == "") {
                HCP_Log(ERR, NETAPP_MODULE) << "Encryption will be retried again, "
                    "before sending REST request" << HCPENDLOG;
                return;
            }
            ret = SetSvmNameFromServiceIp(NetAppServiceIp);
            if (ret != SUCCESS) {
                HCP_Log(ERR, NETAPP_MODULE) << "Setting SVM name/uuid failed, "
                    << " All subsequent REST calls will be prevented"<<HCPENDLOG;
                return;
            }
        }
        if (m_resourceName.at(0) == '/') {
            ret = SetVolumeNameFromShareName(m_resourceName, m_volumeName, m_volumeUuid);
            if (ret != SUCCESS) {
                ret = SetVolumeNameFromPath(m_resourceName, m_volumeName, m_volumeUuid);
                if (ret != SUCCESS) {
                    HCP_Log(ERR, NETAPP_MODULE) << "Setting volume name/uuid failed, "
                        << " All REST calls related to snapshot will be prevented" << HCPENDLOG;
                    return;
                }
            }
        }
    }

    int NetAppNas::SetSvmDetails(std::string svmName, std::string svmUuid)
    {
        this->m_vserverName = svmName;
        this->m_vserverUuid = svmUuid;
        return SUCCESS;
    }

    int NetAppNas::SetVolumeDetails(std::string volumeName, std::string volumeUuid)
    {
        this->m_volumeName = volumeName;
        this->m_volumeUuid = volumeUuid;
        return SUCCESS;
    }

    int NetAppNas::CheckSvmDetails()
    {
        if (m_vserverName.empty() || m_vserverUuid.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "No SVM found: " << HCPENDLOG;
            return FAILED;
        } else {
            return SUCCESS;
        }
    }

    int NetAppNas::CheckVolumeDetails()
    {
        if (m_volumeName.empty() || m_volumeUuid.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "No Volume found: " << HCPENDLOG;
            return FAILED;
        } else {
            return SUCCESS;
        }
    }

    int NetAppNas::ValidateSetSvmResponse(Json::Value &data, std::string serviceIp)
    {
        for (Json::Value::ArrayIndex i = 0; i != data["records"].size(); i++) {
            if (!data["records"][i].isMember("svm") || !data["records"][i].isMember("ip")) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "data format does not have ip/svm "
                    << data["records"][i] << HCPENDLOG;
                continue;
            }
            if (data["records"][i]["ip"].isMember("address") && !data["records"][i]["ip"]["address"].empty() &&
                data["records"][i]["svm"].isMember("name") && !data["records"][i]["svm"]["name"].empty() &&
                data["records"][i]["svm"].isMember("uuid") && !data["records"][i]["svm"]["uuid"].empty() &&
                data["records"][i]["ip"]["address"].asString() == serviceIp) {
                m_vserverName = data["records"][i]["svm"]["name"].asString();
                m_vserverUuid = data["records"][i]["svm"]["uuid"].asString();
                HCP_Log(INFO, NETAPP_MODULE) << "++++++Got SVM: " << m_vserverName
                    << " uuid: " << m_vserverUuid << " for serviceIP: " << serviceIp << HCPENDLOG;
                return SUCCESS;
            }
        }
        if (m_vserverName.empty() || m_vserverUuid.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "No SVM found for svcIP: " << serviceIp << HCPENDLOG;
            return FAILED;
        }
    }

    // sets vserverName and vserverUuid by querying a given service IP
    int NetAppNas::SetSvmNameFromServiceIp(std::string serviceIp)
    {
        if (serviceIp.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "Service IP is empty" << HCPENDLOG;
            return FAILED;
        }

        int iRet;
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/network/ip/interfaces?fields=ip.address%2Csvm";
        std::string errorDes;
        int errorCode;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS) {
            if (data.size() > 0 && data.isMember("records") &&
                (data["records"].isArray() && !data["records"].empty())) {
                return ValidateSetSvmResponse(data, serviceIp);
            } else {
                if (data["records"].isArray() && data["records"].empty()) {
                    HCP_Log(ERR, NETAPP_MODULE) << "No records in response" << HCPENDLOG;
                }
                return FAILED;
            }
        } else {
            return FAILED;
        }
    }

    int NetAppNas::ValidateSetVolumeResponseDataCheck(const Json::Value::ArrayIndex &i, const Json::Value &data,
                                                        std::string sharePath, std::string &volumeName,
                                                        std::string &volumeUuid)
    {
        if (!data["records"][i]["name"].empty() &&
            (data["records"][i]["svm"].isMember("name") && !data["records"][i]["svm"]["uuid"].empty()) &&
            (data["records"][i]["volume"].isMember("name") && !data["records"][i]["volume"]["uuid"].empty()) &&
            (data["records"][i]["svm"]["name"].asString() == m_vserverName && data["records"][i]["svm"]["uuid"].asString() == m_vserverUuid) &&
            data["records"][i]["name"].asString() == sharePath.substr(1)) {
            volumeName = data["records"][i]["volume"]["name"].asString();
            volumeUuid = data["records"][i]["volume"]["uuid"].asString();
            HCP_Log(DEBUG, NETAPP_MODULE) << "++++++Got volumeName: " << volumeName
                << " volumeUuid: " << volumeUuid << HCPENDLOG;
            return SUCCESS;
        }
        return FAILED;
    }

    int NetAppNas::ValidateSetVolumeResponse(Json::Value &data, std::string sharePath,
                                                std::string &volumeName, std::string &volumeUuid)
    {
        for (Json::Value::ArrayIndex i = 0; i != data["records"].size(); i++) {
            if (!data["records"][i].isMember("volume") || !data["records"][i].isMember("name") ||
                !data["records"][i].isMember("svm")) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "data format does not have"
                    " svm/volumeName/uuid/nas details " << data["records"][i] << HCPENDLOG;
                continue;
            }
            if (SUCCESS == ValidateSetVolumeResponseDataCheck(i, data, sharePath, volumeName, volumeUuid)) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "ValidateSetVolumeResponseDataCheck success " << HCPENDLOG;
                return SUCCESS;
            }
        }
        if (volumeName.empty() || volumeUuid.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) <<"No volume found for path: "<<sharePath<<HCPENDLOG;
            return FAILED;
        }
    }

    int NetAppNas::ValidateSetVolumeResponseDataCheck4Nfs(const Json::Value::ArrayIndex &i, const Json::Value &data,
                                                        std::string sharePath, std::string &volumeName,
                                                        std::string &volumeUuid)
    {
        if (!data["records"][i]["name"].empty() && !data["records"][i]["uuid"].empty() &&
            (data["records"][i]["nas"].isMember("path") && !data["records"][i]["nas"]["path"].empty()) &&
            (data["records"][i]["svm"].isMember("name") && !data["records"][i]["svm"]["name"].empty()) &&
            (data["records"][i]["svm"].isMember("uuid") && !data["records"][i]["svm"]["uuid"].empty()) &&
            data["records"][i]["nas"]["path"].asString() == sharePath &&
            data["records"][i]["svm"]["name"].asString() == m_vserverName &&
            data["records"][i]["svm"]["uuid"].asString() == m_vserverUuid) {
            volumeName = data["records"][i]["name"].asString();
            volumeUuid = data["records"][i]["uuid"].asString();
            HCP_Log(DEBUG, NETAPP_MODULE) << "++++++Got volumeName: " << volumeName
                << " volumeUuid: " << volumeUuid << HCPENDLOG;
            return SUCCESS;
        }
        return FAILED;
    }

    int NetAppNas::ValidateSetVolumeResponse4Nfs(Json::Value &data, std::string sharePath,
                                                std::string &volumeName, std::string &volumeUuid)
    {
        for (Json::Value::ArrayIndex i = 0; i != data["records"].size(); i++) {
            if (!data["records"][i].isMember("nas") || !data["records"][i].isMember("name") ||
                !data["records"][i].isMember("uuid") || !data["records"][i].isMember("svm")) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "data format does not have"
                    " svm/volumeName/uuid/nas details " << data["records"][i] << HCPENDLOG;
                continue;
            }
            if (SUCCESS == ValidateSetVolumeResponseDataCheck4Nfs(i, data, sharePath, volumeName, volumeUuid)) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "ValidateSetVolumeResponseDataCheck success " << HCPENDLOG;
                return SUCCESS;
            }
        }
        if (volumeName.empty() || volumeUuid.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) <<"No volume found for path: "<<sharePath<<HCPENDLOG;
            return FAILED;
        }
    }


    int NetAppNas::ValidateSharePath(std::string sharePath, std::string &volumeName,
                                        std::string &volumeUuid)
    {
        if (sharePath.at(0) != '/') {
            volumeName = "";
            volumeUuid = "";
            HCP_Log(ERR, NETAPP_MODULE) << "sharePath should start with / " << HCPENDLOG;
            return FAILED;
        } else {
            return SUCCESS;
        }
    }

    // sets volumeName and volumeUuid by quering a given share path
    int NetAppNas::SetVolumeNameFromPath(std::string sharePath, std::string &volumeName,
                                            std::string &volumeUuid)
    {
        if (CheckSvmDetails() != SUCCESS)
            return FAILED;
        if (ValidateSharePath(sharePath, volumeName, volumeUuid) != SUCCESS)
            return FAILED;
        int iRet;
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/storage/volumes?return_timeout=120&order_by=name&fields=uuid%2Cname%2Cnas%2Csvm";
        std::string errorDes;
        int errorCode;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS) {
            if (data.size() > 0 && data.isMember("records") &&
                (data["records"].isArray() && !data["records"].empty())) {
                return ValidateSetVolumeResponse4Nfs(data, sharePath, volumeName, volumeUuid);
            } else {
                if (data["records"].isArray() && data["records"].empty()) {
                    HCP_Log(ERR, NETAPP_MODULE) << "No records in response" << HCPENDLOG;
                }
                return FAILED;
            }
        } else {
            return FAILED;
        }
    }

    int NetAppNas::SetVolumeNameFromShareName(std::string shareName, std::string &volumeName,
                                            std::string &volumeUuid)
    {
        HCP_Log(DEBUG, NETAPP_MODULE) << "enter SetVolumeNameFromShareName " << HCPENDLOG;
        if (ValidateSharePath(shareName, volumeName, volumeUuid) != SUCCESS) {
            return FAILED;
        }
        // 处理共享名称中的特殊字符
        std::string shareName4Url = DataMoverUtility::UrlEncode(shareName.substr(1));
        int iRet;
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/protocols/cifs/shares?return_timeout=120&max_records=40&fields=name%2Cpath%2Cvolume&name=" + shareName4Url;
        std::string errorDes;
        int errorCode;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS) {
            if (data.size() > 0 && data.isMember("records") &&
                (data["records"].isArray() && !data["records"].empty())) {
                return ValidateSetVolumeResponse(data, shareName, volumeName, volumeUuid);
            } else {
                if (data["records"].isArray() && data["records"].empty()) {
                    HCP_Log(ERR, NETAPP_MODULE) << "No records in response" << HCPENDLOG;
                }
                return FAILED;
            }
        } else {
            return FAILED;
        }
    }

    int NetAppNas::ValidateSetNasPathResponse(Json::Value &data, std::string volumeName,
                                                std::string &sharePath, int &errorCode)
    {
        for (Json::Value::ArrayIndex i = 0; i != data["records"].size(); i++) {
            if (!data["records"][i].isMember("name") || !data["records"][i].isMember("uuid") ||
                !data["records"][i].isMember("nas") || !data["records"][i]["nas"].isMember("path")) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "response data format does not have"
                    "volumeName/uuid/details " << data["records"][i] << HCPENDLOG;
                continue;
            }
            if (!data["records"][i]["name"].empty() && !data["records"][i]["uuid"].empty() &&
                !data["records"][i]["nas"]["path"].empty() &&
                data["records"][i]["name"].asString() == volumeName) {
                sharePath = data["records"][i]["nas"]["path"].asString();
                HCP_Log(INFO, NETAPP_MODULE) << "++++++ share path found: " << sharePath
                    << " for: " << volumeName << HCPENDLOG;
                errorCode = NUMB_ZERO;
                return SUCCESS;
            }
        }
        HCP_Log(ERR, NETAPP_MODULE) << "VolumeName: "<<volumeName<<" has no path " << HCPENDLOG;
        sharePath = "";
        errorCode = NUMB_ZERO;
        return FAILED;
    }

    // sets sharePath by querying a given volume
    int NetAppNas::SetNasPathFromVolume(std::string volumeName, std::string &sharePath, int &errorCode)
    {
        if (volumeName.at(0) == '/' || volumeName.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "VolumeName should be proper"
                << volumeName << HCPENDLOG;
            errorCode = -NUMB_ONE;
            return FAILED;
        }
        if (CheckSvmDetails() != SUCCESS) {
            errorCode = -NUMB_ONE;
            return FAILED;
        }
        int iRet;
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/storage/volumes?fields=uuid,name,svm,nas&name=" + volumeName + "&svm=" + m_vserverName;
        std::string errorDes;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS) {
            if (data.size() > 0 && data.isMember("records") &&
                (data["records"].isArray() && !data["records"].empty())) {
                return ValidateSetNasPathResponse(data, volumeName, sharePath, errorCode);
            } else {
                if (data["records"].isArray() && data["records"].empty()) {
                    HCP_Log(ERR, NETAPP_MODULE) << "No records in response" << HCPENDLOG;
                    errorCode = NUMB_ZERO;
                }
                return FAILED;
            }
        } else {
            errorCode = -NUMB_ONE;
            return FAILED;
        }
    }

    int NetAppNas::ValidateUnmountVolumeResponse(Json::Value &data, std::string volumeName)
    {
        int iRet;
        for (Json::Value::ArrayIndex i = 0; i != data["jobs"].size(); i++) {
            if (!data["jobs"][i].isMember("uuid") || data["jobs"][i]["uuid"].empty()) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "response data format does not"
                    "have uuid or it is empty" << data["jobs"][i] << HCPENDLOG;
                continue;
            }
            if (!data["jobs"][i]["uuid"].empty()) {
                iRet = CheckJobStatus(data["jobs"][i]["uuid"].asString(), "UnmountVolume");
                if (iRet != SUCCESS) {
                    return FAILED;
                } else {
                    HCP_Log(INFO, NETAPP_MODULE) << "Unmount Volume: "
                        << volumeName << " success" << HCPENDLOG;
                    return SUCCESS;
                }
            }
        }
        HCP_Log(ERR, NETAPP_MODULE) << "Couldn't unmount Vol: "<<volumeName<<HCPENDLOG;
        return FAILED;
    }

    int NetAppNas::UnmountVolume(std::string volumeName)
    {
        int iRet;
        if (volumeName.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "No volume to unmount" << HCPENDLOG;
            return SUCCESS;
        }

        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["nas.path"] = "";
        Json::FastWriter jsonWriter;
        req.body = jsonWriter.write(jsonReq);
        req.method = "PATCH";
        req.url = "/api/storage/volumes/?name=" + volumeName;
        std::string errorDes;
        int errorCode;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        // HTTP_202 represents the request is accepted for processing, but does not mean processed yet
        if (iRet == SUCCESS || iRet == HTTP_202) {
            if (data.size() > 0 && data.isMember("jobs") && data["jobs"].isArray()) {
                return ValidateUnmountVolumeResponse(data, volumeName);
            } else {
                if (!data.isMember("jobs") || !data["jobs"].isArray()) {
                    HCP_Log(ERR, NETAPP_MODULE) << "response does not have jobs field"
                        "or it is not an array" << HCPENDLOG;
                }
                if (data.isMember("num_records") && data["num_records"].asInt() == 0) {
                    HCP_Log(ERR, NETAPP_MODULE) <<"Unmount failed, volume doesn't exist: "
                        << HCPENDLOG;
                }
                return FAILED;
            }
        } else {
            return FAILED;
        }
    }

    int NetAppNas::ValidateDeleteVolumeResponse(Json::Value &data, std::string volumeName)
    {
        int iRet;
        for (Json::Value::ArrayIndex i = 0; i != data["jobs"].size(); i++) {
            if (!data["jobs"][i].isMember("uuid") || data["jobs"][i]["uuid"].empty()) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "response data format does not"
                    "have uuid or it is empty" << data["jobs"][i] << HCPENDLOG;
                continue;
            }
            if (!data["jobs"][i]["uuid"].empty()) {
                iRet = CheckJobStatus(data["jobs"][i]["uuid"].asString(), "DeleteVolume");
                if (iRet != SUCCESS) {
                    return FAILED;
                } else {
                    HCP_Log(INFO, NETAPP_MODULE) << "Delete Volume: "
                        << volumeName << " success" << HCPENDLOG;
                    return SUCCESS;
                }
            }
        }
        HCP_Log(ERR, NETAPP_MODULE) << "Couldn't delete Vol: "<<volumeName<<HCPENDLOG;
        return FAILED;
    }

    int NetAppNas::DeleteVolumeCheck(const int& iRet, Json::Value& data, const std::string& volumeName)
    {
        if (iRet == SUCCESS || iRet == HTTP_202) {
            if (data.size() > 0 && data.isMember("jobs") && data["jobs"].isArray()) {
                return ValidateDeleteVolumeResponse(data, volumeName);
            } else {
                if (!data.isMember("jobs") || !data["jobs"].isArray()) {
                    HCP_Log(ERR, NETAPP_MODULE) << "response does not have jobs field"
                        "or it is not an array" << HCPENDLOG;
                }
                if (data.isMember("num_records") && data["num_records"].asInt() == 0) {
                    HCP_Log(ERR, NETAPP_MODULE) << "DeleteVolume failed, doesn't exist"
                        << HCPENDLOG;
                }
            }
        }
        return FAILED;
    }

    int NetAppNas::DeleteVolume(std::string volumeName, bool query)
    {
        int iRet;
        int errorCode = NUMB_ZERO;
        if (volumeName.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "volumeName is empty" << HCPENDLOG;
            return FAILED;
        }
        if (query) {
            iRet = QueryVolume(volumeName, errorCode);
            if (iRet != SUCCESS && errorCode == NUMB_ZERO) {
                HCP_Log(ERR, NETAPP_MODULE) << "No volume to delete " << volumeName << HCPENDLOG;
                return SUCCESS;
            } else if (errorCode == -NUMB_ONE) {
                return FAILED;
            }
        }
        HttpRequest req;
        req.method = "DELETE";
        req.url = "/api/storage/volumes/?name=" + volumeName + "&svm=" + m_vserverName;
        std::string errorDes;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        return DeleteVolumeCheck(iRet, data, volumeName);
    }

    int NetAppNas::Delete()
    {
        return DeleteVolume(m_resourceName);
    }

    int NetAppNas::ValidateQueryVolumeResponse(Json::Value &data, std::string volumeName,
                                                    int &errorCode)
    {
        for (Json::Value::ArrayIndex i = 0; i != data["records"].size(); i++) {
            if (!data["records"][i].isMember("name") || !data["records"][i].isMember("uuid")) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "response data format does not have"
                    "volumeName/uuid/details "  << data["records"][i] << HCPENDLOG;
                continue;
            }
            if (!data["records"][i]["name"].empty() && !data["records"][i]["uuid"].empty() &&
                data["records"][i]["name"].asString() == volumeName &&
                (data["records"][i]["svm"].isMember("name") && !data["records"][i]["svm"]["name"].empty()) &&
                data["records"][i]["svm"]["name"].asString() == m_vserverName &&
                data["records"][i]["svm"]["uuid"].asString() == m_vserverUuid) {
                HCP_Log(INFO, NETAPP_MODULE) << "++++++volumeName: " << volumeName
                    << " exists, volumeUuid: " << data["records"][i]["uuid"].asString() << HCPENDLOG;
                errorCode = NUMB_ZERO;
                return SUCCESS;
            }
        }
        HCP_Log(ERR, NETAPP_MODULE)<<"No volume found for: "<<volumeName<<HCPENDLOG;
        errorCode = -NUMB_ONE;
        return FAILED;
    }

    int NetAppNas::QueryVolume(std::string volumeName, int &errorCode)
    {
        if (volumeName.at(0) == '/' || volumeName.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "VolumeName is improper"<<volumeName<<HCPENDLOG;
            errorCode = -NUMB_ONE;
            return FAILED;
        }
        if (CheckSvmDetails() != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "No SVM found to query volume" << HCPENDLOG;
            errorCode = -NUMB_ONE;
            return FAILED;
        }
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/storage/volumes?fields=uuid,name,svm&name=" + volumeName + "&svm=" + m_vserverName;
        std::string errorDes;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS) {
            if (data.size() > 0 && data.isMember("records") &&
                (data["records"].isArray() && !data["records"].empty())) {
                return ValidateQueryVolumeResponse(data, volumeName, errorCode);
            } else {
                if (data["records"].isArray() && data["records"].empty() &&
                    data["num_records"].asInt() == 0) {
                    HCP_Log(ERR, NETAPP_MODULE)<<"No volume found for: "<<volumeName<<HCPENDLOG;
                    errorCode = NUMB_ZERO;
                }
                return FAILED;
            }
        } else {
            errorCode = -NUMB_ONE;
            return FAILED;
        }
    }

    int NetAppNas::Query(DeviceDetails& info)
    {
        int iRet;
        int errorCode = NUMB_ZERO;
        iRet = QueryVolume(m_resourceName, errorCode);
        if (iRet == SUCCESS && errorCode == NUMB_ZERO) {
            HCP_Log(INFO, NETAPP_MODULE) << "Volume exists: " << m_resourceName<<HCPENDLOG;
            return SUCCESS;
        } else if (iRet == FAILED && errorCode == NUMB_ZERO) {
            HCP_Log(ERR, NETAPP_MODULE) << "Volume doesn't exist: "<<m_resourceName<<HCPENDLOG;
            return FAILED;
        } else {
            return FAILED;
        }
    }

    int NetAppNas::ValidateGetAggregateResponse(Json::Value &data, std::string &aggregate)
    {
        for (Json::Value::ArrayIndex i = 0; i != data["records"].size(); i++) {
            if (!data["records"][i].isMember("name") || !data["records"][i].isMember("uuid") ||
                !data["records"][i].isMember("aggregates")) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "data format does not have"
                    "name/uuid/aggregates " << data["records"][i] << HCPENDLOG;
                continue;
            }
            if (!data["records"][i]["name"].empty() && !data["records"][i]["uuid"].empty() &&
                !data["records"][i]["aggregates"].empty() &&
                !data["records"][i]["aggregates"][0]["name"].empty() &&
                data["records"][i]["name"].asString() == m_vserverName) {
                aggregate = data["records"][i]["aggregates"][0]["name"].asString();
                HCP_Log(INFO, NETAPP_MODULE) << "++++++Found aggregate:" << aggregate
                    << " for svm: " << m_vserverName << HCPENDLOG;
                return SUCCESS;
            }
        }
        HCP_Log(ERR, NETAPP_MODULE) << "++++++Aggregate for: " << m_vserverName
            << " could not found " << HCPENDLOG;
        return FAILED;
    }

    int NetAppNas::GetAggregate(std::string &aggregate)
    {
        int iRet;
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/svm/svms?order_by=name&fields=name%2Cstate%2Caggregates";
        std::string errorDes;
        int errorCode;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS) {
            if (data.size() > 0 && data.isMember("records") &&
                (data["records"].isArray() && !data["records"].empty())) {
                return ValidateGetAggregateResponse(data, aggregate);
            } else {
                if (data["records"].isArray() && data["records"].empty()) {
                    HCP_Log(DEBUG, NETAPP_MODULE) << "No records found " << HCPENDLOG;
                }
                return FAILED;
            }
        } else {
            return iRet;
        }
    }

    int NetAppNas::ValidateCreateVolumeResponse(Json::Value &data)
    {
        int iRet;
        if (data.size() > 0 && data.isMember("job")) {
            if (!data["job"].isMember("uuid") || data["job"]["uuid"].empty()) {
                HCP_Log(ERR, NETAPP_MODULE)<< "no response job uuid/empty" <<HCPENDLOG;
                return FAILED;
            }
            if (!data["job"]["uuid"].empty()) {
                iRet = CheckJobStatus(data["job"]["uuid"].asString(), "CreateVolume");
                if (iRet != SUCCESS) {
                    return FAILED;
                } else {
                    HCP_Log(INFO, NETAPP_MODULE) <<"Create Volume success" << HCPENDLOG;
                    return SUCCESS;
                }
            } else
                return FAILED;
        } else {
            if (!data.isMember("job"))
                HCP_Log(ERR, NETAPP_MODULE) << "response doesn't have job field" <<HCPENDLOG;
            return FAILED;
        }
    }

    int NetAppNas::CreateVolume(unsigned long long size, SECURITY_TYPE secType)
    {
        int errorCode = NUMB_ZERO;
        int iRet = QueryVolume(m_resourceName, errorCode);
        if (iRet == SUCCESS && errorCode == NUMB_ZERO) {
            return SUCCESS;
        } else if (errorCode == -NUMB_ONE)
            return FAILED;
        std::string aggregate {};
        if (GetAggregate(aggregate) != SUCCESS) {
            return FAILED;
        }
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["name"] = m_resourceName;
        jsonReq["size"] = Json::Int64(size);
        jsonReq["guarantee"]["type"] = "none";
        jsonReq["svm"]["name"] = m_vserverName;
        jsonReq["aggregates"][0]["name"] = aggregate;
        if (secType == SECURITY_TYPE::_UNIX) {
            jsonReq["nas"]["unix_permissions"] = "755";
        } else if (secType == SECURITY_TYPE::_NTFS) {
            jsonReq["nas"]["security_style"] = "ntfs";
        } else {
            jsonReq["nas"]["security_style"] = "mixed";
        }
        Json::FastWriter jsonWriter;
        req.body = jsonWriter.write(jsonReq);
        req.method = "POST";
        req.url = "/api/storage/volumes";
        std::string errorDes;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS || iRet == HTTP_202) {
            return ValidateCreateVolumeResponse(data);
        } else {
            return FAILED;
        }
    }

    int NetAppNas::Create(unsigned long long size)
    {
        int ret = CreateVolume(size, SECURITY_TYPE::_UNIX);
        if (ret != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Create volume failure!!" << HCPENDLOG;
            return FAILED;
        }
        return SUCCESS;
    }

    int NetAppNas::ValidateCreateCloneVolumeResponse(Json::Value &data, std::string cloneName)
    {
        int iRet;
        if (data.size() > 0 && data.isMember("job")) {
            if (!data["job"].isMember("uuid") || data["job"]["uuid"].empty()) {
                HCP_Log(ERR, NETAPP_MODULE) << "response data format does not have"
                    "job uuid or it is empty" << HCPENDLOG;
                return FAILED;
            }
            if (!data["job"]["uuid"].empty()) {
                // check if the job is processed successfully
                iRet = CheckJobStatus(data["job"]["uuid"].asString(), "CreateCloneVolume");
                if (iRet != SUCCESS) {
                    return FAILED;
                } else {
                    HCP_Log(INFO, NETAPP_MODULE) << "Create Clone: "
                        << cloneName << " success" << HCPENDLOG;
                    return SUCCESS;
                }
            }
        } else {
            if (!data.isMember("job"))
                HCP_Log(ERR, NETAPP_MODULE) <<"response doesn't have job field" << HCPENDLOG;
            return FAILED;
        }
    }

    int NetAppNas::CreateCloneVolume(std::string cloneName, int &errorCode)
    {
        if (cloneName.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "Clone Name is empty !!" << HCPENDLOG;
            return FAILED;
        }

        int iRet = QueryVolume(cloneName, errorCode);
        if (iRet == SUCCESS && errorCode == NUMB_ZERO) {
            HCP_Log(ERR, NETAPP_MODULE) << "Already a volume exists with same name"
                << HCPENDLOG;
            return FAILED;
        } else if (errorCode == -NUMB_ONE)
            return FAILED;
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["name"] = cloneName;
        jsonReq["svm"]["name"] = m_vserverName;
        jsonReq["clone"]["is_flexclone"] = true;
        jsonReq["clone"]["parent_snapshot"]["name"] = Json::objectValue;
        jsonReq["clone"]["parent_svm"]["name"] = m_vserverName;
        jsonReq["clone"]["parent_svm"]["uuid"] = m_vserverUuid;
        jsonReq["clone"]["parent_volume"]["name"] = m_volumeName;
        jsonReq["clone"]["parent_volume"]["uuid"] = m_volumeUuid;
        jsonReq["guarantee"]["type"] = "none";
        Json::FastWriter jsonWriter;
        req.body = jsonWriter.write(jsonReq);
        req.method = "POST";
        req.url = "/api/storage/volumes";

        std::string errorDes;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS || iRet == HTTP_202) {
            return ValidateCreateCloneVolumeResponse(data, cloneName);
        } else {
            HCP_Log(ERR, NETAPP_MODULE) << "errorDes: " << errorDes << HCPENDLOG;
            return FAILED;
        }
    }

    std::unique_ptr<ControlDevice> NetAppNas::CreateClone(std::string cloneName, int &errorCode)
    {
        return nullptr;
    }

    int NetAppNas::ValidateQuerySnapshotResponse(Json::Value &data, std::string snapshotName,
                                                    int &errorCode)
    {
        for (Json::Value::ArrayIndex i = 0; i != data["records"].size(); i++) {
            if (!data["records"][i].isMember("name") || !data["records"][i].isMember("uuid")) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "data format does not have"
                    "snapname/uuid " << data["records"][i] << HCPENDLOG;
                continue;
            }
            if (!data["records"][i]["name"].empty() && !data["records"][i]["uuid"].empty() &&
                data["records"][i]["name"].asString() == snapshotName) {
                HCP_Log(INFO, NETAPP_MODULE) << "++++++Snapshot exists: " << snapshotName
                    << " for volumeId: " << m_volumeUuid << HCPENDLOG;
                errorCode = NUMB_ZERO;
                return SUCCESS;
            }
        }
        HCP_Log(ERR, NETAPP_MODULE) << "++++++Snapshot " << snapshotName
            << "doesnot exist for volumeId: " << m_volumeUuid << HCPENDLOG;
        errorCode = NUMB_ZERO;
        return FAILED;
    }

    int NetAppNas::QuerySnapshot(std::string snapshotName, int &errorCode)
    {
        if (snapshotName.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "Snapshot name is empty" << HCPENDLOG;
            return FAILED;
        }
        int iRet;
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/storage/volumes/" + m_volumeUuid + "/snapshots/?name=" + snapshotName;
        std::string errorDes;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS) {
            if (data.size() > 0 && data.isMember("records") &&
                (data["records"].isArray() && !data["records"].empty())) {
                return ValidateQuerySnapshotResponse(data, snapshotName, errorCode);
            } else {
                if (data["records"].isArray() && data["records"].empty() && data["num_records"].asInt() == 0) {
                    HCP_Log(INFO, NETAPP_MODULE) << "No records for snapshot: "
                        << snapshotName << HCPENDLOG;
                    errorCode = NUMB_ZERO;
                }
                return FAILED;
            }
        } else {
            errorCode = -NUMB_ONE;
            return iRet;
        }
    }

    std::unique_ptr<ControlDevice> NetAppNas::ValidateCreateSnapshotResponse(Json::Value &data,
                                                                            std::string snapshotName)
    {
        int iRet;
        ControlDeviceInfo deviceInfo = {};
        if (!data["job"].isMember("uuid") || data["job"]["uuid"].empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "data format does not have"
                "job uuid or it is empty" << HCPENDLOG;
            return nullptr;
        }
        if (!data["job"]["uuid"].empty()) {
            // check if the job is processed successfully
            iRet = CheckJobStatus(data["job"]["uuid"].asString(), "CreateSnapshot");
            if (iRet != SUCCESS) {
                return nullptr;
            } else {
                HCP_Log(INFO, NETAPP_MODULE) << "Create Snapshot: " << snapshotName
                    << " success" << HCPENDLOG;
                AssignDeviceInfo(deviceInfo, snapshotName);
                return std::make_unique<NetAppNasSnapshot>(deviceInfo, m_volumeUuid,  m_volumeName);
            }
        } else {
            HCP_Log(ERR, NETAPP_MODULE) << "Job Uuid is empty" << HCPENDLOG;
            return nullptr;
        }
    }

    std::unique_ptr<ControlDevice> NetAppNas::CreateSnapshot(std::string snapshotName, int &errorCode)
    {
        int iRet;
        if (CheckSvmDetails() != SUCCESS || CheckVolumeDetails() != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Empty Fields: m_vserverName="<<m_vserverName<<" m_vserverUuid="
                << m_vserverUuid << " m_volumeName=" << m_volumeName
                << " m_volumeUuid="<< m_volumeUuid << HCPENDLOG;
            return nullptr;
        }
        ControlDeviceInfo deviceInfo = {};
        iRet = QuerySnapshot(snapshotName, errorCode);
        if (iRet == SUCCESS && errorCode == NUMB_ZERO) {
            HCP_Log(ERR, NETAPP_MODULE) << "Snapshot exist: " << snapshotName << HCPENDLOG;
            AssignDeviceInfo(deviceInfo, snapshotName);
            return std::make_unique<NetAppNasSnapshot>(deviceInfo, m_volumeUuid,  m_volumeName);
        }
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["name"] = snapshotName;
        Json::FastWriter jsonWriter;
        req.body = jsonWriter.write(jsonReq);
        req.method = "POST";
        req.url = "/api/storage/volumes/" + m_volumeUuid + "/snapshots";

        std::string errorDes;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        // HTTP_202 represents the request is accepted for processing, but does not mean processed yet
        if (iRet == SUCCESS || iRet == HTTP_202) {
            if (data.size() > 0 && data.isMember("job")) {
                return ValidateCreateSnapshotResponse(data, snapshotName);
            } else {
                if (!data.isMember("job"))
                    HCP_Log(ERR, NETAPP_MODULE) << "rsp doesn'tt have job field"
                        << HCPENDLOG;
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }

    int NetAppNas::CheckCloneFieldsPartOne(Json::Value &data, int index)
    {
        if (!data["records"][index]["clone"].isMember("parent_svm") ||
            !data["records"][index]["clone"].isMember("parent_snapshot") ||
            !data["records"][index]["clone"].isMember("parent_volume") ||
            !data["records"][index]["clone"]["parent_svm"].isMember("name") ||
            !data["records"][index]["clone"]["parent_volume"].isMember("name") ||
            !data["records"][index]["clone"]["parent_volume"].isMember("name") ||
            !data["records"][index]["clone"]["parent_snapshot"].isMember("name")) {
            return FAILED;
        }
        return SUCCESS;
    }

    int NetAppNas::CheckCloneFieldsPartTwo(Json::Value &data, int index)
    {
        if (!data["records"][index]["clone"]["parent_svm"]["name"].empty() &&
            !data["records"][index]["clone"]["parent_volume"]["name"].empty() &&
            !data["records"][index]["clone"]["parent_volume"]["uuid"].empty() &&
            !data["records"][index]["clone"]["parent_snapshot"]["name"].empty()) {
            return SUCCESS;
        }
        return FAILED;
    }

    int NetAppNas::CheckCloneFields(Json::Value &data, int index, bool checkFields)
    {
        if (checkFields) {
            if (CheckCloneFieldsPartOne(data, index) == SUCCESS) {
                return SUCCESS;
            }
        } else {
            if (CheckCloneFieldsPartOne(data, index) == SUCCESS) {
                return SUCCESS;
            }
        }
        return FAILED;
    }

    bool NetAppNas::ValidateQueryParentVolumeResponseDataCheck(Json::Value &data, const Json::Value::ArrayIndex &i)
    {
        if (!data["records"][i].isMember("name") || !data["records"][i].isMember("uuid") ||
            (data["records"][i]["name"].empty() && data["records"][i]["uuid"].empty()) ||
            !data["records"][i].isMember("clone") || !data["records"][i].isMember("svm") ||
            (!data["records"][i]["svm"].isMember("name") || data["records"][i]["svm"]["name"].empty()) ||
            (!data["records"][i]["svm"].isMember("uuid") || data["records"][i]["svm"]["uuid"].empty()) ||
            CheckCloneFields(data, i, true) == FAILED) {
            HCP_Log(DEBUG, NETAPP_MODULE) << "response data format not proper: "
                << data["records"][i] << HCPENDLOG;
            return true;
        }
        return false;
    }

    int NetAppNas::ValidateQueryParentVolumeResponse(Json::Value &data, std::string volumeName,
                                                        std::string &parentVolName, std::string &parentVolUuid,
                                                        std::string &parentSnapshot)
    {
        for (Json::Value::ArrayIndex i = 0; i != data["records"].size(); i++) {
            if (ValidateQueryParentVolumeResponseDataCheck(data, i)) {
                continue;
            }
            if (data["records"][i]["name"].asString() == volumeName &&
                data["records"][i]["svm"]["name"].asString() == m_vserverName &&
                data["records"][i]["svm"]["uuid"].asString() == m_vserverUuid &&
                CheckCloneFields(data, i, false) == SUCCESS &&
                data["records"][i]["clone"]["parent_svm"]["name"].asString() == m_vserverName) {
                parentVolName = data["records"][i]["clone"]["parent_volume"]["name"].asString();
                parentVolUuid = data["records"][i]["clone"]["parent_volume"]["uuid"].asString();
                parentSnapshot = data["records"][i]["clone"]["parent_snapshot"]["name"].asString();

                HCP_Log(INFO, NETAPP_MODULE) << "Got Parent info, parentVolName: "
                    << parentVolName << ", uuid: " << parentVolUuid << "parentSnap: "
                    << parentSnapshot << HCPENDLOG;
                return SUCCESS;
            }
        }
        parentVolName = "";
        parentVolUuid = "";
        parentSnapshot = "";
        HCP_Log(ERR, NETAPP_MODULE)<<"No volume found for: "<<volumeName<<HCPENDLOG;
        return FAILED;
    }

    int NetAppNas::QueryParentVolumeDetails(std::string volumeName, std::string &parentVolName,
                                                std::string &parentVolUuid, std::string &parentSnapshot)
    {
        if (volumeName.at(0) == '/' || volumeName.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "VolumeName is improper"<<volumeName<<HCPENDLOG;
            return FAILED;
        }
        if (CheckSvmDetails() != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "No SVM found to query volume" << HCPENDLOG;
            return FAILED;
        }
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/storage/volumes?fields=uuid,name,svm,clone&name=" + volumeName + "&svm=" + m_vserverName;
        std::string errorDes;
        int errorCode = NUMB_ZERO;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS) {
            if (data.size() > 0 && data.isMember("records") &&
                (data["records"].isArray() && !data["records"].empty())) {
                return ValidateQueryParentVolumeResponse(data, volumeName, parentVolName,
                                                        parentVolUuid, parentSnapshot);
            } else {
                if (data["records"].isArray() && data["records"].empty() &&
                    data["num_records"].asInt() == 0) {
                    HCP_Log(ERR, NETAPP_MODULE)<<"Given Volume Doesn't exist" << HCPENDLOG;
                }
                return FAILED;
            }
        } else {
            return FAILED;
        }
    }

    int NetAppNas::DeleteParentSnapshot(std::string parentVolName, std::string parentVolUuid,
                                            std::string parentSnapshot)
    {
        if (parentVolName.empty() || parentVolUuid.empty() || parentSnapshot.empty()) {
            HCP_Log(ERR, NETAPP_MODULE) << "Parent Info not proper: parentVolumeName="
                << parentVolName << ", parentVolumeUuid=" << parentVolUuid
                << ", parentSnapshot=" << parentSnapshot << HCPENDLOG;
            return FAILED;
        }
        std::string currentVolumeName = this->m_volumeName;
        std::string currentVolumeUuid = this->m_volumeUuid;
        int ret = SetVolumeDetails(parentVolName, parentVolUuid);
        if (ret == FAILED) {
            return FAILED;
        }

        ret = DeleteSnapshot(parentSnapshot);
        if (ret == FAILED) {
            return FAILED;
        }

        ret = SetVolumeDetails(currentVolumeName, currentVolumeUuid);
        if (ret == FAILED) {
            return FAILED;
        }
        HCP_Log(INFO, NETAPP_MODULE) << "DeleteParentSnapshot: " << parentSnapshot
            << " success " << HCPENDLOG;
        return SUCCESS;
    }

    int NetAppNas::ValidateDeleteSnapshotResponse(Json::Value &data, std::string snapshotNama)
    {
        int iRet;
        for (Json::Value::ArrayIndex i = 0; i != data["jobs"].size(); i++) {
            if (!data["jobs"][i].isMember("uuid") || data["jobs"][i]["uuid"].empty()) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "response data format does not have"
                    "uuid or it is empty" << data["jobs"][i] << HCPENDLOG;
                continue;
            }
            if (!data["jobs"][i]["uuid"].empty()) {
                iRet = CheckJobStatus(data["jobs"][i]["uuid"].asString(), "DeleteSnapshot");
                if (iRet != SUCCESS) {
                    return FAILED;
                } else {
                    HCP_Log(INFO, NETAPP_MODULE) << "Delete Snapshot: "
                        << snapshotNama << " success" << HCPENDLOG;
                    return SUCCESS;
                }
            }
        }
        HCP_Log(ERR, NETAPP_MODULE) << "Couldn't delete Snapshot: "<<snapshotNama<<HCPENDLOG;
        return FAILED;
    }

    int NetAppNas::DeleteSnapshot(std::string SnapshotName)
    {
        int iRet;
        if (CheckSvmDetails() != SUCCESS || CheckVolumeDetails() != SUCCESS) {
            HCP_Log(ERR, NETAPP_MODULE) << "Fields: m_vserverName="<<m_vserverName<<" m_vserverUuid="
                << m_vserverUuid << " m_volumeName=" << m_volumeName
                << " m_volumeUuid=" << m_volumeUuid << HCPENDLOG;
            return FAILED;
        }
        int errorCode = NUMB_ZERO;
        iRet = QuerySnapshot(SnapshotName, errorCode);
        if (iRet != SUCCESS) {
            if (errorCode == NUMB_ZERO) {
                return SUCCESS;
            }
            if (errorCode == -NUMB_ONE) {
                HCP_Log(ERR, NETAPP_MODULE) << "Query Snapshot not success: "
                    << SnapshotName << HCPENDLOG;
                return FAILED;
            }
        }
        HttpRequest req;
        req.method = "DELETE";
        req.url = "/api/storage/volumes/" + m_volumeUuid + "/snapshots/?name=" + SnapshotName;
        std::string errorDes;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        // HTTP_202 represents the request is accepted for processing, but does not mean processed yet
        if (iRet == SUCCESS || iRet == HTTP_202) {
            if (data.size() > 0 && data.isMember("jobs") && data["jobs"].isArray()) {
                return ValidateDeleteSnapshotResponse(data, SnapshotName);
            } else {
                if (!data.isMember("jobs") || !data["jobs"].isArray()) {
                    HCP_Log(ERR, NETAPP_MODULE) << "response does not have jobs field"
                        "or it is not an array" << HCPENDLOG;
                }
                return FAILED;
            }
        } else {
            return FAILED;
        }
    }

    int NetAppNas::ValidateJobStatusResponse(Json::Value &data, std::string jobUuid,
                                                std::string &status)
    {
        if (data.size() > 0 && data.isMember("uuid") && data.isMember("state") &&
            data["uuid"].asString() == jobUuid) {
            if (data["state"].asString() == JOB_SUCCESS && data["code"].asInt() == 0) {
                HCP_Log(INFO, NETAPP_MODULE) << "Job status: success" << HCPENDLOG;
                status = JOB_SUCCESS;
                return SUCCESS;
            } else if (data["state"].asString() == JOB_RUNNING) {
                HCP_Log(INFO, NETAPP_MODULE) << "Job status: " << JOB_RUNNING << HCPENDLOG;
                status = JOB_RUNNING;
                return SUCCESS;
            } else if (data["state"].asString() == JOB_QUEUED) {
                HCP_Log(INFO, NETAPP_MODULE) << "Job status: " << JOB_QUEUED << HCPENDLOG;
                status = JOB_QUEUED;
                return SUCCESS;
            } else if (data["state"].asString() == JOB_FAILURE) {
                HCP_Log(ERR, NETAPP_MODULE) << "Job status: failed, message: "
                        << data["message"].asString() << HCPENDLOG;
                status = JOB_FAILURE;
                return FAILED;
            } else {
                HCP_Log(ERR, NETAPP_MODULE) << "Job state status: "
                    << data["state"].asString() << HCPENDLOG;
                status = JOB_RETRY;
                return SUCCESS;
            }
        } else {
            HCP_Log(ERR, NETAPP_MODULE) << "Incorrect response" << HCPENDLOG;
            status = JOB_INVALID;
            return FAILED;
        }
    }

    int NetAppNas::CheckJobStatus(std::string jobUuid, std::string jobName)
    {
        int retryNum = 0;
        int retryTimes = 4;
        int ret = SUCCESS;

        HttpRequest req;
        req.method = "GET";
        req.url = "/api/cluster/jobs/" + jobUuid;
        std::string errorDes;
        int errorCode;
        Json::Value data;
        std::string jobStatus;
        HCP_Log(INFO, NETAPP_MODULE) << "Checking Job status for: " <<jobName<<HCPENDLOG;
        while (retryNum < retryTimes) {
            ret = SendRequestOnce(req, data, errorDes, errorCode);
            if (ret == SUCCESS || ret == HTTP_202) {
                jobStatus = "";
                ret = ValidateJobStatusResponse(data, jobUuid, jobStatus);
                if (jobStatus == JOB_SUCCESS) {
                    ret = SUCCESS;
                    break;
                } else if (jobStatus == JOB_RUNNING || jobStatus == JOB_QUEUED) {
                    DelayTimeSendRequest(jobName);
                    retryNum++;
                    continue;
                } else if (jobStatus == JOB_FAILURE) {
                    ret = FAILED;
                    break;
                } else if (jobStatus == JOB_RETRY) {
                    retryNum++;
                    continue;
                } else if (jobStatus == JOB_INVALID) {
                    ret = FAILED;
                    break;
                } else {
                    ret = FAILED;
                    break;
                }
            } else {
                ret = FAILED;
                break;
            }
        }
        if (ret == SUCCESS) {
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

    void NetAppNas::AssignDeviceInfo(ControlDeviceInfo &deviceInfo, std::string deviceName)
    {
        deviceInfo.deviceName = deviceName;
        deviceInfo.url = NetAppIP;
        deviceInfo.port = NetAppPort;
        deviceInfo.userName = NetAppUsername;
        deviceInfo.password = NetAppPassword;
        deviceInfo.poolId = NetAppPoolId;
        deviceInfo.serviceIp = NetAppServiceIp;
    }

    void NetAppNas::SetRetryAttr(int _retryTimes, int _retryIntervalTime)
    {
        this->retryTimes = _retryTimes;
        this->retryIntervalTime = _retryIntervalTime;
        HCP_Log(INFO, NETAPP_MODULE) << "set retry times: " << this->retryTimes << HCPENDLOG;
    }

    int NetAppNas::SendRequest(HttpRequest &req, Json::Value &data,
                                    std::string &errorDes, int &errorCode)
    {
        // 检查存储设备是否含有证书和吊销列表信息
        if (!certification.empty()) {
            req.cert = certification;
            req.isVerify = VCENTER_VERIFY;
        }
        if (!crl.empty()) {
            req.revocationList = crl;
        }
        int retryNum = 0;
        while (retryNum < this->retryTimes) {
            HCP_Log(INFO, NETAPP_MODULE) << "send request for "<< (retryNum + 1)
                << " time to " << HCPENDLOG;
            int ret = SUCCESS;
            if (m_encryptedKey.empty()) {
                ret = Base64Encryption(NetAppUsername, NetAppPassword);
                if (ret != SUCCESS || m_encryptedKey == ENCRYPT_FAILED) {
                    HCP_Log(ERR, NETAPP_MODULE) << "Error in encryption!!" << HCPENDLOG;
                    return FAILED;
                }
            } else if (m_encryptedKey == ENCRYPT_FAILED) {
                HCP_Log(ERR, NETAPP_MODULE) << "Encryption failed already!!" << HCPENDLOG;
                return FAILED;
            }

            if (ret == SUCCESS) {
                ret = SendRequestOnce(req, data, errorDes, errorCode);
                if (ret == SUCCESS && errorCode == SUCCESS) {
                    HCP_Log(INFO, NETAPP_MODULE) << "send request success" << HCPENDLOG;
                    return SUCCESS;
                }
                if (ret == HTTP_202 || errorCode == HTTP_202) {
                    HCP_Log(INFO, NETAPP_MODULE) << "send request accepted but"
                        "not yet processed " << HCPENDLOG;
                    return HTTP_202;
                } else if (ret != SUCCESS && errorCode == NUMB_ZERO) {
                    HCP_Log(INFO, NETAPP_MODULE) << "Curl is success but http error, errorDes="
                        << errorDes << ", Retrying again" << HCPENDLOG;
                }
            }

            DelayTimeSendRequest("SendRequest");
            retryNum++;
        }
        HCP_Log(ERR, NETAPP_MODULE) << "send request failed." << HCPENDLOG;
        return FAILED;
    }

    bool NetAppNas::GetEnableProxy()
    {
        auto configReader = ConfigReaderImpl::instance();
        std::string backupScene = configReader->GetBackupSceneFromXml("backup_scene");
        return backupScene == "0" ? false : true;
    }

    int NetAppNas::SendRequestOnce(HttpRequest req, Json::Value &data,
                                        std::string &errorDes, int &errorCode)
    {
        int iRet = FAILED;
        HttpRequest request = req;
        request.url = curl_http + NetAppIP + ":" + NetAppPort + req.url;
        request.enableProxy = GetEnableProxy();
        (void)request.heads.insert(std::make_pair(std::string("Authorization"), m_encryptedKey));
        std::shared_ptr<IHttpResponse> rsp;
        iRet = SendHttpReq(rsp, request, errorDes, errorCode);
        if (iRet != SUCCESS) {
            if (iRet == HTTP_202 || iRet == HTTP_401) {
                HCP_Log(DEBUG, NETAPP_MODULE) << "++++iRet: " << iRet << HCPENDLOG;
                errorCode = iRet;
                return ResponseSuccessHandle(req, rsp, data, errorDes, errorCode);
            }
            // get when curl send success,http response error for httpstatuscodeforRetry
            if (errorCode == NUMB_ZERO) {
                return iRet;
            }
            return FAILED;
        }
        iRet = ResponseSuccessHandle(req, rsp, data, errorDes, errorCode);
        return iRet;
    }

    int NetAppNas::SendHttpReq(std::shared_ptr<IHttpResponse> &rsp, const HttpRequest &req,
                                    std::string &errorDes, int& errorCode)
    {
        HttpRequest tempReq = req;
        tempReq.url = FormatFullUrl(tempReq.url);
        rsp = fs_pHttpCLient->SendRequest(tempReq);
        if (NULL == rsp.get()) {
            HCP_Log(ERR, NETAPP_MODULE) << "Return response is empty." << HCPENDLOG;
            return FAILED;
        }
        if (!rsp->Success()) {
            // 1.curl success,http response error with http status codes
            if (rsp->GetErrCode() == 0) {
                errorDes = rsp->GetHttpStatusDescribe(); // http status error description
                errorCode = rsp->GetErrCode();
                HCP_Log(ERR, NETAPP_MODULE) << tempReq.url << " Curl ok,HttpStatusCode: "
                    << rsp->GetHttpStatusCode() << ", Http response is " << errorDes << HCPENDLOG;
                return rsp->GetHttpStatusCode(); // return http status code
                // 2.curl error,need directly retry
            } else {
                errorDes = rsp->GetErrString();
                errorCode = rsp->GetErrCode();
                HCP_Log(ERR, NETAPP_MODULE) << tempReq.url << " Curl error. errorCode: " << errorCode
                    << "errorDes:" << errorDes << HCPENDLOG;
            }
            SetErrorCode(errorCode);
            return FAILED;
            // 3. curl success, http response success
        } else {
            errorDes = rsp->GetErrString();
            errorCode = rsp->GetErrCode();
            HCP_Log(DEBUG, NETAPP_MODULE) << "curl success and http success " << HCPENDLOG;
            return SUCCESS;
        }
        return SUCCESS;
    }

    int NetAppNas::ResponseSuccessHandle(HttpRequest req, std::shared_ptr<IHttpResponse>& rsp,
                                            Json::Value &data, std::string &errorDes, int &errorCode)
    {
        int Ret = SUCCESS;
        if (errorCode == HTTP_401) {
            HCP_Log(ERR, NETAPP_MODULE) << "Auth Failed...Retrying again!!" << HCPENDLOG;
            Ret = Base64Encryption(NetAppUsername, NetAppPassword);
            if (Ret != SUCCESS || m_encryptedKey.empty() || m_encryptedKey == ENCRYPT_FAILED) {
                HCP_Log(ERR, NETAPP_MODULE) << "Retry encryption Failed!" << HCPENDLOG;
                return FAILED;
            }
            HttpRequest request = req;
            request.url = curl_http + NetAppIP + ":" + NetAppPort + req.url;
            (void)request.heads.insert(std::make_pair(std::string("Authorization"), m_encryptedKey));
            Ret = SendHttpReq(rsp, request, errorDes, errorCode);
            if (Ret != SUCCESS || Ret == HTTP_401) {
                HCP_Log(ERR, NETAPP_MODULE) << "Failed to retry auth!! Check UserName/PW"<<HCPENDLOG;
                return FAILED;
            }
        }
        Ret = ParseResponse(rsp->GetBody(), data, errorDes, errorCode);
        return Ret;
    }

    int NetAppNas::ParseResponse(const std::string &json_data, Json::Value &data,
                                    std::string &errorDes, int &errorCode)
    {
        Json::Value jsonValue;
        Json::Reader reader;
        if (!reader.parse(json_data.c_str(), jsonValue)) {
            errorDes = "Parse json string failed";
            return FAILED;
        }

        if (!jsonValue.isMember("_links")) {
            if (jsonValue.isMember("job") && !jsonValue["job"].isMember("_links")) {
                errorDes = "Json object format is error ";
            }
        }

        if (jsonValue.isMember("error")) {
            errorDes = "Json object format is error. ";
            // netApp打快照失败返回的code是string类型，直接使用asInt会coredump
            int netAppErrCode = 0;
            if (jsonValue["error"]["code"].isIntegral()) {
                netAppErrCode = jsonValue["error"]["code"].asInt();
            } else {
                try {
                    std::string stringCode = jsonValue["error"]["code"].asString();
                    netAppErrCode = std::stoi(stringCode.c_str());
                } catch (const std::invalid_argument &e) {
                    WARNLOG("Invalid convert : %s", jsonValue["error"]["code"].asString().c_str());
                }
            }
            if (netAppErrCode != SUCCESS) {
                HCP_Log(WARN, NETAPP_MODULE) << "code : "
                    << netAppErrCode << ", Describe : "
                    << jsonValue["error"]["message"].asString() << HCPENDLOG;

                errorDes = jsonValue["error"]["message"].asString();
                errorCode = netAppErrCode;
            }
            return FAILED;
        }

        if (jsonValue.isObject()) {
            data = jsonValue;
        }
        return SUCCESS;
    }

    int NetAppNas::Base64Encryption(const std::string& userName, const std::string& password)
    {
        this->m_encryptedKey = "";
        using namespace boost::archive::iterators;
        std::string plainKey = userName + ":" + password;

        typedef base64_from_binary<transform_width<std::string::const_iterator, 6, 8>> Base64EncodeIterator;
        std::stringstream encodedResult;
        try {
            copy(Base64EncodeIterator(plainKey.begin()), Base64EncodeIterator(plainKey.end()),
                std::ostream_iterator<char>(encodedResult));
        } catch (...) {
            HCP_Log(ERR, NETAPP_MODULE) << "Exception occured while encoding" << HCPENDLOG;
            m_encryptedKey = ENCRYPT_FAILED;
            return FAILED;
        }
        size_t equal_count = (NUMB_THREE - plainKey.length() % NUMB_THREE) % NUMB_THREE;
        for (size_t i = 0; i < equal_count; i++) {
            encodedResult.put('=');
        }

        std::string encryptedkey = encodedResult.str();
        if (encryptedkey.empty()) {
            this->m_encryptedKey = "";
            HCP_Log(ERR, NETAPP_MODULE) << "Failed encoding" << HCPENDLOG;
            return FAILED;
        } else {
            this->m_encryptedKey = "Basic " + encryptedkey;
            return SUCCESS;
        }
    }

    int NetAppNas::Base64Decryption(std::string encryptedkey, std::string &plainKey)
    {
        using namespace boost::archive::iterators;
        if (encryptedkey.empty()) {
            plainKey = "";
            HCP_Log(ERR, NETAPP_MODULE) << "Failed decoding" << HCPENDLOG;
            return FAILED;
        }

        typedef transform_width<binary_from_base64<std::string::const_iterator>, 8, 6> Base64DecodeIterator;
        std::stringstream decodeResult;
        try {
            copy(Base64DecodeIterator(encryptedkey.begin()), Base64DecodeIterator(encryptedkey.end()),
                std::ostream_iterator<char>(decodeResult));
        } catch (...) {
            HCP_Log(ERR, NETAPP_MODULE) << "Exception occured while decoding" << HCPENDLOG;
            plainKey = "";
            return FAILED;
        }

        plainKey = decodeResult.str();
        return SUCCESS;
    }

    void NetAppNas::DelayTimeSendRequest(const std::string &delayForJobName)
    {
        auto now = std::chrono::steady_clock::now();
        while ((double(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() -
            now).count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den) <
            retryIntervalTime) {
            HCP_Log(INFO, NETAPP_MODULE) << "Waiting for storage device to complete... "
                << delayForJobName << HCPENDLOG;
            sleep(SEND_REQ_RETRY_INTERVAL);
        }
        return;
    }

    int NetAppNas::DestroyDeviceSession()
    {
        if (!m_encryptedKey.empty())
            m_encryptedKey = "";
        return SUCCESS;
    }
}

