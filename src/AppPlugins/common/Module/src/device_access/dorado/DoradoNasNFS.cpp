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
#include "device_access/dorado/DoradoNasNFS.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "device_access/dorado/DoradoNas.h"
#include "system/System.hpp"
#include "common/Path.h"
#include "common/JsonUtils.h"

namespace Module {
    namespace {
        constexpr int WAIT_FILE_SYSTEM_READY_COUNT = 60;
        constexpr int WAIT_FILE_SYSTEM_READY_TIME = 3000;
        constexpr int FILE_SYSTEM_RERUNNING_STATUS_ONLINE = 27;
    }

    DoradoNasNFS::~DoradoNasNFS() {
    }

    int DoradoNasNFS::Bind(HostInfo &host, const std::string &shareId)
    {
        int iRet = FAILED;
        int nfsShareId = 0;
        if (shareId.empty()) {
            DeviceDetails info;
            iRet = Query(info);
            if (iRet != SUCCESS) {
                return iRet;
            }
            nfsShareId = info.deviceId;
        } else {
            if (!checkStringIsDigit(shareId)) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Invalid shareId." << DBG(shareId) << HCPENDLOG;
                return FAILED;
            }
            nfsShareId = atoi(shareId.c_str());
        }
        HCP_Log(INFO, DORADO_MODULE_NAME) << DBG(nfsShareId) << DBG(shareId) << HCPENDLOG;

        if (host.hostIp != "") {
            iRet = SUCCESS;
            if (!CheckNFSShareClientExist(std::to_string(nfsShareId), host.hostIp)) {
                iRet = NFSShareAddClient(host.hostIp, nfsShareId);
            } else {
                HCP_Log(INFO, DORADO_MODULE_NAME) << "Client to share already exist! dont need add client" << HCPENDLOG;
            }
            if (iRet != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Add Client to share failed! errorCode:" << iRet << HCPENDLOG;
                return iRet;
            }
        }
        for (auto ip : host.hostIpList) {
            iRet = SUCCESS;
            if (!CheckNFSShareClientExist(std::to_string(nfsShareId), ip)) {
                iRet = NFSShareAddClient(ip, nfsShareId);
            } else {
                HCP_Log(INFO, DORADO_MODULE_NAME) << "Client to share already exist! dont need add client" << HCPENDLOG;
            }
            if (iRet != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Add Client to share failed! errorCode:" << iRet << HCPENDLOG;
                return iRet;
            }
        }
        return SUCCESS;
    }

    bool DoradoNasNFS::CheckNFSShareClientExist(std::string deviceId, std::string clientIp)
    {
        std::vector<std::string> clientIpList;
        int ret = QueryNFSShareClient(deviceId, clientIpList);
        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "deviceId:" << deviceId << "clientIp:" << clientIp << HCPENDLOG;
        // query err will be see as not find
        if (ret != SUCCESS && ret != DoradoErrorCode::FILESYSTEMNOTEXIST) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query NFS share client failed!" << HCPENDLOG;
            return false;
        }
        auto iter = find(clientIpList.begin(), clientIpList.end(), clientIp);
        if (iter != clientIpList.end()) {
            return true;
        } else {
            return false;
        }
    }

    int DoradoNasNFS::UnBind(HostInfo host, const std::string &shareId)
    {
        int iRet = FAILED;
        DeviceDetails info;
        iRet = Query(info);
        if (iRet != SUCCESS && iRet != DoradoErrorCode::FILESYSTEMNOTEXIST) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query Filesystem Failed:" << iRet << HCPENDLOG;
            return FAILED;
        }

        if (iRet == DoradoErrorCode::FILESYSTEMNOTEXIST) {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "filesystem not exist!" << iRet << HCPENDLOG;
            return SUCCESS;
        }

        std::string nfsShareId = shareId.empty() ? std::to_string(info.deviceId) : shareId;
        HCP_Log(INFO, DORADO_MODULE_NAME) << DBG(nfsShareId) << DBG(shareId) << HCPENDLOG;

        return DeleteNFSShareClient(host.hostIpList, nfsShareId);
    }

    int DoradoNasNFS::Create(unsigned long long size)
    {
        int ret = CreateFileSystem(size, UNIX);
        if (ret != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Create filesystem failure! errorCode:" << ret << HCPENDLOG;
            return ret;
        }
        return CreateNFSShare(ResourceName, fileSystemId);
    }

    int DoradoNasNFS::Query(DeviceDetails &info)
    {
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query filesystem failure! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }
        return QueryNFSShare(info, fileSystemId);
    }

    int DoradoNasNFS::NFSShareAddClient(std::string name, int ID)
    {
        HttpRequest req;
        req.method = "POST";
        req.url = "NFS_SHARE_AUTH_CLIENT";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["NAME"] = name;
        jsonValue["PARENTID"] = ID;
        jsonValue["ACCESSVAL"] = 1;
        jsonValue["SYNC"] = Syncmode::SYNC;
        jsonValue["ALLSQUASH"] = 1;
        jsonValue["ROOTSQUASH"] = 1;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;

        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                   || errorCode == DoradoErrorCode::ALREADYINWHITE)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNasNFS::QueryNFSShare(DeviceDetails &info, std::string fsId)
    {
        HttpRequest req;
        req.method = "GET";
        req.url = "NFSHARE?filter=FSID::" + fsId;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && data.isArray() && data.size() > 0 && data[0].size() > 0) {
            info.deviceId = atoi(data[0]["ID"].asString().c_str());
            info.deviceUniquePath = data[0]["SHAREPATH"].asString();
            if (data[0].isMember("fileHandleByteAlignmentSwitch")) {
                info.fileHandleByteAlignmentSwitch = data[0]["fileHandleByteAlignmentSwitch"].asBool();
            }
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Nfs share already exists!" << HCPENDLOG;
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNasNFS::QueryNFSShare(std::vector<NasSharedInfo> &infos, std::string fsId)
    {
        std::string url = "NFSHARE?filter=FSID::" + fsId;
        int iRet = QueryNasShare(infos, url, "NFS");
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME)
                    << "Query NFS Share failed! errorCode:" << iRet << HCPENDLOG;
            return FAILED;
        }
        return SUCCESS;
    }

    int DoradoNasNFS::DeleteNFSShare(DeviceDetails info)
    {
        HttpRequest req;
        int iRet;
        req.method = "DELETE";
        req.url = "NFSHARE/" + std::to_string(info.deviceId);
        std::string errorDes;
        int errorCode;
        Json::Value data;

        iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                   || errorCode == DoradoErrorCode::FILESYSTEMNOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNasNFS::CreateShare()
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "enter CreateShare " << HCPENDLOG;
        for (int i = 0; i < WAIT_FILE_SYSTEM_READY_COUNT; ++i) {
            DeviceDetails info;
            int ret = QueryFileSystem(info);
            if (ret != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "FileSystem not exist! errorCode:" << ret << HCPENDLOG;
                return FAILED;
            }
            // check file system status
            if (info.status == FILE_SYSTEM_RERUNNING_STATUS_ONLINE) {
                HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Crate NFS share, file system name:" << ResourceName << HCPENDLOG;
                return CreateNFSShare(ResourceName, fileSystemId);
            }
            // wait 3s
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FILE_SYSTEM_READY_TIME));
        }

        return FAILED;
    }

    int DoradoNasNFS::CreateNFSShare(std::string fileSystemName, std::string FsId)
    {
        DeviceDetails info;
        int iRet = QueryNFSShare(info, FsId);
        if (iRet == SUCCESS) {
            // 如果需要开启四字节对齐，而共享没有开启四字节对齐，需要删除共享重新创建
            if (!(isFileHandleByteAligment) || (isFileHandleByteAligment && info.fileHandleByteAlignmentSwitch)) {
                HCP_Log(INFO, DORADO_MODULE_NAME) << "NFS Share FileSystem has been exist." << HCPENDLOG;
                return SUCCESS;
            }
            DeleteNFSShare(info);
        }
        HttpRequest req;
        req.method = "POST";
        req.url = "NFSHARE";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["SHAREPATH"] = "/" + fileSystemName + "/";
        jsonValue["NAME"] = fileSystemName;
        jsonValue["FSID"] = FsId;
        if (isFileHandleByteAligment) {
            jsonValue["fileHandleByteAlignmentSwitch"] = isFileHandleByteAligment;
        }
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;

        iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                   || errorCode == DoradoErrorCode::NFSSHAREALREADYEXIST)) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Create nfs share finished!" << HCPENDLOG;
            return QueryNFSShare(info, FsId);
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNasNFS::CreateNFSShare()
    {
        HCP_Log(INFO, DORADO_MODULE_NAME) << "enter CreateNFSShare " << HCPENDLOG;
        for (int i = 0; i < WAIT_FILE_SYSTEM_READY_COUNT; ++i) {
            DeviceDetails info;
            int ret = QueryFileSystem(info);
            if (ret != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "FileSystem not exist! errorCode:" << ret << HCPENDLOG;
                return FAILED;
            }
            // check file system status
            if (info.status == FILE_SYSTEM_RERUNNING_STATUS_ONLINE) {
                HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Crate NFS share, file system name:" << ResourceName << HCPENDLOG;
                return CreateNFSShareExact(ResourceName, fileSystemId);
            }
            // wait 3s
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FILE_SYSTEM_READY_TIME));
        }

        return FAILED;
    }

    int DoradoNasNFS::CreateNFSShareExact(std::string fileSystemName, std::string FsId)
    {
        std::string shareName = "/" + fileSystemName + "/";
        HttpRequest req;
        req.method = "POST";
        req.url = "NFSHARE";
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["SHAREPATH"] = shareName;
        jsonValue["NAME"] = fileSystemName;
        jsonValue["FSID"] = FsId;
        if (isFileHandleByteAligment) {
            jsonValue["fileHandleByteAlignmentSwitch"] = isFileHandleByteAligment;
        }
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;

        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS || errorCode == DoradoErrorCode::NFSSHAREALREADYEXIST)) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "Create nfs share finished!" << HCPENDLOG;
            std::vector<NasSharedInfo> shares;
            if (QueryNFSShare(shares, FsId) != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Query nfs share failed!" << HCPENDLOG;
                return FAILED;
            }
            auto item = std::find_if(shares.begin(), shares.end(), [&](NasSharedInfo &info) {
                HCP_Log(DEBUG, DORADO_MODULE_NAME) << "NFS Share=" << info.deviceDetail.deviceUniquePath << HCPENDLOG;
                return (info.deviceDetail.deviceUniquePath == shareName);
            });
            if (item != shares.end()) {
                HCP_Log(INFO, DORADO_MODULE_NAME) << "Query nfs share success!" << HCPENDLOG;
                return SUCCESS;
            }
            HCP_Log(ERR, DORADO_MODULE_NAME) << "NFS share not found!" << DBG(shareName) << HCPENDLOG;
            return FAILED;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNasNFS::Delete()
    {
        DeviceDetails info;
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(INFO, DORADO_MODULE_NAME) << "File System not exist" << HCPENDLOG;
            return SUCCESS;
        }

        iRet = QueryNFSShare(info, fileSystemId);
        if (iRet == SUCCESS) {
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Delete nfs share Start!" << HCPENDLOG;
            iRet = DeleteNFSShare(info);
            if (iRet != SUCCESS) {
                HCP_Log(ERR, DORADO_MODULE_NAME) << "Delete nfs share failed!" << HCPENDLOG;
                return iRet;
            }
            HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Delete nfs share sucess!" << HCPENDLOG;
        }

        HCP_Log(DEBUG, DORADO_MODULE_NAME) << "Delete nfs clone Start!" << HCPENDLOG;
        if (isDeleteParentSnapShot) {
            iRet = DeleteFileSystemAndParentSnapshot();
        } else {
            iRet = DeleteFileSystem();
        }
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Delete nfs filesystem failed" << HCPENDLOG;
            return iRet;
        }
        HCP_Log(INFO, DORADO_MODULE_NAME) << "Delete nfs share and filesystem success!" << HCPENDLOG;
        return SUCCESS;
    }

    std::unique_ptr<ControlDevice> DoradoNasNFS::CreateClone(std::string volumeName, int &errorCode)
    {
        DeviceDetails info;
        std::string cloneFsId;
        ControlDeviceInfo deviceInfo = {};
        int ret = FAILED;
        ret = QueryFileSystem(info);
        if (ret != SUCCESS) {
            errorCode = ret;
            HCP_Log(ERR, DORADO_MODULE_NAME) << ResourceName
                                             << " source filesystem is not exist! errorCode:" << ret << HCPENDLOG;
            return nullptr;
        }

        ret = CreateCloneFileSystem(volumeName, cloneFsId);
        if (ret != SUCCESS) {
            errorCode = ret;
            HCP_Log(ERR, DORADO_MODULE_NAME) << volumeName
                                             << " create clone filesystem failed! errorCode:" << ret << HCPENDLOG;
            return nullptr;
        }

        AssignDeviceInfo(deviceInfo, volumeName);
        std::unique_ptr<DoradoNasNFS> cloneFileSystemObj = std::make_unique<DoradoNasNFS>(deviceInfo, cloneFsId);
        if (cloneFileSystemObj == nullptr) {
            HCP_Logger_noid(ERR, DORADO_MODULE_NAME) << " create clone filesystem failed! errorCode:" << ret
                                                     << HCPENDLOG;
            return nullptr;
        }
        cloneFileSystemObj->SetIsDeleteParentSnapShotFlag(true);
        ret = cloneFileSystemObj->CreateShare();
        if (ret != SUCCESS) {
            HCP_Logger_noid(ERR, DORADO_MODULE_NAME) << "Create filesystemShare failed! errorCode:" << ret << HCPENDLOG;
            return nullptr;
        }
        return cloneFileSystemObj;
    }

    int DoradoNasNFS::QueryServiceHost(std::vector<std::string> &ipList, IP_TYPE ipType)
    {
        Json::Value data;
        int ret = QueryLIFPortList(ipList, data);
        if (ret != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME)
                    << "Query Dorado lif port failed! errorCode:" << ret << HCPENDLOG;
            return ret;
        }
        FilterLogicPort(data, ipList, ipType);
        if (ipList.size() == 0) {
            HCP_Log(ERR, DORADO_MODULE_NAME)
                    << "there is no data lif port normal!" << HCPENDLOG;
            return FAILED;
        }
        return SUCCESS;
    }

    void DoradoNasNFS::FilterLogicPort(Json::Value data, std::vector<std::string> &nfsIPList, IP_TYPE ipType)
    {
        for (int i = 0; i < data.size(); i++) {
            Json::Value oneNode = data[i];
            std::string proto = oneNode["SUPPORTPROTOCOL"].asString();
            std::string role = oneNode["ROLE"].asString();
            std::string RunningStatus = oneNode["RUNNINGSTATUS"].asString();
            if (RunningStatus == RUNNINGSTATUS_LINKUP &&
                (proto == SUPPORTPROTOCOL_NFS_CIFS || proto == SUPPORTPROTOCOL_NFS) &&
                (role == PORT_ROLE_SERVICE || role == PORT_ROLE_MANAGE_SERVICE)) {
                Json::Value ip;
                if (ipType == IP_TYPE::IP_V4
                    && oneNode.isMember("IPV4ADDR")
                    && oneNode["IPV4ADDR"].asString() != "") {
                    ip = oneNode["IPV4ADDR"];
                    nfsIPList.push_back(ip.asString());
                } else if (ipType == IP_TYPE::IP_V6
                           && oneNode.isMember("IPV6ADDR")
                           && oneNode["IPV6ADDR"].asString() != "") {
                    ip = oneNode["IPV6ADDR"];
                    nfsIPList.push_back(ip.asString());
                }
                HCP_Log(INFO, DORADO_MODULE_NAME) << "push back nfs Portal " << ip.asString() << HCPENDLOG;
            }
        }
        return;
    }

    int DoradoNasNFS::QueryNFSShareClient(const std::string shareId, std::vector<std::string> &iPList)
    {
        HttpRequest req;
        req.method = "GET";
        req.url = "NFS_SHARE_AUTH_CLIENT?filter=PARENTID::" + shareId;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            NFSShareIDList.clear();
            for (int i = 0; i < data.size(); i++) {
                Json::Value oneNode = data[i];
                std::string shareID = oneNode["ID"].asString();
                std::string name = oneNode["NAME"].asString();
                NFSShareIDList[name] = shareID;
                iPList.push_back(name);
            }
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int DoradoNasNFS::QueryNFSShareClient(NasSharedInfo &info, std::string shareId)
    {
        std::string url = "NFS_SHARE_AUTH_CLIENT?filter=PARENTID::" + shareId;
        int iRet = QueryNasShareClient(info, url, "NFS");
        if (iRet != SUCCESS) {
            HCP_Log(ERR, DORADO_MODULE_NAME)
                    << "Query NFS Share client failed! errorCode:" << iRet << HCPENDLOG;
            return FAILED;
        }
        return SUCCESS;
    }

    int DoradoNasNFS::DeleteNFSShareClient(const std::vector<std::string> &iPList, const std::string shareId)
    {
        std::vector<std::string> nasShareIPList;
        int ret = QueryNFSShareClient(shareId, nasShareIPList);
        if (ret != SUCCESS && ret != DoradoErrorCode::FILESYSTEMNOTEXIST) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "Query NFS share client failed!" << HCPENDLOG;
            return FAILED;
        }

        if (ret == DoradoErrorCode::FILESYSTEMNOTEXIST) {
            HCP_Log(ERR, DORADO_MODULE_NAME) << "NFS share client not exist!" << HCPENDLOG;
            return SUCCESS;
        }

        if (iPList.empty()) {
            std::unordered_map<std::string, std::string>::iterator iter;
            for (iter = NFSShareIDList.begin(); iter != NFSShareIDList.end(); iter++) {
                int ret = DeleteNFSShareClient(iter->second);
                if (ret != SUCCESS) {
                    HCP_Log(ERR, DORADO_MODULE_NAME) << "Delete NFS share client failed! IP:"
                                                     << iter->first << " client id:" << iter->second << HCPENDLOG;
                    return FAILED;
                }
            }
            return SUCCESS;
        }

        for (auto ip : iPList) {
            auto iter = NFSShareIDList.find(ip);
            if (iter != NFSShareIDList.end()) {
                int ret = DeleteNFSShareClient(iter->second);
                if (ret != SUCCESS) {
                    HCP_Log(ERR, DORADO_MODULE_NAME) << "Delete NFS share client failed! IP:"
                                                     << iter->first << " client id:" << iter->second << HCPENDLOG;
                    return FAILED;
                }
                NFSShareIDList.erase(iter);
            }
        }
        return SUCCESS;
    }

    int DoradoNasNFS::DeleteNFSShareClient(std::string shareClientId)
    {
        HttpRequest req;
        req.method = "DELETE";
        req.url = "NFS_SHARE_AUTH_CLIENT/" + shareClientId;
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }
}

