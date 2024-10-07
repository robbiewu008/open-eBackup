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
#ifndef DORADO_BLOCK_H
#define DORADO_BLOCK_H

//#include "framework/MessageProcess.h"
#include "log/Log.h"
//#include "utility/HttpClientInterface.h"
#include "device_access/Const.h"
#include "device_access/ControlDevice.h"
#include "device_access/SessionCache.h"
//#include "common/DiskCommDef.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <utility>
#include "error.h"
#include "define/Types.h"
#include "curl_http/HttpClientInterface.h"

namespace Module {
    const std::string DORADO_MODULE_NAME = "Dorado";
    const int NUM_1 = 1;
    const std::string INNER_SAFE_IP = "protectengine-e-dma.dpa.svc.cluster.local";
    const std::string INNER_SAFE_PORT = "30070";
    const std::string OUT_DORADO_PORT = "8088";
    const std::string DEVICE_ID = "xxx";
    constexpr int DORADO_NUMBER_TWO = 2;
    constexpr int DORADO_DEFAULT_PARENT_TYPE = 40;

    namespace DoradoErrorCode {
        const int FILESYSTEMALREADYEXIST = 1077948993;
        const int NFSSHAREALREADYEXIST = 1077939724;
        const int CIFSSHAREALREADYEXIST = 1077939715;
        const int CIFSSHARE_PERMISSON_EXIST = 1077939718;
        const int DATATURBOSHAERAREADYEXIST = 1077941008;
        const int DATATURBOSHARENOTEXIST = 1077941000;
        const int USERNAMEALREADYEXIST = 1077949059;
        const int ALREADYINWHITE = 1077939727;
        const int FILESYSTEMNOTEXIST = 1077939726;
        const int FILESYSTEMIDNOTEXIST = 1073752065;
        const int FILESYSTEMSNAPSHOTEXIST = 1073754142;
        const int HOSTLUNMAPPINGEXIST = 1073804588;
        const int HOSTEXIST = 1077948993;
        const int NOTNEEDADDNUMBER = 1073947144;
        const int SNAPSHOT_NOTEXIST = 1077937872;
        const int ORINGIN_FILESYSTEM_NOTEXSIT = 1073754136;
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
        const int RETURN_SNAP_REACH_FILE_SYSTEM_MAX_NUM = 1073754137;
        const int RETURN_SNAP_REACH_ENTIRE_SYSTEM_MAX_NUM = 1073754138;
        const int RETRUN_SNAP_REACH_PARENT_FS_MAX_NUM = 1073844275;
        const int RETURN_SNAP_FS_SPACE_NOT_ENOUGH = 1073754124;
        const int DTREENAMEISEXIST = 1077955353;
        const int DTREENOTEXIST = 1077955336;
        const int FILESYSTEM_IN_WRITE_PROTECTION_STATE = 1073754115;
        const int FILESYSTEM_DATA_STATUS_INCONSISTENT = 1073754153;
        const int PARAMETER_INCORRECT = 50331651;
        const int UNAUTH = -401;
    };  // namespace DoradoErrorCode

    const int g_noNeedRetryErrorCode[] = {
            DoradoErrorCode::FILESYSTEMALREADYEXIST, DoradoErrorCode::NFSSHAREALREADYEXIST,
            DoradoErrorCode::CIFSSHAREALREADYEXIST, DoradoErrorCode::ALREADYINWHITE,
            DoradoErrorCode::FILESYSTEMNOTEXIST,
            DoradoErrorCode::FILESYSTEMSNAPSHOTEXIST, DoradoErrorCode::FSSNAPSHOT_NOTEXIST,
            DoradoErrorCode::HOSTLUNMAPPINGEXIST, DoradoErrorCode::FILESYSTEMIDNOTEXIST,
            DoradoErrorCode::WINDOWSUSERNOTEXIST,
            DoradoErrorCode::HOSTEXIST, DoradoErrorCode::SNAPSHOT_NOTEXIST, DoradoErrorCode::NOTNEEDADDNUMBER,
            DoradoErrorCode::LUN_HOST_MAPPING_EXIST, DoradoErrorCode::LUNGROUP_HOST_MAPPING_NOTEXIST,
            DoradoErrorCode::FC_AND_ISCSI_NOTEXIST, DoradoErrorCode::LUNGROUP_HOST_MAPPING_EXIST,
            DoradoErrorCode::LUN_HOST_MAPPING_NOTEXIST, DoradoErrorCode::RETURN_SNAP_REACH_FILE_SYSTEM_MAX_NUM,
            DoradoErrorCode::RETURN_SNAP_REACH_ENTIRE_SYSTEM_MAX_NUM,
            DoradoErrorCode::RETRUN_SNAP_REACH_PARENT_FS_MAX_NUM,
            DoradoErrorCode::DTREENOTEXIST, DoradoErrorCode::DTREENAMEISEXIST,
            DoradoErrorCode::FILESYSTEM_IN_WRITE_PROTECTION_STATE, DoradoErrorCode::FILESYSTEM_DATA_STATUS_INCONSISTENT,
            DoradoErrorCode::RETURN_SNAP_FS_SPACE_NOT_ENOUGH, DoradoErrorCode::PARAMETER_INCORRECT,
            DoradoErrorCode::USERNAMEALREADYEXIST
    };

    enum RepLocalResType {
        LUN = 11,
        FILE_SYSTEM = 40
    };

    enum RepSynchronizeType {
        REPLICATION_MANUAL = 1,
        AFTER_SYNCHRONIZATION_BEGINS,
        AFTER_SYNCHRONIZATION_ENDS,
        SPECIFY_POLICY
    };

    enum IpRuleOperation {
        ADD = 1,
        QUERY,
        DELETE,
    };

    enum RepSpeedType {
        LOW = 1,
        MEDIUM,
        HIGH,
        HIGHEST
    };

    enum ReplicationModel {
        SYNCHRONOUS_REPLICATION = 1,
        ASYNCHRONOUS_REPLICATION
    };

    enum RecoverPolicy {
        RECOVER_AUTOMATIC = 1,
        RECOVER_MANUAL
    };

    enum SyncSnapPolicy {
        NOT_SYNC_SNAP,
        SAME_AS_SOURCE,
        USER_SNAP_RETENTION_NUM,
        USER_SNAP_RETENTION_WITH_TAG_NUM // 与配置的标签匹配的用户快照将同步到从端存储系统
    };

    enum UserSnapRetentionNum {
        SAN_SNAPSHOT_NUM = 512,
        NAS_SNAPSHOT_NUM = 1024,
        NAS_SNAPSHOT_WITH_TAG_NUM = 4000
    };

    enum SecresDataStatus {
        SYNCHRONIZED = 1,
        COMPLETE = 2,
        INCOMPLETE = 5
    };

    enum NetPlaneType {
        NAS_STORAGE_KUBERNETES = 0,
        NAS_CONTAINER_KUBERNETES = 1
    };

    struct RestResult {
        int errorCode;
        std::string errDesc;
        std::string errSuggestion;
    };

/**
 * 1. Invoke interface 'LoginIscsiTarget' to obtain the IQN and setup host and
 *    scsi initor(hostId can be used as the host name)
 * 2. Invoke interface 'Bind' to bind with specific host
 * 3. Invoke interface 'ScanLunAfterAttach' to scan lun that is/are mounted local server
 */

    class DoradoBlock : public ControlDevice {
    public:
        explicit DoradoBlock(ControlDeviceInfo deviceInfo, bool readFromK8s = true,
                             bool useSharedSession = true) {
            fs_pHttpCLient = IHttpClient::GetInstance();
            Compress = deviceInfo.compress;
            Dedup = deviceInfo.dedup;
            isShowSnapDir = deviceInfo.isShowSnapDir;
            SectorSize = deviceInfo.sectorSize;
            DoradoIP = "";
            this->useCache = useSharedSession;
            this->sessionPtr = nullptr;
            if (readFromK8s) {
                GetConnectedIP();
            } else {
                DoradoIP = deviceInfo.url;
                DoradoPort = deviceInfo.port;
            }
            m_enableProxy = deviceInfo.enableProxy; // 通过管理网络连接，后续会修改变量名称
            DoradoUsername = deviceInfo.userName;
            DoradoPassword = deviceInfo.password;
            DoradoPoolId = deviceInfo.poolId;
            ResourceName = deviceInfo.deviceName;
            certification = deviceInfo.cert;
            crl = deviceInfo.crl;
            HCP_Logger_noid(DEBUG, DORADO_MODULE_NAME)
                    << DoradoIP << "," << DoradoPort << "," << DoradoUsername << "," << DoradoPoolId << HCPENDLOG;
            InitHttpStatusCodeForRetry();
        }

        explicit DoradoBlock() {}

        virtual ~DoradoBlock() {
            if (useCache && m_sessionCache != nullptr) {
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
            IHttpClient::ReleaseInstance(fs_pHttpCLient);
            Module::CleanMemoryPwd(DoradoPassword);
        }

        int GetDoradoIp(std::string& doradoPanelIp);
        static int GetDoradoIp(std::string& doradoPanelIp, std::string& doradoPort);
        int GetDoradoIpFromCommanLine(std::string &doradoPanelIp);

        int LoginIscsiTarget(const std::string &iscsiIP, std::string &iqnNumber);

        int Bind(HostInfo &host, const std::string &shareId = "") override;

        int UnBind(HostInfo host, const std::string &shareId = "") override;

        int Mount(DeviceMountInfo mountInfo, const std::string &shareName = "") override {
            return Module::FAILED;
        }

        int UnMount(DeviceMountInfo mountInfo) override {
            return Module::FAILED;
        }

        int UnMount() override {
            return Module::FAILED;
        }

        int CreateShare() override {
            return Module::FAILED;
        }

        int QueryContorllerCnt(int &outCnt) override {
            return Module::FAILED;
        }

        int Create(unsigned long long size) override;

        int Delete() override;

        int Query(DeviceDetails &info) override;

        int ExtendSize(unsigned long long size) override;

        std::unique_ptr<ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;

        std::unique_ptr<ControlDevice> CreateClone(std::string cloneName, int &errorCode) override;

        int QueryServiceHost(std::vector<std::string> &iscsiList, IP_TYPE ipType = IP_TYPE::IP_V4) override;

        int QueryServiceIpController(
                std::vector<std::pair<std::string, std::string>> &ipControllerList,
                IP_TYPE ipType = IP_TYPE::IP_V4) override;

        int QuerySnapshotList(std::vector<FSSnapshotInfo> &snapshots) override;

        int DeleteSnapshot(std::string SnapshotName) override;

        int Revert(std::string SnapshotName) override;

        int QueryRevertInfo(const std::string &resourceName, std::string &rollbackRate,
                            std::string &rollbackStatus) override;

        int CreateReplication(
                int localResId, int rResId, std::string rDevId, int bandwidth, std::string &repId) override;

        int ActiveReplication(std::string repId) override;

        int QueryReplication(ReplicationPairInfo &replicationPairInfo) override;

        int UpdateReplication(int bandwidth, std::string pairId);

        int DeleteReplication(std::string pairId) override;

        int SplitReplication(std::string pairId);

        int CreateRemoteDevice(
                std::string localPort, std::string remoteIP, std::string remoteUser, std::string remotePassWord,
                std::string &devicdID);

        int CreateRemoteDeviceUser(const std::string &userName, const std::string &passWord);

        int GetLogicPortNameList(std::vector<std::string> &iscsiList);

        int BindRepportgroupsToRemoteDevice(
                std::string devicdID, std::string localGroupId, std::string remoteGroupId);

        void Clean() override;

        int GetTheServiceHost(std::string &iscsiIP);

        std::string ScanLunAfterAttach(std::string &lunID);

        int GetLunInfoByName(DeviceDetails &info, std::string &errDes);

        int GetLunInfoByID(DeviceDetails &info, std::string &errDes);

        int GetLunInfoByWWN(DeviceDetails &info, std::string &errDes);

        int GetSnapShotInfo(const std::string &snapShotId, SnapshotInfo &info, std::string &errDes);
        int GetSnapShotInfoByID(DeviceDetails &info, std::string &errDes);
        int GetSnapShotInfoByName(DeviceDetails &info, std::string &errDes);
        int
        GetSnapshotAllocDiffBitmap(const std::string &objectId, SnapshotDiffBitmap &diffBitmap, std::string &errDes);

        int GetSnapshotUnsharedDiffBitmap(const std::string &objectId, const std::string &parentObjectId,
                                          SnapshotDiffBitmap &diffBitmap, std::string &errDes);

        int GetSnapShoCoupleUuid(const std::string snapShotId, std::string &coupleUuid, std::string &errDes);

        int GetSnapshotByCoupleUuid(const std::string coupleUuid, std::string &snapShotId, std::string &errDes);

        int SendRequest(HttpRequest &req, Json::Value &data, std::string &errorDes,
                        int &errorCode, bool lockSession = false);
        int SendRequest(HttpRequest &req, Json::Value &data, Module::RestResult& result, bool lockSession = false);

        void SetResourceName(const std::string &resourceName) {
            ResourceName = resourceName;
        }

        void SetDoradoIP(const std::string &doradoIp, std::string doradoPort = OUT_DORADO_PORT) {
            DoradoIP = doradoIp;
            DoradoPort = std::move(doradoPort);
        }
        int GetConnectedIP();

        void SetRetryAttr(int _retryTimes = 3, int _retryIntervalTime = 10) override;

        void SetCurlTimeOut(uint64_t tmpTimeOut = 90) override;

        int TestDeviceConnection() override;
        bool OperateIpRule(const std::string& ip, const std::string doradoIp,
            const IpRuleOperation& operation,  int &errorCode);

        int retryIntervalTime = 90;
        int retryTimes = 3;
    protected:
        SessionInfo Login();

        void DeleteDeviceSession();

        void CreateDeviceSession();

        void LoginAndGetSessionInfo();

        void LoginAndGetSessionInfoFromPM();

        int Logout(SessionInfo sessionInfo);

        int SendRequestSpliced(HttpRequest &req, Json::Value &data, std::string &errorDes);

        int SendRequestOnce(HttpRequest req, Json::Value &data, Module::RestResult& result);

        int SendRequestEx(HttpRequest &req, Json::Value &data, std::string &errorDes,
                          int &errorCode, SessionInfo &sessionInfo);

        bool GetJsonValue(const Json::Value &jsValue, std::string strKey, std::string &strValue);

        int ParseCookie(const std::set<std::string> &cookie_values, SessionInfo &sessionInfo);

        int GetErrorCodeAndDesFromBody(const Json::Value &jsonValue, Module::RestResult& result, const std::string key);

        int ParseResponse(const std::string &json_data, Json::Value &data, std::string &errorDes, int &errorCode);
        int ParseResponse(const std::string &json_data, Json::Value &data, Module::RestResult& result);

        int CreateHost(const std::string hostName, const std::string os, std::string &hostID);

        int QueryHost(const std::string hostName, std::string &hostID);

        int CreateISCSIPort(const std::string id);

        int QueryISCSIPort(const std::string id);

        int CreateIscsiHostMapping(const std::string iscsinitor, HostInfo &host);

        int QueryIscsiHostMapping(const std::string iscsinitor, HostInfo &host);

        int QueryLunDescription(std::string volumeName, std::string &description);

        int CreateFcPort(const std::string id);

        int CreateFcHostMapping(const std::string fc, HostInfo &hostId);

        int QueryFcPort(const std::string id);

        int ResponseSuccessHandle(HttpRequest req,
                                  std::shared_ptr<IHttpResponse> &rsp, Json::Value &data, Module::RestResult& result);

        int QueryFcHostMapping(const std::string id, HostInfo &host);

        int NameLegalization(std::string &name);

        int QueryHostFcPort(std::string hostName, std::string fcPort);

        int QueryHostISCSIPort(std::string hostName, std::string iscsiPort);

        int CreateHostMapping(const std::string hostName, const std::string lunName);

        int DeleteHostLunMapping(const std::string hostName, const std::string lunName);

        int QueryLunGroupByLunId(std::string &lunGroupName, const int &lunId);

        int QueryHostLunMapping(const std::string hostName, const std::string lunName);

        int QuerySnapshotDescription(std::string SnapshotName, std::string &desp);

        int QuerySnapshot(std::string SnapshotName, int &id, std::string &WWN);

        int QueryLUN(
                std::string volumeName, int &volumeId, std::string &wwn, unsigned long long &capacity,
                unsigned long long &usedCapacity);

        int IterateIscsiHost(Json::Value data, std::vector<std::string> &iscsiList, IP_TYPE ipType);

        void AssignDeviceInfo(ControlDeviceInfo &deviceInfo, std::string SnapshotName);

        int GetSnapshotAllocDiffBitmapImp(
                const std::string &objectId, SnapshotDiffBitmap &diffBitmap, std::string &errDes);

        int GetSnapshotUnsharedDiffBitmapImp(const std::string &objectId, const std::string &parentObjectId,
                                             SnapshotDiffBitmap &diffBitmap, std::string &errDes);

        void DoScanAfterAttach(const std::string &wwn, std::string &lunPath);

        int CheckReplicationPair(int lunId, std::string devId, std::string &pairId);

        void DelayTimeSendRequest();

        int SendHttpReq(
                std::shared_ptr<IHttpResponse> &rsp, const HttpRequest &req, std::string &errorDes, int &errorCode);

        int isNeedRetryErrorCode(const int &errorCode);

        void InitHttpStatusCodeForRetry();

        bool DoradoResposeNeedRetry(const int ret);
        mp_int32 SendRequestToOSA(HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode);
        mp_int32 SendRequestToOSAOnce(HttpRequest req, Json::Value &data, std::string &errorDes, int &errorCode);

    protected:
        bool useCache;
        std::shared_ptr<Session> sessionPtr;
        static std::mutex m_doradoSessionMutex;
        static SessionInfo m_doradoSession;
        IHttpClient *fs_pHttpCLient = nullptr;
        std::string DoradoIP = "";
        std::string DoradoPort = "";
        std::string DoradoUsername = "";
        std::string DoradoPassword = "";
        int DoradoPoolId = -1;
        unsigned long long Capacity = 0;
        std::string curl_http = "https://";
        std::vector<int> httpRspStatusCodeForRetry;
        uint64_t CurlTimeOut = 90;
        static std::unique_ptr<SessionCache> m_sessionCache;
        bool m_enableProxy{false};
    };
}
#endif  // DORADO_BLOCK_H
