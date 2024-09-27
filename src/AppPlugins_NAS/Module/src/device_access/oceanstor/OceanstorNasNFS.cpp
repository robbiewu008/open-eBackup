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
#include "device_access/oceanstor/OceanstorNasNFS.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "device_access/oceanstor/OceanstorNas.h"
#include "common/JsonUtils.h"
#include "system/System.hpp"
#include "common/Path.h"
#include "device_access/oceanstor/OceanstorNasSnapshot.h"

namespace Module {
    OceanstorNasNFS::~OceanstorNasNFS() {
    }

    int OceanstorNasNFS::Bind(HostInfo &host, const std::string &shareId) {
        int iRet;
        DeviceDetails info;
        iRet = Query(info);
        if (iRet != SUCCESS) {
            return iRet;
        }
        if (host.hostIp != "") {
            iRet = NFSShareAddClient(host.hostIp, info.deviceId);
            if (iRet != SUCCESS) {
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Add Client to share success! errorCode:" << iRet <<
                                                    HCPENDLOG;
                return iRet;
            }
        }
        for (auto ip : host.hostIpList) {
            iRet = NFSShareAddClient(ip, info.deviceId);
            if (iRet != SUCCESS) {
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Add Client to share success! errorCode:" << iRet <<
                                                    HCPENDLOG;
                return iRet;
            }
        }
        return SUCCESS;
    }

    int OceanstorNasNFS::UnBind(HostInfo host, const std::string &shareId) {
        int iRet;
        DeviceDetails info;
        iRet = Query(info);
        if (iRet != SUCCESS && iRet != OceanstorErrorCode::FILESYSTEMNOTEXIST) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Query Filesystem Failed:" << iRet << HCPENDLOG;
            return FAILED;
        }

        if (iRet == OceanstorErrorCode::FILESYSTEMNOTEXIST) {
            HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << "filesystem not exist!" << iRet << HCPENDLOG;
            return SUCCESS;
        }
        return DeleteNFSShareClient(host.hostIpList, std::to_string(info.deviceId));
    }

    int OceanstorNasNFS::Query(DeviceDetails &info) {
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Query filesystem failure! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }
        return QueryNFSShare(info, fileSystemId);
    }

    int OceanstorNasNFS::QueryFileSystem(DeviceDetails &info) {
        if (fileSystemName.empty() == true) {
            if (GetFsNameFromShareName() != SUCCESS) {
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Get FS name from Sharename Failed" << HCPENDLOG;
                return FAILED;
            }
        }
        int ret = OceanstorNas::QueryFileSystem(fileSystemName, info);
        if (ret != SUCCESS) {
            return ret;
        }
        fileSystemId = std::to_string(info.deviceId);
        return SUCCESS;
    }

    int OceanstorNasNFS::NFSShareAddClient(std::string name, int ID) {
        HttpRequest req;
        req.method = "POST";
        req.url = "NFS_SHARE_AUTH_CLIENT";
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["NAME"] = name;
        jsonValue["PARENTID"] = ID;
        jsonValue["ACCESSVAL"] = 1;
        jsonValue["SYNC"] = 1;
        jsonValue["ALLSQUASH"] = 1;
        jsonValue["ROOTSQUASH"] = 1;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                || errorCode == OceanstorErrorCode::ALREADYINWHITE)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int OceanstorNasNFS::QueryNFSShare(DeviceDetails &info, std::string fsId) {
        HttpRequest req;
        req.method = "GET";
        req.url = "NFSHARE?range=[0-100]&filter=FSID::" + fsId;
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "&vstoreId=" + vstoreId;
        }
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && data[0].size() > 0 && errorCode == SUCCESS) {
            info.deviceId = atoi(data[0]["ID"].asString().c_str());
            info.deviceUniquePath = data[0]["SHAREPATH"].asString();
            HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "Nfs share hase exists!" << HCPENDLOG;
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int OceanstorNasNFS::DeleteNFSShare(DeviceDetails info) {
        HttpRequest req;
        int iRet;
        req.method = "DELETE";
        req.url = "NFSHARE/" + std::to_string(info.deviceId);
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        std::string errorDes;
        int errorCode;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                || errorCode == OceanstorErrorCode::FILESYSTEMNOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int OceanstorNasNFS::CreateShare() {
        DeviceDetails info;
        int ret = QueryFileSystem(info);
        if (ret != SUCCESS) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "FileSystem not exist! errorCode:" << ret << HCPENDLOG;
            return FAILED;
        }
        return CreateNFSShare(ResourceName, fileSystemId);
    }

    int OceanstorNasNFS::CreateNFSShare(std::string fileSystemName, std::string FsId) {
        DeviceDetails info;
        if (QueryNFSShare(info, FsId) == SUCCESS) {
            HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "NFS Share FileSystem has been exist." << HCPENDLOG;
            return SUCCESS;
        }
        HttpRequest req;
        req.method = "POST";
        req.url = "NFSHARE";
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["SHAREPATH"] = "/" + fileSystemName + "/";
        jsonValue["NAME"] = "/" + fileSystemName;
        jsonValue["FSID"] = FsId;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                || errorCode == OceanstorErrorCode::NFSSHAREALREADYEXIST)) {
            DeviceDetails info;
            HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "Create nfs share finished!" << HCPENDLOG;
            return QueryNFSShare(info, FsId);
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }


    int OceanstorNasNFS::Delete() {
        DeviceDetails info;
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "File System not exist" << HCPENDLOG;
            return SUCCESS;
        }

        iRet = QueryNFSShare(info, fileSystemId);
        if (iRet == SUCCESS) {
            HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << "Delete nfs share Start!" << HCPENDLOG;
            iRet = DeleteNFSShare(info);
            if (iRet != SUCCESS) {
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Delete nfs share failed!" << HCPENDLOG;
                return iRet;
            }
            HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << "Delete nfs share sucess!" << HCPENDLOG;
        }

        HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << "Delete nfs clone Start!" << HCPENDLOG;
        if (isDeleteParentSnapShot) {
            iRet = DeleteFileSystemAndParentSnapshot();
        } else {
            iRet = DeleteFileSystem();
        }
        if (iRet != SUCCESS) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Delete nfs filesystem failed" << HCPENDLOG;
            return iRet;
        }
        HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "Delete nfs share and filesystem success!" << HCPENDLOG;
        return SUCCESS;
    }

    int OceanstorNasNFS::DeleteShare() {
        DeviceDetails info;
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "File System not exist" << HCPENDLOG;
            return SUCCESS;
        }

        iRet = QueryNFSShare(info, fileSystemId);
        if (iRet == SUCCESS) {
            HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << "Delete nfs share Start!" << HCPENDLOG;
            iRet = DeleteNFSShare(info);
            if (iRet != SUCCESS) {
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Delete nfs share failed!" << HCPENDLOG;
                return iRet;
            }
            HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << "Delete nfs share sucess!" << HCPENDLOG;
        }
        return FAILED;
    }

    std::unique_ptr <ControlDevice> OceanstorNasNFS::CreateClone(std::string volumeName, int &errorCode) {
        DeviceDetails info;
        std::string cloneFsId;
        ControlDeviceInfo deviceInfo = {};
        int ret = QueryFileSystem(info);
        if (ret != SUCCESS) {
            errorCode = ret;
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << ResourceName
                                                << " source filesystem is not exist! errorCode:" << ret << HCPENDLOG;
            return nullptr;
        }

        ret = CreateCloneFileSystem(volumeName, cloneFsId);
        if (ret != SUCCESS) {
            errorCode = ret;
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << volumeName
                                                << " create clone filesystem failed! errorCode:" << ret << HCPENDLOG;
            return nullptr;
        }

        ret = CreateNFSShare(volumeName, cloneFsId);
        if (ret != SUCCESS) {
            errorCode = ret;
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << volumeName
                                                << " create clone filesystem share failed! errorCode:" << ret
                                                << HCPENDLOG;
            return nullptr;
        }

        AssignDeviceInfo(deviceInfo, volumeName);
        std::unique_ptr <OceanstorNasNFS> cloneFileSystemObj = std::make_unique<OceanstorNasNFS>(deviceInfo, cloneFsId);
        if (cloneFileSystemObj == nullptr) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << " create clone filesystem failed! errorCode:" << ret << HCPENDLOG;
            return nullptr;
        }
        cloneFileSystemObj->SetIsDeleteParentSnapShotFlag(true);
        return cloneFileSystemObj;
    }

    int OceanstorNasNFS::QueryNFSShareClient(const std::string shareId, std::vector<std::string> &iPList) {
        HttpRequest req;
        req.method = "GET";
        req.url = "NFS_SHARE_AUTH_CLIENT?filter=PARENTID::" + shareId;
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "&vstoreId=" + vstoreId;
        }
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

    int OceanstorNasNFS::DeleteNFSShareClient(const std::vector<std::string> &iPList, const std::string shareId) {
        std::vector<std::string> nasShareIPList;
        int ret = QueryNFSShareClient(shareId, nasShareIPList);
        if (ret != SUCCESS && ret != OceanstorErrorCode::FILESYSTEMNOTEXIST) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Query NFS share client failed!" << HCPENDLOG;
            return FAILED;
        }

        if (ret == OceanstorErrorCode::FILESYSTEMNOTEXIST) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "NFS share client not exist!" << HCPENDLOG;
            return SUCCESS;
        }

        if (iPList.empty()) {
            std::unordered_map<std::string, std::string>::iterator iter;
            for (iter = NFSShareIDList.begin(); iter != NFSShareIDList.end(); iter++) {
                int ret = DeleteNFSShareClient(iter->second);
                if (ret != SUCCESS) {
                    HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Delete NFS share client failed! IP:"
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
                    HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Delete NFS share client failed! IP:"
                                                        << iter->first << " client id:" << iter->second << HCPENDLOG;
                    return FAILED;
                }
                NFSShareIDList.erase(iter);
            }
        }
        return SUCCESS;
    }

    int OceanstorNasNFS::DeleteNFSShareClient(std::string shareClientId) {
        HttpRequest req;
        req.method = "DELETE";
        req.url = "NFS_SHARE_AUTH_CLIENT/" + shareClientId;
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    static std::string StripSlash(std::string path)
    {
        while (!path.empty() && path.front() == '/') {
            path.erase(path.begin());
        }
        while (!path.empty() && path.back() == '/') {
            path.pop_back();
        }
        return path;
    }

    int OceanstorNasNFS::GetFsNameFromShareName() {
        if (vstoreId.empty()) {
            GetVstoreId();
        }
        HttpRequest req;
        req.method = "GET";
        std::string errorDes;
        Json::Value data;
        int iRet;
        int errorCode = FAILED;
        std::string resourceName = ResourceName;
        ModifySpecialCharForURL(resourceName);
        req.url = "NFSHARE?range=[0-100]&filter=SHAREPATH::/" + resourceName;
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "&vstoreId=" + vstoreId;
        }
        iRet = SendRequest(req, data, errorDes, errorCode);
        std::string sharePathStripped = StripSlash(ResourceName);
        ModifySpecialCharForFSNameCheck(sharePathStripped);
        HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << sharePathStripped << HCPENDLOG;
        if (iRet == SUCCESS && data.size() > 0 && errorCode == SUCCESS) {
            for (const auto& dataTraverse : data) {
                HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << dataTraverse["SHAREPATH"].asString() << HCPENDLOG;
                if (sharePathStripped.compare(StripSlash(dataTraverse["SHAREPATH"].asString())) == 0) {
                    fileSystemName = sharePathStripped;
                    return SUCCESS;
                }
            }
        }
        return FAILED;
    }


    std::unique_ptr <ControlDevice> OceanstorNasNFS::CreateSnapshot(std::string SnapshotName, int &errorCode) {
        std::string id;
        ControlDeviceInfo deviceInfo;
        deviceInfo.deviceName = SnapshotName;
        deviceInfo.url = OceanstorIP;
        deviceInfo.port = OceanstorPort;
        deviceInfo.userName = OceanstorUsername;
        deviceInfo.password = OceanstorPassword;
        deviceInfo.poolId = OceanstorPoolId;
        deviceInfo.compress = Compress;
        deviceInfo.dedup = Dedup;

        int ret = QuerySnapshot(SnapshotName, id);
        if (ret == SUCCESS) {
            return std::make_unique<OceanstorNasSnapshot>(deviceInfo, fileSystemId, vstoreId, "/" + fileSystemName + "/");
        }

        HttpRequest req;
        req.method = "POST";
        req.url = "FSSNAPSHOT";
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        jsonValue["PARENTTYPE"] = OCEANSTOR_DEFAULT_PARENT_TYPE;
        jsonValue["NAME"] = SnapshotName;
        jsonValue["PARENTID"] = fileSystemId;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return std::make_unique<OceanstorNasSnapshot>(deviceInfo, fileSystemId, vstoreId, "/" + fileSystemName + "/");
        } else {
            return nullptr;
        }
    }

    void OceanstorNasNFS::ModifySpecialCharForURL(std::string &stringName) {
        HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << stringName << HCPENDLOG;
        boost::replace_all(stringName, "%", "%25");
        boost::replace_all(stringName, " ", "%20");
        boost::replace_all(stringName, "\\", "\\\\");
        boost::replace_all(stringName, "#", "%23");
        boost::replace_all(stringName, "&", "%26");
        boost::replace_all(stringName, "+", "%2B");
        boost::replace_all(stringName, "=", "%3D");
        boost::replace_all(stringName, ":", "\\:");
        boost::replace_all(stringName, "!", "\\!");
        HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << stringName << HCPENDLOG;
        return;
    }

    void OceanstorNasNFS::ModifySpecialCharForFSNameCheck(std::string &stringName) {
        HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << stringName << HCPENDLOG;
        boost::replace_all(stringName, "&", "&amp;");
        boost::replace_all(stringName, "(", "&#40;");
        boost::replace_all(stringName, ")", "&#41;");
        boost::replace_all(stringName, "<", "&lt;");
        boost::replace_all(stringName, ">", "&gt;");
        HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << stringName << HCPENDLOG;
        return;
    }
}

