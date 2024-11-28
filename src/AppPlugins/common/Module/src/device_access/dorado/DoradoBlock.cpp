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
#include "device_access/dorado/DoradoBlock.h"
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "device_access/Const.h"
#include "device_access/dorado/DoradoBlockSnapshot.h"
#include "error.h"
#include "system/System.hpp"
#include "common/Utils.h"
#include "common/JsonUtils.h"
#include "common/Timer.h"
#include "device_access/k8s/K8sutil.h"
#include "curl_http/CurlHttpClient.h"
#include "config_reader/ConfigIniReaderImpl.h"

using namespace std::chrono;
namespace Module {
    std::mutex DoradoBlock::m_doradoSessionMutex;
    SessionInfo DoradoBlock::m_doradoSession;
    std::unique_ptr<SessionCache> DoradoBlock::m_sessionCache = std::make_unique<SessionCache>(DORADO_MODULE_NAME);
    namespace {
        constexpr int HOST_TYPE = 21;
        constexpr int LINUX_TYPE = 0;
        constexpr int TIME_OUT = 120;
        constexpr int DORADO_LINK_TYPE = 2;
        constexpr int ERROR_DME_STORAGE_CREATE_INITIATOR_FAILED = 1593987339;
        constexpr int ERROR_DME_STORAGE_MAP_HOST_FAILED = 1593987341;
        constexpr int ERROR_DME_STORAGE_MAP_VOLUME_FAILED = 1593987340;
        const std::string URL_GET_TOKEN_IP =
            "https://pm-system-base.dpa.svc.cluster.local:30081/v1/internal/local-storage/session";
    } // namespace

/*
login FusionStorage and get token
Date : 2020/03/03
return : Success.Module::SUCCESS, failed:Module::FAILED or HTTP ERROR CODE.
Description:
             1.login DoradoBlock and get token
             2.cache token for this instance
*/

    SessionInfo DoradoBlock::Login()
    {
        HttpRequest req;
        req.method = "POST";
        req.url = "sessions";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode;
        jsonValue["username"] = DoradoUsername;
        jsonValue["password"] = DoradoPassword;
        jsonValue["scope"] = "0";
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        SessionInfo sessionInfo{};
        int iRet = SendRequestEx(req, data, errorDes, errorCode, sessionInfo);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            if (data.isMember("deviceid") && data.isMember("iBaseToken")) {
                sessionInfo.token = data["iBaseToken"].asString();
                sessionInfo.device_id = data["deviceid"].asString();
            } else {
                iRet = Module::FAILED;
            }
        }
        SetErrorCode(errorCode);
        SetExtendInfo(errorDes);
        for (std::string::iterator it = req.body.begin(); it != req.body.end(); ++it) {
            *it = 0;
        }
        return sessionInfo;
    }

    int DoradoBlock::TestDeviceConnection()
    {
        int ret = Module::SUCCESS;
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Testing Connection..." << HCPENDLOG;
        SessionInfo sessionInfo = Login();
        if (sessionInfo.device_id.empty() || sessionInfo.token.empty() ||
            sessionInfo.cookie.empty()) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Login Dorado Failed!!"
                                                "Invalid session: deviceId: " << sessionInfo.device_id << HCPENDLOG;
            return Module::FAILED;
        }
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Connection success, Logging out" << HCPENDLOG;
        ret = Logout(sessionInfo);
        if (ret != Module::SUCCESS) {
            HCP_Log(WARN, DORADO_MODULE_NAME) << "Connection success, "
                                                 "but Logout Failed" << HCPENDLOG;
        } else {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Connection success, "
                                                 "and Logout also success" << HCPENDLOG;
        }
        return Module::SUCCESS;
    }

    int DoradoBlock::GetConnectedIP()
    {
        int iRet = DoradoBlock::GetDoradoIp(DoradoIP, DoradoPort);
        if (iRet == Module::SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "get ipv4addr success, doradoIp:" << DoradoIP << HCPENDLOG;
            return Module::SUCCESS;
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Can not find the connect IP" << HCPENDLOG;
        return Module::FAILED;
    }
/*
logout at destruct this instance
Date : 2020/03/03
return : Success.Module::SUCCESS, failed:Module::FAILED or HTTP ERROR CODE.
Description:
             1.logout at destruct this instance
*/
    int DoradoBlock::Logout(SessionInfo sessionInfo)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start Authentication Exit. " << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "DELETE";
        req.url = "sessions";
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequestEx(req, data, errorDes, errorCode, sessionInfo);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    void DoradoBlock::Clean()
    {
        if (fs_pHttpCLient) {
            fs_pHttpCLient = NULL;
        }
    }

/*
Create fusionstorage Lun
Date : 2020/03/03
out params:id -> LUN id.
          :WWN  -> LUN WWN.
return : Success.Module::SUCCESS, failed:Module::FAILED or HTTP ERROR CODE.
Description:
             1.Create fusionstorage Lun
*/

    int DoradoBlock::Create(unsigned long long size)
    {
        std::string originalName = ResourceName;
        // the max length of the LUN name is 32
        if (ResourceName.length() > MAX_LENGTH) {
            ResourceName = ResourceName.substr(0, MAX_LENGTH);
        }
        std::string description;
        QueryLunDescription(ResourceName, description);
        if (description == originalName) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "already exist " << ResourceName << HCPENDLOG;
            return Module::SUCCESS;
        }
        LunParams nas(ResourceName, Compress, Dedup, DoradoPoolId, size);
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start create dorado lun " << ResourceName << HCPENDLOG;
        int mpRet = QueryLUN(nas.volumeName, ResourceId, Wwn, nas.Size, nas.usedSize);
        if (mpRet == Module::SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "already exist lun " << nas.volumeName << HCPENDLOG;
            return Module::SUCCESS;
        }
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq[NAME] = ResourceName;
        jsonReq[PARENTID] = DoradoPoolId;
        jsonReq[CAPACITY] = (Json::UInt64)(size * KB * TWO);
        jsonReq[ENABLECOMPRESSION] = Compress;
        jsonReq[ENABLESMARTDEDUP] = Dedup;
        jsonReq[DESCRIPTION] = originalName;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "lun";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data["ID"].asString() != "") {
            return QueryLUN(nas.volumeName, ResourceId, Wwn, nas.Size, nas.usedSize);
        } else {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Create failed! " << HCPENDLOG;
            return 1593987342; // ERROR_DME_STORAGE_CREATE_FAILED;HCP_Error_Code_List.xls的宏定义
        }
    }

    int DoradoBlock::NameLegalization(std::string &name)
    {
        std::vector<std::string>::iterator iter;
        int begin = -1;
        std::string result = "";
        for (int i = 0; i < name.length(); i++) {
            if ((begin = illegalChar.find(name[i], begin + 1)) == std::string::npos) {
                result = result + name[i];
            }
        }
        name = result;
        return Module::SUCCESS;
    }

    int DoradoBlock::LoginIscsiTarget(const std::string &iscsiIP, std::string &iqnNumber)
    {
        std::vector<std::string> cmdoutput;
        std::vector<std::string> stderroutput;
        std::vector<std::string> paramList;
        paramList.clear();
        cmdoutput.clear();
        stderroutput.clear();
        paramList.push_back(iscsiIP);
        std::string cmd = "iscsiadm -m discovery -t st -p '?'";
        if (runShellCmdWithOutput(DEBUG, "V6Storage_RunShell", 0, cmd, paramList, cmdoutput, stderroutput) != 0) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "login failed. iscsiadm -m discovery -t st -p ?" << HCPENDLOG;
            return Module::FAILED;
        }
        int n = cmdoutput[0].size();
        std::string IqnAddress;
        for (int i = 0; i < n; i++) {
            if (cmdoutput[0][i] == ' ') {
                IqnAddress = cmdoutput[0].substr(i + 1, n - i - 1);
            }
        }
        cmd = "iscsiadm -m node -T '?' -p '?' -l";
        paramList.clear();
        cmdoutput.clear();
        paramList.push_back(IqnAddress);
        paramList.push_back(iscsiIP);
        if (runShellCmdWithOutput(DEBUG, "V6Storage_RunShell", 0, cmd, paramList, cmdoutput, stderroutput) != 0) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "login failed. iscsiadm -m node -T ? -p ? -l" << HCPENDLOG;
            return Module::FAILED;
        }
        cmd = "cat /etc/iscsi/initiatorname.iscsi";
        paramList.clear();
        cmdoutput.clear();
        if (runShellCmdWithOutput(DEBUG, "V6Storage_RunShell", 0, cmd, paramList, cmdoutput, stderroutput) != 0) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "login failed. cat /etc/iscsi/initiatorname.iscsi" << HCPENDLOG;
            return Module::FAILED;
        }
        n = cmdoutput[0].size();
        for (int i = 0; i < n; i++) {
            if (cmdoutput[0][i] == '=') {
                iqnNumber = cmdoutput[0].substr(i + 1, n - i - 1);
                break;
            }
        }
        return Module::SUCCESS;
    }

    int DoradoBlock::Bind(HostInfo &host, const std::string &shareId)
    {
        int ret;

        for (auto iter = host.iscsinitor.begin(); iter != host.iscsinitor.end(); ++iter) {
            if (iter->second == ISCSI) {
                ret = CreateISCSIPort(iter->first);
                if (ret != Module::SUCCESS) {
                    HCP_Log(ERR, DORADO_MODULE_NAME) << "Create ISCSI Port failed! " << HCPENDLOG;
                    return ERROR_DME_STORAGE_CREATE_INITIATOR_FAILED;
                }
                ret = CreateIscsiHostMapping(iter->first, host);
                if (ret != Module::SUCCESS) {
                    HCP_Log(ERR, DORADO_MODULE_NAME) << "Create Iscsi Host Mapping failed! " << HCPENDLOG;
                    return ERROR_DME_STORAGE_MAP_HOST_FAILED;
                }
            } else if (iter->second == FC) {
                ret = CreateFcPort(iter->first);
                if (ret != Module::SUCCESS) {
                    HCP_Log(ERR, DORADO_MODULE_NAME) << "Create FC Port failed! " << HCPENDLOG;
                    return ERROR_DME_STORAGE_CREATE_INITIATOR_FAILED;
                }
                ret = CreateFcHostMapping(iter->first, host);
                if (ret != Module::SUCCESS) {
                    HCP_Log(ERR, DORADO_MODULE_NAME) << "Create FC Host Mapping failed! " << HCPENDLOG;
                    return ERROR_DME_STORAGE_MAP_HOST_FAILED;
                }
            }
        }

        // the max length of the LUN name is 32
        if (ResourceName.length() > MAX_LENGTH) {
            ResourceName = ResourceName.substr(0, MAX_LENGTH);
        }
        ret = CreateHostMapping(host.hostId, ResourceName);
        if (ret != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Create Host Mapping failed! " << HCPENDLOG;
            return Module::FAILED;
        }
        return ret;
    }

    int DoradoBlock::CreateHost(const std::string hostName, const std::string hostIp, std::string &hostID)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start create host " << hostName << HCPENDLOG;
        int mpRet = QueryHost(hostName, hostID);
        if (mpRet == Module::SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "already exist host" << hostName << HCPENDLOG;
            return Module::SUCCESS;
        }
        HttpRequest req;
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        jsonReq[NAME] = hostName;
        jsonReq[IP] = hostIp;
        jsonReq[OPERATIONSYSTEM] = LINUX_TYPE;
        req.method = "POST";
        req.url = "host";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            hostID = data["ID"].asString();
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::QueryHost(const std::string hostName, std::string &hostId)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query host " << hostName << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "GET";
        req.url = "host?filter=NAME::" + hostName;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data.size() > 0) {
            hostId = data[0]["ID"].asString();
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::CreateISCSIPort(const std::string id)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start create iscsi port." << HCPENDLOG;
        int mpRet = QueryISCSIPort(id);
        if (mpRet == Module::SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "already exist iscsi port." << HCPENDLOG;
            return Module::SUCCESS;
        }
        HttpRequest req;
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        jsonReq[ID] = id;
        req.method = "POST";
        req.url = "iscsi_initiator";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::CreateFcPort(const std::string id)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start create FC port." << HCPENDLOG;
        int mpRet = QueryFcPort(id);
        if (mpRet == Module::SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "already exist FC port." << HCPENDLOG;
            return Module::SUCCESS;
        }
        HttpRequest req;
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        jsonReq[ID] = id;
        req.method = "POST";
        req.url = "fc_initiator";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::CreateIscsiHostMapping(const std::string iscsi, HostInfo &host)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME)
                << "Start bind iscsi port," << " and bind to host " << host.hostId << HCPENDLOG;
        int mpRet = QueryIscsiHostMapping(iscsi, host);
        if (mpRet == Module::SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "already binded iscsi to " << host.hostId << HCPENDLOG;
            return Module::SUCCESS;
        }
        std::string tmpHostId;
        int ret = CreateHost(host.hostId, host.hostIp, tmpHostId);
        if (ret != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Create host failed! " << HCPENDLOG;
            return Module::FAILED;
        }
        HttpRequest req;
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        jsonReq[PARENTTYPE] = HOST_TYPE;
        jsonReq[PARENTID] = tmpHostId;
        req.method = "PUT";
        req.url = "iscsi_initiator/" + iscsi;
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::CreateFcHostMapping(const std::string fc, HostInfo &host)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME)
                << "Start bind fc port " << " and bind to host " << host.hostId << HCPENDLOG;
        int mpRet = QueryFcHostMapping(fc, host);
        if (mpRet == Module::SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "already binded fc to " << host.hostId << HCPENDLOG;
            return Module::SUCCESS;
        }
        std::string tmpHostId;
        int ret = CreateHost(host.hostId, host.hostIp, tmpHostId);
        if (ret != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Create host failed! " << HCPENDLOG;
            return Module::FAILED;
        }
        HttpRequest req;
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        jsonReq[PARENTTYPE] = HOST_TYPE;
        jsonReq[PARENTID] = tmpHostId;
        req.method = "PUT";
        req.url = "fc_initiator/" + fc;
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::QueryISCSIPort(const std::string id)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query iscsi port." << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "GET";
        req.url = "iscsi_initiator/" + id;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        HCP_Log(DEBUG, DORADO_MODULE_NAME)
                << iRet << "." << errorCode << "port list size: " << data.size() << HCPENDLOG;
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::QueryFcPort(const std::string id)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query FC port." << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "GET";
        req.url = "fc_initiator/" + id;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        HCP_Log(DEBUG, DORADO_MODULE_NAME)
                << iRet << "." << errorCode << "port list size: " << data.size() << HCPENDLOG;
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::QueryIscsiHostMapping(const std::string id, HostInfo &host)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query iscsi " << " to host " << host.hostId << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "GET";
        req.url = "iscsi_initiator/" + id;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data["ISFREE"] == "false") {
            if (data["PARENTNAME"] != host.hostId) {
                host.hostId = data["PARENTNAME"].asString();
            }
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::QueryFcHostMapping(const std::string id, HostInfo &host)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query fc " << " to host " << host.hostId << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "GET";
        req.url = "fc_initiator/" + id;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data["ISFREE"] == "false") {
            if (data["PARENTNAME"] != host.hostId) {
                host.hostId = data["PARENTNAME"].asString();
            }
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::QueryHostISCSIPort(std::string hostName, std::string iscsiPort)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME)
                << "Start query iscsi port bind " << hostName << " to iscsiPort." << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "iscsi_initiator/" + iscsiPort;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data["PARENTNAME"] == hostName) {
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::QueryHostFcPort(std::string hostName, std::string fcPort)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME)
                << "Start query fc port bind " << hostName << " to " << fcPort << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "fc_initiator/" + fcPort;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data["PARENTNAME"] == hostName) {
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    namespace {
        const std::string HOSTNAME = "hostName";
        const std::string LUNNAME = "lunName";
        const std::string LUNGROUPNAME = "lunGroupName";
    } // namespace

    int DoradoBlock::CreateHostMapping(const std::string hostName, const std::string lunName)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME)
                << "Start create host mapping " << hostName << " to lun " << lunName << HCPENDLOG;
        int lunId;
        std::string wwn;
        unsigned long long capacity;
        unsigned long long usedCapacity;
        int mpRet = QueryLUN(lunName, lunId, wwn, capacity, usedCapacity);
        if (mpRet != Module::SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME)
                    << "Query Lun " << lunName << "failed." << HCPENDLOG;
            return Module::FAILED;
        }

        std::string lunGroupName;
        mpRet = QueryLunGroupByLunId(lunGroupName, lunId);
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq[HOSTNAME] = hostName;
        if (mpRet == Module::SUCCESS) {
            jsonReq[LUNGROUPNAME] = lunGroupName;
        } else {
            jsonReq[LUNNAME] = lunName;
        }
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "api/v2/mapping";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS &&
            (errorCode == Module::SUCCESS || errorCode == DoradoErrorCode::LUN_HOST_MAPPING_EXIST ||
             errorCode == DoradoErrorCode::LUNGROUP_HOST_MAPPING_EXIST)) {
            return Module::SUCCESS;
        } else {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Create Host Mapping failed." << HCPENDLOG;
            return ERROR_DME_STORAGE_MAP_VOLUME_FAILED;
        }
    }

    int DoradoBlock::DeleteHostLunMapping(const std::string hostName, const std::string lunName)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME)
                << "Start delete host mapping " << hostName << " to lun " << lunName << HCPENDLOG;
        int lunId;
        std::string wwn;
        unsigned long long capacity;
        unsigned long long usedCapacity;
        int mpRet = QueryLUN(lunName, lunId, wwn, capacity, usedCapacity);
        if (mpRet != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME)
                    << "Query Lun " << lunName << "failed." << HCPENDLOG;
            return Module::FAILED;
        }

        std::string hostId;
        mpRet = QueryHost(hostName, hostId);
        if (mpRet != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query hostName " << hostName << "failed." << HCPENDLOG;
            return Module::FAILED;
        }

        HttpRequest req;
        req.method = "DELETE";
        std::string lunGroupName;
        mpRet = QueryLunGroupByLunId(lunGroupName, lunId);

        req.url = (mpRet == Module::SUCCESS) ? "api/v2/mapping?hostName=" + hostName + "&lunGroupName=" + lunGroupName
                                             : "api/v2/mapping?hostId=" + hostId + "&lunId=" + std::to_string(lunId);

        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS &&
            (errorCode == Module::SUCCESS || errorCode == DoradoErrorCode::LUN_HOST_MAPPING_NOTEXIST ||
             errorCode == DoradoErrorCode::LUNGROUP_HOST_MAPPING_NOTEXIST)) {
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::QueryHostLunMapping(const std::string hostName, const std::string lunName)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "query host mapping" << hostName << " to lun " << lunName << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "GET";
        req.url = "api/v2/mapping?hostName=" + hostName + "&lunName=" + lunName;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::QueryLunGroupByLunId(std::string &lunGroupName, const int &lunId)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME)
                << "Start query lun group by lun id: " << lunId << HCPENDLOG;

        HttpRequest req;
        req.method = "GET";
        req.url = "lungroup/associate?ASSOCIATEOBJTYPE=11&ASSOCIATEOBJID=" + std::to_string(lunId);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && !data.empty() && data.isArray()) {
            lunGroupName = data[0]["NAME"].asString();
            return Module::SUCCESS;
        } else {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query lun group failed." << HCPENDLOG;
            return Module::FAILED;
        }
    }

    int DoradoBlock::Query(DeviceDetails &info)
    {
        unsigned long long capacity = 0;
        unsigned long long usedCapacity = 0;
        // the max length of the LUN name is 32
        if (ResourceName.length() > MAX_LENGTH) {
            ResourceName = ResourceName.substr(0, MAX_LENGTH);
        }
        int ret = QueryLUN(ResourceName, ResourceId, Wwn, capacity, usedCapacity);
        if (ret != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query LUN failed. " << HCPENDLOG;
            return Module::FAILED;
        }
        info.deviceId = ResourceId;
        info.deviceName = ResourceName;
        info.deviceUniquePath = Wwn;
        info.totalCapacity = capacity;
        info.usedCapacity = usedCapacity;
        return ret;
    }

    int DoradoBlock::QueryLUN(
        std::string volumeName, int &volumeId, std::string &wwn, unsigned long long &capacity,
        unsigned long long &usedCapacity)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query lun " << volumeName << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "lun?filter=NAME::" + volumeName;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data.size() > 0) {
            volumeId = std::stoi(data[0]["ID"].asString());
            wwn = data[0]["WWN"].asString();
            std::istringstream capa(data[0]["CAPACITY"].asString());
            capa >> capacity;
            std::istringstream usedCapa(data[0]["ALLOCCAPACITY"].asString());
            usedCapa >> usedCapacity;
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::QueryLunDescription(std::string volumeName, std::string &description)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query lun " << volumeName << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "lun?filter=NAME::" + volumeName;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data.size() > 0) {
            description = data[0]["DESCRIPTION"].asString();
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::UnBind(HostInfo host, const std::string &shareId)
    {
        // the max length of the LUN name is 32
        if (host.hostId.length() > MAX_LENGTH) {
            host.hostId = host.hostId.substr(0, MAX_LENGTH);
        }
        if (ResourceName.length() > MAX_LENGTH) {
            ResourceName = ResourceName.substr(0, MAX_LENGTH);
        }
        return DeleteHostLunMapping(host.hostId, ResourceName);
    }

/*
delete volume with name
Date : 2020/03/03
return : Success.Module::SUCCESS, failed:Module::FAILED or HTTP ERROR CODE.
Description:
             1.delete volume with name
*/
    int DoradoBlock::Delete()
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start delete lun " << ResourceName << HCPENDLOG;
        int volumeId;
        std::string wwn;
        unsigned long long capacity;
        unsigned long long usedCapacity;
        int mpRet = QueryLUN(ResourceName, volumeId, wwn, capacity, usedCapacity);
        if (mpRet == Module::SUCCESS) {
            HttpRequest req;
            req.method = "DELETE";
            req.url = "lun/" + std::to_string(volumeId);
            Json::Value data;
            std::string errorDes;
            int errorCode;
            int iRet = SendRequest(req, data, errorDes, errorCode);
            if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
                return Module::SUCCESS;
            } else {
                return Module::FAILED;
            }
        } else {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Lun " << ResourceName << " does not exist." << HCPENDLOG;
            return Module::FAILED;
        }
    }

    int DoradoBlock::ExtendSize(unsigned long long size)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start extend lun " << ResourceName << HCPENDLOG;
        int volumeId;
        std::string wwn;
        unsigned long long capacity;
        unsigned long long usedCapacity;
        int mpRet = QueryLUN(ResourceName, volumeId, wwn, capacity, usedCapacity);
        if (mpRet == Module::SUCCESS) {
            HttpRequest req;
            Json::Value jsonReq;
            jsonReq[CAPACITY] = (Json::UInt64)(size * KB * TWO);
            jsonReq[ID] = volumeId;
            Json::FastWriter jsonWriter;
            req.method = "PUT";
            req.url = "lun/expand";
            req.body = jsonWriter.write(jsonReq);
            Json::Value data;
            std::string errorDes;
            int errorCode;
            int iRet = SendRequest(req, data, errorDes, errorCode);
            if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
                return Module::SUCCESS;
            } else {
                return Module::FAILED;
            }
        } else {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Lun " << ResourceName << " does not exist." << HCPENDLOG;
            return Module::FAILED;
        }
    }

    int DoradoBlock::ParseCookie(const std::set<std::string> &cookie_values, SessionInfo &sessionInfo)
    {
        if (cookie_values.empty()) {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Cookie is empty." << HCPENDLOG;
            return Module::FAILED;
        }

        std::string cookie_value = *cookie_values.begin();

        std::vector<std::string> strs;
        (void) boost::split(strs, cookie_value, boost::is_any_of(";"));
        if (strs.size() > 0) {
            sessionInfo.cookie = strs[0];
            return Module::SUCCESS;
        }
        return Module::FAILED;
    }

    int DoradoBlock::SendHttpReq(
        std::shared_ptr<IHttpResponse> &rsp, const HttpRequest &req, std::string &errorDes, int &errorCode)
    {
        HttpRequest tempReq = req;
        tempReq.url = FormatFullUrl(tempReq.url);
        if (m_enableProxy || GetEnableProxy()) {
            tempReq.enableProxy = true;
        }
        rsp = fs_pHttpCLient->SendRequest(tempReq, CurlTimeOut);
        if (NULL == rsp.get()) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Return response is empty. " << HCPENDLOG;
            return Module::FAILED;
        }
        if (!rsp->Success()) {
            // 1.curl success,http response error with http status codes
            if (rsp->GetErrCode() == 0) {
                errorDes = rsp->GetHttpStatusDescribe(); // http status error description
                errorCode = rsp->GetErrCode();
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Curl ok,HttpStatusCode: " << rsp->GetHttpStatusCode()
                                                 << " , Http response error. Error is " << errorDes << HCPENDLOG;
                return rsp->GetHttpStatusCode(); // return http status code
                // 2.curl error,need directly retry
            } else {
                errorDes = rsp->GetErrString();
                errorCode = rsp->GetErrCode();
                HCP_Log(ERR, DORADO_MODULE_NAME) << " Curl error. errorCode: " << errorCode
                                                 << "errorDes:" << errorDes << HCPENDLOG;
            }
            SetErrorCode(errorCode);
            return Module::FAILED;
            // 3. curl success, http response success
        } else {
            errorDes = rsp->GetErrString();
            errorCode = rsp->GetErrCode();
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "curl success and http success " << HCPENDLOG;
            return Module::SUCCESS;
        }
        return Module::SUCCESS;
    }

    int DoradoBlock::SendRequestOnce(HttpRequest req, Json::Value &data, Module::RestResult& result)
    {
        if (this->sessionPtr == nullptr) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Invalid session" << HCPENDLOG;
            return Module::FAILED;
        }
        int iRet = Module::FAILED;
        HttpRequest request = req;
        // 存储存在两种接口：
        // https://8.40.97.155:8088/deviceManager/rest/2102354PBB10MC100006/USER_DATATURBO
        // https://8.40.97.155:8088/api/v2/task/add_dataturbo_auth
        if (req.url.find("api/v2") != 0) {
            request.url = curl_http + DoradoIP + ":" + DoradoPort + "/deviceManager/rest/" + this->sessionPtr->deviceId
                          + "/" + req.url;
        } else {
            request.url = curl_http + DoradoIP + ":" + DoradoPort + "/" + req.url;
        }

        (void) request.heads.insert(std::make_pair(std::string("Cookie"), this->sessionPtr->cookie));
        (void) request.heads.insert(std::make_pair(std::string("iBaseToken"), this->sessionPtr->token));

        SetErrorCode(DORADO_ERROR_CODE_OK);
        std::shared_ptr<IHttpResponse> rsp;
        iRet = SendHttpReq(rsp, request, result.errDesc, result.errorCode);
        if (iRet != Module::SUCCESS) {
            // get when curl send success,http response error for httpstatuscodeforRetry
            if (result.errorCode == 0) {
                return iRet;
            }
            return Module::FAILED;
        }
        return ResponseSuccessHandle(req, rsp, data, result);
    }

    bool DoradoBlock::OperateIpRule(const std::string& ip, const std::string podIp,
        const IpRuleOperation& operation,  int &errorCode)
    {
        DBGLOG("enter operate ip rule for ip : %s", ip.c_str());
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["task_type"] = "backup";
        jsonReq["destination_ip"] = ip;
        req.url = "https://" + podIp + ":30173/v1/internal/deviceManager/rest/ip_rule/";
        INFOLOG("req.url : %s", req.url.c_str());
        req.method = "POST";
        req.isVerify = INTERNAL_VERIFY;
        if (operation == IpRuleOperation::ADD) {
            req.url += "add";
        } else if (operation == IpRuleOperation::DELETE) {
            req.url += "delete";
        } else {
            ERRLOG("IpRuleOperation failed");
            return false;
        }
        Json::FastWriter jsonWriter;
        req.body = jsonWriter.write(jsonReq);
        Json::Value rsp;
        bool iRet = SendHttpRequest(req, rsp, TIME_OUT, false);
        if (!iRet || (!rsp.isMember("error") && !rsp["error"].isMember("code") &&
            rsp["error"]["code"].asInt() != 0)) {
            errorCode = rsp["error"]["code"].asInt();
            ERRLOG("add or delete ip %s failed, error :%d", ip.c_str(), errorCode);
            return false;
        }
        DBGLOG("Add or delete ip route for %s success!", ip.c_str());
        return true;
    }

    int DoradoBlock::ResponseSuccessHandle(HttpRequest req,
                                           std::shared_ptr<IHttpResponse> &rsp, Json::Value &data,
                                           Module::RestResult& result)
    {
        int Ret = ParseResponse(rsp->GetBody(), data, result);
        SetErrorCode(result.errorCode);
        SetExtendInfo(result.errDesc);
        if (result.errorCode == DoradoErrorCode::UNAUTH ||
            result.errorCode == DoradoErrorCode::NOUSERPERMISSION ||
            result.errorCode == DoradoErrorCode::AUTHIPINCONSISTENCY) {
            SessionInfo sessionInfo = Login();
            if (sessionInfo.device_id.empty() || sessionInfo.token.empty() || sessionInfo.cookie.empty()) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Login Dorado Failed! deviceId: " << sessionInfo.device_id
                                                 << HCPENDLOG;
                return Module::FAILED;
            }
            if (this->sessionPtr == nullptr) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Invalid session" << HCPENDLOG;
                return Module::FAILED;
            }
            // Refresh session
            this->sessionPtr->deviceId = sessionInfo.device_id;
            this->sessionPtr->cookie = sessionInfo.cookie;
            this->sessionPtr->token = sessionInfo.token;
            HttpRequest request = req;
            if (req.url.find("api/v2") != 0) {
                request.url =
                        curl_http + DoradoIP + ":" + DoradoPort + "/deviceManager/rest/" + this->sessionPtr->deviceId
                        + "/" + req.url;
            } else {
                request.url = curl_http + DoradoIP + ":" + DoradoPort + "/" + req.url;
            }
            (void) request.heads.insert(std::make_pair(std::string("Cookie"), this->sessionPtr->cookie));
            (void) request.heads.insert(std::make_pair(std::string("iBaseToken"), this->sessionPtr->token));
            Ret = SendHttpReq(rsp, request, result.errDesc, result.errorCode);
            if (Ret != Module::SUCCESS) {
                return Module::FAILED;
            }

            Ret = ParseResponse(rsp->GetBody(), data, result);
        }
        return Ret;
    }

    void DoradoBlock::SetRetryAttr(int _retryTimes, int _retryIntervalTime)
    {
        retryTimes = _retryTimes;
        retryIntervalTime = _retryIntervalTime;
        HCP_Log(INFO, DORADO_MODULE_NAME) << "set retry times: " << retryTimes << HCPENDLOG;
    }

    void DoradoBlock::SetCurlTimeOut(uint64_t tmpTimeOut)
    {
        if (tmpTimeOut > MIN_CURL_TIME_OUT) {
            CurlTimeOut = tmpTimeOut;
        }
    }

    void DoradoBlock::DelayTimeSendRequest()
    {
        auto now = std::chrono::steady_clock::now();
        while ((double(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() -
                                                                             now).count()) *
                std::chrono::microseconds::period::num / std::chrono::microseconds::period::den) <
               retryIntervalTime) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Waiting for storage device ... " << HCPENDLOG;
            sleep(1);
        }
        return;
    }

    void DoradoBlock::DeleteDeviceSession()
    {
        m_sessionCache->DeleteSession(DoradoIP, DoradoUsername, DoradoPort,
                                      [this](SessionInfo sesInfo) -> int { return Logout(sesInfo); });
    }

    void DoradoBlock::CreateDeviceSession()
    {
        this->sessionPtr = m_sessionCache->CreateSession(DoradoIP, DoradoUsername, DoradoPort,
                                                         [this]() -> SessionInfo { return Login(); });
    }

    void DoradoBlock::LoginAndGetSessionInfo()
    {
        if (useCache && m_sessionCache != nullptr) {
            CreateDeviceSession();
        } else {
            SessionInfo sessionInfo = Login();
            if (sessionInfo.token.empty() || sessionInfo.cookie.empty() || sessionInfo.device_id.empty()) {
                this->sessionPtr = nullptr;
            } else {
                this->sessionPtr = std::make_shared<Session>(sessionInfo.token, sessionInfo.device_id,
                                                             sessionInfo.cookie);
            }
        }
        return;
    }

    // 对已经拼接好的请求体发送http请求，将响应体放进data
    int DoradoBlock::SendRequestSpliced(HttpRequest &req, Json::Value &data, std::string &errorDes)
    {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "SendHttpRequest ,req has been spliced. " << req.url << HCPENDLOG;
        int iRet = FAILED;
        if (m_enableProxy || GetEnableProxy()) {
            req.enableProxy = true;
        }
        std::shared_ptr<IHttpResponse> rsp = fs_pHttpCLient->SendRequest(req);
        if (NULL == rsp.get()) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Return response is empty. " << HCPENDLOG;
            return FAILED;
        }
        if (!rsp->Success()) {
            if (rsp->GetErrCode() != 0) {
                errorDes = rsp->GetHttpStatusDescribe();
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Http response error. Error  is " << errorDes << HCPENDLOG;
            } else {
                errorDes = rsp->GetErrString();
                HCP_Log(ERR, DORADO_MODULE_NAME)
                    << "Send http request occur network error. Error  is " << errorDes << HCPENDLOG;
            }
            return iRet;
        }
        
        Json::Reader reader;
        if (!reader.parse((std::string)rsp->GetBody(), data)) {
            errorDes = "Parse json string failed";
            return FAILED;
        }
        return SUCCESS;
    }
    
    // 功能：将从PM获取到token、cookie和deviceId，赋值给this->sessionPtr
    void DoradoBlock::LoginAndGetSessionInfoFromPM()
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Get session from PM" << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = URL_GET_TOKEN_IP;
        req.isVerify = INTERNAL_VERIFY;
        Module::RestResult result;
        Json::Value data;
        SessionInfo sessionInfo{};
        if (SendRequestSpliced(req, data, result.errDesc) == SUCCESS) {
            if (data.isMember("sessionToken") && data.isMember("sessionCookie") && data.isMember("deviceId")) {
                sessionInfo.token = data["sessionToken"].asString();
                sessionInfo.cookie = data["sessionCookie"].asString();
                sessionInfo.device_id = data["deviceId"].asString();
                HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Get Session from PM successed!" << HCPENDLOG;
                this->sessionPtr = std::make_shared<Session>(sessionInfo.token,
                    sessionInfo.device_id, sessionInfo.cookie);
            } else {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Get session from PM failed, errorDes: "
                    << result.errDesc << HCPENDLOG;
            }
        }
        if (data.isMember("error")) {
            if (GetErrorCodeAndDesFromBody(data, result, "error") != Module::SUCCESS) {
                HCP_Log(WARN, DORADO_MODULE_NAME) << "Get error failed, data" << data.asString() << HCPENDLOG;
            }
        } else if (data.isMember("result")) {
            if (GetErrorCodeAndDesFromBody(data, result, "result") != Module::SUCCESS) {
                HCP_Log(WARN, DORADO_MODULE_NAME) << "Get result failed, data" << data.asString() << HCPENDLOG;
            }
        }
        SetErrorCode(result.errorCode);
        SetExtendInfo(result.errDesc);
        for (std::string::iterator it = req.body.begin(); it != req.body.end(); ++it) {
            *it = 0;
        }
        return;
    }

    int DoradoBlock::SendRequest(HttpRequest &req, Json::Value &data, std::string &errorDes,
        int &errorCode, bool lockSession)
    {
        Module::RestResult result;
        int ret = SendRequest(req, data, result, lockSession);
        errorDes = result.errDesc;
        errorCode = result.errorCode;
        return ret;
    }

    int DoradoBlock::SendRequest(HttpRequest &req, Json::Value &data, Module::RestResult& result, bool lockSession)
    {
        // 检查存储设备是否含有证书和吊销列表信息
        if (!certification.empty()) {
            req.cert = certification;
            req.isVerify = VCENTER_VERIFY;
        }
        req.revocationList = !crl.empty() ? crl : req.revocationList;
        int retryNum = 0;
        while (retryNum < retryTimes) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "send request for " << (retryNum + 1)
                                              << " time to " << WIPE_SENSITIVE(req.url) << HCPENDLOG;
            int ret = Module::SUCCESS;
            if (this->sessionPtr == nullptr) {
                if (DoradoIP == INNER_SAFE_IP) {
                    LoginAndGetSessionInfoFromPM();
                } else {
                    LoginAndGetSessionInfo();
                }
                if (this->sessionPtr == nullptr) {
                    HCP_Log(ERR, DORADO_MODULE_NAME) << "Invalid session" << HCPENDLOG;
                }
            }
            curl_http = "https://";
            if (DoradoIP == INNER_SAFE_IP) {
                req.isVerify = DO_NOT_VERIFY;
            }
            if (this->sessionPtr != nullptr) {
                if (lockSession) {
                    std::lock_guard<std::mutex> lock(this->sessionPtr->sessionMutex);
                    ret = SendRequestOnce(req, data, result);
                } else {
                    ret = SendRequestOnce(req, data, result);
                }
                if (ret == Module::SUCCESS) {
                    HCP_Log(INFO, DORADO_MODULE_NAME) << "send requests success " << HCPENDLOG;
                    return Module::SUCCESS;
                }
            }
            // 1.when curl success and ret not Module::FAILED, ret is httpStatusCode,so judge whether ret is
            // in httpRspStatusCodeForRetry for retry.2.when when curl success and ret is Module::FAILED,
            // DoradoResposeNeedRetry, not judge http retry code, directly retry.3.when errorCode not 0,
            // mean curl failed,directly retry.
            if (result.errorCode == 0 && !DoradoResposeNeedRetry(ret) &&
                std::find(httpRspStatusCodeForRetry.begin(), httpRspStatusCodeForRetry.end(), ret)
                == httpRspStatusCodeForRetry.end()) {
                HCP_Log(INFO, DORADO_MODULE_NAME) << "not retry send msg for httpstatuscode:" << ret << HCPENDLOG;
                break;
            }
            DelayTimeSendRequest();
            retryNum++;
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "send request failed. " << HCPENDLOG;
        return Module::FAILED;
    }

    int DoradoBlock::QuerySnapshot(std::string SnapshotName, int &id, std::string &WWN)
    {
        if (SnapshotName.length() > MAX_LENGTH) {
            SnapshotName = SnapshotName.substr(0, MAX_LENGTH);
        }
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query snapshot " << SnapshotName << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "SNAPSHOT?filter=NAME::" + SnapshotName;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data.size() > 0) {
            id = std::stoi(data[0]["ID"].asString());
            WWN = data[0]["WWN"].asString();
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Query Snapshot success.  " << SnapshotName << HCPENDLOG;
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::QuerySnapshotDescription(std::string SnapshotName, std::string &desp)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query snapshot " << SnapshotName << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "SNAPSHOT?filter=NAME::" + SnapshotName;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data.size() > 0) {
            desp = data[0]["DESCRIPTION"].asString();
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    std::unique_ptr<ControlDevice> DoradoBlock::CreateSnapshot(std::string SnapshotName, int &errorCode)
    {
        int id;
        std::string sWwn;
        std::string originalName = SnapshotName;
        // the max length of the LUN name is 32
        if (SnapshotName.length() > MAX_LENGTH) {
            SnapshotName = SnapshotName.substr(0, MAX_LENGTH);
        }
        std::string desp;
        int ret = QuerySnapshotDescription(SnapshotName, desp);
        if (ret == Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Duplicate snapshot description: " << SnapshotName << HCPENDLOG;
            return nullptr;
        }
        if (desp == originalName) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Duplicate snapshot name: " << SnapshotName << HCPENDLOG;
            return nullptr;
        }
        ControlDeviceInfo deviceInfo = {};
        HCP_Log(INFO, DORADO_MODULE_NAME)
                << "Start create snapshot " << SnapshotName << " for " << ResourceName << HCPENDLOG;
        int mpRet = QuerySnapshot(SnapshotName, id, sWwn);
        if (mpRet == Module::SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "already exist snapshot. " << SnapshotName << HCPENDLOG;
            return nullptr;
        } else {
            HttpRequest req;
            Json::Value jsonReq;
            jsonReq["PARENTID"] = ResourceId;
            jsonReq["NAME"] = SnapshotName;
            jsonReq["DESCRIPTION"] = originalName;
            Json::FastWriter jsonWriter;
            req.method = "POST";
            req.url = "snapshot";
            req.body = jsonWriter.write(jsonReq);
            Json::Value data;
            std::string errorDes;
            int iRet = SendRequest(req, data, errorDes, errorCode);
            if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
                mpRet = QuerySnapshot(SnapshotName, id, sWwn);
                if (mpRet != Module::SUCCESS) {
                    HCP_Log(ERR, DORADO_MODULE_NAME)
                            << "Query snapshot after creation " << SnapshotName << " failed" << HCPENDLOG;
                    return nullptr;
                }
            }
        }
        AssignDeviceInfo(deviceInfo, SnapshotName);
        return std::make_unique<DoradoBlockSnapshot>(deviceInfo, id, sWwn);
    }

    void DoradoBlock::AssignDeviceInfo(ControlDeviceInfo &deviceInfo, std::string SnapshotName)
    {
        deviceInfo.deviceName = SnapshotName;
        deviceInfo.url = DoradoIP;
        deviceInfo.port = DoradoPort;
        deviceInfo.userName = DoradoUsername;
        deviceInfo.password = DoradoPassword;
        deviceInfo.poolId = DoradoPoolId;
    }

// Writable Snapshot
    std::unique_ptr<ControlDevice> DoradoBlock::CreateClone(std::string volumeName, int &errorCode)
    {
        unsigned long long size;
        unsigned long long usedSize;
        int id;
        std::string wwn;
        if (ResourceName.length() > MAX_LENGTH) {
            ResourceName = ResourceName.substr(0, MAX_LENGTH);
        }
        if (volumeName.length() > MAX_LENGTH) {
            volumeName = volumeName.substr(0, MAX_LENGTH);
        }
        int ret = QueryLUN(ResourceName, ResourceId, wwn, size, usedSize);
        if (ret != Module::SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "lun " << ResourceName << " does not exist" << HCPENDLOG;
            return nullptr;
        }
        ControlDeviceInfo deviceInfo = {};
        HCP_Log(INFO, DORADO_MODULE_NAME)
                << "Start create clone " << volumeName << " from volume " << ResourceName << HCPENDLOG;
        return CreateSnapshot(volumeName, errorCode);
    }

    int DoradoBlock::QueryServiceHost(std::vector<std::string> &iscsiList, IP_TYPE ipType)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query iscsi host " << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "GET";
        req.url = "lif";
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            IterateIscsiHost(data, iscsiList, ipType);
            return Module::SUCCESS;
        } else {
            return Module::FAILED;
        }
    }

    int DoradoBlock::IterateIscsiHost(Json::Value data, std::vector<std::string> &iscsiList, IP_TYPE ipType)
    {
        for (int i = 0; i < data.size(); i++) {
            Json::Value oneNode = data[i];

            if (oneNode["SUPPORTPROTOCOL"].asString() == SUPPORTPROTOCOL_ISCSI) {
                Json::Value iscsiPortalList;
                if (ipType == IP_TYPE::IP_V4) {
                    iscsiPortalList = oneNode["IPV4ADDR"];
                } else if (ipType == IP_TYPE::IP_V6) {
                    iscsiPortalList = oneNode["IPV6ADDR"];
                }

                std::string ipPort = iscsiPortalList.asString() + ":3260";
                iscsiList.push_back(ipPort);
                HCP_Log(INFO, DORADO_MODULE_NAME) << "push back iscsiPortal " << ipPort << HCPENDLOG;
            }
        }
        return Module::SUCCESS;
    }


    int DoradoBlock::GetTheServiceHost(std::string &iscsiIP)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start get the usable iscsi host " << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "GET";
        req.url = "lif";
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            for (int i = 0; i < data.size(); i++) {
                Json::Value oneNode = data[i];
                if (oneNode["SUPPORTPROTOCOL"].asString() == SUPPORTPROTOCOL_ISCSI &&
                    oneNode["RUNNINGSTATUS"].asString() == RUNNINGSTATUS_LINKUP) {
                    Json::Value iscsiPortal = oneNode["IPV4ADDR"];
                    iscsiIP = iscsiPortal.asString();
                    HCP_Log(INFO, DORADO_MODULE_NAME) << "Find the iscsiIP." << HCPENDLOG;
                    return Module::SUCCESS;
                }
            }
        }

        HCP_Log(ERR, DORADO_MODULE_NAME) << "Can not find the usable iscsiIP." << HCPENDLOG;
        return Module::FAILED;
    }

    void DoradoBlock::DoScanAfterAttach(const std::string &wwn, std::string &lunPath)
    {
        std::string cmd = "sh /usr/bin/rescan-scsi-bus.sh";
        std::vector<std::string> paramList;
        std::vector<std::string> cmdoutput;
        std::vector<std::string> stderroutput;
        int iRet = runShellCmdWithOutput(DEBUG, DORADO_MODULE_NAME, 0, cmd, paramList, cmdoutput, stderroutput);
        if (iRet != 0) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Excute rescan-scsi-bus.sh filed " << HCPENDLOG;
        }

        cmdoutput.clear();
        stderroutput.clear();
        paramList.clear();
        cmd = "ls -l /dev/disk/by-id/ | grep wwn-0x" + wwn;
        iRet = runShellCmdWithOutput(DEBUG, DORADO_MODULE_NAME, 0, cmd, paramList, cmdoutput, stderroutput);
        if ((iRet == 0) && (cmdoutput.size() > 0) && (cmdoutput[0].length() > 0)) {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << " cmdoutput[0]=" << cmdoutput[0] << HCPENDLOG;
            std::vector<std::string> results;
            boost::algorithm::split(results, cmdoutput[0], boost::is_any_of("/"));
            if (results.size() > TWO) {
                lunPath = "/dev/" + results[TWO];
            }
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << " lunPath =" << lunPath << HCPENDLOG;
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Scan Lun filed " << HCPENDLOG;
    }

    std::string DoradoBlock::ScanLunAfterAttach(std::string &lunID)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start scan lun after attach" << HCPENDLOG;
        std::string lunPath = "";
        DeviceDetails lunInfo;
        lunInfo.deviceId = std::stoi(lunID);
        std::string errDes;
        int iRet = GetLunInfoByID(lunInfo, errDes);
        if (iRet != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Get lun info by id failed,errDes is " << errDes << HCPENDLOG;
            return lunPath;
        }

        DoScanAfterAttach(lunInfo.deviceUniquePath, lunPath);
        return lunPath;
    }

    int DoradoBlock::QuerySnapshotList(std::vector<FSSnapshotInfo> &snapshots)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query snapshot list" << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "SNAPSHOT";
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            for (int i = 0; i < data.size(); i++) {
                FSSnapshotInfo item{};
                Json::Value onesnap = data[i];
                item.snapshotName = onesnap["NAME"].asString();
                item.volumeName = onesnap["PARENTNAME"].asString();
                snapshots.push_back(item);
            }
            return Module::SUCCESS;
        }
        return Module::FAILED;
    }

    int DoradoBlock::Revert(std::string SnapshotName)
    {
        int id;
        std::string WWN;
        int ret = QuerySnapshot(SnapshotName, id, WWN);
        if (ret == Module::SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Start revert snapshot " << SnapshotName << HCPENDLOG;
            HttpRequest req;
            Json::Value jsonReq;
            jsonReq["ID"] = id;
            Json::FastWriter jsonWriter;
            req.method = "PUT";
            req.url = "snapshot/rollback";
            req.body = jsonWriter.write(jsonReq);
            Json::Value data;
            std::string errorDes;
            int errorCode;
            int iRet = SendRequest(req, data, errorDes, errorCode);
            if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
                HCP_Log(INFO, DORADO_MODULE_NAME) << "Revert Success. " << HCPENDLOG;
                return Module::SUCCESS;
            } else {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Revert Failed. " << HCPENDLOG;
                return Module::FAILED;
            }
        } else {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Snapshot does not exist. " << HCPENDLOG;
        }
    }

    int DoradoBlock::QueryRevertInfo(
        const std::string &resourceName, std::string &rollbackRate, std::string &rollbackStatus)
    {
        return Module::SUCCESS;
    }

    int DoradoBlock::SendRequestEx(HttpRequest &req, Json::Value &data, std::string &errorDes,
                                   int &errorCode, SessionInfo &sessionInfo)
    {
        int iRet = Module::FAILED;
        if (req.method == "DELETE") {
            req.url = "https://" + DoradoIP + ":" + DoradoPort + "/deviceManager/rest/" +
                      sessionInfo.device_id + "/" + req.url;
        } else {
            req.url = "https://" + DoradoIP + ":" + DoradoPort + "/deviceManager/rest" + "/xxxxx/" + req.url;
        }
        (void) req.heads.insert(std::make_pair(std::string("Cookie"), sessionInfo.cookie));
        (void) req.heads.insert(std::make_pair(std::string("iBaseToken"), sessionInfo.token));
        req.url = FormatFullUrl(req.url);
        if (DoradoIP == INNER_SAFE_IP) {
            req.isVerify = DO_NOT_VERIFY;
        }
        if (m_enableProxy || GetEnableProxy()) {
            req.enableProxy = true;
        }
        std::shared_ptr<IHttpResponse> rsp = fs_pHttpCLient->SendRequest(req);
        if (NULL == rsp.get()) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Return response is empty. " << HCPENDLOG;
            return Module::FAILED;
        }
        if (!rsp->Success()) {
            if (rsp->GetErrCode() != 0) {
                errorDes = rsp->GetHttpStatusDescribe();
                HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Http response error. Error  is " << errorDes << HCPENDLOG;
            } else {
                errorDes = rsp->GetErrString();
                HCP_Log(DEBUG, DORADO_MODULE_NAME)
                        << "Send http request occur network error. Error  is " << errorDes << HCPENDLOG;
            }
            return iRet;
        } else {
            if (req.method == "DELETE") {
                return ParseResponse(rsp->GetBody(), data, errorDes, errorCode);
            }
            iRet = ParseCookie(rsp->GetCookies(), sessionInfo);
            if (iRet != Module::SUCCESS) {
                return iRet;
            }
            return ParseResponse(rsp->GetBody(), data, errorDes, errorCode);
        }
    }

    bool DoradoBlock::GetEnableProxy()
    {
        auto configReader = ConfigReaderImpl::instance();
        std::string backupScene = configReader->GetBackupSceneFromXml("backup_scene");
        return backupScene == "0" ? false : true;
    }

    int DoradoBlock::isNeedRetryErrorCode(const int &errorCode)
    {
        int size = sizeof(g_noNeedRetryErrorCode) / sizeof(int);
        for (int i = 0; i < size; i++) {
            if (errorCode == g_noNeedRetryErrorCode[i]) {
                return Module::FAILED;
            }
        }
        return Module::SUCCESS;
    }

    void DoradoBlock::InitHttpStatusCodeForRetry()
    {
        ConfigReader::getIntValueVector("MicroService", "HttpStatusCodesForRetry", ",",
                                        httpRspStatusCodeForRetry);
    }

    bool DoradoBlock::DoradoResposeNeedRetry(const int ret)
    {
        // when errorCode ==0 && ret == Module::FAILED mean dorado response need retry
        return (ret == Module::FAILED) ? true : false;
    }

    int DoradoBlock::ParseResponse(const std::string &json_data, Json::Value &data,
        std::string &errorDes, int &errorCode)
    {
        Module::RestResult result;
        int ret = ParseResponse(json_data, data, result);
        errorDes = result.errDesc;
        errorCode = result.errorCode;
        return ret;
    }

    int DoradoBlock::ParseResponse(
        const std::string &json_data, Json::Value &data, Module::RestResult& result)
    {
        Json::Value jsonValue;
        Json::Reader reader;
        int iRet = Module::SUCCESS;
        if (!reader.parse(json_data, jsonValue)) {
            result.errDesc = "Parse json string failed";
            return Module::FAILED;
        }
        if (jsonValue.isMember("error")) {
            iRet = GetErrorCodeAndDesFromBody(jsonValue, result, "error");
            if (iRet != Module::SUCCESS) {
                return Module::FAILED;
            }
        } else if (jsonValue.isMember("result")) {
            iRet = GetErrorCodeAndDesFromBody(jsonValue, result, "result");
            if (iRet != Module::SUCCESS) {
                return Module::FAILED;
            }
        }

        if (jsonValue.isMember("data")) {
            data = jsonValue["data"];
        }
        return Module::SUCCESS;
    }

    int DoradoBlock::GetErrorCodeAndDesFromBody(const Json::Value &jsonValue, Module::RestResult& result,
        const std::string key)
    {
        if (!jsonValue.isMember(key) || !jsonValue[key].isMember("code") ||
            !jsonValue[key].isMember("description")) {
            result.errDesc = "Json object format is error. ";
            return Module::FAILED;
        }

        result.errDesc = jsonValue[key]["description"].asString();
        result.errorCode = jsonValue[key]["code"].asInt();
        result.errSuggestion = jsonValue[key]["suggestion"].asString();
        if (result.errorCode != Module::SUCCESS && isNeedRetryErrorCode(result.errorCode) == Module::SUCCESS) {
            HCP_Log(WARN, DORADO_MODULE_NAME)
                    << "code : " << result.errorCode << ", Describe : " << result.errDesc
                    << ", suggestion: " << result.errSuggestion << HCPENDLOG;
            return Module::FAILED;
        }
        return Module::SUCCESS;
    }

    bool DoradoBlock::GetJsonValue(const Json::Value &jsValue, std::string strKey, std::string &strValue)
    {
        if (jsValue.isArray()) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Json is Array." << HCPENDLOG;
            return false;
        }
        if (jsValue.isMember(strKey)) {
            if (jsValue[strKey].isString()) {
                strValue = jsValue[strKey].asString();
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Get Json value failed." << HCPENDLOG;
                return true;
            } else {
                HCP_Log(ERR, DORADO_MODULE_NAME)
                        << "The value Json key is not string." << HCPENDLOG;
                return false;
            }
        }
        strValue = "";
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Json key does not exist." << HCPENDLOG;
        return false;
    }

    int DoradoBlock::GetLunInfoByName(DeviceDetails &info, std::string &errDes)
    {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Start get lun info: " << info.deviceName << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "lun?filter=NAME::" + info.deviceName;
        Json::Value data;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && !data.empty() && data.isArray()) {
            info.deviceUniquePath = data[0]["WWN"].asString();
            std::string strTotalCap = data[0]["CAPACITY"].asString();
            std::string strUsedCap = data[0]["ALLOCCAPACITY"].asString();
            std::string strId = data[0]["ID"].asString();
            if (!checkStringIsDigit(strTotalCap) || !checkStringIsDigit(strUsedCap) || !checkStringIsDigit(strId)) {
                return Module::FAILED;
            }
            info.deviceId = boost::lexical_cast<int>(strId);
            info.totalCapacity = boost::lexical_cast<uint64_t>(strTotalCap);
            info.totalCapacity *= CAPACITY_COEFFICIENT;
            info.usedCapacity = boost::lexical_cast<uint64_t>(strUsedCap);
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Lun info: " << DBG(info.deviceId) << DBG(info.deviceUniquePath)
                                               << DBG(info.totalCapacity) << DBG(info.usedCapacity) << HCPENDLOG;
            return Module::SUCCESS;
        }
        return Module::FAILED;
    }

    int DoradoBlock::GetLunInfoByID(DeviceDetails &info, std::string &errDes)
    {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Start get lun info: " << info.deviceId << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "lun/" + std::to_string(info.deviceId);
        Json::Value data;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && !data.empty() && data.isArray()) {
            info.deviceName = data["NAME"].asString();
            info.deviceUniquePath = data["WWN"].asString();
            std::string strTotalCap = data[0]["CAPACITY"].asString();
            std::string strUsedCap = data[0]["ALLOCCAPACITY"].asString();
            if (!checkStringIsDigit(strTotalCap) || !checkStringIsDigit(strUsedCap)) {
                return Module::FAILED;
            }
            info.totalCapacity = boost::lexical_cast<uint64_t>(strTotalCap);
            info.totalCapacity *= CAPACITY_COEFFICIENT;
            info.usedCapacity = boost::lexical_cast<uint64_t>(strUsedCap);
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Lun info: " << DBG(info.deviceName) << DBG(info.deviceUniquePath)
                                               << DBG(info.totalCapacity) << DBG(info.usedCapacity) << HCPENDLOG;
            return Module::SUCCESS;
        }
        return Module::FAILED;
    }

    int DoradoBlock::GetLunInfoByWWN(DeviceDetails &info, std::string &errDes)
    {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Start get lun info: " << info.deviceUniquePath << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "lun?filter=WWN::" + info.deviceUniquePath;
        Json::Value data;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && !data.empty() && data.isArray()) {
            info.deviceName = data[0]["NAME"].asString();
            std::string strTotalCap = data[0]["CAPACITY"].asString();
            std::string strUsedCap = data[0]["ALLOCCAPACITY"].asString();
            std::string strId = data[0]["ID"].asString();
            if (!checkStringIsDigit(strTotalCap) || !checkStringIsDigit(strUsedCap) || !checkStringIsDigit(strId)) {
                return Module::FAILED;
            }
            info.deviceId = boost::lexical_cast<int>(strId);
            info.totalCapacity = boost::lexical_cast<uint64_t>(strTotalCap);
            info.totalCapacity *= CAPACITY_COEFFICIENT;
            info.usedCapacity = boost::lexical_cast<uint64_t>(strUsedCap);
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Lun info: " << DBG(info.deviceId) << DBG(info.deviceUniquePath)
                                               << DBG(info.totalCapacity) << DBG(info.usedCapacity) << HCPENDLOG;
            return Module::SUCCESS;
        }
        return Module::FAILED;
    }

    int DoradoBlock::GetSnapshotAllocDiffBitmap(
        const std::string &objectId, SnapshotDiffBitmap &diffBitmap, std::string &errDes)
    {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Start get Snapshot alloc diffBitmap: " << objectId << HCPENDLOG;
        Timer timer;
        diffBitmap.bitmap.clear();
        const unsigned long long getBitmapSizeOneTime = ONE_MB * MAX_BITMAP_LENGTH_ONCE;
        const unsigned long long getBitmapTimes = (diffBitmap.size + getBitmapSizeOneTime - 1) / getBitmapSizeOneTime;

        unsigned long long offset = diffBitmap.offset;
        for (unsigned long long i = 0; i < getBitmapTimes; i++, offset += getBitmapSizeOneTime) {
            unsigned long long size = (offset + getBitmapSizeOneTime) > (diffBitmap.offset + diffBitmap.size)
                                      ? (diffBitmap.offset + diffBitmap.size - offset)
                                      : getBitmapSizeOneTime;
            std::string bitmap;
            bool bCheck = ((offset + size) <= (diffBitmap.offset + diffBitmap.size)) && (size <= getBitmapSizeOneTime);
            if (bCheck == false) {
                HCP_Log(ERR, DORADO_MODULE_NAME)
                        << "Param invalid." << DBG(offset) << DBG(size) << DBG(diffBitmap.chunkSize)
                        << DBG(diffBitmap.offset)
                        << DBG(diffBitmap.size) << HCPENDLOG;
                return Module::FAILED;
            }
            SnapshotDiffBitmap subDiffBitmap;
            subDiffBitmap.offset = offset;
            subDiffBitmap.size = size;
            subDiffBitmap.chunkSize = diffBitmap.chunkSize;
            int ret = GetSnapshotAllocDiffBitmapImp(objectId, subDiffBitmap, errDes);
            if (ret != Module::SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME)
                        << "Get Snapshot alloc diffBitmap failed." << DBG(objectId) << DBG(offset) << DBG(size)
                        << DBG(diffBitmap.chunkSize) << DBG(errDes) << DBG(ret) << HCPENDLOG;
                return ret;
            }
            diffBitmap.bitmap += subDiffBitmap.bitmap;
        }

        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Get Snapshot alloc diffBitmap success." << DBG(objectId)
                                           << DBG(diffBitmap.offset) << DBG(diffBitmap.size)
                                           << DBG(diffBitmap.chunkSize)
                                           << DBG(diffBitmap.bitmap) << DBG(timer.DurationAsMicroSeconds())
                                           << HCPENDLOG;
        return Module::SUCCESS;
    }

    int DoradoBlock::GetSnapshotAllocDiffBitmapImp(
        const std::string &objectId, SnapshotDiffBitmap &diffBitmap, std::string &errDes)
    {
        HttpRequest req;
        req.method = "GET";
        req.url =
                "lun_bitmap/get_alloc?BITMAPCHUNKSIZE=" + std::to_string(diffBitmap.chunkSize) + "&LUNID=" + objectId +
                "&SEGMENTLENGTHBYTES=" + std::to_string(diffBitmap.size) +
                "&SEGMENTSTARTOFFSETBYTES=" + std::to_string(diffBitmap.offset);
        Json::Value data;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data.size() > 0) {
            diffBitmap.bitmap = data["CHUNKBITMAP"].asString();
            return Module::SUCCESS;
        }
        return Module::FAILED;
    }

    int DoradoBlock::GetSnapshotUnsharedDiffBitmap(
        const std::string &objectId, const std::string &parentObjectId, SnapshotDiffBitmap &diffBitmap,
        std::string &errDes)
    {
        HCP_Log(DEBUG, DORADO_MODULE_NAME)
                << "Start get Snapshot Unshared diffBitmap: " << DBG(objectId) << DBG(parentObjectId) << HCPENDLOG;
        Timer timer;
        diffBitmap.bitmap.clear();
        const unsigned long long getBitmapSizeOneTime = ONE_MB * MAX_BITMAP_LENGTH_ONCE;
        const unsigned long long getBitmapTimes = (diffBitmap.size + getBitmapSizeOneTime - 1) / getBitmapSizeOneTime;

        unsigned long long offset = diffBitmap.offset;
        for (unsigned long long i = 0; i < getBitmapTimes; i++, offset += getBitmapSizeOneTime) {
            unsigned long long size = (offset + getBitmapSizeOneTime) > (diffBitmap.offset + diffBitmap.size)
                                      ? (diffBitmap.offset + diffBitmap.size - offset)
                                      : getBitmapSizeOneTime;
            std::string bitmap;
            bool bCheck = ((offset + size) <= (diffBitmap.offset + diffBitmap.size)) && (size <= getBitmapSizeOneTime);
            if (bCheck == false) {
                HCP_Log(ERR, DORADO_MODULE_NAME)
                        << "Param invalid." << DBG(offset) << DBG(size) << DBG(diffBitmap.chunkSize)
                        << DBG(diffBitmap.offset)
                        << DBG(diffBitmap.size) << HCPENDLOG;
                return Module::FAILED;
            }
            SnapshotDiffBitmap subDiffBitmap;
            subDiffBitmap.offset = offset;
            subDiffBitmap.size = size;
            subDiffBitmap.chunkSize = diffBitmap.chunkSize;
            int ret = GetSnapshotUnsharedDiffBitmapImp(objectId, parentObjectId, subDiffBitmap, errDes);
            if (ret != Module::SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME)
                        << "Get Snapshot alloc diffBitmap failed." << DBG(objectId) << DBG(parentObjectId)
                        << DBG(offset)
                        << DBG(size) << DBG(diffBitmap.chunkSize) << DBG(errDes) << DBG(ret) << HCPENDLOG;
                return ret;
            }
            diffBitmap.bitmap += subDiffBitmap.bitmap;
        }

        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Get Snapshot alloc diffBitmap success." << DBG(objectId)
                                           << DBG(parentObjectId) << DBG(diffBitmap.offset) << DBG(diffBitmap.size)
                                           << DBG(diffBitmap.chunkSize) << DBG(diffBitmap.bitmap)
                                           << DBG(timer.DurationAsMicroSeconds()) << HCPENDLOG;
        return Module::SUCCESS;
    }

    int DoradoBlock::GetSnapshotUnsharedDiffBitmapImp(
        const std::string &objectId, const std::string &parentObjectId, SnapshotDiffBitmap &diffBitmap,
        std::string &errDes)
    {
        HttpRequest req;
        req.method = "GET";
        req.url = "lun_bitmap/get_unshared?BASELUNID=" + parentObjectId + "&LUNID=" + objectId +
                  "&BITMAPCHUNKSIZE=" + std::to_string(diffBitmap.chunkSize) +
                  "&SEGMENTLENGTHBYTES=" + std::to_string(diffBitmap.size) +
                  "&SEGMENTSTARTOFFSETBYTES=" + std::to_string(diffBitmap.offset);
        Json::Value data;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data.size() > 0) {
            diffBitmap.bitmap = data["CHUNKBITMAP"].asString();
            return Module::SUCCESS;
        }
        return Module::FAILED;
    }

    int DoradoBlock::CheckReplicationPair(int lunId, std::string devId, std::string &pairId)
    {
        HttpRequest req;
        req.method = "GET";
        req.url = "replicationpair/associate?ASSOCIATEOBJTYPE=11&ASSOCIATEOBJID=" + std::to_string(lunId);
        Json::Value data;
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != Module::SUCCESS || errorCode != Module::SUCCESS) {
            return Module::FAILED;
        } else {
            for (int i = 0; i < data.size(); i++) {
                Json::Value pair = data[i];
                if (pair["REMOTEDEVICEID"].asString() == devId) {
                    pairId = pair["ID"].asString();
                    HCP_Log(DEBUG, DORADO_MODULE_NAME) << "found replication pair with remote device " << devId
                                                       << ", lunId " << lunId << " success." << HCPENDLOG;
                    return Module::SUCCESS;
                }
            }
            HCP_Log(ERR, DORADO_MODULE_NAME) << "not found replication pair with remote device " << devId << ", lunId "
                                             << lunId << "." << HCPENDLOG;
            return Module::FAILED;
        }
    }

    int DoradoBlock::CreateReplication(
        int lunId, int rLunId, std::string rDevId, int bandwidth, std::string &pairId)
    {
        int ret = CheckReplicationPair(lunId, rDevId, pairId);
        if (ret == Module::SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME)
                    << "already exist replication pair on remote device " << rDevId << ", lunId " << lunId << "."
                    << HCPENDLOG;
            return Module::SUCCESS;
        }
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["LOCALRESID"] = lunId;
        /*  11: LUN
            40: file system */
        jsonReq["LOCALRESTYPE"] = LUN;
        jsonReq["REMOTEDEVICEID"] = rDevId;
        jsonReq["REMOTERESID"] = rLunId;
        jsonReq["bandwidth"] = bandwidth;
        /*  1: Manual.
            2: Timed wait after synchronization begins.
            3: Timed wait after synchronization ends.
            4: Specify the time policy. */
        jsonReq["SYNCHRONIZETYPE"] = REPLICATION_MANUAL;

        if (bandwidth != 0) {
            jsonReq["bandwidth"] = bandwidth;
        } else {
            /*  1: low
                2: medium
                3: high
                4: highest */
            jsonReq["SPEED"] = HIGHEST;
        }
        /*  1: "RECOVER_AUTOMATIC": automatic replication
            2: "RECOVER_MANUAL": manual replication */
        jsonReq["RECOVERYPOLICY"] = RECOVER_MANUAL;
        jsonReq["ENABLECOMPRESS"] = true;
        /*  1: synchronous replication
            2: asynchronous replication */
        jsonReq["REPLICATIONMODEL"] = ASYNCHRONOUS_REPLICATION;
        /*  0:"NOT_SYNC_SNAP": User snapshots are not synchronized.
            1:"SAME_AS_SOURCE": Snapshots on the secondary storage system must be the same as
                those on the primary storage system.
            2:"USER_SNAP_RETENTION_NUM": A specified number of user snapshots are retained
                on the secondary storage system. */
        jsonReq["syncSnapPolicy"] = USER_SNAP_RETENTION_NUM;
        jsonReq["userSnapRetentionNum"] = SAN_SNAPSHOT_NUM;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "replicationpair";
        req.body = jsonWriter.write(jsonReq);
        std::string errDes;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != Module::SUCCESS || errorCode != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "create replication pair failed!" << HCPENDLOG;
            return Module::FAILED;
        } else {
            pairId = data["ID"].asString();
            HCP_Log(INFO, DORADO_MODULE_NAME) << "create replication pair successed!" << HCPENDLOG;
            return Module::SUCCESS;
        }
    }

    int DoradoBlock::ActiveReplication(std::string pairId)
    {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["ID"] = pairId;
        Json::FastWriter jsonWriter;
        req.method = "put";
        req.url = "REPLICATIONPAIR/sync";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        std::string errDes;
        SetErrorCode(DORADO_ERROR_CODE_OK);
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != Module::SUCCESS || errorCode != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "start sync replication pair " << pairId << " failed!" << HCPENDLOG;
            SetErrorCode(errorCode);
            return Module::FAILED;
        } else {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "start sync replication pair " << pairId << " success!" << HCPENDLOG;
            return Module::SUCCESS;
        }
    }

    int DoradoBlock::QueryReplication(ReplicationPairInfo &replicationPairInfo)
    {
        HttpRequest req;
        Json::Value data;
        req.method = "GET";
        req.url = "REPLICATIONPAIR/" + replicationPairInfo.pairID;
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != Module::SUCCESS || errorCode != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "query replication pair: "
                                             << replicationPairInfo.pairID << " details failed!" << HCPENDLOG;
            return errorCode;
        } else {
            /*  1: Normal
                23: Synchronizing
                33: To be recovered
                34: Interrupted
                26: Split
                35: Invalid
                110: Standby */
            replicationPairInfo.status = std::stoi(data["RUNNINGSTATUS"].asString());
            replicationPairInfo.bandWidth = std::stoll(data["bandwidth"].asString());
            replicationPairInfo.progress = std::stoi(
                (data["REPLICATIONPROGRESS"].asString() == "") ? "-1" : data["REPLICATIONPROGRESS"].asString());
            replicationPairInfo.secresDataStatus = std::stoi(data["SECRESDATASTATUS"].asString());
            replicationPairInfo.remoteResID = data["REMOTERESID"].asString();
            replicationPairInfo.remoteResName = data["REMOTERESNAME"].asString();
            if (data["ISPRIMARY"].asString() == "true") {
                replicationPairInfo.isPrimary = true;
            } else {
                replicationPairInfo.isPrimary = false;
            }
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "query replication pair: "
                                               << replicationPairInfo.pairID << " details success!" << HCPENDLOG;
            return Module::SUCCESS;
        }
    }

    int DoradoBlock::UpdateReplication(int bandwidth, std::string pairId)
    {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        if (bandwidth == 0) {
            jsonReq["SPEED"] = HIGHEST;
        } else {
            jsonReq["bandwidth"] = bandwidth;
        }
        Json::FastWriter jsonWriter;
        req.method = "PUT";
        req.url = "REPLICATIONPAIR/" + pairId;
        req.body = jsonWriter.write(jsonReq);
        std::string errDes;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != Module::SUCCESS || errorCode != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Update replication pair " << pairId << " failed!" << HCPENDLOG;
            return Module::FAILED;
        } else {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Update replication pair " << pairId << " success!" << HCPENDLOG;
            return Module::SUCCESS;
        }
    }

    int DoradoBlock::DeleteReplication(std::string pairId)
    {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["ID"] = pairId;
        jsonReq["ISLOCALDELETE"] = false;
        Json::FastWriter jsonWriter;
        req.method = "DELETE";
        req.url = "REPLICATIONPAIR/";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != Module::SUCCESS || errorCode != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Delete replication pair " << pairId << " failed!" << HCPENDLOG;
            return Module::FAILED;
        } else {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Delete replication pair " << pairId << " success!" << HCPENDLOG;
            return Module::SUCCESS;
        }
    }

    int DoradoBlock::GetSnapShoCoupleUuid(const std::string snapShotId, std::string &coupleUuid, std::string &errDes)
    {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Start get snapshot coupleUuid: " << snapShotId << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "snapshot/" + snapShotId;
        Json::Value data;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data.size() > 0) {
            coupleUuid = data["coupleUuid"].asString();
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "SnapShot info: " << DBG(snapShotId) << DBG(coupleUuid)
                                               << HCPENDLOG;
            return Module::SUCCESS;
        } else {
            return errorCode;
        }
    }

    int DoradoBlock::GetSnapshotByCoupleUuid(const std::string coupleUuid, std::string &snapShotId,
        std::string &errDes)
    {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Start get snapshot by coupleUuid: " << coupleUuid << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "get_snapshot_by_couple_uuid?coupleUuid=" + coupleUuid;
        Json::Value data;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS && data.size() > 0) {
            snapShotId = data["id"].asString();
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "SnapShot info: " << DBG(coupleUuid) << DBG(snapShotId)
                                               << HCPENDLOG;
            return Module::SUCCESS;
        } else {
            return errorCode;
        }
    }

    int DoradoBlock::SplitReplication(std::string pairId)
    {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["ID"] = pairId;
        Json::FastWriter jsonWriter;
        req.method = "put";
        req.url = "REPLICATIONPAIR/split";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        std::string errDes;
        SetErrorCode(DORADO_ERROR_CODE_OK);
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != Module::SUCCESS || errorCode != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "start split replication pair " << pairId << " failed!" << HCPENDLOG;
            SetErrorCode(errorCode);
            return Module::FAILED;
        } else {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "start split replication pair " << pairId << " success!" << HCPENDLOG;
            return Module::SUCCESS;
        }
    }

    int DoradoBlock::CreateRemoteDeviceUser(const std::string &userName, const std::string &passWord)
    {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["NAME"] = userName;
        jsonReq["DESCRIPTION"] = "Read only user";
        jsonReq["PASSWORD"] = passWord;
        jsonReq["ROLEID"] = "12";
        jsonReq["SCOPE"] = 0;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "user";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        SetErrorCode(DORADO_ERROR_CODE_OK);
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != Module::SUCCESS || errorCode != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Create Remote Device User " << " failed!" << HCPENDLOG;
            SetErrorCode(errorCode);
            return Module::FAILED;
        } else {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Create Remote Device User " << " success!" << HCPENDLOG;
            return Module::SUCCESS;
        }
    }

    int DoradoBlock::CreateRemoteDevice(
        std::string localPort, std::string remoteIP, std::string remoteUser, std::string remotePassWord,
        std::string &devicdID)
    {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["ARRAYTYPE"] = "1";
        jsonReq["FASTWRITEENABLED"] = false;
        jsonReq["ISCSILINK_REMOTEPASSWORD"] = remotePassWord;
        jsonReq["ISCSILINK_REMOTEUSERNAME"] = remoteUser;
        jsonReq["LIF_PORT_NAME"] = localPort;
        jsonReq["LINKTYPE"] = DORADO_LINK_TYPE;
        jsonReq["REMOTEIP"] = remoteIP;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "remotedevice";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        SetErrorCode(DORADO_ERROR_CODE_OK);
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != Module::SUCCESS || errorCode != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Create Remote Device failed,error code is:  " << errorCode
                                             << HCPENDLOG;
            SetErrorCode(errorCode);
            return errorCode;
        }
        devicdID = data["ID"].asString();
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Create Remote Device" << devicdID << " success!" << HCPENDLOG;
        return Module::SUCCESS;
    }

    int DoradoBlock::GetLogicPortNameList(std::vector<std::string> &iscsiList)
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start get the LogicPortNameList " << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "GET";
        req.url = "lif";
        Json::Value data;
        std::string errorDes;
        int errorCode;
        SetErrorCode(DORADO_ERROR_CODE_OK);
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == Module::SUCCESS && errorCode == Module::SUCCESS) {
            for (int i = 0; i < data.size(); i++) {
                Json::Value oneNode = data[i];
                if (oneNode["ROLE"].asString() == PORT_ROLE_REPLICATION &&
                    oneNode["RUNNINGSTATUS"].asString() == RUNNINGSTATUS_LINKUP) {
                    Json::Value iscsiPortal = oneNode["NAME"];
                    iscsiList.push_back(iscsiPortal.asString());
                    HCP_Log(INFO, DORADO_MODULE_NAME) << "Find the iscsiIP name:  "
                                                      << iscsiPortal.asString() << HCPENDLOG;
                }
            }
        } else {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Can not find the usable iscsiIPnames:  " << HCPENDLOG;
            SetErrorCode(errorCode);
            return Module::FAILED;
        }

        return Module::SUCCESS;
    }

    int DoradoBlock::BindRepportgroupsToRemoteDevice(
        std::string devicdID, std::string localGroupId, std::string remoteGroupId)
    {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["LOCAL_REP_PORT_GROUP_ID"] = localGroupId;
        jsonReq["REMOTE_REP_PORT_GROUP_ID"] = remoteGroupId;
        Json::FastWriter jsonWriter;
        req.method = "PUT";
        req.url = "remote_device/" + devicdID;
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != Module::SUCCESS || errorCode != Module::SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "BindRepportgroupsToRemoteDevice failed:  " << errorCode << HCPENDLOG;
            return Module::FAILED;
        }
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "BindRepportgroupsToRemoteDevice " << devicdID << " success!"
                                           << HCPENDLOG;
        return Module::SUCCESS;
    }

int DoradoBlock::GetDoradoIp(std::string& doradoPanelIp, std::string& doradoPort)
{
    doradoPanelIp = k8s::GetConfigMapItem("common-conf", "dorado.mgrip");
    if (!doradoPanelIp.empty()) {
        doradoPort = OUT_DORADO_PORT;
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Query dorado management ip address from configmap! IP:"
            << doradoPanelIp << " doradoPort:" << doradoPort << HCPENDLOG;
    } else {
        // dorado env
        doradoPanelIp = INNER_SAFE_IP;
        doradoPort = INNER_SAFE_PORT;
        HCP_Log(INFO, DORADO_MODULE_NAME) << "get dorado ip:" << doradoPanelIp
            << " doradoPort:" << doradoPort << HCPENDLOG;
    }
    return Module::SUCCESS;
}

int DoradoBlock::GetDoradoIp(std::string& doradoPanelIp)
{
    // 1. if get ip from command line failed, try to get from config map
    doradoPanelIp = k8s::GetConfigMapItem("common-conf", "dorado.mgrip");
    if (!doradoPanelIp.empty()) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Query dorado management ip address from configmap! IP:"
            << doradoPanelIp << HCPENDLOG;
        return Module::SUCCESS;
    }

    // 2. get ip from command line
    return GetDoradoIpFromCommanLine(doradoPanelIp);
}

    int DoradoBlock::GetDoradoIpFromCommanLine(std::string &doradoPanelIp)
    {
        std::vector<std::string> paramList;
        std::vector<std::string> output;
        std::vector<std::string> erroutput;
        std::string cmd_common("ip route | grep 'cbri1600' | awk '{print $9}'");

        HCP_Log(INFO, DORADO_MODULE_NAME) << "Begin to query dorado management ip address from ip route." << HCPENDLOG;

        int ret = Module::runShellCmdWithOutput(INFO, DORADO_MODULE_NAME, 0, cmd_common, paramList,
                                                output, erroutput);
        if ((ret == 0) && (output.size() == 1)) {
            doradoPanelIp = output[0];
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Query dorado management ip address from ip route! IP:" <<
                                              doradoPanelIp << HCPENDLOG;
            return Module::SUCCESS;
        }

        std::string cmd_safe("ip route | grep 'via' | grep 'eth0' | grep -v 'default' | awk '{print $3}'");
        ret = Module::runShellCmdWithOutput(INFO, DORADO_MODULE_NAME, 0, cmd_safe, paramList, output, erroutput);
        if ((ret == 0) && (output.size() == 1)) {
            doradoPanelIp = output[0];
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Query dorado management ip address from ip route! IP:" <<
                                              doradoPanelIp << HCPENDLOG;
            return Module::SUCCESS;
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "get dorado ip failed." << HCPENDLOG;
        return Module::FAILED;
    }

    int DoradoBlock::QueryServiceIpController(
        std::vector<std::pair<std::string, std::string>> &ipControllerList, IP_TYPE ipType)
    {
        return Module::FAILED;
    }

    int DoradoBlock::DeleteSnapshot(std::string SnapshotName)
    {
        return Module::SUCCESS;
    }
}
