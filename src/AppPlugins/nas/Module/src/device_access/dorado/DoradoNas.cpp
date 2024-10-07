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
#include "device_access/dorado/DoradoNas.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <curl/curl.h>
#include "system/System.hpp"
#include "common/Path.h"
#include "common/Utils.h"
#include "device_access/dorado/DoradoNasSnapshot.h"
#include "device_access/Const.h"
#include "common/JsonUtils.h"
#include "device_access/k8s/K8sutil.h"

/*
Create fusionstorage Lun
Date : 2020/03/03
out params:id -> LUN id.
          :WWN  -> LUN WWN.
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.Create fusionstorage Lun
*/

namespace Module {
    namespace {
        const int RETRY_TIME = 30;
        constexpr int NUM_3 = 3;
        constexpr int FILESYSTEM = 40;
        constexpr int DORADO_LINK_TYPE = 2; // 使用用户名和密码方式鉴权链接
// The 8 KB reduction rate is good, and the 16 KB performance is good, 8 KB is the default value.
        const unsigned long long DEFAULT_VARIABLE_SEGMENT = 8;
// Only 32 KB is supported. The default value is 32 KB.
        const unsigned int SECTORSIZE = 32768;
        const int PARENTTYPE_NUM = 40;
        const std::string COMMON_CONF = "common-conf";
        const std::string DORADO_MGRIP = "dorado.mgrip";
    }

    DoradoNas::~DoradoNas() {
        if (MountInfo.autoUmount && !MountInfo.mountPoint.empty()) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Auto umount " << MountInfo.mountPoint << HCPENDLOG;
            UnMount();
        }
    }

    int DoradoNas::VerifyDoradoIP(std::string doradoIP) {
        SetDoradoIP(doradoIP);
        SetRetryAttr(1);
        DeviceDetails info;
        return Query(info);
    }

    int DoradoNas::QueryFileSystem(DeviceDetails &info) {
        int ret = QueryFileSystem(ResourceName, info);
        if (ret != SUCCESS) {
            return ret;
        }
        fileSystemId = std::to_string(info.deviceId);
        return SUCCESS;
    }

    int DoradoNas::GetDeviceInfo(const Json::Value &data, DeviceDetails &info) {
        info.deviceId = std::atoi(data[0]["ID"].asString().c_str());
        info.deviceName = data[0]["NAME"].asString();
        info.Compress = (data[0]["ENABLECOMPRESSION"].asString() == "true") ? true : false;
        info.Dedup = (data[0]["ENABLEDEDUP"].asString() == "true") ? true : false;
        int secStyle = std::atoi(data[0]["securityStyle"].asString().c_str());
        if (secStyle < static_cast<int>(NATIVE) || secStyle > static_cast<int>(UNIX)) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Not found filesystem SecurityStyle, DeviceName: " <<
                                             info.deviceName << "SecurityStyle: " << secStyle << HCPENDLOG;
            return FAILED;
        }
        info.securityStyle = static_cast<SecurityStyle>(secStyle);
        std::istringstream capa(data[0]["CAPACITY"].asString());
        capa >> info.totalCapacity;
        info.usedCapacity = 0;
        return SUCCESS;
    }

    int DoradoNas::QueryFileSystem(std::string fileName, DeviceDetails &info) {
        HttpRequest req;
        EncodeUrlFileName(fileName);
        req.method = "GET";
        req.url = "filesystem?filter=NAME::" + fileName;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && data.isArray() && data.size() > 0 && data[0].size() > 0) {
            if (GetDeviceInfo(data, info) != SUCCESS) {
                return FAILED;
            }
            if (data[0]["AVAILABLECAPCITY"].asString() != "") {
                std::istringstream availCapa(data[0]["AVAILABLECAPCITY"].asString());
                long long availableCapacity;
                availCapa >> availableCapacity;
                info.usedCapacity = info.totalCapacity - availableCapacity;
            }
            if (data[0]["MINSIZEFSCAPACITY"].asString() != "") {
                std::istringstream minSize(data[0]["MINSIZEFSCAPACITY"].asString());
                minSize >> info.minSizeOfFileSys;
                HCP_Log(DEBUG, DORADO_MODULE_NAME) << "minSizeOfFileSys: " << info.minSizeOfFileSys << HCPENDLOG;
            }
            if (!data[0]["RUNNINGSTATUS"].asString().empty()) {
                std::istringstream runningStatus(data[0]["RUNNINGSTATUS"].asString());
                runningStatus >> info.status;
                HCP_Logger_noid(DEBUG, DORADO_MODULE_NAME) << "running status: " << info.status << HCPENDLOG;
            }
            if (!data[0]["TOTALSAVEDCAPACITY"].asString().empty()) {
                std::istringstream totalSaveCapacity(data[0]["TOTALSAVEDCAPACITY"].asString());
                totalSaveCapacity >> info.totalSaveCapacity;
                HCP_Logger_noid(DEBUG, DORADO_MODULE_NAME) << "total save capacity: "
                                                           << info.totalSaveCapacity << HCPENDLOG;
            }
            if (!data[0]["OWNINGCONTROLLER"].asString().empty()) {
                std::istringstream ctler(data[0]["OWNINGCONTROLLER"].asString());
                ctler >> info.owningController;
                HCP_Logger_noid(DEBUG, DORADO_MODULE_NAME) << "owningcontroller: " << info.owningController
                                                           << HCPENDLOG;
            }
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    void DoradoNas::EncodeUrlFileName(std::string &fileName) {
        CURL *curlHandle = curl_easy_init();
        char *output = curl_easy_escape(curlHandle, fileName.c_str(), fileName.size());
        if (output != nullptr) {
            fileName = output;
            curl_free(output);
        }
        if (curlHandle != nullptr) {
            curl_easy_cleanup(curlHandle);
            curlHandle = nullptr;
        }
    }

    int DoradoNas::QueryFileSystemByID(const std::string &fileName, const std::string &fsID, DeviceDetails &info) {
        HttpRequest req;
        req.method = "GET";
        req.url = "filesystem?filter=NAME::" + fileName;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            return FAILED;
        }
        for (int i = 0; i < data.size(); i++) {
            Json::Value fsInfo = data[i];
            if (fsInfo["ID"].asString() == fsID) {
                info.deviceId = std::atoi(fsID.c_str());
                info.deviceName = fsInfo["NAME"].asString();
                info.Compress = (fsInfo["ENABLECOMPRESSION"].asString() == "true") ? true : false;
                info.Dedup = (fsInfo["ENABLEDEDUP"].asString() == "true") ? true : false;
                std::istringstream capa(fsInfo["CAPACITY"].asString());
                capa >> info.totalCapacity;
                info.usedCapacity = 0;
                if (fsInfo["AVAILABLECAPCITY"].asString() != "") {
                    std::istringstream availCapa(fsInfo["AVAILABLECAPCITY"].asString());
                    long long availableCapacity;
                    availCapa >> availableCapacity;
                    info.usedCapacity = info.totalCapacity - availableCapacity;
                }
                if (fsInfo["MINSIZEFSCAPACITY"].asString() != "") {
                    std::istringstream minSize(fsInfo["MINSIZEFSCAPACITY"].asString());
                    minSize >> info.minSizeOfFileSys;
                    HCP_Log(DEBUG, DORADO_MODULE_NAME) << "minSizeOfFileSys: " << info.minSizeOfFileSys << HCPENDLOG;
                }
                return SUCCESS;
            }
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Not found filesystem, name " << fileName
                                         << ", fsID " << fsID << "." << HCPENDLOG;
        return FAILED;
    }

    int DoradoNas::CreateFileSystem(unsigned long long size, SecurityStyle secStyle) {
        DeviceDetails info;
        int iRet = QueryFileSystem(info);
        if (iRet == SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "filesystem(" << ResourceName << ") has been exist!" << HCPENDLOG;
            return SUCCESS;
        }
        // distAlg parameter description: controller affinity mode(default)
        unsigned int distAlg = ConfigReader::getInt("General", "FileSystemSetDistAlg");
        if (isCapacityBalanceMode) {
            isCapacityBalanceMode = false;
            distAlg = ConfigReader::getInt("General", "CapacityBalanceModeDistAlg");
        }
        if (distAlg < 0 || distAlg > CONTROLLER_AFFINITY_MODE) {
            distAlg = CONTROLLER_AFFINITY_MODE;
        }
        HCP_Logger_noid(INFO, DORADO_MODULE_NAME) << "filesystem(" << ResourceName << ") DistAlg:" << distAlg
                                                  << HCPENDLOG;
        unsigned long long variableSegment = ConfigReader::getInt("General", "FileSystemSetVariableSegment");
        if (variableSegment < 0) {
            variableSegment = DEFAULT_VARIABLE_SEGMENT; // Default value: the 8 KB reduction rate is good.
        }
        HttpRequest req;
        req.method = "POST";
        req.url = "filesystem";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["ALLOCTYPE"] = Thin;
        jsonValue["NAME"] = ResourceName;
        jsonValue["PARENTID"] = DoradoPoolId;
        jsonValue["ENABLEDEDUP"] = Dedup;
        jsonValue["securityStyle"] = secStyle;
        std::string cmpress = Compress ? "1" : "0";
        jsonValue["COMPRESSION"] = cmpress;
        jsonValue["CAPACITY"] = (Json::UInt64)(size * KB * TWO);
        jsonValue["SECTORSIZE"] = SECTORSIZE; // Only 32 KB is supported. The default value is 32 KB.
        jsonValue["ISSHOWSNAPDIR"] = isShowSnapDir;
        jsonValue["distAlg"] = distAlg;
        jsonValue["variableSegment"] = static_cast<Json::UInt64>(variableSegment);
        jsonValue["fileSystemMode"] = 0; // 0: local, 1: HyperMetro.
        req.body = jsonWriter.write(jsonValue);
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "fileSystemMode=local." << HCPENDLOG;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && (errorCode == SUCCESS || errorCode == DoradoErrorCode::FILESYSTEMALREADYEXIST)) {
            return QueryFileSystem(info);
        }
        return errorCode;
    }

    int DoradoNas::Create(unsigned long long size) {
        return CreateFileSystem(size, UNIX);
    }

    int DoradoNas::Query(DeviceDetails &info) {
        return QueryFileSystem(info);
    }

    int DoradoNas::Bind(HostInfo &host, const std::string &shareId) {
        return FAILED;
    }

    int DoradoNas::UnBind(HostInfo host, const std::string &shareId) {
        HCP_Log(ERR, DORADO_MODULE_NAME) << "unsupport operation Create Clone for file system!" << HCPENDLOG;
        return FAILED;
    }

/*
create link clone volume with special snapshot
Date : 2020/03/03
out params:id -> link clone volume ID
          :WWN -> link clone volume WWN
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.create link clone volume with special snapshot
*/
    std::unique_ptr<ControlDevice> DoradoNas::CreateClone(std::string volumeName, int &errorCode) {
        return nullptr;
    }


    int DoradoNas::CreateCloneFileSystem(std::string volumeName, std::string &fsid) {
        HttpRequest req;
        req.method = "POST";
        req.url = "filesystem";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["NAME"] = volumeName;
        jsonValue["PARENTFILESYSTEMID"] = fileSystemId;
        jsonValue["ALLOCTYPE"] = "1";
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;

        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                   || errorCode == DoradoErrorCode::FILESYSTEMALREADYEXIST)) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Create clone file system finished!" << HCPENDLOG;
            DeviceDetails info;
            if (QueryFileSystem(volumeName, info) != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Query Clone FileSystem Failed!" << HCPENDLOG;
                return FAILED;
            }
            fsid = std::to_string(info.deviceId);
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

/*
delete volume with name
Date : 2020/03/03
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.delete volume with name
*/
    int DoradoNas::Delete() {
        return DeleteFileSystem();
    }

    int DoradoNas::DeleteFileSystem() {
        HttpRequest req;
        req.method = "DELETE";
        req.url = "filesystem/" + fileSystemId;
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;

        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS ||
                                   errorCode == DoradoErrorCode::FILESYSTEMIDNOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNas::QuerySnapshot(std::string SnapshotName, std::string &id) {
        int ret;
        DeviceDetails info;
        HttpRequest req;
        if (fileSystemId.empty()) {
            ret = QueryFileSystem(info);
            if (ret != SUCCESS) {
                HCP_Log(INFO, DORADO_MODULE_NAME) << "Query fileSytemID failed" << HCPENDLOG;
                return FAILED;
            }
        }
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Query fileSytemID = " << fileSystemId << HCPENDLOG;
        req.method = "GET";
        req.url = "FSSNAPSHOT?PARENTID=" + fileSystemId + "&filter=NAME::" + SnapshotName;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && data.isArray() && data.size() > 0 && data[0].size() > 0) {
            id = data[0]["ID"].asString();
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNas::DeleteSnapshot(std::string SnapshotName, std::string id) {
        HttpRequest req;
        req.method = "DELETE";
        req.url = "FSSNAPSHOT/" + id;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                   || errorCode == DoradoErrorCode::FSSNAPSHOT_NOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNas::DeleteSnapshotWithVstoreId(const std::string &SnapshotName, const std::string &id,
                                                   const std::string &vstoreId) {
        HttpRequest req;
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        jsonValue["vstoreId"] = vstoreId;
        req.body = jsonWriter.write(jsonValue);

        req.method = "DELETE";
        req.url = "FSSNAPSHOT/" + id;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS || errorCode == DoradoErrorCode::FSSNAPSHOT_NOTEXIST
            || errorCode == DoradoErrorCode::ORINGIN_FILESYSTEM_NOTEXSIT) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

/*
create snapshot for special volume
Date : 2020/03/03
out params:id -> snapshot ID
          :WWN -> snapshot WWN
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.create snapshot for special volume
*/
    std::unique_ptr<ControlDevice> DoradoNas::CreateSnapshot(std::string SnapshotName, int &errorCode) {
        std::string id;
        ControlDeviceInfo deviceInfo;
        deviceInfo.deviceName = SnapshotName;
        deviceInfo.url = DoradoIP;
        deviceInfo.port = DoradoPort;
        deviceInfo.userName = DoradoUsername;
        deviceInfo.password = DoradoPassword;
        deviceInfo.poolId = DoradoPoolId;
        deviceInfo.compress = Compress;
        deviceInfo.dedup = Dedup;
        int ret = QuerySnapshot(SnapshotName, id);
        if (ret == SUCCESS) {
            return std::make_unique<DoradoNasSnapshot>(deviceInfo, fileSystemId, "/" + ResourceName + "/", readK8s);
        }
        HttpRequest req;
        req.method = "POST";
        req.url = "FSSNAPSHOT";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;

        jsonValue["PARENTTYPE"] = PARENTTYPE_NUM;
        jsonValue["NAME"] = SnapshotName;
        jsonValue["PARENTID"] = fileSystemId;
        jsonValue["snapTag"] = "A8000";
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if ((iRet == SUCCESS && errorCode == SUCCESS) ||
            errorCode == DoradoErrorCode::FILESYSTEMSNAPSHOTEXIST) {
            return std::make_unique<DoradoNasSnapshot>(deviceInfo, fileSystemId, "/" + ResourceName + "/", readK8s);
        }
        HCP_Log(INFO, DORADO_MODULE_NAME) << "create snapshot failed: " << errorCode << HCPENDLOG;
        return nullptr;
    }

/*
query all iscsi host,need Manual open on DeviceManager.
Date : 2020/03/03
out params:iscsiList -> iscsi ip and port List,value like ["1.1.1.1:8000","1.1.1.2:8000"]
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.query all iscsi host,need Manual open on DeviceManager.
*/
    int DoradoNas::QueryServiceHost(std::vector<std::string> &ipList, IP_TYPE ipType) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query nfs host " << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "GET";
        req.url = "lif";
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data.isArray() && data.size() > 0 && data[0].size() > 0) {
            IterateNFSHost(data, ipList, ipType);
            return SUCCESS;
        }
        return errorCode;
    }

    bool DoradoNas::HandleHostData(Json::Value oneNode, IP_TYPE ipType, std::vector<LogicPortInfo> &logicPorts) {
        std::string proto = oneNode["SUPPORTPROTOCOL"].asString();
        if (proto != SUPPORTPROTOCOL_NFS_CIFS && proto != SUPPORTPROTOCOL_NFS
            && proto != SUPPORTPROTOCOL_CIFS) {
            return false;
        }

        std::string role = oneNode["ROLE"].asString();
        if (role != PORT_ROLE_SERVICE && role != PORT_ROLE_MANAGE_SERVICE) {
            return false;
        }

        std::string RunningStatus = oneNode["RUNNINGSTATUS"].asString();
        if (RunningStatus != RUNNINGSTATUS_LINKUP) {
            return false;
        }

        LogicPortInfo logicPort;
        if (ipType == IP_TYPE::IP_V4) {
            logicPort.IP = oneNode["IPV4ADDR"].asString();
            logicPort.gateway = oneNode["IPV4GATEWAY"].asString();
            logicPort.mask = oneNode["IPV4MASK"].asString();
        } else if (ipType == IP_TYPE::IP_V6) {
            logicPort.IP = oneNode["IPV6ADDR"].asString();
            logicPort.gateway = oneNode["IPV6GATEWAY"].asString();
            logicPort.mask = oneNode["IPV6MASK"].asString();
        }
        if (logicPort.IP == "") {
            HCP_Log(WARN, DORADO_MODULE_NAME) << "Failed to query nfs host." << HCPENDLOG;
            return false;
        }
        logicPort.dataProtocol = oneNode["SUPPORTPROTOCOL"].asString();
        logicPort.runStatus = oneNode["RUNNINGSTATUS"].asString();
        logicPort.activeStatus = oneNode["OPERATIONALSTATUS"].asString();
        logicPorts.push_back(logicPort);
        HCP_Log(INFO, DORADO_MODULE_NAME) << "push back nfs Portal " << logicPort.IP << HCPENDLOG;
        return true;
    }

    int DoradoNas::QueryNasShareClient(NasSharedInfo &info, std::string url, std::string type) {
        HttpRequest req;
        req.method = "GET";
        req.url = url;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query NAS share client info failed!" << HCPENDLOG;
            return (errorCode == 0) ? FAILED : errorCode;
        }

        for (int i = 0; i < data.size(); i++) {
            Json::Value oneNode = data[i];
            SharedClientInfo sharedClient;
            sharedClient.name = oneNode["NAME"].asString();
            if (type == "CIFS") {
                if (oneNode["ACCESSVAL"].asInt() == 1) {
                    sharedClient.authorityLevel = "Read-Write";
                } else {
                    sharedClient.authorityLevel = "Read-only";
                }
                sharedClient.type = oneNode["DOMAINTYPE"].asString();
            } else {
                if (oneNode["ACCESSVAL"].asString() == "1") {
                    sharedClient.authorityLevel = "Read-Write";
                } else {
                    sharedClient.authorityLevel = "Read-only";
                }
                sharedClient.type = "Host";
                if (sharedClient.name.find_first_of("@") != std::string::npos) {
                    sharedClient.type = "NetwrokGroup";
                }
            }
            sharedClient.ID = oneNode["ID"].asString();
            info.clients.push_back(sharedClient);
        }

        return SUCCESS;
    }

    int DoradoNas::QueryNasShare(std::vector<NasSharedInfo> &infos, std::string url, std::string type) {
        HttpRequest req;
        req.method = "GET";
        req.url = url;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query NAS info successfully"
                                             << HCPENDLOG;
            return (errorCode == 0) ? FAILED : errorCode;
        }
        for (int index = 0; index < data.size(); index++) {
            NasSharedInfo info;
            info.deviceDetail.deviceId = atoi(data[index]["ID"].asString().c_str());
            info.deviceDetail.deviceUniquePath = data[index]["SHAREPATH"].asString();
            if (type == "CIFS") {
                info.deviceDetail.deviceName = data[index]["NAME"].asString();
            }
            infos.push_back(info);
        }
        return SUCCESS;
    }

    int DoradoNas::QueryServiceHost(std::vector<LogicPortInfo> &logicPorts, IP_TYPE ipType) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query nfs host." << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "GET";
        req.url = "lif";
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS || !data.isArray() || data.size() == 0 ||
            data[0].size() <= 0) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Failed to query nfs host." << HCPENDLOG;
            return errorCode;
        }

        for (int i = 0; i < data.size(); i++) {
            Json::Value oneNode = data[i];
            if (!HandleHostData(oneNode, ipType, logicPorts)) {
                continue;
            }
        }
    }

    int
    DoradoNas::GetLifPort(std::vector<std::string> &ownCtlIP, std::vector<std::string> &otherCtlIP, IP_TYPE ipType) {
        HCP_Logger_noid(INFO, DORADO_MODULE_NAME) << "Enter..." << HCPENDLOG;
        DeviceDetails info = {};
        if (QueryFileSystem(info) != SUCCESS) {
            return FAILED;
        }
        std::vector<std::pair<std::string, std::string>> ipControllerList;
        if (QueryServiceIpController(ipControllerList, ipType) != SUCCESS) {
            return FAILED;
        }

        for (auto item : ipControllerList) {
            HCP_Logger_noid(DEBUG, DORADO_MODULE_NAME) << "item.first: " << item.first << info.owningController
                                                       << HCPENDLOG;
            if (info.owningController == item.second) {
                ownCtlIP.push_back(item.first);
            } else {
                otherCtlIP.push_back(item.first);
            }
        }
        return SUCCESS;
    }

    int DoradoNas::QueryServiceIpController(
            std::vector<std::pair<std::string, std::string>> &ipControllerList, IP_TYPE ipType) {
        Json::Value data;
        std::vector<std::string> ipList;
        int ret = QueryLIFPortList(ipList, data);
        if (ret != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME)
                    << "Query Dorado lif port failed! errorCode:" << ret << HCPENDLOG;
            return ret;
        }
        ret = GetLogicPortAndController(data, ipControllerList, ipType);
        if (ret != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME)
                    << "Filter Dorado lif port and controller failed! errorCode:" << ret << HCPENDLOG;
            return ret;
        }
        return SUCCESS;
    }

    void DoradoNas::GetLogicPortAndControllerAction(
            const Json::Value &oneNode, std::vector<std::pair<std::string, std::string>> &ipControllerList,
            IP_TYPE ipType) {
        std::string proto = oneNode["SUPPORTPROTOCOL"].asString();
        std::string role = oneNode["ROLE"].asString();
        std::string RunningStatus = oneNode["RUNNINGSTATUS"].asString();
        if (RunningStatus == RUNNINGSTATUS_LINKUP && (proto == SUPPORTPROTOCOL_NFS_CIFS
                                                      || proto == SUPPORTPROTOCOL_NFS) &&
            (role == PORT_ROLE_SERVICE || role == PORT_ROLE_MANAGE_SERVICE)) {
            Json::Value ip;
            if (ipType == IP_TYPE::IP_V4) {
                ip = oneNode["IPV4ADDR"];
            } else if (ipType == IP_TYPE::IP_V6) {
                ip = oneNode["IPV6ADDR"];
            }
            ipControllerList.emplace_back(ip.asString(), oneNode["HOMECONTROLLERID"].asString());
            HCP_Log(INFO, DORADO_MODULE_NAME) << "push back nfs Portal: " << ip.asString()
                                              << " ControllerID: " << oneNode["HOMECONTROLLERID"].asString()
                                              << HCPENDLOG;
        }
    }

    int DoradoNas::GetLogicPortAndController(
            const Json::Value &data, std::vector<std::pair<std::string, std::string>> &ipControllerList, IP_TYPE ipType) {
        if (!data.isArray()) {
            return FAILED;
        }
        for (auto oneNode : data) {
            if (!oneNode.isMember("SUPPORTPROTOCOL") || !oneNode.isMember("ROLE") ||
                !oneNode.isMember("RUNNINGSTATUS") ||
                !oneNode.isMember("HOMECONTROLLERID") || !oneNode.isMember("IPV4ADDR") ||
                !oneNode.isMember("IPV6ADDR")) {
                return FAILED;
            }
            GetLogicPortAndControllerAction(oneNode, ipControllerList, ipType);
        }
        if (ipControllerList.empty()) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "there is no data lif port normal!" << HCPENDLOG;
            return FAILED;
        }
        return SUCCESS;
    }

/*
query all iscsi host,need Manual open on DeviceManager.
Date : 2020/03/03
out params:iscsiList -> iscsi ip and port List,value like ["1.1.1.1:8000","1.1.1.2:8000"]
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.query all iscsi host,need Manual open on DeviceManager.
*/
    int DoradoNas::ExtendSize(unsigned long long size) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start change file system size!" << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq[CAPACITY] = (Json::UInt64)(size * KB * TWO);
        jsonReq[ID] = fileSystemId;
        Json::FastWriter jsonWriter;
        req.method = "PUT";
        req.url = "filesystem";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return SUCCESS;
        }
        return errorCode;
    }

    int DoradoNas::Revert(std::string SnapshotName) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start Revert" << HCPENDLOG;
        HttpRequest req;
        req.method = "PUT";
        req.url = "FSSNAPSHOT/ROLLBACK_FSSNAPSHOT";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        jsonValue["rollbackTargetObjName"] = SnapshotName;
        jsonValue["PARENTNAME"] = ResourceName;
        jsonValue["rollbackSpeed"] = NUM_3;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return SUCCESS;
        }
        return errorCode;
    }

    int
    DoradoNas::QueryRevertInfo(const std::string &resourceName, std::string &rollbackRate, std::string &rollbackStatus) {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Query Revert" << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "FSSNAPSHOT/QUERY_FS_SNAPSHOT_ROLLBACK?PARENTNAME=" + resourceName;
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            if (data.isObject() && data.isMember("rollbackRate") && data.isMember("rollbackStatus")) {
                rollbackRate = data["rollbackRate"].asString();
                rollbackStatus = data["rollbackStatus"].asString();
                return SUCCESS;
            }
        }
        return errorCode;
    }

    int DoradoNas::QueryRevertSnapshotId(
            const std::string &resourceName, std::string &snapshotId, std::string &rollbackRate, std::string &rollbackStatus) {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Query revert SanpshotId" << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "FSSNAPSHOT/QUERY_FS_SNAPSHOT_ROLLBACK?PARENTNAME=" + resourceName;
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            if (data.isObject() && data.isMember("rollbackTargetObjID") && data.isMember("rollbackRate") &&
                data.isMember("rollbackStatus")) {
                snapshotId = data["rollbackTargetObjID"].asString();
                rollbackRate = data["rollbackRate"].asString();
                rollbackStatus = data["rollbackStatus"].asString();
                return SUCCESS;
            }
        }
        return errorCode;
    }

    int DoradoNas::QuerySnapshotRollBackStatus(const std::string &fileSystemId, std::string &snapshotId,
                                                    std::string &rollbackRate, std::string &rollbackStatus,
                                                    std::string &endTime) {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Query Snapshot rollback status by SnapshotId" << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "FSSNAPSHOT/QUERY_FS_SNAPSHOT_ROLLBACK?PARENTID=" + fileSystemId;
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            if (data.isObject() && data.isMember("rollbackTargetObjID") && data.isMember("rollbackRate") &&
                data.isMember("rollbackStatus")) {
                snapshotId = data["rollbackTargetObjID"].asString();
                rollbackRate = data["rollbackRate"].asString();
                rollbackStatus = data["rollbackStatus"].asString();
                endTime = data["rollbackEndtime"].asString();
                return SUCCESS;
            }
        }
        return errorCode;
    }

    int DoradoNas::QuerySnapshotRollBackStatusV2(const std::string &fileSystemId, std::string &snapshotId,
                                                    std::string &rollbackRate, std::string &rollbackStatus,
                                                    std::string &endTime) {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Query Snapshot rollback status by SnapshotId" << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "FSSNAPSHOT/QUERY_FS_SNAPSHOT_ROLLBACK?PARENTID=" + fileSystemId;
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            if (data.isObject() && data.isMember("rollbackTargetObjID") && data.isMember("rollbackStatus")) {
                snapshotId = data["rollbackTargetObjID"].asString();
                rollbackStatus = data["rollbackStatus"].asString();
                endTime = data["rollbackEndtime"].asString();
            }
            // rollbackRate 是可选字段
            if (data.isMember("rollbackRate")) {
                rollbackRate = data["rollbackRate"].asString();
            }
            return SUCCESS;
        }
        return errorCode;
    }

    int DoradoNas::QuerySnapshotRollBackStatusV3(const std::string &fileSystemId, const std::string &snapshotName,
                                                    std::string &rollbackStatus, std::string &endTime) {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Query Snapshot rollback status by SnapshotName" << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "fssnapshot?PARENTID=" + fileSystemId + "&NAME=" + snapshotName + "&dtreeId=0";
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if(iRet == SUCCESS && errorCode == SUCCESS) {
            if (data.isObject() && data.isMember("rollbackTargetObjID") && data.isMember("rollbackStatus")) {
                rollbackStatus = data["rollbackStatus"].asString();
                endTime = data["rollbackEndtime"].asString();
            }
            return SUCCESS;
        }
        return errorCode;
    }

    int DoradoNas::QuerySnapshotRollBackStatusV4(
        const std::string &fileSystemId, std::string &snapshotId, std::string &rollbackStatus, std::string &endTime)
    {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Query Snapshot rollback status by SnapshotName" << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "fssnapshot/" + snapshotId;
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            if (data.isObject() && data.isMember("rollbackStatus") && data.isMember("rollbackEndtime")) {
                rollbackStatus = data["rollbackStatus"].asString();
                endTime = data["rollbackEndtime"].asString();
            }
            return SUCCESS;
        }
        return errorCode;
    }

    int DoradoNas::QuerySnapshotList(std::vector<FSSnapshotInfo> &snapshots) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query snapshot list" << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "SNAPSHOT";
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data.size() > 0) {
            for (int i = 0; i < data.size(); i++) {
                FSSnapshotInfo item{};
                Json::Value onesnap = data[i];
                item.snapshotName = onesnap["NAME"].asString();
                item.volumeName = onesnap["PARENTNAME"].asString();
                snapshots.push_back(item);
            }
            return SUCCESS;
        }
        return errorCode;
    }

    int DoradoNas::QueryContorllerCnt(int &outCnt) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query control cnt." << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "CONTROLLER/count";
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            if (data.isMember("COUNT") && data["COUNT"].isString()) {
                outCnt = std::stoi(data["COUNT"].asString());
                HCP_Log(INFO, DORADO_MODULE_NAME) << "Query controller cnt suc. Cnt: " << outCnt << HCPENDLOG;
                return SUCCESS;
            }
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Query controller cnt failed. ErrCode" << iRet << ", " << errorDes
                                         << HCPENDLOG;
        return FAILED;
    }

    int DoradoNas::QueryLIFPortList(std::vector<std::string> &ipList, Json::Value &data) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query nfs host " << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "GET";
        req.url = "lif";
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data.size() > 0) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }


    void DoradoNas::IterateNFSHost(Json::Value data, std::vector<std::string> &nfsIPList, IP_TYPE ipType) {
        for (int i = 0; i < data.size(); i++) {
            Json::Value oneNode = data[i];
            std::string proto = oneNode["SUPPORTPROTOCOL"].asString();
            std::string role = oneNode["ROLE"].asString();
            std::string RunningStatus = oneNode["RUNNINGSTATUS"].asString();
            if (RunningStatus == RUNNINGSTATUS_LINKUP && (proto == SUPPORTPROTOCOL_NFS_CIFS
                                                          || proto == SUPPORTPROTOCOL_NFS) && (role == PORT_ROLE_SERVICE
                                                                                               || role ==
                                                                                                  PORT_ROLE_MANAGE_SERVICE)) {
                Json::Value ip;
                if (ipType == IP_TYPE::IP_V4) {
                    ip = oneNode["IPV4ADDR"];
                } else if (ipType == IP_TYPE::IP_V6) {
                    ip = oneNode["IPV6ADDR"];
                }
                nfsIPList.push_back(ip.asString());
                HCP_Log(INFO, DORADO_MODULE_NAME) << "push back nfs Portal " << ip.asString() << HCPENDLOG;
            }
        }
    }

    int DoradoNas::CheckReplicationPair(int lunId, int &rfsId, std::string devId, std::string &pairId) {
        HttpRequest req;
        req.method = "GET";
        req.url = "REPLICATIONPAIR?filter=LOCALRESID::" + std::to_string(lunId) + "&range=[0-100]";
        Json::Value data;
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            return FAILED;
        } else {
            for (int i = 0; i < data.size(); i++) {
                Json::Value pair = data[i];
                if (pair["REMOTEDEVICEID"].asString() == devId) {
                    pairId = pair["ID"].asString();
                    auto strLunId = pair["REMOTERESID"].asString();
                    rfsId = boost::lexical_cast<int>(strLunId);
                    HCP_Log(DEBUG, DORADO_MODULE_NAME) << "found replication pair with remote device " << devId
                                                       << ", lunId " << lunId << " success." << HCPENDLOG;
                    return SUCCESS;
                }
            }
            HCP_Log(ERR, DORADO_MODULE_NAME) << "not found replication pair with remote device " << devId << ", lunId "
                                             << lunId << "." << HCPENDLOG;
            return FAILED;
        }
    }

    int DoradoNas::QueryReplicationPairIds(std::string lunId, std::vector<std::string> &pairIdList) {
        HttpRequest req;
        req.method = "GET";
        req.url = "REPLICATIONPAIR?filter=LOCALRESID::" + lunId + "&range=[0-100]";
        Json::Value data;
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            return FAILED;
        } else {
            for (int i = 0; i < data.size(); i++) {
                Json::Value pair = data[i];
                std::string pairId = pair["ID"].asString();
                pairIdList.push_back(pairId);
                HCP_Log(DEBUG, DORADO_MODULE_NAME) << "found replication pair id " << pairId << HCPENDLOG;
                return SUCCESS;
            }
            HCP_Log(ERR, DORADO_MODULE_NAME) << "not found replication pair id for filesystem ID " << lunId
                                             << HCPENDLOG;
            return FAILED;
        }
    }

    int DoradoNas::CreateReplication(
            int fsId, int &rfsId, std::string rDevId, int bandwidth, std::string &pairId) {
        int ret = CheckReplicationPair(fsId, rfsId, rDevId, pairId);
        if (ret == SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME)
                    << "already exist replication pair on remote device " << rDevId << ", fsId " << fsId << "."
                    << HCPENDLOG;
            ret = UpdateReplication(bandwidth, pairId);
            if (ret != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "UpdateReplication ERR" << HCPENDLOG;
                return FAILED;
            }
            return SUCCESS;
        }
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["LOCALRESID"] = fsId;
        jsonReq["LOCALRESTYPE"] = FILE_SYSTEM;
        jsonReq["REMOTEDEVICEID"] = rDevId;
        jsonReq["REMOTERESID"] = rfsId;
        jsonReq["RECOVERYPOLICY"] = RECOVER_MANUAL;
        jsonReq["SYNCHRONIZETYPE"] = REPLICATION_MANUAL;
        jsonReq["ENABLECOMPRESS"] = true;
        jsonReq["REPLICATIONMODEL"] = ASYNCHRONOUS_REPLICATION;
        jsonReq["syncSnapPolicy"] = USER_SNAP_RETENTION_NUM;
        jsonReq["userSnapRetentionNum"] = NAS_SNAPSHOT_NUM;
        if (bandwidth != 0) {
            jsonReq["bandwidth"] = bandwidth;
        } else {
            jsonReq["SPEED"] = HIGHEST;
        }
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "replicationpair";
        req.body = jsonWriter.write(jsonReq);
        std::string errDes;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "create nas replication pair failed!" << HCPENDLOG;
            return errorCode;
        } else {
            pairId = data["ID"].asString();
            HCP_Log(INFO, DORADO_MODULE_NAME) << "create nas replication pair successed!" << HCPENDLOG;
            return SUCCESS;
        }
    }

    int DoradoNas::CreateShare() {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Not support create share!" << HCPENDLOG;
        return FAILED;
    }

    int DoradoNas::GetUsablePoolID(unsigned long long size, std::string &poolId) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query pool list" << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "storagepool";
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            for (int i = 0; i < data.size(); i++) {
                Json::Value onePool = data[i];
                long long poolUsableSize = std::stoi(onePool["USERFREECAPACITY"].asString());
                if (onePool["RUNNINGSTATUS"].asString() == "27" && onePool["HEALTHSTATUS"].asString() == "1"
                    && poolUsableSize > size) {
                    poolId = onePool["ID"].asString();
                    HCP_Log(INFO, DORADO_MODULE_NAME) << "query pool successed!: " << poolId << HCPENDLOG;
                    return SUCCESS;
                }
            }
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Query pool failed!" << HCPENDLOG;
        return FAILED;
    }

    int DoradoNas::QueryReplicationPortIpInfo(std::vector<LogicalPorts> &ipList) {
        HttpRequest req;
        req.method = "GET";
        req.url = "lif";
        Json::Value data;
        int errorCode;
        std::string errDes;
        std::string roleId = "4";

        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Not found replication logical port " << HCPENDLOG;
            return FAILED;
        }
        for (int i = 0; i < data.size(); i++) {
            Json::Value port = data[i];
            if (port["ROLE"].asString() == roleId) {
                LogicalPorts logicPort;
                logicPort.name = port["NAME"].asString();
                logicPort.ipv4Address = port["IPV4ADDR"].asString();
                logicPort.ipv6Address = port["IPV6ADDR"].asString();
                ipList.push_back(logicPort);
                HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Found replicationlogical port " << logicPort.name
                                                   << ", ipv4address " << logicPort.ipv4Address
                                                   << ", ipv6address " << logicPort.ipv6Address
                                                   << " success." << HCPENDLOG;
            }
        }
        if (ipList.size() == 0) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Not found replication logical port " << HCPENDLOG;
            return FAILED;
        }
        return SUCCESS;
    }

    int DoradoNas::QueryReplicationRepportgroup(const std::string &groupName, std::string &groupId) {
        HttpRequest req;
        Json::Value data;
        req.method = "GET";
        req.url = "repportgroup";
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "QueryReplicationRepportgroup failed:  " << errorCode << HCPENDLOG;
            return FAILED;
        } else {
            for (int i = 0; i < data.size(); i++) {
                Json::Value oneNode = data[i];
                if (oneNode["NAME"].asString() == groupName) {
                    groupId = oneNode["ID"].asString();
                    return SUCCESS;
                }
            }
        }
        return FAILED;
    }

    int DoradoNas::CreateReplicationRepportgroup(
            const std::vector<std::string> &iscsiList, const std::string &groupName, std::string &groupId) {
        auto ret = QueryReplicationRepportgroup(groupName, groupId);
        if (ret == SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "QueryReplicationRepportgroup success" << HCPENDLOG;
            ret = AddReplicationRepportgroupMember(groupId, iscsiList);
            if (ret != SUCCESS) {
                HCP_Log(INFO, DORADO_MODULE_NAME) << "AddReplicationRepportgroupMember failed" << HCPENDLOG;
                return FAILED;
            }
            return SUCCESS;
        }

        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["NAME"] = groupName;
        std::string str = "";
        for (int i = 0; i < iscsiList.size(); i++) {
            if (i != iscsiList.size() - 1) {
                str += (iscsiList[i] + ",");
            } else {
                str += iscsiList[i];
            }
        }
        jsonReq["ETH_LOGICAL_PORT_LIST"] = str;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "repportgroup";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Crqeate ReplicationRepportgroup failed:  " << errorCode << HCPENDLOG;
            return FAILED;
        } else {
            groupId = data["ID"].asString();
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Create Repportgroup " << groupId << " success!" << HCPENDLOG;
            return SUCCESS;
        }
    }

    int DoradoNas::QueryNasSnapShotByName(std::string snapShotName, std::string parentId, std::string &snapShotId) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start query NasSnapShot" << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "fssnapshot?NAME=" + snapShotName + "&PARENTID=" + parentId + "&dtreeId=0";
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            snapShotId = data["ID"].asString();
            return SUCCESS;
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Query nas snapshot failed!" << HCPENDLOG;
        return FAILED;
    }

    int DoradoNas::GetFileSystemNameByID(DeviceDetails &info, std::string &errDes) {
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Start get lun info: " << info.deviceId << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "filesystem/" + std::to_string(info.deviceId);
        Json::Value data;
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data.size() > 0) {
            info.deviceName = data["NAME"].asString();
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "filesystem: " << DBG(info.deviceName) << HCPENDLOG;
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

    int DoradoNas::DeleteRemoteDevice(std::string devicdID, std::string &errDes) {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        req.method = "DELETE";
        req.url = "remote_device/" + devicdID + "?AUTO_FREE_PORT=1";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "DeleteRemoteDevice: " << devicdID << " success!" << HCPENDLOG;
            return SUCCESS;
        } else if (iRet != SUCCESS && errorCode == DoradoErrorCode::REMOTE_DEVICE_NOTEXIST) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << " Remote device not exist, DeleteRemoteDevice: " << devicdID
                                              << " success!" << HCPENDLOG;
            return SUCCESS;
        } else {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Delete RemoteDevice failed:  " << errorCode << HCPENDLOG;
            return FAILED;
        }
    }

    int DoradoNas::QueryRemoteDevice(std::string devicdID, std::string &localGroupId,
                                          std::string &remoteGroupId, std::string &remoteESN) {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        req.method = "GET";
        req.url = "remote_device/" + devicdID;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query RemoteDevice failed:  " << errorCode << HCPENDLOG;
            return FAILED;
        } else {
            localGroupId = data["LOCAL_REP_PORT_GROUP_ID"].asString();
            remoteGroupId = data["REMOTE_REP_PORT_GROUP_ID"].asString();
            remoteESN = data["SN"].asString();
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Query RemoteDevice: " << devicdID
                                              << ". localGroupId: " << localGroupId << ". remoteGroupId: "
                                              << remoteGroupId
                                              << ". remote ESN is: " << remoteESN << " success!" << HCPENDLOG;
            return SUCCESS;
        }
    }

    int DoradoNas::BatchQueryRemoteDevice(std::string esn, std::string &devicdID) {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        req.method = "GET";
        req.url = "remote_device?range=[0-100]";
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            for (int i = 0; i < data.size(); i++) {
                Json::Value oneRemoteDevice = data[i];
                if (oneRemoteDevice["SN"].asString() == esn) {
                    devicdID = oneRemoteDevice["ID"].asString();
                    HCP_Log(INFO, DORADO_MODULE_NAME) << "Find the RemoteDevice:  " << devicdID << HCPENDLOG;
                    return SUCCESS;
                }
            }
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Can not find the usable RemoteDevice. esn: " << esn << HCPENDLOG;
            return FAILED;
        } else {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Can not find the usable RemoteDevice. esn: " << esn
                                             << ". errorDes: " << errorDes << HCPENDLOG;
            return FAILED;
        }
    }

    int DoradoNas::BatchQueryRemoteDevice(std::string esn, std::string &devicdID,
                                               int &healthStatus, int &runStatus) {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        req.method = "GET";
        req.url = "remote_device?range=[0-100]";
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Can not find the usable RemoteDevice. esn: " << esn
                                             << ", errorCode: " << errorCode << ", errorDes: " << errorDes
                                             << ", request result: " << iRet << HCPENDLOG;
            return FAILED;
        }

        for (int i = 0; i < data.size(); i++) {
            Json::Value oneRemoteDevice = data[i];
            if (oneRemoteDevice["SN"].asString() == esn) {
                devicdID = oneRemoteDevice["ID"].asString();
                HCP_Log(INFO, DORADO_MODULE_NAME) << "Find the RemoteDevice:  " << devicdID << HCPENDLOG;
                std::istringstream healthStatusStream(oneRemoteDevice["HEALTHSTATUS"].asString());
                healthStatusStream >> healthStatus;
                HCP_Logger_noid(DEBUG, DORADO_MODULE_NAME) << "RemoteDevice health status: " << healthStatus
                                                           << HCPENDLOG;

                std::istringstream runningStatus(oneRemoteDevice["RUNNINGSTATUS"].asString());
                runningStatus >> runStatus;
                HCP_Logger_noid(DEBUG, DORADO_MODULE_NAME) << "RemoteDevice running status: " << runStatus << HCPENDLOG;
                return SUCCESS;
            }
        }
        return FAILED;
    }

    int DoradoNas::RelinkRemoteDevice(std::string localPort, std::string remoteIP, std::string remoteUser,
                                           std::string remotePassWord, std::string deviceID) {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["FASTWRITEENABLED"] = false;
        jsonReq["REMOTEPASSWORD"] = remotePassWord;
        jsonReq["REMOTEUSERNAME"] = remoteUser;
        jsonReq["LIF_PORT_NAME"] = localPort;
        jsonReq["LINKTYPE"] = DORADO_LINK_TYPE;
        jsonReq["REMOTEIP"] = remoteIP;
        Json::FastWriter jsonWriter;
        req.method = "PUT";
        req.url = "remote_device/" + deviceID;
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        SetErrorCode(DORADO_ERROR_CODE_OK);
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Relink Remote Device failed, error code is " << errorCode
                                             << ",errorDes: " << errDes << ", request result is" << iRet << HCPENDLOG;
            SetErrorCode(errorCode);
            return errorCode;
        }
        deviceID = data["ID"].asString();
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Relink Remote Device" << deviceID << " success!" << HCPENDLOG;
        return SUCCESS;
    }

    int DoradoNas::BatchQueryRemoteDevice(std::string &devicdID) {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        req.method = "GET";
        req.url = "remote_device?range=[0-100]";
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            for (int i = 0; i < data.size(); i++) {
                Json::Value oneRemoteDevice = data[i];
                devicdID = oneRemoteDevice["ID"].asString();
                HCP_Log(INFO, DORADO_MODULE_NAME) << "Find the usable RemoteDevice:  " << devicdID << HCPENDLOG;
                return SUCCESS;
            }
        } else {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Can not find the usable RemoteDevice. errorDes: " << errorDes
                                             << HCPENDLOG;
            return FAILED;
        }
    }

    int DoradoNas::BatchQueryAllRemoteDevice(std::vector<std::string> &devicdIDList) {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        req.method = "GET";
        req.url = "remote_device/";
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            for (int i = 0; i < data.size(); i++) {
                Json::Value oneRemoteDevice = data[i];
                auto devicdID = oneRemoteDevice["ID"].asString();
                devicdIDList.push_back(devicdID);
                HCP_Log(INFO, DORADO_MODULE_NAME) << "Find the usable RemoteDevice:  " << devicdID << HCPENDLOG;
                return SUCCESS;
            }
        } else {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Can not find the usable RemoteDevice:  " << HCPENDLOG;
            return FAILED;
        }
    }

    int DoradoNas::AddReplicationRepportgroupMember(std::string groupId, const std::vector<std::string> &iscsiList) {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["ID"] = groupId;
        std::string str = "";
        for (int i = 0; i < iscsiList.size(); i++) {
            if (i != iscsiList.size() - 1) {
                str += (iscsiList[i] + ",");
            } else {
                str += iscsiList[i];
            }
        }
        jsonReq["ETH_LOGICAL_PORT_LIST"] = str;
        Json::FastWriter jsonWriter;
        req.method = "PUT";
        req.url = "repportgroup/add";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (errorCode == DoradoErrorCode::NOTNEEDADDNUMBER) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Not need Add Repportgroup." << HCPENDLOG;
            return SUCCESS;
        }
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Add ReplicationRepportgroup member failed: " << errorCode << HCPENDLOG;
            return FAILED;
        } else {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Add Repportgroup: " << groupId << " member success!" << HCPENDLOG;
            return SUCCESS;
        }
    }

    int
    DoradoNas::RemoveReplicationRepportgroupMember(std::string groupId, const std::vector<std::string> &iscsiList) {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["ID"] = groupId;
        std::string str = "";
        for (int i = 0; i < iscsiList.size(); i++) {
            if (i != iscsiList.size() - 1) {
                str += (iscsiList[i] + ",");
            } else {
                str += iscsiList[i];
            }
        }
        jsonReq["ETH_LOGICAL_PORT_LIST"] = str;
        Json::FastWriter jsonWriter;
        req.method = "PUT";
        req.url = "repportgroup/remove";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Remove ReplicationRepportgroup member failed: " << errorCode
                                             << HCPENDLOG;
            return FAILED;
        } else {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Remove Repportgroup: " << groupId << " member success!" << HCPENDLOG;
            return SUCCESS;
        }
    }

    int DoradoNas::CheckPerformanceStatisticSwitch() {
        HttpRequest req;
        Json::Value data;
        Json::FastWriter jsonWriter;
        req.method = "Get";
        req.url = "performance_statistic_switch";
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            for (int i = 0; i < data.size(); i++) {
                if (data[i]["CMO_PERFORMANCE_SWITCH"].asString() == "1") {
                    HCP_Log(INFO, DORADO_MODULE_NAME) << "Check performance statistic switch success!" << HCPENDLOG;
                    return SUCCESS;
                }
            }
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Check performance statistic switch failed: " << errorCode << HCPENDLOG;
        return FAILED;
    }

    int DoradoNas::OpenPerformanceStatisticSwitch() {
        if (CheckPerformanceStatisticSwitch() == SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Performance statistic switch is open!" << HCPENDLOG;
            return SUCCESS;
        }
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        req.method = "PUT";
        req.url = "performance_statistic_switch";
        jsonReq["CMO_PERFORMANCE_SWITCH"] = "1";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Open performance statistic switch failed: " << errorCode << HCPENDLOG;
            return FAILED;
        } else {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Open performance statistic switch success!" << HCPENDLOG;
            return SUCCESS;
        }
    }

    int DoradoNas::QueryReplicationPerformance(std::string pairID, uint64_t &bandwidth) {
        HttpRequest req;
        Json::Value data;
        req.method = "GET";
        req.url = "performace_statistic/cur_statistic_data?CMO_STATISTIC_UUID=263:" + pairID +
                  "&CMO_STATISTIC_DATA_ID_LIST=801";
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query RemoteDevice failed:  " << errorCode << HCPENDLOG;
            return FAILED;
        } else {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "data: " << WIPE_SENSITIVE(data.toStyledString()) << HCPENDLOG;
            for (int i = 0; i < data.size(); i++) {
                std::string indicators = data[i]["CMO_STATISTIC_DATA_LIST"].asString();
                if (indicators != "") {
                    bandwidth = boost::lexical_cast<uint64_t>(indicators);
                    HCP_Log(INFO, DORADO_MODULE_NAME) << "QueryReplicationPerformance success, speed: " << bandwidth
                                                      << HCPENDLOG;
                    return SUCCESS;
                }
            }
            HCP_Log(ERR, DORADO_MODULE_NAME) << "indicators is empty  " << HCPENDLOG;
            return FAILED;
        }
    }

    int DoradoNas::QueryFileSystemSnapShotNum(std::string fileSystemId, unsigned long long &count) {
        HttpRequest req;
        Json::Value data;
        Json::FastWriter jsonWriter;
        req.method = "GET";
        req.url = "FSSNAPSHOT/count?vstoreId=0&PARENTID=" + fileSystemId;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query FileSystemSnapShotNum failed:  " << errorDes << HCPENDLOG;
            return FAILED;
        } else {
            count = std::stoi(data["COUNT"].asString());
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Query FileSystemSnapShotNum: " << fileSystemId
                                              << ". count =  " << count << " success!" << HCPENDLOG;
            return SUCCESS;
        }
    }

    int DoradoNas::GetESN(std::string &esn) {
        HttpRequest req;
        Json::Value data;
        Json::FastWriter jsonWriter;
        req.method = "GET";
        req.url = "system/";
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Get dorado esn failed:  " << errorDes << HCPENDLOG;
            return errorCode;
        } else {
            esn = data["ID"].asString();
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Get dorado esn: " << esn << " success!" << HCPENDLOG;
            return SUCCESS;
        }
    }

    int DoradoNas::DeleteFileSystemAndParentSnapshot() {
        HttpRequest req;
        req.method = "DELETE";
        req.url = "filesystem/" + fileSystemId + "?ISDELETEPARENTSNAPSHOT=true";
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;

        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS ||
                                   errorCode == DoradoErrorCode::FILESYSTEMIDNOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNas::DeleteSnapshot(std::string SnapshotName) {
        std::string id;
        int ret = QuerySnapshot(SnapshotName, id);
        if (ret != SUCCESS) {
            return ret;
        }
        HttpRequest req;
        req.method = "DELETE";
        req.url = "FSSNAPSHOT/" + id;
        std::string errorDes;
        int errorCode;
        SetErrorCode(DORADO_ERROR_CODE_OK);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS) {
            SetErrorCode(errorCode);
            return iRet;
        }
        return iRet;
    }

    int DoradoNas::CreateDTree(const std::string &dtreeName) {
        HttpRequest req;
        req.method = "POST";
        req.url = "QUOTATREE";
        Json::Value jsonReq;
        Json::FastWriter jsonWriter;
        jsonReq["NAME"] = dtreeName;
        jsonReq["PARENTTYPE"] = DTREEFS;
        jsonReq["PARENTID"] = fileSystemId;
        jsonReq["QUOTASWITCH"] = "false";
        jsonReq["securityStyle"] = UNIX;
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && (errorCode == SUCCESS ||
                                   errorCode == DoradoErrorCode::DTREENAMEISEXIST)) {
            return SUCCESS;
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Create Dtree failed. dtree name:" << dtreeName << " errorCode:"
                                         << errorCode
                                         << HCPENDLOG;
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNas::QueryDTree(const std::string &dtreeName) {
        HttpRequest req;
        req.method = "GET";
        req.url = "QUOTATREE?PARENTID=" + fileSystemId + "&NAME=" + dtreeName;
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return SUCCESS;
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Query Dtree failed. dtree name:" << dtreeName << " errorCode:" << errorCode
                                         << HCPENDLOG;
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNas::DeleteDTree(const std::string &dtreeName) {
        HttpRequest req;
        req.method = "DELETE";
        req.url = "QUOTATREE?PARENTID=" + fileSystemId + "&NAME=" + dtreeName;
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && (errorCode == SUCCESS ||
                                   errorCode == DoradoErrorCode::DTREENOTEXIST)) {
            return SUCCESS;
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Delete Dtree failed. dtree name:" << dtreeName << " errorCode:"
                                         << errorCode
                                         << HCPENDLOG;
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNas::GetReplicationPairID(const std::string &fsID, const std::string &remoteDeviceEsn,
                                             std::string &pairID) {
        HttpRequest req;
        req.method = "GET";
        req.url = "replicationpair/associate?ASSOCIATEOBJTYPE=40&ASSOCIATEOBJID=" + fsID;
        Json::Value data;
        int errorCode;
        std::string errDes;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            return FAILED;
        } else {
            for (int i = 0; i < data.size(); i++) {
                Json::Value pair = data[i];
                if (pair["REMOTEDEVICESN"].asString() == remoteDeviceEsn) {
                    pairID = pair["ID"].asString();
                    HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Found replication pair with remote device "
                                                       << remoteDeviceEsn
                                                       << ", fsID " << fsID << " success." << HCPENDLOG;
                    return SUCCESS;
                }
            }
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Not found replication pair with remote device " << remoteDeviceEsn
                                             << ", fsID " << fsID << "." << HCPENDLOG;
            return FAILED;
        }
    }

    int DoradoNas::SwitchOverPrimaryAndSecondResource(const std::string &pairId) {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["ID"] = pairId;
        Json::FastWriter jsonWriter;
        req.method = "put";
        req.url = "REPLICATIONPAIR/switch";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        std::string errDes;
        SetErrorCode(DORADO_ERROR_CODE_OK);
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "start switch replication pair " << pairId << " failed!" << HCPENDLOG;
            SetErrorCode(errorCode);
            return FAILED;
        } else {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "start switch replication pair " << pairId << " success!"
                                               << HCPENDLOG;
            return SUCCESS;
        }
    }

    int DoradoNas::SecondaryResourceProtectEnable(const std::string &pairId) {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["ID"] = pairId;
        Json::FastWriter jsonWriter;
        req.method = "put";
        req.url = "REPLICATIONPAIR/SET_SECODARY_WRITE_LOCK";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        std::string errDes;
        SetErrorCode(DORADO_ERROR_CODE_OK);
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "SecondaryResourceProtectEnable " << pairId << " failed!" << HCPENDLOG;
            SetErrorCode(errorCode);
            return FAILED;
        } else {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "SecondaryResourceProtectEnable " << pairId << " success!"
                                               << HCPENDLOG;
            return SUCCESS;
        }
    }

    int DoradoNas::SecondaryResourceProtectDisable(const std::string &pairId) {
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["ID"] = pairId;
        Json::FastWriter jsonWriter;
        req.method = "put";
        req.url = "REPLICATIONPAIR/CANCEL_SECODARY_WRITE_LOCK";
        req.body = jsonWriter.write(jsonReq);
        int errorCode;
        std::string errDes;
        SetErrorCode(DORADO_ERROR_CODE_OK);
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "SecondaryResourceProtectDisable " << pairId << " failed!" << HCPENDLOG;
            SetErrorCode(errorCode);
            return FAILED;
        } else {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "SecondaryResourceProtectDisable " << pairId << " success!"
                                               << HCPENDLOG;
            return SUCCESS;
        }
    }

    int DoradoNas::CreateSnapshotWithSnapTag(const std::string &snapshotName, const std::string &snapTag,
                                                  std::string &snapshotId) {
        if (QuerySnapshot(snapshotName, snapshotId) == SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Snapshot " << snapshotName << " has been existed." << HCPENDLOG;
            return SUCCESS;
        }
        HttpRequest req;
        req.method = "POST";
        req.url = "FSSNAPSHOT";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        jsonValue["PARENTTYPE"] = FILESYSTEM;
        jsonValue["NAME"] = snapshotName;
        jsonValue["PARENTID"] = fileSystemId;
        jsonValue["snapTag"] = snapTag;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int errorCode = 0;
        SetErrorCode(DORADO_ERROR_CODE_OK);
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS || errorCode == DoradoErrorCode::FILESYSTEMSNAPSHOTEXIST)) {
            snapshotId = data["ID"].asString();
            return SUCCESS;
        }
        SetErrorCode(errorCode);
        HCP_Log(INFO, DORADO_MODULE_NAME) << "create snapshot failed: " << errorCode << HCPENDLOG;
        return FAILED;
    }

    int DoradoNas::CreateSnapshotWithSnapTagAndVstoreId(const std::string &snapshotName, const std::string &snapTag,
                                                             std::string &snapshotId, const std::string &vstoreId) {
        if (QuerySnapshot(snapshotName, snapshotId) == SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Snapshot " << snapshotName << " has been existed." << HCPENDLOG;
            return SUCCESS;
        }
        HttpRequest req;
        req.method = "POST";
        req.url = "FSSNAPSHOT";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        jsonValue["PARENTTYPE"] = FILESYSTEM;
        jsonValue["NAME"] = snapshotName;
        jsonValue["PARENTID"] = fileSystemId;
        jsonValue["snapTag"] = snapTag;
        jsonValue["vstoreId"] = vstoreId;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int errorCode = 0;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS || errorCode == DoradoErrorCode::FILESYSTEMSNAPSHOTEXIST)) {
            snapshotId = data["ID"].asString();
            return SUCCESS;
        }
        HCP_Log(INFO, DORADO_MODULE_NAME) << "create snapshot failed: " << errorCode << HCPENDLOG;
        return FAILED;
    }


    int DoradoNas::QueryRemoteDeviceUserByName(const std::string &userName, std::string &remoteDevUserID) {
        HttpRequest req;
        Json::Value data;
        req.method = "GET";
        req.url = "user";
        std::string errDes;
        int errorCode = 0;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Cannot find the remote device user " << userName
                                             << HCPENDLOG;
            return FAILED;
        } else {
            for (int i = 0; i < data.size(); i++) {
                if (data[i]["NAME"].asString() == userName && data[i]["ROLEID"].asString() == "12") {
                    HCP_Log(INFO, DORADO_MODULE_NAME) << "Found the remote device user " << userName
                                                      << HCPENDLOG;
                    remoteDevUserID = data[i]["ID"].asString();
                    return SUCCESS;
                }
            }
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Cannot find the remote device user " << userName << HCPENDLOG;
        return FAILED;
    }

    int DoradoNas::DeleteRemoteDeviceUserByID(const std::string &remoteDevUserID) {
        HttpRequest req;
        Json::Value data;
        req.method = "DELETE";
        req.url = "user/" + remoteDevUserID;
        std::string errDes;
        int errorCode = 0;
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Delete the remote device user " << remoteDevUserID
                                             << "fail!" << HCPENDLOG;
            return FAILED;
        }
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Delete the remote device user " << remoteDevUserID
                                          << HCPENDLOG;
        return SUCCESS;
    }

    int DoradoNas::RollBackBySnapShotId(const std::string &SnapshotName, const std::string &snapshotId) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start Revert" << HCPENDLOG;
        HttpRequest req;
        req.method = "PUT";
        req.url = "fssnapshot/rollback_fssnapshot?t=" + snapshotId;
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        jsonValue["rollbackTargetObjName"] = SnapshotName;
        jsonValue["PARENTID"] = fileSystemId;
        jsonValue["rollbackSpeed"] = NUM_3;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        std::string errorDes;
        int errorCode = FAILED;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return SUCCESS;
        }
        return errorCode;
    }

    int DoradoNas::RollBackBySnapShotIdWithVstoreId(const std::string &snapshotName,
            const std::string &snapshotId, const std::string &vstoreId, Module::RestResult& result) {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Start Revert" << HCPENDLOG;
        HttpRequest req;
        req.method = "PUT";
        req.url = "fssnapshot/rollback_fssnapshot?t=" + snapshotId;
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        jsonValue["rollbackTargetObjName"] = snapshotName;
        jsonValue["PARENTID"] = fileSystemId;
        jsonValue["rollbackSpeed"] = NUM_3;
        jsonValue["vstoreId"] = vstoreId;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        result.errorCode = FAILED;
        int iRet = SendRequest(req, data, result);
        if (iRet == SUCCESS && result.errorCode == SUCCESS) {
            return SUCCESS;
        }
        return result.errorCode;
    }

    int DoradoNas::CreateReplicationWithSnapTag(const int &fsId, const std::string &rDevId,
                                                     const int &bandwidth, const std::string &snapTag,
                                                     int &rfsId, std::string &pairId) {
        int ret = CheckReplicationPair(fsId, rfsId, rDevId, pairId);
        if (ret == SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME)
                    << "already exist replication pair on remote device " << rDevId << ", fsId " << fsId << "."
                    << HCPENDLOG;
            ret = UpdateReplication(bandwidth, pairId);
            if (ret != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "UpdateReplication ERR" << HCPENDLOG;
                return FAILED;
            }
            return SUCCESS;
        }
        HttpRequest req;
        Json::Value data;
        Json::Value jsonReq;
        jsonReq["LOCALRESID"] = fsId;
        jsonReq["LOCALRESTYPE"] = FILE_SYSTEM;
        jsonReq["REMOTEDEVICEID"] = rDevId;
        jsonReq["REMOTERESID"] = rfsId;
        jsonReq["RECOVERYPOLICY"] = RECOVER_MANUAL;
        jsonReq["SYNCHRONIZETYPE"] = REPLICATION_MANUAL;
        jsonReq["ENABLECOMPRESS"] = true;
        jsonReq["REPLICATIONMODEL"] = ASYNCHRONOUS_REPLICATION;
        jsonReq["syncSnapPolicy"] = USER_SNAP_RETENTION_WITH_TAG_NUM;
        jsonReq["snapTagList"] = {snapTag};
        jsonReq["snapTagListRetentionNum"] = {NAS_SNAPSHOT_WITH_TAG_NUM};

        if (bandwidth != 0) {
            jsonReq["bandwidth"] = bandwidth;
        } else {
            jsonReq["SPEED"] = HIGHEST;
        }
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "replicationpair";
        req.body = jsonWriter.write(jsonReq);
        std::string errDes;
        int errorCode;
        SetErrorCode(DORADO_ERROR_CODE_OK);
        int iRet = SendRequest(req, data, errDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "create nas replication pair failed!" << HCPENDLOG;
            SetErrorCode(errorCode);
            return errorCode;
        } else {
            pairId = data["ID"].asString();
            HCP_Log(INFO, DORADO_MODULE_NAME) << "create nas replication pair successed!" << HCPENDLOG;
            return SUCCESS;
        }
    }
}
