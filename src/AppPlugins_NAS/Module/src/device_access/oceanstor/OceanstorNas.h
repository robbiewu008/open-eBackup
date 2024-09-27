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
#ifndef OCEANSTOR_NAS_H
#define OCEANSTOR_NAS_H

#include <boost/algorithm/string.hpp>
#include "define/Types.h"
#include "log/Log.h"
#include "curl_http/HttpClientInterface.h"
#include "device_access/Const.h"
#include "device_access/ControlDevice.h"
//#include "common/DiskCommDef.h"
#include "device_access/SessionCache.h"

namespace Module {
    constexpr int OCEANSTOR_NUMBER_TWO = 2;
    constexpr int OCEANSTOR_DEFAULT_PARENT_TYPE = 40;

    namespace OceanstorErrorCode {
        const int FILESYSTEMALREADYEXIST = 1077948993;
        const int NFSSHAREALREADYEXIST = 1077940500;
        const int CIFSSHAREALREADYEXIST = 1077939715;
        const int ALREADYINWHITE = 1077939727;
        const int FILESYSTEMNOTEXIST = 1077939726;
        const int FILESYSTEMIDNOTEXIST = 1073752065;
        const int FILESYSTEMSNAPSHOTEXIST = 1073754142;
        const int HOSTLUNMAPPINGEXIST = 1073804588;
        const int HOSTEXIST = 1077948993;
        const int NOTNEEDADDNUMBER = 1073947144;
        const int SNAPSHOT_NOTEXIST = 1077937872;
        const int FSSNAPSHOT_NOTEXIST = 1073754118;
        const int WINDOWSUSERNOTEXIST = 37749698;
        const int NOUSERPERMISSION = 1077949058;
        const int AUTHIPINCONSISTENCY = 1073793620;
        const int LUNGROUP_HOST_MAPPING_NOTEXIST = 1073804589;
        const int LUN_HOST_MAPPING_NOTEXIST = 1073804587;
        const int LUN_HOST_MAPPING_EXIST = 1073804588;
        const int LUNGROUP_HOST_MAPPING_EXIST = 1073804590;
        const int FC_AND_ISCSI_NOTEXIST = 1077948996;
        const int REMOTE_DEVICE_NOTEXIST = 37100137;
        const int REMOTE_USER_NOTEXIST = 1077949057;
        const int SYSTEM_BUSY = 1077948995;  // The system is busy
        const int UNAUTH = -401;
    };  // namespace OceanstorErrorCode

    const int g_noNeedRetryOceanstorErrorCode[] = {
            OceanstorErrorCode::FILESYSTEMALREADYEXIST, OceanstorErrorCode::NFSSHAREALREADYEXIST,
            OceanstorErrorCode::CIFSSHAREALREADYEXIST, OceanstorErrorCode::ALREADYINWHITE,
            OceanstorErrorCode::FILESYSTEMNOTEXIST, OceanstorErrorCode::FILESYSTEMSNAPSHOTEXIST,
            OceanstorErrorCode::FSSNAPSHOT_NOTEXIST, OceanstorErrorCode::HOSTLUNMAPPINGEXIST,
            OceanstorErrorCode::FILESYSTEMIDNOTEXIST, OceanstorErrorCode::WINDOWSUSERNOTEXIST,
            OceanstorErrorCode::HOSTEXIST, OceanstorErrorCode::SNAPSHOT_NOTEXIST,
            OceanstorErrorCode::NOTNEEDADDNUMBER, OceanstorErrorCode::LUN_HOST_MAPPING_EXIST,
            OceanstorErrorCode::LUNGROUP_HOST_MAPPING_NOTEXIST, OceanstorErrorCode::FC_AND_ISCSI_NOTEXIST,
            OceanstorErrorCode::LUNGROUP_HOST_MAPPING_EXIST, OceanstorErrorCode::LUN_HOST_MAPPING_NOTEXIST
    };
    const std::string OCEANSTOR_MODULE_NAME = "OceanStor";

    class OceanstorNas : public ControlDevice {
    public:
        explicit OceanstorNas(ControlDeviceInfo deviceInfo, bool useSharedSession = true) {
            fs_pHttpCLient = IHttpClient::GetInstance();
            Compress = deviceInfo.compress;
            Dedup = deviceInfo.dedup;
            OceanstorIP = deviceInfo.url;
            OceanstorPort = deviceInfo.port;
            OceanstorUsername = deviceInfo.userName;
            OceanstorPassword = deviceInfo.password;
            OceanstorPoolId = deviceInfo.poolId;
            OceanstorServiceIP = deviceInfo.serviceIp;
            ResourceName = deviceInfo.deviceName;
            certification = deviceInfo.cert;
            crl = deviceInfo.crl;
            this->useCache = useSharedSession;
            this->sessionPtr = nullptr;
            HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << OceanstorIP << "," << OceanstorPort << "," <<
                                                  OceanstorPoolId << HCPENDLOG;
            InitHttpStatusCodeForRetry();
        }

        OceanstorNas(ControlDeviceInfo deviceInfo, std::string fsId, bool useSharedSession = true) : fileSystemId(fsId) {
            fs_pHttpCLient = IHttpClient::GetInstance();
            Compress = deviceInfo.compress;
            Dedup = deviceInfo.dedup;
            OceanstorIP = deviceInfo.url;
            OceanstorPort = deviceInfo.port;
            OceanstorUsername = deviceInfo.userName;
            OceanstorPassword = deviceInfo.password;
            OceanstorPoolId = deviceInfo.poolId;
            OceanstorServiceIP = deviceInfo.serviceIp;
            ResourceName = deviceInfo.deviceName;
            certification = deviceInfo.cert;
            crl = deviceInfo.crl;
            this->useCache = useSharedSession;
            this->sessionPtr = nullptr;
            HCP_Log(DEBUG, OCEANSTOR_MODULE_NAME) << OceanstorIP << "," << OceanstorPort << "," <<
                                                  OceanstorPoolId << HCPENDLOG;
            InitHttpStatusCodeForRetry();
        }

        virtual ~OceanstorNas();

        int Query(DeviceDetails &info) override;

        std::unique_ptr <ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;

        int DeleteSnapshot(std::string SnapshotName) override;

        int QuerySnapshot(std::string SnapshotName, std::string &id);

        void Clean() override;

        int SendRequest(HttpRequest &req, Json::Value &data, std::string &errorDes,
                        int &errorCode, bool lockSession = false);

        void SetCurlTimeOut(uint64_t tmpTimeOut = 90) override;

        int QueryContorllerCnt(int &outCnt) override {
            return FAILED;
        }

        int retryIntervalTime = 10;
        int retryTimes = 3;
    protected:
        SessionInfo Login();

        void DeleteDeviceSession();

        void CreateDeviceSession();

        void LoginAndGetSessionInfo();

        int Logout(SessionInfo sessionInfo);

        int SendRequestOnce(HttpRequest req, Json::Value &data, std::string &errorDes, int &errorCode);

        int SendRequestEx(HttpRequest &req, Json::Value &data, std::string &errorDes,
                          int &errorCode, SessionInfo &sessionInfo);

        bool GetJsonValue(const Json::Value &jsValue, std::string strKey, std::string &strValue);

        int ParseCookie(const std::set <std::string> &cookie_values, SessionInfo &sessionInfo);

        int ParseResponse(const std::string &json_data, Json::Value &data, std::string &errorDes, int &errorCode);

        bool GetEnableProxy();

        virtual int QueryFileSystem(DeviceDetails &info);

        int QueryFileSystem(std::string fileName, DeviceDetails &info);

        int ResponseSuccessHandle(HttpRequest req,
                                  std::shared_ptr <IHttpResponse> &rsp, Json::Value &data, std::string &errorDes,
                                  int &errorCode);

        int StartSnapshotDiffSession(std::string BaseSnapshotName,
                                     std::string IncrementalSnapshotName, std::string &sessionId);

        int GetSnapshotDiffChanges(std::string sessionId, SnapdiffInfo &SnapdiffInfo,
                                   SnapdiffMetadataInfo metadatInfo[], int metadataListLen);

        int EndSnapshotDiffSession(std::string sessionId);

        void DelayTimeSendRequest();

        int SendHttpReq(
                std::shared_ptr <IHttpResponse> &rsp, const HttpRequest &req, std::string &errorDes, int &errorCode);

        int isNeedRetryErrorCode(const int &errorCode);

        void InitHttpStatusCodeForRetry();

        bool OceanstorResposeNeedRetry(const int ret);

        int DeleteFileSystem();

        int GetVstoreId();

        int CreateCloneFileSystem(std::string volumeName, std::string &fsid);

        void AssignDeviceInfo(ControlDeviceInfo &deviceInfo, std::string volumeName);

        int DeleteFileSystemAndParentSnapshot();

        int SetIsDeleteParentSnapShotFlag(bool flag) {
            isDeleteParentSnapShot = flag;
        }

        int Bind(HostInfo &host, const std::string &shareId = "") {
            return FAILED;
        }

        int UnBind(HostInfo host, const std::string &shareId = "") {
            return FAILED;
        }

        int Mount(DeviceMountInfo mountInfo, const std::string &shareName = "") {
            return FAILED;
        }

        int UnMount(DeviceMountInfo mountInfo) {
            return FAILED;
        }

        int UnMount() {
            return FAILED;
        }

        int Create(unsigned long long size) {
            return FAILED;
        }

        int Delete() {
            return FAILED;
        }

        std::unique_ptr <ControlDevice> CreateClone(std::string cloneName, int &errorCode) {
            return nullptr;
        }

        int QueryServiceHost(std::vector<std::string> &iscsiList, IP_TYPE ipType = IP_TYPE::IP_V4) {
            return FAILED;
        }

        int QueryServiceIpController(
                std::vector<std::pair<std::string, std::string>> &ipControllerList, IP_TYPE ipType = IP_TYPE::IP_V4) {
            return FAILED;
        }

        int ExtendSize(unsigned long long size) {
            return FAILED;
        }

        int QuerySnapshotList(std::vector<FSSnapshotInfo> &snapshots) {
            return FAILED;
        }

        int Revert(std::string SnapshotName) {
            return FAILED;
        }

        int QueryRevertInfo(const std::string &resourceName, std::string &rollbackRate, std::string &rollbackStatus) {
            return FAILED;
        }

        int CreateReplication(int localResId, int rResId, std::string rDevId, int bandwidth,
                              std::string &repId) {
            return FAILED;
        }

        int ActiveReplication(std::string repId) {
            return FAILED;
        }

        int QueryReplication(ReplicationPairInfo &replicationPairInfo) {
            return FAILED;
        }

        int DeleteReplication(std::string pairId) {
            return FAILED;
        }

        int CreateShare() {
            return FAILED;
        }

    protected:
        bool useCache;
        std::shared_ptr <Session> sessionPtr;
        IHttpClient *fs_pHttpCLient = NULL;
        std::string OceanstorIP = "";
        std::string OceanstorPort = "";
        std::string OceanstorUsername = "";
        std::string OceanstorPassword = "";
        int OceanstorPoolId = -1;
        std::string OceanstorServiceIP = "";
        unsigned long long Capacity = 0;
        std::string fileSystemId = "";
        std::string vstoreId = "";
        std::string curl_http = "https://";
        std::vector<int> httpRspStatusCodeForRetry;
        std::vector<int> oceanstorErrorCodesForRetry;
        bool isDeleteParentSnapShot = false;
        uint64_t CurlTimeOut = 90;
        static std::unique_ptr <SessionCache> m_oceanstorSessionCache;
    };
}

#endif // OCEANSTOR_NAS_H
