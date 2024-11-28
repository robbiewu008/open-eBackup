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
#include "device_access/dorado/DoradoNasCifs.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "system/System.hpp"
#include "common/Path.h"
#include "common/CleanMemPwd.h"
#include "device_access/dorado/DoradoNasSnapshot.h"
#include "common/JsonUtils.h"

namespace Module {
    DoradoNasCIFS::~DoradoNasCIFS() {}

    int DoradoNasCIFS::Bind(HostInfo &host, const std::string &shareId)
    {
        int iRet;
        std::string userName = host.chapAuthName;
        if (userName.size() > MAXNAMELENGTH) {
            userName = userName.substr(0, MAXNAMELENGTH - 1);
        }

        DeviceDetails info;
        iRet = Query(info);
        if (iRet != SUCCESS) {
            return iRet;
        }
        iRet = CIFSShareAddClient(userName, info.deviceId, host.domainName);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Add  User to Cifsshare Failed! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }
        return SUCCESS;
    }

    int DoradoNasCIFS::AddShareClient(HostInfo &host, const std::string &sharePath, std::string &cifsShareName)
    {
        std::string userName = host.chapAuthName;
        if (userName.size() > MAXNAMELENGTH) {
            userName = userName.substr(0, MAXNAMELENGTH - 1);
        }

        DeviceDetails info;
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query filesystem failure! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }
        iRet = QueryCifsShare(sharePath, info, cifsShareName);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query cifs share failure! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "QueryCifsShare success: " << info.deviceName << HCPENDLOG;

        iRet = CIFSShareAddClient(userName, info.deviceId, host.domainName);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Add  User to Cifsshare Failed! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }

        return SUCCESS;
    }

    int DoradoNasCIFS::UnBind(HostInfo host, const std::string &shareId)
    {
        HCP_Log(ERR, DORADO_MODULE_NAME) << "do not need to unbind!" << HCPENDLOG;
        return SUCCESS;
    }

    int DoradoNasCIFS::Create(unsigned long long size)
    {
        int ret = CreateFileSystem(size, NTFS);
        if (ret != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Create filesystem failure! errorCode:" << ret << HCPENDLOG;
            return ret;
        }
        return CreateCifsShare(ResourceName, fileSystemId);
    }

    int DoradoNasCIFS::Query(DeviceDetails &info)
    {
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query filesystem failure! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }
        return QueryCifsShare(info, fileSystemId);
    }

    int DoradoNasCIFS::QueryFileSystem(DeviceDetails &info)
    {
        if (fileSystemName.empty() == true) {
            if (GetFsNameFromShareName() != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Get FS name from Sharename Failed" << HCPENDLOG;
                return FAILED;
            }
        }
        int ret = DoradoNas::QueryFileSystem(fileSystemName, info);
        if (ret != SUCCESS) {
            return ret;
        }
        fileSystemId = std::to_string(info.deviceId);
        return SUCCESS;
    }

    int DoradoNasCIFS::CIFSShareAddClient(std::string name, int ID, const std::string &domainName)
    {
        HttpRequest req;
        req.method = "POST";
        req.url = "CIFS_SHARE_AUTH_CLIENT";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["NAME"] = domainName.empty() ? name : domainName + "\\" + name;
        jsonValue["PARENTID"] = ID;
        jsonValue["DOMANTYPE"] = domainName.empty() ? LOCAL_USER : AD_USER;
        jsonValue["PERMISSION"] = FULL_CONTROL;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS || errorCode == DoradoErrorCode::CIFSSHAREALREADYEXIST ||
            errorCode == DoradoErrorCode::CIFSSHARE_PERMISSON_EXIST) {
            return SUCCESS;
        }
        HCP_Log(ERR, DORADO_MODULE_NAME) << "Add Cifs Client Failed!" << HCPENDLOG;
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNasCIFS::QueryCifsShareClient(NasSharedInfo &info, std::string sharedId)
    {
        std::string url = "CIFS_SHARE_AUTH_CLIENT?filter=PARENTID::" + sharedId;
        int iRet = QueryNasShareClient(info, url, "CIFS");
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME)
                    << "Query CIFS Share client failed! errorCode:" << iRet << HCPENDLOG;
            return FAILED;
        }
        return SUCCESS;
    }

    int DoradoNasCIFS::DeleteCIFSShare(DeviceDetails info)
    {
        HttpRequest req;
        int iRet;
        req.method = "DELETE";
        req.url = "CIFSHARE/" + std::to_string(info.deviceId);
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;

        iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                   || errorCode == DoradoErrorCode::FILESYSTEMNOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNasCIFS::Delete()
    {
        DeviceDetails info;
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "File System not exist!" << HCPENDLOG;
            return SUCCESS;
        }

        iRet = QueryCifsShare(info, fileSystemId);
        if (iRet == SUCCESS) {
            iRet = DeleteCIFSShare(info);
            if (iRet != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Delete cifs share failed!" << HCPENDLOG;
                return FAILED;
            }
        }
        iRet = DeleteFileSystem();
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Delete cifs filesystem failed" << HCPENDLOG;
            return FAILED;
        }
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Delete cifs share and filesystem success!" << HCPENDLOG;
        return SUCCESS;
    }

    int DoradoNasCIFS::DeleteShare(const std::string sharePath, const std::string shareName)
    {
        DeviceDetails info;
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "File System not exist!" << HCPENDLOG;
            return SUCCESS;
        }
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "File System: " << info.deviceName << HCPENDLOG;
        iRet = QueryCifsShare(sharePath, info, shareName);
        if (iRet == SUCCESS) {
            iRet = DeleteCIFSShare(info);
            if (iRet != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Delete cifs share failed!" << HCPENDLOG;
                return FAILED;
            }
        } else {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "QueryCifsShare failed: " << info.deviceName << HCPENDLOG;
        }

        return SUCCESS;
    }

    int DoradoNasCIFS::CreateShare()
    {
        DeviceDetails info;
        int ret = QueryFileSystem(info);
        if (ret != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "FileSystem not exist! errorCode:" << ret << HCPENDLOG;
            return FAILED;
        }
        return CreateCifsShare(ResourceName, std::to_string(info.deviceId));
    }

    int DoradoNasCIFS::CreateCifsShare(std::string fileSystemName, std::string FsId)
    {
        HttpRequest req;
        req.method = "POST";
        req.url = "CIFSHARE";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["SHAREPATH"] = "/" + ResourceName + "/";
        jsonValue["NAME"] = fileSystemName;
        jsonValue["FSID"] = FsId;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;

        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if ((iRet == SUCCESS && errorCode == SUCCESS)
            || errorCode == DoradoErrorCode::CIFSSHAREALREADYEXIST) {
            DeviceDetails info;
            std::string sharePath = "/" + ResourceName + "/";
            std::string shareName = fileSystemName;
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Create cifs share finished!" << HCPENDLOG;
            return QueryCifsShare(sharePath, info, shareName);
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNasCIFS::QueryCifsShare(DeviceDetails &info, std::string fsId)
    {
        HttpRequest req;
        req.method = "GET";
        req.url = "CIFSHARE?filter=FSID::" + fsId;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && data.isArray() && data.size() > 0 && data[0].size() > 0) {
            info.deviceId = atoi(data[0]["ID"].asString().c_str());
            info.deviceUniquePath = data[0]["NAME"].asString();
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Filesystem query Success!" << HCPENDLOG;
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNasCIFS::QueryCifsShare(const std::string sharePath, DeviceDetails &info, std::string cifsShareName)
    {
        HttpRequest req;
        req.method = "GET";
        req.url = "CIFSHARE?filter=FSID::" + fileSystemId;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data = Json::Value(Json::arrayValue);
        int iRet = SendRequest(req, data, errorDes, errorCode);
        std::string cifsSharePath;
        std::string tempCIFSShareName;
        HCP_Log(INFO, DORADO_MODULE_NAME) << "data.size: " << data.size() << HCPENDLOG;
        if (iRet == SUCCESS && data.size() > 0) {
            for (int i = 0; i < data.size(); ++i) {
                cifsSharePath = data[i]["SHAREPATH"].asString();
                tempCIFSShareName = data[i]["NAME"].asString();
                HCP_Log(DEBUG, DORADO_MODULE_NAME)
                        << "cifsSharePath: " << cifsSharePath << " ,share name: " << tempCIFSShareName << HCPENDLOG;
                if (cifsSharePath == sharePath && tempCIFSShareName == cifsShareName) {
                    info.deviceId = atoi(data[i]["ID"].asString().c_str());
                    HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Filesystem query Success!" << HCPENDLOG;
                    return SUCCESS;
                }
                continue;
            }
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNasCIFS::QueryCifsShare(std::vector<NasSharedInfo> &infos, std::string fsId)
    {
        std::string url = "CIFSHARE?filter=FSID::" + fsId;
        int iRet = QueryNasShare(infos, url, "CIFS");
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME)
                    << "Query CIFS Share failed! errorCode:" << iRet << HCPENDLOG;
            return FAILED;
        }
        return SUCCESS;
    }

    int DoradoNasCIFS::DeleteWindowsUser(std::string userName)
    {
        HttpRequest req;
        req.method = "DELETE";
        req.url = "WINDOWS_USER?NAME=" + userName;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                   || errorCode == DoradoErrorCode::WINDOWSUSERNOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNasCIFS::CreateWindowUser(std::string userName, std::string password)
    {
        HttpRequest req;
        req.method = "POST";
        req.url = "WINDOWS_USER";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["NAME"] = userName;
        jsonValue["PASSWORD"] = password;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && data.isArray() && data.size() > 0 && data[0].size() > 0) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    std::unique_ptr<ControlDevice> DoradoNasCIFS::CreateClone(std::string volumeName, int &errorCode)
    {
        DeviceDetails info;
        std::string cloneFsId;
        ControlDeviceInfo deviceInfo = {};
        int ret = FAILED;
        ret = QueryFileSystem(info);
        if (ret != SUCCESS) {
            errorCode = ret;
            HCP_Log(ERR, DORADO_MODULE_NAME) << ResourceName
                                             << " source filesystem is not exist!" << HCPENDLOG;
            return nullptr;
        }

        ret = CreateCloneFileSystem(volumeName, cloneFsId);
        if (ret != SUCCESS) {
            errorCode = ret;
            HCP_Log(ERR, DORADO_MODULE_NAME) << volumeName
                                             << " create clone filesystem failed! errorCode:" << ret << HCPENDLOG;
            return nullptr;
        }
        ret = CreateCifsShare(volumeName, cloneFsId);
        if (ret != SUCCESS) {
            errorCode = ret;
            HCP_Log(ERR, DORADO_MODULE_NAME) << volumeName << " create clone filesystem share failed!" << HCPENDLOG;
            return nullptr;
        }

        AssignDeviceInfo(deviceInfo, volumeName);
        return std::make_unique<DoradoNasCIFS>(deviceInfo, cloneFsId, readK8s);
    }

    int DoradoNasCIFS::QueryServiceHost(std::vector<std::string> &ipList, IP_TYPE ipType)
    {
        Json::Value data;
        int ret = QueryLIFPortList(ipList, data);
        if (ret != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query Dorado lif port failed! errorCode:" << ret << HCPENDLOG;
            return ret;
        }
        FilterLogicPort(data, ipList, ipType);
        if (ipList.size() == 0) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "there is no data lif port normal!" << HCPENDLOG;
            return FAILED;
        }
        return SUCCESS;
    }

    void DoradoNasCIFS::FilterLogicPort(Json::Value data, std::vector<std::string> &cifsIPList, IP_TYPE ipType)
    {
        for (int i = 0; i < data.size(); i++) {
            Json::Value oneNode = data[i];
            std::string proto = oneNode["SUPPORTPROTOCOL"].asString();
            std::string role = oneNode["ROLE"].asString();
            std::string RunningStatus = oneNode["RUNNINGSTATUS"].asString();
            if (RunningStatus == RUNNINGSTATUS_LINKUP && (proto == SUPPORTPROTOCOL_NFS_CIFS
                                                          || proto == SUPPORTPROTOCOL_CIFS) &&
                (role == PORT_ROLE_SERVICE
                 || role == PORT_ROLE_MANAGE_SERVICE)) {
                Json::Value ip;
                if (ipType == IP_TYPE::IP_V4) {
                    ip = oneNode["IPV4ADDR"];
                } else if (ipType == IP_TYPE::IP_V6) {
                    ip = oneNode["IPV6ADDR"];
                }
                cifsIPList.push_back(ip.asString());
                HCP_Log(INFO, DORADO_MODULE_NAME) << "push back cifs Portal " << ip.asString() << HCPENDLOG;
            }
        }
        return;
    }

    int DoradoNasCIFS::GetFsNameFromShareName()
    {
        HttpRequest req;
        req.method = "GET";
        std::string errorDes;
        Json::Value data;
        int iRet;
        int errorCode = FAILED;
        std::string resourceName = ResourceName;
        boost::replace_all(resourceName, " ", "%20");
        req.url = "CIFSHARE?range=[0-100]&filter=NAME::" + resourceName;
        iRet = SendRequest(req, data, errorDes, errorCode);
        std::string shareName = ResourceName;
        if (iRet == SUCCESS && data.size() > 0 && errorCode == SUCCESS) {
            for (auto dataTraverse : data) {
                if (shareName.compare(dataTraverse["NAME"].asString()) == 0) {
                    std::string path = dataTraverse["SHAREPATH"].asString();
                    fileSystemName = path.substr(1, path.size() - DORADO_NUMBER_TWO);
                    return SUCCESS;
                }
            }
        }
        return FAILED;
    }

    std::unique_ptr<ControlDevice> DoradoNasCIFS::CreateSnapshot(std::string SnapshotName, int &errorCode)
    {
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
            return std::make_unique<DoradoNasSnapshot>(deviceInfo, fileSystemId, "/" + fileSystemName + "/", readK8s);
        }
        HttpRequest req;
        req.method = "POST";
        req.url = "FSSNAPSHOT";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        jsonValue["PARENTTYPE"] = DORADO_DEFAULT_PARENT_TYPE;
        jsonValue["NAME"] = SnapshotName;
        jsonValue["PARENTID"] = fileSystemId;
        jsonValue["snapTag"] = "A8000";
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS || errorCode == DoradoErrorCode::FILESYSTEMSNAPSHOTEXIST)) {
            return std::make_unique<DoradoNasSnapshot>(deviceInfo, fileSystemId, "/" + fileSystemName + "/", readK8s);
        }
        HCP_Log(INFO, DORADO_MODULE_NAME) << "create snapshot failed: " << errorCode << HCPENDLOG;
        return nullptr;
    }
}
