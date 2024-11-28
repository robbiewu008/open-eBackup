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
#ifndef FUSION_STORAGE_BLOCK_H
#define FUSION_STORAGE_BLOCK_H

#include "define/Types.h"
#include "log/Log.h"
#include "curl_http/HttpClientInterface.h"
#include "device_access/Const.h"
#include "device_access/ControlDevice.h"
#include "device_access/SessionCache.h"

namespace Module {
    const std::string FUSION_STORAGE_MODULE_NAME = "FusionStorage";
    constexpr int FUSIONSTORE_NUMBER_TWO = 2;

    static std::unique_ptr <SessionCache> g_fusionStorageSessionCache = std::make_unique<SessionCache>(
            FUSION_STORAGE_MODULE_NAME);

    class FusionStorageBlock : public ControlDevice {
    public:
        FusionStorageBlock(ControlDeviceInfo deviceInfo, bool useSharedSession = true)
        {
            fs_pHttpCLient = IHttpClient::GetInstance();
            Compress = deviceInfo.compress;
            Dedup = deviceInfo.dedup;
            FusionStorageIP = deviceInfo.url;
            FusionStoragePort = deviceInfo.port;
            FusionStorageUsername = deviceInfo.userName;
            FusionStoragePassword = deviceInfo.password;
            FusionStoragePoolId = deviceInfo.poolId;
            ResourceName = deviceInfo.deviceName;
            certification = deviceInfo.cert;
            crl = deviceInfo.crl;
            m_enableProxy = deviceInfo.enableProxy;
            HCP_Log(INFO, FUSION_STORAGE_MODULE_NAME) << "enableProxy - " << m_enableProxy << HCPENDLOG;
            this->useCache = useSharedSession;
            this->sessionPtr = nullptr;
            HCP_Log(DEBUG, FUSION_STORAGE_MODULE_NAME)
                    << FusionStorageIP << "," << FusionStoragePort << "," << FusionStoragePoolId
                    << HCPENDLOG;
        }

        virtual ~FusionStorageBlock()
        {
            if (useCache && g_fusionStorageSessionCache != nullptr) {
                if (this->sessionPtr != nullptr) {
                    DeleteDeviceSession();
                }
            } else {
                if (this->sessionPtr != nullptr) {
                    SessionInfo sessionInfo;
                    sessionInfo.token = this->sessionPtr->token;
                    sessionInfo.cookie = this->sessionPtr->cookie;
                    sessionInfo.device_id = this->sessionPtr->deviceId;
                    Logout(sessionInfo);
                }
            }
            if (fs_pHttpCLient != nullptr) {
                delete fs_pHttpCLient;
                fs_pHttpCLient = nullptr;
            }
            CleanMemoryPwd(FusionStoragePassword);
        }

        int Bind(HostInfo &host, const std::string &shareId = "") override;

        int UnBind(HostInfo host, const std::string &shareId = "") override;

        int Mount(DeviceMountInfo mountInfo, const std::string &shareName = "") override
        {
            return FAILED;
        }

        int UnMount(DeviceMountInfo mountInfo) override
        {
            return FAILED;
        }

        int UnMount() override
        {
            return FAILED;
        }

        int CreateShare() override
        {
            return FAILED;
        }

        int QueryContorllerCnt(int &outCnt) override
        {
            return FAILED;
        }

        int Create(unsigned long long size) override;

        int Delete() override;

        int Query(DeviceDetails &info) override;

        std::unique_ptr <ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;

        std::unique_ptr <ControlDevice> CreateClone(std::string cloneName, int &errorCode) override;

        int QueryServiceHost(std::vector<std::string> &iscsiList, IP_TYPE ipType = IP_TYPE::IP_V4) override;

        int QueryServiceIpController(
                std::vector<std::pair<std::string, std::string>> &ipControllerList,
                IP_TYPE ipType = IP_TYPE::IP_V4) override;

        int ExtendSize(unsigned long long size) override;

        int QuerySnapshotList(std::vector<FSSnapshotInfo> &snapshots) override;

        int Revert(std::string SnapshotName) override;

        int QueryRevertInfo(
                const std::string &resourceName, std::string &rollbackRate, std::string &rollbackStatus) override
        {
            return FAILED;
        }

        int CreateReplication(
                int localResId, int rResId, std::string rDevId, int bandwidth, std::string &repId) override;

        int ActiveReplication(std::string repId) override;

        int QueryReplication(ReplicationPairInfo &replicationPairInfo) override;

        int DeleteReplication(std::string pairId) override;

        void Clean() override;

        int DeleteSnapshot(std::string SnapshotName) override;

        void SetRetryAttr(int _retryTimes = 3, int _retryIntervalTime = 10) override;

        void SetCurlTimeOut(uint64_t tmpTimeOut = 90) override;

        int retryIntervalTime = 10;

        int retryTimes = 3;

    protected:
        /*
        Create fusionstorage Lun
        Date : 2020/03/03
        out params:id -> LUN id.
                  :WWN  -> LUN WWN.
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                     1.Create fusionstorage Lun
        */
        int CreateLUN(LunParams params, int &id, std::string &WWN);

        /*
        query fusionstorage Lun by lun name
        Date : 2020/03/03
        out params:id -> LUN id.
                :WWN  -> LUN WWN.
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.query fusionstorage Lun by lun name
        */
        int QueryLUN(std::string volumeName, int &id, std::string &WWN,
                     unsigned long long &size, unsigned long long &usedSize);

        /*
        delete volume with name
        Date : 2020/03/03
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                     1.delete volume with name
        */
        int DeleteLUN(std::string volumeName);

        /*
        create fusionstorage host by host name and host ip address
        Date : 2020/03/03
        out params:HOST -> host name.equal with UUID
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.create fusionstorage host by host name and host ip address
        */
        int CreateHost(const std::string UUID, const std::string ip);

        /*
        query fusionstorage host by host name
        Date : 2020/03/03
        out params:HOST -> host name.equal with UUID
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.query fusionstorage host by host name
        */
        int QueryHost(const std::string UUID);

        /*
        create fusionstorage port with iscsi connector
        Date : 2020/03/03
        out params:iscsi -> port name.equal with InitiatorName
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.create fusionstorage port with iscsi connector
        */
        int CreateISCSIPort(const std::string InitiatorName);

        /*
        query fusionstorage port with iscsi connector
        Date : 2020/03/03
        out params:iscsi -> port name.equal with InitiatorName
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.query fusionstorage port with iscsi connector
        */
        int QueryISCSIPort(const std::string InitiatorName);

        /*
        bind fusionstorage port to host
        Date : 2020/03/03
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.bind fusionstorage port to host
        */
        int BindInitiator(std::string hostName, std::string iscsiPort);

        /*
        query is fusionstorage port bind to this host
        Date : 2020/03/03
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.query is fusionstorage port bind to this host
        */
        int QueryHostISCSIPort(std::string hostName, std::string iscsiPort);

        /*
        attach lun to special host
        Date : 2020/03/03
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.attach lun to special host
        */
        int CreateHostMapping(const std::string hostName, const std::string lunName);

        int DeleteHostMapping(const std::string hostName, const std::string lunName);

        /*
        query is lun attached to special host
        Date : 2020/03/03
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.query is lun attached to special host
        */
        int QueryHostMapping(const std::string hostName, const std::string volumeName);

        int QuerySnapshot(std::string SnapshotName, int &id, std::string &WWN);

        SessionInfo Login();

        void DeleteDeviceSession();

        void CreateDeviceSession();

        void LoginAndGetSessionInfo();

        int Logout(SessionInfo sessionInfo);

        int ParseBodyV1(const std::string &json_data, Json::Value &data, std::string &errorDes, int &errorCode);

        int ParseBodyV2(const std::string &json_data, Json::Value &data, std::string &errorDes, int &errorCode);

        virtual int SendRequest(HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode,
                                bool lockSession = false);

        bool GetJsonValue(const Json::Value &jsValue, std::string strKey, std::string &strValue);

        void IterateIscsiHostAction(const Json::Value &iscsiPortalList, std::vector<std::string> &iscsiList);

        int IterateIscsiHost(Json::Value data, std::vector<std::string> &iscsiList);

        void DelayTimeSendRequest();

        bool FusionStorageResposeNeedRetry(const int ret);

        int SendRequestOnce(HttpRequest req, Json::Value &data, std::string &errorDes, int &errorCode);

        int SendRequestEx(HttpRequest &req, Json::Value &data, std::string &errorDes,
                          int &errorCode, SessionInfo &sessionInfo);

        int ResponseSuccessHandle(HttpRequest req,
                                  std::shared_ptr <IHttpResponse> &rsp, Json::Value &data, std::string &errorDes,
                                  int &errorCode);

        int SendHttpReq(
                std::shared_ptr <IHttpResponse> &rsp, const HttpRequest &req, std::string &errorDes, int &errorCode);

        void InitHttpStatusCodeForRetry();

    protected:
        bool useCache;
        std::shared_ptr <Session> sessionPtr;
        IHttpClient *fs_pHttpCLient = NULL;
        std::string FusionStorageIP = "";
        std::string FusionStoragePort = "";
        std::string FusionStorageUsername = "";
        std::string FusionStoragePassword = "";
        int FusionStoragePoolId = -1;
        std::vector<int> httpRspStatusCodeForRetry;
        uint64_t CurlTimeOut = 90;

        bool m_enableProxy {false};
    };
}
#endif  // FUSION_STORAGE_BLOCK_H
