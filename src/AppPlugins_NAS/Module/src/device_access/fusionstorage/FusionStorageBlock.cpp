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
#include "device_access/fusionstorage/FusionStorageBlock.h"
#include "device_access/fusionstorage/FSBlockSnapshot.h"
#include "common/JsonUtils.h"
#include "system/System.hpp"

namespace Module {
    namespace {
        constexpr int ERRORCODE_VOLUME_NOTEXIST = 50150005;
        constexpr int ERRORCODE_SNAPSHOT_NOTEXIST = 50150006;
        const std::string USER_NAME = "user_name";
        const std::string PASSWORD = "password";
        const std::string VOLUMENAME = "name";
        const std::string POOLID = "pool_id";
        const std::string ENABLE_DEDUP = "enable_deduplication";
        const std::string ENABLE_COMPRESS = "enable_compression";
        const std::string PORTNAMES = "portNames";
        const std::string PORTLIST = "portList";
        const std::string PORTNAME = "portName";
        const std::string HOSTNAME = "hostName";
        const std::string IPADDRESS = "ipAddress";
        const std::string HOSTCOUNT = "hostCount";
        const std::string HOSTLUNLIST = "hostLunList";
        const std::string LUNID = "lunId";
        const std::string LUNNAMES = "lunNames";
        constexpr int SEND_REQ_RETRY_INTERVAL = 10;
    } // namespace

/*
login FusionStorage and get token
Date : 2020/03/03
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.login FusionStorage and get token
             2.cache token for this instance
*/

    SessionInfo FusionStorageBlock::Login() {
        HCP_Log(DEBUG, FUSION_STORAGE_MODULE_NAME)
                << "Start get FusionStorage " << FusionStorageIP << " token." << HCPENDLOG;
        Json::Value jsonReq;
        jsonReq[USER_NAME] = FusionStorageUsername;
        jsonReq[PASSWORD] = FusionStoragePassword;
        Json::FastWriter jsonWriter;
        HttpRequest req;
        req.method = "POST";
        req.url = "/api/v2/aa/sessions";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        SessionInfo sessionInfo{};
        sessionInfo.device_id = "XXX";
        sessionInfo.cookie = "XXX";
        std::string errorDes;
        int errorCode;
        int iRet = SendRequestEx(req, data, errorDes, errorCode, sessionInfo);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            if (data.isMember("x_auth_token")) {
                sessionInfo.token = data["x_auth_token"].asString();
            } else {
                iRet = FAILED;
            }
        }
        SetErrorCode(errorCode);
        SetExtendInfo(errorDes);
        return sessionInfo;
    }

/*
logout at destruct this instance
Date : 2020/03/03
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.logout at destruct this instance
*/
    int FusionStorageBlock::Logout(SessionInfo sessionInfo) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start Authentication Exit. " << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        req.method = "DELETE";
        req.url = "/api/v2/aa/sessions";
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequestEx(req, data, errorDes, errorCode, sessionInfo);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

    int FusionStorageBlock::SendRequestEx(HttpRequest &req, Json::Value &data, std::string &errorDes,
                                          int &errorCode, SessionInfo &sessionInfo) {
        if (fs_pHttpCLient == NULL) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "HttpClient is NULL. " << HCPENDLOG;
            return FAILED;
        }
        HttpRequest request = req;
        req.url = "https://" + FusionStorageIP + ":" + FusionStoragePort + req.url;
        req.heads.insert(std::make_pair("Accept", "application/json"));
        req.heads.insert(std::make_pair("Accept-Encoding", "gzip"));
        req.heads.insert(std::make_pair("Accept-Language", "en"));
        req.heads.insert(std::make_pair("Cache-Control", "no-cache"));
        req.heads.insert(std::make_pair("Connection", "keep-alive"));
        req.heads.insert(std::make_pair("X-Auth-Token", sessionInfo.token));
        req.url = FormatFullUrl(req.url);
        std::shared_ptr <IHttpResponse> rsp = fs_pHttpCLient->SendRequest(req);
        if (rsp == NULL) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Return response is empty. " << HCPENDLOG;
            return FAILED;
        }
        if (!rsp->Success()) {
            if (rsp->GetErrCode() == 0) {
                errorDes = rsp->GetHttpStatusDescribe();
                HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "StatusCode: " << rsp->GetHttpStatusCode() <<
                                                         " , Http response error. Error is " << errorDes << HCPENDLOG;
                return rsp->GetHttpStatusCode();  // return http status code
            } else {
                errorDes = rsp->GetErrString();
                HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "StatusCode: " << rsp->GetHttpStatusCode() <<
                                                         " , Send http request occur network error. Error is "
                                                         << errorDes << HCPENDLOG;
                return FAILED;
            }
        } else {
            if (req.url.find("/v2/") != std::string::npos) {
                return ParseBodyV2(rsp->GetBody(), data, errorDes, errorCode);
            } else {
                return ParseBodyV1(rsp->GetBody(), data, errorDes, errorCode);
            }
        }
        return SUCCESS;
    }


    void FusionStorageBlock::Clean() {
        DeleteDeviceSession();
    }

/*
Create fusionstorage Lun
Date : 2020/03/03
out params:id -> LUN id.
          :WWN  -> LUN WWN.
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.Create fusionstorage Lun
*/

    int FusionStorageBlock::Create(unsigned long long size) {
        LunParams lun(ResourceName, Compress, Dedup, FusionStoragePoolId, size);
        int ret = CreateLUN(lun, ResourceId, Wwn);
        if (ret != SUCCESS) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Create LUN failed!" << HCPENDLOG;
            return FAILED;
        }
        return SUCCESS;
    }

    int FusionStorageBlock::Bind(HostInfo &host, const std::string &shareId) {
        int ret = CreateHost(host.hostId, host.hostIp);
        if (ret != SUCCESS) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Create host failed!"
            HCPENDLOG;
            return FAILED;
        }

        for (auto iter = host.iscsinitor.begin(); iter != host.iscsinitor.end(); ++iter) {
            if (iter->second == ISCSI) {
                ret = CreateISCSIPort(iter->first);
                if (ret != SUCCESS) {
                    HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Create ISCSI Port failed!" << HCPENDLOG;
                    return FAILED;
                }

                ret = BindInitiator(host.hostId, iter->first);
                if (ret != SUCCESS) {
                    HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Bind failed!" << HCPENDLOG;
                    return FAILED;
                }
            }
        }

        ret = CreateHostMapping(host.hostId, ResourceName);
        if (ret != SUCCESS) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Create Host Mapping failed!" << HCPENDLOG;
            return FAILED;
        }
        return ret;
    }

    int FusionStorageBlock::CreateLUN(LunParams params, int &id, std::string &WWN) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start create lun " << params.volumeName << HCPENDLOG;
        int mpRet = QueryLUN(params.volumeName, id, WWN, params.Size, params.usedSize);
        if (mpRet == SUCCESS) {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "already exist lun " << params.volumeName << HCPENDLOG;
            return SUCCESS;
        }
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq[VOLUMENAME] = params.volumeName;
        jsonReq[POOLID] = params.poolid;
        jsonReq[CAPACITY] = (Json::UInt64) params.Size;
        jsonReq[ENABLE_COMPRESS] = params.Compress;
        jsonReq[ENABLE_DEDUP] = params.Dedup;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "/api/v2/block_service/volumes";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data["id"].asInt() != 0) {
            return QueryLUN(params.volumeName, id, WWN, params.Size, params.usedSize);
        } else {
            return FAILED;
        }
    }

    int FusionStorageBlock::Query(DeviceDetails &info) {
        unsigned long long size = 0;
        unsigned long long usedSize = 0;
        int ret = QueryLUN(ResourceName, ResourceId, Wwn, size, usedSize);
        if (ret != SUCCESS) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Query LUN failed. " << HCPENDLOG;
            return FAILED;
        }
        const int forward = 2;
        if (Wwn.find("0x") != std::string::npos) {
            Wwn = Wwn.substr(Wwn.find("0x") + forward);
        } else {
            Wwn = Wwn;
        }
        info.deviceId = ResourceId;
        info.deviceName = ResourceName;
        info.deviceUniquePath = Wwn;
        info.Compress = Compress;
        info.Dedup = Dedup;
        info.totalCapacity = size;
        info.usedCapacity = usedSize;
        return ret;
    }

/*
query fusionstorage Lun by lun name
Date : 2020/03/03
out params:id -> LUN id.
          :WWN  -> LUN WWN.
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.query fusionstorage Lun by lun name
*/
    int FusionStorageBlock::QueryLUN(
            std::string volumeName, int &id, std::string &WWN, unsigned long long &size, unsigned long long &usedSize) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start query lun " << volumeName << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/v2/block_service/volumes?name=" + volumeName;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data["id"].asInt() != 0) {
            id = data["id"].asInt();
            WWN = data["wwn"].asString();
            size = data["capacity"].asInt();
            usedSize = data["used_capacity"].asInt();
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

/*
create fusionstorage host by host name and host ip address
Date : 2020/03/03
out params:HOST -> host name.equal with UUID
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.create fusionstorage host by host name and host ip address
*/

    int FusionStorageBlock::CreateHost(const std::string UUID, const std::string ip) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start create host " << UUID << HCPENDLOG;
        int mpRet = QueryHost(UUID);
        if (mpRet == SUCCESS) {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "already exist host" << UUID << HCPENDLOG;
            return SUCCESS;
        }
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq[HOSTNAME] = UUID;
        jsonReq[IPADDRESS] = ip;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "/dsware/service/iscsi/createHost";
        req.body = jsonWriter.write(jsonReq);
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

/*
query fusionstorage host by host name
Date : 2020/03/03
out params:HOST -> host name.equal with UUID
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.query fusionstorage host by host name
*/
    int FusionStorageBlock::QueryHost(const std::string UUID) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start query host " << UUID << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "/dsware/service/iscsi/queryAllHost?hostName=" + UUID;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS && data[HOSTCOUNT].asInt() == 1) {
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

/*
create fusionstorage port with iscsi connector
Date : 2020/03/03
out params:iscsi -> port name.equal with InitiatorName
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.create fusionstorage port with iscsi connector
*/

    int FusionStorageBlock::CreateISCSIPort(const std::string InitiatorName) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start create iscsi port " << InitiatorName << HCPENDLOG;
        int mpRet = QueryISCSIPort(InitiatorName);
        if (mpRet == SUCCESS) {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "already exist iscsi port " << InitiatorName << HCPENDLOG;
            return SUCCESS;
        }
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq[PORTNAME] = InitiatorName;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "/dsware/service/iscsi/createPort";
        req.body = jsonWriter.write(jsonReq);
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

/*
query fusionstorage port with iscsi connector
Date : 2020/03/03
out params:iscsi -> port name.equal with InitiatorName
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.query fusionstorage port with iscsi connector
*/
    int FusionStorageBlock::QueryISCSIPort(const std::string InitiatorName) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start query iscsi port " << InitiatorName << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq[PORTNAME] = InitiatorName;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "/dsware/service/iscsi/queryPortInfo";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        HCP_Log(DEBUG, FUSION_STORAGE_MODULE_NAME)
                << iRet << "." << errorCode << "port list size: " << data[PORTLIST].size() << HCPENDLOG;
        if (iRet == SUCCESS && errorCode == SUCCESS && data[PORTLIST].size() == 1) {
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

/*
bind fusionstorage port to host
Date : 2020/03/03
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.bind fusionstorage port to host
*/

    int FusionStorageBlock::BindInitiator(std::string hostName, std::string iscsiPort) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                << "Start bind iscsi port " << hostName << "," << iscsiPort << HCPENDLOG;
        int mpRet = QueryHostISCSIPort(hostName, iscsiPort);
        if (mpRet == SUCCESS) {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                    << "already bind iscsi port " << hostName << "," << iscsiPort << HCPENDLOG;
            return SUCCESS;
        }
        HttpRequest req;
        Json::Value jsonReq;
        Json::Value portList;
        portList.append(Json::Value(iscsiPort));
        jsonReq[HOSTNAME] = hostName;
        jsonReq[PORTNAMES] = portList;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "/dsware/service/iscsi/addPortToHost";
        req.body = jsonWriter.write(jsonReq);
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

/*
query is fusionstorage port bind to this host
Date : 2020/03/03
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.query is fusionstorage port bind to this host
*/
    int FusionStorageBlock::QueryHostISCSIPort(std::string hostName, std::string iscsiPort) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                << "Start query iscsi port bind " << hostName << "," << iscsiPort << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq[HOSTNAME] = hostName;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "/dsware/service/iscsi/queryPortFromHost";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            for (int i = 0; i < data[PORTLIST].size(); i++) {
                if (data[PORTLIST][i] == iscsiPort) {
                    return SUCCESS;
                }
            }
        }
        return FAILED;
    }

/*
attach lun to special host
Date : 2020/03/03
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.attach lun to special host
*/

    int FusionStorageBlock::CreateHostMapping(const std::string hostName, const std::string lunName) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                << "Start create host mapping" << hostName << "," << lunName << HCPENDLOG;
        int mpRet = QueryHostMapping(hostName, lunName);
        if (mpRet == SUCCESS) {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                    << "Already create host mapping " << hostName << "," << lunName << HCPENDLOG;
            return SUCCESS;
        }
        HttpRequest req;
        Json::Value jsonReq;
        Json::Value lunList;
        lunList.append(Json::Value(lunName));
        jsonReq[HOSTNAME] = hostName;
        jsonReq[LUNNAMES] = lunList;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "/dsware/service/iscsi/addLunsToHost";
        req.body = jsonWriter.write(jsonReq);
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

    int FusionStorageBlock::UnBind(HostInfo host, const std::string &shareId) {
        return DeleteHostMapping(host.hostId, ResourceName);
    }

    int FusionStorageBlock::DeleteHostMapping(const std::string hostName, const std::string lunName) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                << "Start delete host mapping" << hostName << "," << lunName << HCPENDLOG;
        int mpRet = QueryHostMapping(hostName, lunName);
        if (mpRet == MP_NOTEXIST) {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                    << "not exist host mapping " << hostName << "," << lunName << HCPENDLOG;
            return SUCCESS;
        }
        HttpRequest req;
        Json::Value jsonReq;
        Json::Value lunList;
        lunList.append(Json::Value(lunName));
        jsonReq[HOSTNAME] = hostName;
        jsonReq[LUNNAMES] = lunList;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "/dsware/service/iscsi/deleteLunFromHost";
        req.body = jsonWriter.write(jsonReq);
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

/*
query is lun attached to special host
Date : 2020/03/03
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.query is lun attached to special host
*/
    int FusionStorageBlock::QueryHostMapping(const std::string hostName, const std::string lunName) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                << "query host mapping" << hostName << "," << lunName << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq[HOSTNAME] = hostName;
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "/dsware/service/iscsi/queryHostLunInfo";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            for (int i = 0; i < data[HOSTLUNLIST].size(); i++) {
                Json::Value lunIDAndName = data[HOSTLUNLIST][i];
                if (lunIDAndName["lunName"].asString() == lunName) {
                    return SUCCESS;
                }
            }
            return MP_NOTEXIST;
        }
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
    std::unique_ptr <ControlDevice> FusionStorageBlock::CreateClone(std::string volumeName, int &errorCode) {
        unsigned long long size;
        unsigned long long usedSize;
        int id;
        std::string wwn;
        ControlDeviceInfo deviceInfo = {};
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                << "Start create clone from volume " << ResourceName << ",volume " << volumeName << HCPENDLOG;
        int mpRet = QueryLUN(volumeName, id, wwn, size, usedSize);
        if (mpRet == SUCCESS) {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "already exist lun " << volumeName << HCPENDLOG;
        } else {
            HttpRequest req;
            Json::Value jsonReq;
            jsonReq["name"] = volumeName;
            jsonReq["source_volume_name"] = ResourceName;
            Json::FastWriter jsonWriter;
            req.method = "POST";
            req.url = "/api/v2/block_service/clone_volumes";
            req.body = jsonWriter.write(jsonReq);
            Json::Value data;
            std::string errorDes;
            int iRet = SendRequest(req, data, errorDes, errorCode);
            if (iRet == SUCCESS && errorCode == SUCCESS && data["id"].asInt() != 0) {
                QueryLUN(volumeName, id, wwn, size, usedSize);
            } else {
                return nullptr;
            }
        }
        deviceInfo.deviceName = volumeName;
        deviceInfo.url = FusionStorageIP;
        deviceInfo.port = FusionStoragePort;
        deviceInfo.userName = FusionStorageUsername;
        deviceInfo.password = FusionStoragePassword;
        deviceInfo.poolId = FusionStoragePoolId;
        return std::make_unique<FusionStorageBlock>(deviceInfo);
    }

/*
delete volume with name
Date : 2020/03/03
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.delete volume with name
*/
    int FusionStorageBlock::Delete() {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start delete lun " << ResourceName << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["name"] = ResourceName;
        Json::FastWriter jsonWriter;
        req.method = "DELETE";
        req.url = "/api/v2/block_service/volumes";
        req.body = jsonWriter.write(jsonReq);
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

/*
create snapshot for special volume
Date : 2020/03/03
out params:id -> snapshot ID
          :WWN -> snapshot WWN
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.create snapshot for special volume
*/
    std::unique_ptr <ControlDevice> FusionStorageBlock::CreateSnapshot(std::string SnapshotName, int &errorCode) {
        int id;
        std::string sWwn;
        ControlDeviceInfo deviceInfo = {};
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                << "Start create " << ResourceName << " snapshot " << SnapshotName << HCPENDLOG;
        int mpRet = QuerySnapshot(SnapshotName, id, sWwn);
        if (mpRet == SUCCESS) {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "already exist snapshot. " << SnapshotName << HCPENDLOG;
        } else {
            HttpRequest req;
            Json::Value jsonReq;
            jsonReq["volume_name"] = ResourceName;
            jsonReq["name"] = SnapshotName;
            Json::FastWriter jsonWriter;
            req.method = "POST";
            req.url = "/api/v2/block_service/snapshots";
            req.body = jsonWriter.write(jsonReq);
            Json::Value data;
            std::string errorDes;
            int iRet = SendRequest(req, data, errorDes, errorCode);
            if (iRet == SUCCESS && errorCode == SUCCESS && data["id"].asInt() != 0) {
                mpRet = QuerySnapshot(SnapshotName, id, sWwn);
                if (mpRet != SUCCESS) {
                    return nullptr;
                }
            }
        }
        deviceInfo.deviceName = SnapshotName;
        deviceInfo.url = FusionStorageIP;
        deviceInfo.port = FusionStoragePort;
        deviceInfo.userName = FusionStorageUsername;
        deviceInfo.password = FusionStoragePassword;
        deviceInfo.poolId = FusionStoragePoolId;
        return std::make_unique<FSBlockSnapshot>(deviceInfo, id, sWwn);
    }

    int FusionStorageBlock::QuerySnapshot(std::string SnapshotName, int &id, std::string &WWN) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start query snapshot " << SnapshotName << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "/dsware/service/v1.3/snapshot/queryByName?snapshotName=" + SnapshotName;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            id = data["snapshot"]["snapshotId"].asInt();
            WWN = data["snapshot"]["wwn"].asString();
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

/*
query all iscsi host,need Manual open on DeviceManager.
Date : 2020/03/03
out params:iscsiList -> iscsi ip and port List,value like ["1.1.1.1:8000","1.1.1.2:8000"]
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.query all iscsi host,need Manual open on DeviceManager.
*/
    int FusionStorageBlock::QueryServiceHost(std::vector<std::string> &iscsiList, IP_TYPE ipType) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start query iscsi host " << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["need"] = "all";
        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "/dsware/service/v1.3/iscsi/port/list";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            IterateIscsiHost(data, iscsiList);
            return SUCCESS;
        } else {
            return FAILED;
        }
    }

    void
    FusionStorageBlock::IterateIscsiHostAction(const Json::Value &iscsiPortalList, std::vector<std::string> &iscsiList) {
        for (int v = 0; v < iscsiPortalList.size(); v++) {
            if (iscsiPortalList[v]["iscsiStatus"].asString() == "active") {
                iscsiList.push_back(iscsiPortalList[v]["iscsiPortal"].asString());
                HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                        << "push back iscsiPortal" << iscsiPortalList[v]["iscsiPortal"].asString() << HCPENDLOG;
            }
        }
    }

    int FusionStorageBlock::IterateIscsiHost(Json::Value data, std::vector<std::string> &iscsiList) {
        for (int i = 0; i < data["nodeResultList"].size(); i++) {
            Json::Value oneNode = data["nodeResultList"][i];
            if (oneNode["status"].asString() == "successful") {
                Json::Value iscsiPortalList = oneNode["iscsiPortalList"];
                IterateIscsiHostAction(iscsiPortalList, iscsiList);
            }
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
    int FusionStorageBlock::ExtendSize(unsigned long long size) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start query iscsi host " << HCPENDLOG;
        HttpRequest req;
        Json::Value jsonReq;
        jsonReq["name"] = ResourceName;
        jsonReq["capacity"] = (Json::UInt64) size;
        Json::FastWriter jsonWriter;
        req.method = "PUT";
        req.url = "/api/v2/block_service/volume_capacity";
        req.body = jsonWriter.write(jsonReq);
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet == SUCCESS && errorCode == SUCCESS) {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                    << "extend volume " << ResourceName << " size to " << size << " success." << HCPENDLOG;
            return SUCCESS;
        } else {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                    << "extend volume " << ResourceName << " size to " << size << " failed." << HCPENDLOG;
            return FAILED;
        }
    }

    void FusionStorageBlock::DeleteDeviceSession() {
        g_fusionStorageSessionCache->DeleteSession(FusionStorageIP, FusionStorageUsername, FusionStoragePort,
                                                   [this](SessionInfo sesInfo) -> int { return Logout(sesInfo); });
    }

    void FusionStorageBlock::CreateDeviceSession() {
        this->sessionPtr = g_fusionStorageSessionCache->CreateSession(FusionStorageIP, FusionStorageUsername,
                                                                      FusionStoragePort,
                                                                      [this]() -> SessionInfo { return Login(); });
    }

    void FusionStorageBlock::LoginAndGetSessionInfo() {
        if (useCache && g_fusionStorageSessionCache != nullptr) {
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

    void FusionStorageBlock::DelayTimeSendRequest() {
        auto now = std::chrono::steady_clock::now();
        while ((double(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() -
                                                                             now).count()) *
                std::chrono::microseconds::period::num / std::chrono::microseconds::period::den) <
               retryIntervalTime) {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Waiting for storage device ... " << HCPENDLOG;
            sleep(SEND_REQ_RETRY_INTERVAL);
        }
        return;
    }

    bool FusionStorageBlock::FusionStorageResposeNeedRetry(const int ret) {
        // when errorCode ==0 && ret == FAILED mean dorado response need retry
        if (ret == FAILED) {
            return true;
        }
        return false;
    }

    void FusionStorageBlock::SetRetryAttr(int _retryTimes, int _retryIntervalTime) {
        retryTimes = _retryTimes;
        retryIntervalTime = _retryIntervalTime;
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "set retry times: " << retryTimes << HCPENDLOG;
    }

    void FusionStorageBlock::SetCurlTimeOut(uint64_t tmpTimeOut) {
        if (tmpTimeOut > MIN_CURL_TIME_OUT) {
            CurlTimeOut = tmpTimeOut;
        }
    }

    void FusionStorageBlock::InitHttpStatusCodeForRetry() {
        ConfigReader::getIntValueVector("MicroService", "HttpStatusCodesForRetry", ",",
                                        httpRspStatusCodeForRetry);
    }

    int FusionStorageBlock::SendRequest(HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode,
                                        bool lockSession) {
        // 检查存储设备是否含有证书和吊销列表信息
        if (!certification.empty()) {
            req.cert = certification;
            req.isVerify = VCENTER_VERIFY;
        }
        if (!crl.empty()) {
            req.revocationList = crl;
        }
        int retryNum = 0;
        while (retryNum < retryTimes) {
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "send request for " << (retryNum + 1) << " time to " <<
                                                      WIPE_SENSITIVE(req.url) << HCPENDLOG;
            int ret = SUCCESS;
            if (this->sessionPtr == nullptr) {
                LoginAndGetSessionInfo();
                if (this->sessionPtr == nullptr) {
                    HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Invalid session" << HCPENDLOG;
                }
            }
            if (this->sessionPtr != nullptr) {
                if (lockSession) {
                    std::lock_guard <std::mutex> lock(this->sessionPtr->sessionMutex);
                    ret = SendRequestOnce(req, data, errorDes, errorCode);
                } else {
                    ret = SendRequestOnce(req, data, errorDes, errorCode);
                }
                if (ret == SUCCESS) {
                    HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "send requests success " << HCPENDLOG;
                    return SUCCESS;
                }
            }
            // 1.when curl success and ret not FAILED, ret is httpStatusCode,
            // so judge whether ret is in httpRspStatusCodeForRetry for retry.
            // 2.when when curl success and ret is FAILED,
            // FusionStorageResposeNeedRetry, not judge http retry code, directly retry.
            // 3.when errorCode not 0,mean curl failed,directly retry.
            if (errorCode == 0 && !FusionStorageResposeNeedRetry(ret) &&
                std::find(httpRspStatusCodeForRetry.begin(), httpRspStatusCodeForRetry.end(), ret)
                == httpRspStatusCodeForRetry.end()) {
                HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "not retry send msg for httpstatuscode:" << ret
                                                          << HCPENDLOG;
                break;
            }
            DelayTimeSendRequest();
            retryNum++;
        }
        HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "send request failed. " << HCPENDLOG;
        return FAILED;
    }

    int FusionStorageBlock::SendRequestOnce(HttpRequest req, Json::Value &data, std::string &errorDes, int &errorCode) {
        if (this->sessionPtr == nullptr) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Invalid session" << HCPENDLOG;
            return FAILED;
        }
        int iRet = FAILED;
        HttpRequest request = req;
        request.enableProxy = m_enableProxy;
        request.heads.insert(std::make_pair("Accept", "application/json"));
        request.heads.insert(std::make_pair("Accept-Encoding", "gzip"));
        request.heads.insert(std::make_pair("Accept-Language", "en"));
        request.heads.insert(std::make_pair("Cache-Control", "no-cache"));
        request.heads.insert(std::make_pair("Connection", "keep-alive"));
        request.heads.insert(std::make_pair("X-Auth-Token", this->sessionPtr->token));
        request.url = "https://" + FusionStorageIP + ":" + FusionStoragePort + req.url;
        std::shared_ptr <IHttpResponse> rsp;
        iRet = SendHttpReq(rsp, request, errorDes, errorCode);
        if (iRet != SUCCESS) {
            // get when curl send success,http response error for httpstatuscodeforRetry
            if (errorCode == 0) {
                return iRet;
            }
            return FAILED;
        }
        iRet = ResponseSuccessHandle(req, rsp, data, errorDes, errorCode);
        return iRet;
    }

    int FusionStorageBlock::SendHttpReq(std::shared_ptr <IHttpResponse> &rsp, const HttpRequest &req,
                                        std::string &errorDes, int &errorCode) {
        HttpRequest tempReq = req;
        tempReq.url = FormatFullUrl(tempReq.url);
        rsp = fs_pHttpCLient->SendRequest(tempReq, CurlTimeOut);
        if (rsp == NULL) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Return response is empty. " << HCPENDLOG;
            return FAILED;
        }
        if (!rsp->Success()) {
            // 1.curl success,http response error with http status codes
            if (rsp->GetErrCode() == 0) {
                errorDes = rsp->GetHttpStatusDescribe(); // http status error description
                errorCode = rsp->GetErrCode();
                HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Curl ok,HttpStatusCode: " << rsp->GetHttpStatusCode()
                                                         << " , Http response error. Error is " << errorDes
                                                         << HCPENDLOG;
                return rsp->GetHttpStatusCode(); // return http status code
                // 2.curl error,need directly retry
            } else {
                errorDes = rsp->GetErrString();
                errorCode = rsp->GetErrCode();
                HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << " Curl error. errorCode: " << errorCode
                                                         << "errorDes:" << errorDes << HCPENDLOG;
            }
            SetErrorCode(errorCode);
            return FAILED;
            // 3. curl success, http response success
        } else {
            errorDes = rsp->GetErrString();
            errorCode = rsp->GetErrCode();
            return SUCCESS;
        }
        return SUCCESS;
    }

    int FusionStorageBlock::ResponseSuccessHandle(HttpRequest req,
                                                  std::shared_ptr <IHttpResponse> &rsp, Json::Value &data,
                                                  std::string &errorDes, int &errorCode) {
        int Ret;
        if (req.url.find("/v2/") != std::string::npos) {
            Ret = ParseBodyV2(rsp->GetBody(), data, errorDes, errorCode);
        } else {
            Ret = ParseBodyV1(rsp->GetBody(), data, errorDes, errorCode);
        }
        if (errorCode != SUCCESS) {
            SessionInfo sessionInfo = Login();
            if (sessionInfo.device_id.empty() || sessionInfo.token.empty() || sessionInfo.cookie.empty()) {
                HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Login FusionStorage Failed! deviceId: " <<
                                                         sessionInfo.device_id << HCPENDLOG;
                return FAILED;
            }
            if (this->sessionPtr == nullptr) {
                HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Invalid session" << HCPENDLOG;
                return FAILED;
            }
            // Refresh session
            this->sessionPtr->deviceId = sessionInfo.device_id;
            this->sessionPtr->cookie = sessionInfo.cookie;
            this->sessionPtr->token = sessionInfo.token;
            HttpRequest request = req;
            request.url = "https://" + FusionStorageIP + ":" + FusionStoragePort + req.url;
            request.heads.insert(std::make_pair("Accept", "application/json"));
            request.heads.insert(std::make_pair("Accept-Encoding", "gzip"));
            request.heads.insert(std::make_pair("Accept-Language", "en"));
            request.heads.insert(std::make_pair("Cache-Control", "no-cache"));
            request.heads.insert(std::make_pair("Connection", "keep-alive"));
            request.heads.insert(std::make_pair("X-Auth-Token", this->sessionPtr->token));
            Ret = SendHttpReq(rsp, request, errorDes, errorCode);
            if (Ret != SUCCESS) {
                return FAILED;
            }

            if (req.url.find("/v2/") != std::string::npos) {
                Ret = ParseBodyV2(rsp->GetBody(), data, errorDes, errorCode);
            } else {
                Ret = ParseBodyV1(rsp->GetBody(), data, errorDes, errorCode);
            }
        }
        return Ret;
    }

/*
analisys response package with V2 format
Date : 2020/03/03
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.analisys response package with V2 format
*/
    int FusionStorageBlock::ParseBodyV2(
            const std::string &json_data, Json::Value &data, std::string &errorDes, int &errorCode) {
        Json::Value jsonValue;
        Json::Reader reader;
        if (!reader.parse(json_data, jsonValue)) {
            errorDes = "Parse json string failed";
            return FAILED;
        }
        if (!jsonValue.isMember("result") || !jsonValue["result"].isMember("code") ||
            !jsonValue["result"].isMember("description")) {
            errorDes = "Json object format is error. ";
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Json object error." << HCPENDLOG;
            return FAILED;
        }
        errorDes = jsonValue["result"]["description"].asString();
        errorCode = jsonValue["result"]["code"].asInt();
        if (errorCode != SUCCESS) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME)
                    << "code : " << errorCode << ", Describe : " << errorDes << HCPENDLOG;
            return FAILED;
        }
        if (jsonValue.isMember("data")) {
            data = jsonValue["data"];
        }
        return SUCCESS;
    }

/*
analisys response package with V1 format
Date : 2020/03/03
return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
Description:
             1.analisys response package with V1 format
*/
    int FusionStorageBlock::ParseBodyV1(
            const std::string &json_data, Json::Value &data, std::string &errorDes, int &errorCode) {
        const int errCodeV1 = 1;
        const int errCodeV2 = 2;
        Json::Value jsonValue;
        Json::Reader reader;
        if (!reader.parse(json_data, jsonValue)) {
            errorDes = "Parse json string failed";
            return FAILED;
        }
        if (!jsonValue.isMember("result")) {
            errorDes = "Json object format is error. ";
            return FAILED;
        }
        if (jsonValue["result"] == errCodeV1) {
            for (int i = 0; i < jsonValue["detail"].size(); i++) {
                Json::Value oneErr = jsonValue["detail"][i];
                errorDes = oneErr["description"].asString();
                errorCode = oneErr["errorCode"].asInt();
                HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME)
                        << "code : " << errorCode << ", Describe : " << errorDes << HCPENDLOG;
            }
            return FAILED;
        } else if (jsonValue["result"] == errCodeV2) {
            errorDes = jsonValue["description"].asString();
            errorCode = jsonValue["errorCode"].asInt();
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME)
                    << "code : " << errorCode << ", Describe : " << errorDes << HCPENDLOG;
            return FAILED;
        } else {
            errorDes = "";
            errorCode = 0;
            data = jsonValue;
        }
        return SUCCESS;
    }

    bool FusionStorageBlock::GetJsonValue(const Json::Value &jsValue, std::string strKey, std::string &strValue) {
        if (jsValue.isArray()) {
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Json is Array." << HCPENDLOG;
            return false;
        }
        if (jsValue.isMember(strKey)) {
            if (jsValue[strKey].isString()) {
                strValue = jsValue[strKey].asString();
                HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Json value is:" << WIPE_SENSITIVE(strValue) << HCPENDLOG;
                return true;
            } else {
                HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME)
                        << "The value Json key " << WIPE_SENSITIVE(strKey) << "is not string." << HCPENDLOG;
                return false;
            }
        } else {
            strValue = "";
            HCP_Log(ERR, FUSION_STORAGE_MODULE_NAME) << "Json key " << WIPE_SENSITIVE(strKey) << "does not exist."
                                                     << HCPENDLOG;
            return false;
        }
    }

    int FusionStorageBlock::Revert(std::string SnapshotName) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME)
                << "Start revert snapshot " << SnapshotName << " to " << ResourceId << HCPENDLOG;

        HttpRequest req;
        Json::Value jsonReq;

        jsonReq["name"] = SnapshotName;
        jsonReq["action"] = "rollback";
        jsonReq["rollback_target_name"] = ResourceId;

        Json::FastWriter jsonWriter;
        req.method = "POST";
        req.url = "/api/v2/block_service/snapshots";
        req.body = jsonWriter.write(jsonReq);
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

    int FusionStorageBlock::QuerySnapshotList(std::vector<FSSnapshotInfo> &snapshots) {
        HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "Start query snapshot list." << HCPENDLOG;
        HttpRequest req;
        req.method = "GET";
        req.url = "/api/v2/block_service/snapshots?volume_name=" + ResourceName;
        Json::Value data;
        std::string errorDes;
        int errorCode;
        int iRet = SendRequest(req, data, errorDes, errorCode);
        if (iRet != SUCCESS || errorCode != SUCCESS) {
            return FAILED;
        }

        for (int i = 0; i < data.size(); i++) {
            FSSnapshotInfo item{};
            Json::Value onesnap = data[i];
            item.snapshotName = onesnap["name"].asString();
            item.volumeName = onesnap["volume_name"].asString();
            snapshots.push_back(item);
        }

        return true;
    }

    int FusionStorageBlock::CreateReplication(
            int localResId, int rResId, std::string rDevId, int bandwidth, std::string &repId) {
        return FAILED;
    }

    int FusionStorageBlock::ActiveReplication(std::string repId) {
        return FAILED;
    }

    int FusionStorageBlock::QueryReplication(ReplicationPairInfo &replicationPairInfo) {
        return FAILED;
    }

    int FusionStorageBlock::DeleteReplication(std::string pairId) {
        return FAILED;
    }

    int FusionStorageBlock::QueryServiceIpController(
            std::vector<std::pair<std::string, std::string>> &ipControllerList, IP_TYPE ipType) {
        return FAILED;
    }

    int FusionStorageBlock::DeleteSnapshot(std::string SnapshotName) {
        return SUCCESS;
    }
}
