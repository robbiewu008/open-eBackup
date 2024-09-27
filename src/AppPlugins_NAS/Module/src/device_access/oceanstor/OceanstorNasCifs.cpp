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
#include "device_access/oceanstor/OceanstorNasCifs.h"
#include "device_access/oceanstor/OceanstorNasNFS.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "device_access/oceanstor/OceanstorNas.h"
#include "common/JsonUtils.h"
//#include "common/Utility.h"
#include "system/System.hpp"
#include "common/Path.h"
#include "common/CleanMemPwd.h"
#include "device_access/oceanstor/OceanstorNasSnapshot.h"
#include "device_access/oceanstor/DataMoverUtility.h"

namespace Module {
    OceanstorNasCIFS::~OceanstorNasCIFS() {
    }

    int OceanstorNasCIFS::Bind(HostInfo &host, const std::string &shareId) {
        int iRet;
        std::string userName = host.chapAuthName;
        std::string password = host.chapPassword;

        DeviceDetails info;
        iRet = Query(info);
        if (iRet != SUCCESS) {
            return iRet;
        }

        iRet = CIFSShareAddClient(userName, info.deviceId, host.domainName);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Add  User to Cifsshare Failed! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }

        // clear memeory storage password
        CleanMemoryPwd(password);
        return SUCCESS;
    }

    int OceanstorNasCIFS::UnBind(HostInfo host, const std::string &shareId) {
        HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "do not need to unbind!" << HCPENDLOG;
        return SUCCESS;
    }

    int OceanstorNasCIFS::Query(DeviceDetails &info) {
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Query filesystem failure! errorCode:" << iRet << HCPENDLOG;
            return iRet;
        }
        return QueryCifsShare(info, fileSystemId);
    }

    int OceanstorNasCIFS::QueryFileSystem(DeviceDetails &info) {
        HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << "CIFS QueryFileSystem Enter" << HCPENDLOG;
        fileSystemName = ResourceName;
        int ret = OceanstorNas::QueryFileSystem(fileSystemName, info);
        if (ret != SUCCESS) {
            return ret;
        }
        fileSystemId = std::to_string(info.deviceId);
        return SUCCESS;
    }

    int OceanstorNasCIFS::CIFSShareAddClient(std::string name, int ID, const std::string &domainName) {
        HttpRequest req;
        req.method = "POST";
        req.url = "CIFS_SHARE_AUTH_CLIENT";
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["NAME"] = domainName.empty() ? name : domainName + "\\" + name;
        jsonValue["PARENTID"] = ID;
        jsonValue["DOMANTYPE"] = domainName.empty() ? DOMAINTYPE_ENUM::LOCAL_USER : DOMAINTYPE_ENUM::AD_USER;
        jsonValue["PERMISSION"] = PERMISSION_ENUM::FULL_CONTROL;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode, true);
        if (iRet == SUCCESS && (errorCode == SUCCESS || errorCode == OceanstorErrorCode::CIFSSHAREALREADYEXIST)) {
            return SUCCESS;
        }
        HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Add Cifs Client Failed!" << HCPENDLOG;
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int OceanstorNasCIFS::DeleteCIFSShare(DeviceDetails info) {
        HttpRequest req;
        int iRet;
        req.method = "DELETE";
        req.url = "CIFSHARE/" + std::to_string(info.deviceId);
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                   || errorCode == OceanstorErrorCode::FILESYSTEMNOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int OceanstorNasCIFS::Delete() {
        DeviceDetails info;
        int iRet = QueryFileSystem(info);
        if (iRet != SUCCESS) {
            HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "File System not exist!" << HCPENDLOG;
            return SUCCESS;
        }

        iRet = QueryCifsShare(info, fileSystemId);
        if (iRet == SUCCESS) {
            iRet = DeleteCIFSShare(info);
            if (iRet != SUCCESS) {
                HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Delete cifs share failed!" << HCPENDLOG;
                return FAILED;
            }
        }

        // 删除NFS共享
        ControlDeviceInfo deviceInfo;
        deviceInfo.url = OceanstorIP;
        deviceInfo.port = OceanstorPort;
        deviceInfo.userName = OceanstorUsername;
        deviceInfo.password = OceanstorPassword;
        deviceInfo.poolId = OceanstorPoolId;
        deviceInfo.compress = Compress;
        deviceInfo.dedup = Dedup;
        OceanstorNasNFS oceanstorNas(deviceInfo, fileSystemId);
        oceanstorNas.SetResourceName(this->ResourceName);
        oceanstorNas.DeleteShare();

        HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << "Delete cifs clone Start!" << HCPENDLOG;
        if (isDeleteParentSnapShot) {
            iRet = DeleteFileSystemAndParentSnapshot();
        } else {
            iRet = DeleteFileSystem();
        }
        if (iRet != SUCCESS) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "Delete cifs filesystem failed" << HCPENDLOG;
            return FAILED;
        }
        HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "Delete cifs share and filesystem success!" << HCPENDLOG;
        return SUCCESS;
    }

    int OceanstorNasCIFS::CreateShare() {
        DeviceDetails info;
        int ret = QueryFileSystem(info);
        if (ret != SUCCESS) {
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << "FileSystem not exist! errorCode:" << ret << HCPENDLOG;
            return FAILED;
        }
        return CreateCifsShare(ResourceName, std::to_string(info.deviceId));
    }

    int OceanstorNasCIFS::CreateCifsShare(std::string fileSystemName, std::string FsId) {
        HttpRequest req;
        req.method = "POST";
        req.url = "CIFSHARE";
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
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
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                   || errorCode == OceanstorErrorCode::CIFSSHAREALREADYEXIST)) {
            DeviceDetails info;
            HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "Create cifs share finished!" << HCPENDLOG;
            return QueryCifsShare(info, fileSystemId);
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int OceanstorNasCIFS::QueryCifsShare(DeviceDetails &info, std::string fsId) {
        HttpRequest req;
        req.method = "GET";
        req.url = "CIFSHARE?filter=FSID::" + fsId;
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "&vstoreId=" + vstoreId;
        }
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data[0].size() > 0) {
            info.deviceId = atoi(data[0]["ID"].asString().c_str());
            info.deviceUniquePath = data[0]["NAME"].asString();
            HCP_Log(INFO, OCEANSTOR_MODULE_NAME) << "Filesystem query Success!" << HCPENDLOG;
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int OceanstorNasCIFS::DeleteWindowsUser(std::string userName) {
        HttpRequest req;
        req.method = "DELETE";
        req.url = "WINDOWS_USER?NAME=" + userName;
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "&vstoreId=" + vstoreId;
        }
        std::string errorDes;
        int errorCode = FAILED;
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && (errorCode == SUCCESS
                                   || errorCode == OceanstorErrorCode::WINDOWSUSERNOTEXIST)) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    int OceanstorNasCIFS::CreateWindowUser(std::string userName, std::string password) {
        HttpRequest req;
        req.method = "POST";
        req.url = "WINDOWS_USER";
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "?vstoreId=" + vstoreId;
        }
        Json::Value jsonValue;
        Json::FastWriter jsonWriter;
        std::string errorDes;
        int errorCode = FAILED;
        jsonValue["NAME"] = userName;
        jsonValue["PASSWORD"] = password;
        req.body = jsonWriter.write(jsonValue);
        Json::Value data;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && data[0].size() > 0 && errorCode == SUCCESS) {
            return SUCCESS;
        }
        return (errorCode == 0) ? FAILED : errorCode;
    }

    std::unique_ptr <ControlDevice> OceanstorNasCIFS::CreateClone(std::string volumeName, int &errorCode) {
        DeviceDetails info;
        std::string cloneFsId;
        ControlDeviceInfo deviceInfo = {};
        int ret = QueryFileSystem(info);
        if (ret != SUCCESS) {
            errorCode = ret;
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << ResourceName
                                                << " source filesystem is not exist!" << HCPENDLOG;
            return nullptr;
        }

        ret = CreateCloneFileSystem(volumeName, cloneFsId);
        if (ret != SUCCESS) {
            errorCode = ret;
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << volumeName
                                                << " create clone filesystem failed! errorCode:" << ret << HCPENDLOG;
            return nullptr;
        }
        ret = CreateCifsShare(volumeName, cloneFsId);
        if (ret != SUCCESS) {
            errorCode = ret;
            HCP_Log(ERR, OCEANSTOR_MODULE_NAME) << volumeName << " create clone filesystem share failed!" <<
                                                HCPENDLOG;
            return nullptr;
        }
        AssignDeviceInfo(deviceInfo, volumeName);
        return std::make_unique<OceanstorNasCIFS>(deviceInfo, cloneFsId);
    }

    int OceanstorNasCIFS::GetFsNameFromShareName() {
        if (vstoreId.empty()) {
            GetVstoreId();
        }
        HttpRequest req;
        req.method = "GET";
        std::string errorDes;
        Json::Value data;
        int iRet;
        int errorCode = FAILED;
        std::string resourceName = DataMoverUtility::UrlEncode(ResourceName);
        /* OceanStor 使用的逻辑字符串 "!", " or ", " and " 需要添加转义字符'\', %20为URLEncode编码后的空格字符 */
        boost::replace_all(resourceName, "!", "\\!");
        boost::replace_all(resourceName, "%20or%20", "%20\\or%20");
        boost::replace_all(resourceName, "%20and%20", "%20\\and%20");
        req.url = "CIFSHARE?filter=NAME:" + resourceName;
        if (!vstoreId.empty() && vstoreId != "0") {
            req.url += "&vstoreId=" + vstoreId;
        }
        iRet = SendRequest(req, data, errorDes, errorCode);
        std::string shareName = DataMoverUtility::OceanStorSeriesEscapeChar(ResourceName);
        if (iRet == SUCCESS && data.size() > 0 && errorCode == SUCCESS) {
            for (auto dataTraverse : data) {
                if (shareName.compare(dataTraverse["NAME"].asString()) == 0) {
                    std::string path = dataTraverse["SHAREPATH"].asString();
                    fileSystemName = path.substr(1, path.size() - OCEANSTOR_NUMBER_TWO);
                    return SUCCESS;
                }
            }
        }
        return FAILED;
    }

    std::unique_ptr <ControlDevice> OceanstorNasCIFS::CreateSnapshot(std::string SnapshotName, int &errorCode) {
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
        if (fileSystemId.empty()) {
            ret = GetFsNameFromShareName();
            if (ret != SUCCESS) {
                return nullptr;
            }

            DeviceDetails info;
            ret = OceanstorNas::QueryFileSystem(fileSystemName, info);
            if (ret != SUCCESS) {
                return nullptr;
            }
            fileSystemId = std::to_string(info.deviceId);
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
}

