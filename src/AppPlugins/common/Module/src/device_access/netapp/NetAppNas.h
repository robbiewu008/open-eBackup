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
#ifndef NETAPP_NAS_H
#define NETAPP_NAS_H

#include <map>
#include <string>
#include <mutex>
#include <tuple>
#include <utility>
#include <iostream>
#include <sstream>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include "json/json.h"
#include "curl_http/HttpClientInterface.h"
#include "define/Types.h"
#include "log/Log.h"
#include "device_access/Const.h"
#include "common/CleanMemPwd.h"
#include "device_access/ControlDevice.h"


namespace Module {
    const std::string NETAPP_MODULE = "NetApp";
    const std::string ENCRYPT_FAILED = "ENCRYPT_FAILED";
    enum SECURITY_TYPE {_NTFS = 1, _UNIX = 2, _MIXED = 3};
    const int HTTP_202 = 202;
    const int HTTP_401 = 401;
    const std::string JOB_SUCCESS = "success";
    const std::string JOB_RUNNING = "running";
    const std::string JOB_QUEUED = "queued";
    const std::string JOB_FAILURE = "failure";
    const std::string JOB_RETRY = "retry";
    const std::string JOB_INVALID = "invalid_job";
    const int NUMB_ZERO = 0;
    const int NUMB_ONE = 1;
    const int NUMB_THREE = 3;
    const int SEND_REQ_RETRY_INTERVAL = 10;

    class NetAppNas : public ControlDevice {
    public:
        explicit NetAppNas(ControlDeviceInfo deviceInfo, bool querySvm = true)
        {
            fs_pHttpCLient = IHttpClient::GetInstance();
            ResourceName = "";
            NetAppIP = deviceInfo.url;
            NetAppPort = deviceInfo.port;
            NetAppUsername = deviceInfo.userName;
            NetAppPassword = deviceInfo.password;
            NetAppPoolId = deviceInfo.poolId;
            NetAppServiceIp = deviceInfo.serviceIp;
            certification = deviceInfo.cert;
            crl = deviceInfo.crl;
            this->m_resourceName = deviceInfo.deviceName;
            Init(querySvm);
        };

        explicit NetAppNas()
        {
        };

        virtual ~NetAppNas()
        {
            DestroyDeviceSession();
            if (fs_pHttpCLient) {
                delete fs_pHttpCLient;
                fs_pHttpCLient = NULL;
            }
            CleanMemoryPwd(NetAppPassword);
        };

        int Delete() override;
        int Create(unsigned long long size) override;
        int Query(DeviceDetails &info) override;
        int DeleteSnapshot(std::string SnapshotName) override;
        std::unique_ptr<ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;
        std::unique_ptr<ControlDevice> CreateClone(std::string cloneName, int &errorCode) override;

        int Bind(HostInfo &host, const std::string &shareId = "") override
        {
            return SUCCESS;
        }
        int UnBind(HostInfo host, const std::string &shareId = "") override
        {
            return SUCCESS;
        }
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
        int QueryServiceHost(std::vector<std::string> &iscsiList,
                                IP_TYPE ipType = IP_TYPE::IP_V4) override
        {
            return FAILED;
        }
        int QueryServiceIpController(std::vector<std::pair<std::string,
                std::string>> &ipControllerList,
                                        IP_TYPE ipType = IP_TYPE::IP_V4) override
        {
            return FAILED;
        }
        int ExtendSize(unsigned long long size) override
        {
            return FAILED;
        }
        int QuerySnapshotList(std::vector<FSSnapshotInfo> &snapshots) override
        {
            return FAILED;
        }
        int Revert(std::string SnapshotName) override
        {
            return FAILED;
        }
        int QueryRevertInfo(const std::string& resourceName,
                                std::string& rollbackRate,
                                std::string& rollbackStatus) override
        {
            return FAILED;
        }
        int CreateReplication(int localResId,
                                int rResId, std::string rDevId,
                                int bandwidth, std::string &repId) override
        {
            return FAILED;
        }
        int ActiveReplication(std::string repId) override
        {
            return FAILED;
        }
        int QueryReplication(ReplicationPairInfo& replicationPairInfo) override
        {
            return FAILED;
        }
        int DeleteReplication(std::string pairId) override
        {
            return FAILED;
        }
        int CreateShare() override
        {
            return FAILED;
        }
        void Clean() override
        {
            return;
        }

        int QueryContorllerCnt(int &outCnt) override
        {
            return FAILED;
        }

        int SetShareParam(ShareParam& param) override
        {
            m_shareParam = param;
        }

        bool GetEnableProxy();

        int TestDeviceConnection() override;
        void SetRetryAttr(int _retryTimes = 3, int _retryIntervalTime = 10) override;
        int retryIntervalTime = 10;
        int retryTimes = 3;
    protected:
        void Init(bool querySvm);
        void AssignDeviceInfo(ControlDeviceInfo &deviceInfo, std::string deviceName);
        int CheckSvmDetails();
        int CheckVolumeDetails();
        int SetSvmDetails(std::string svmName, std::string svmUuid = "");
        int SetVolumeDetails(std::string volumeName, std::string volumeUuid = "");
        int SetSvmNameFromServiceIp(std::string serviceIp);
        int ValidateSetSvmResponse(Json::Value &data, std::string serviceIp);
        int SetNasPathFromVolume(std::string volumeName, std::string &sharePath, int &errorCode);
        int ValidateSetNasPathResponse(Json::Value &data, std::string volumeName,
                                            std::string &sharePath, int &errorCode);
        std::unique_ptr<ControlDevice> ValidateCreateSnapshotResponse(Json::Value &data,
                                                                    std::string snapshotName);
        int ValidateDeleteSnapshotResponse(Json::Value &data, std::string snapshotNama);
        int QueryVolume(std::string volumeName, int &errorCode);
        int ValidateQueryVolumeResponse(Json::Value &data, std::string volumeName, int &errorCode);
        int QuerySnapshot(std::string snapshotName, int &errorCode);
        int ValidateQuerySnapshotResponse(Json::Value &data, std::string snapshotName, int &errorCode);
        int ValidateCreateVolumeResponse(Json::Value &data);
        int CreateVolume(unsigned long long size, SECURITY_TYPE secType);
        int QueryVolumeAndAggregate(std::string volumeName, std::string aggregate, int &errorCode);
        int UnmountVolume(std::string volumeName);
        int ValidateUnmountVolumeResponse(Json::Value &data, std::string volumeName);
        int GetAggregate(std::string &aggregate);
        int ValidateGetAggregateResponse(Json::Value &data, std::string &aggregate);
        int ValidateDeleteVolumeResponse(Json::Value &data, std::string volumeName);
        int DeleteVolumeCheck(const int& iRet, Json::Value& data, const std::string& volumeName);
        int DeleteVolume(std::string volumeName, bool query = true);
        int CreateCloneVolume(std::string cloneName, int &errorCode);
        int ValidateCreateCloneVolumeResponse(Json::Value &data, std::string cloneName);
        int CheckJobStatus(std::string jobUuid, std::string jobName);
        int ValidateJobStatusResponse(Json::Value &data, std::string jobUuid, std::string &status);
        int ValidateSetVolumeResponseDataCheck(const Json::Value::ArrayIndex &i, const Json::Value &data,
                std::string sharePath, std::string &volumeName, std::string &volumeUuid);
        int ValidateSetVolumeResponse(Json::Value &data, std::string sharePath,
                                        std::string &volumeName, std::string &volumeUuid);
        int ValidateSetVolumeResponseDataCheck4Nfs(const Json::Value::ArrayIndex &i, const Json::Value &data,
                std::string sharePath, std::string &volumeName, std::string &volumeUuid);
        int ValidateSetVolumeResponse4Nfs(Json::Value &data, std::string sharePath,
                                        std::string &volumeName, std::string &volumeUuid);

        int ValidateSharePath(std::string sharePath, std::string &volumeName, std::string &volumeUuid);
        int SetVolumeNameFromPath(std::string sharePath, std::string &volumeName, std::string &volumeUuid);
        int SetVolumeNameFromShareName(std::string shareName, std::string &volumeName, std::string &volumeUuid);
        int DeleteParentSnapshot(std::string parentVolName, std::string parentVolUuid,
                                    std::string parentSnapshot);
        int QueryParentVolumeDetails(std::string volumeName, std::string &parentVolName,
                                        std::string &parentVolUuid, std::string &parentSnapshot);
        bool ValidateQueryParentVolumeResponseDataCheck(Json::Value &data, const Json::Value::ArrayIndex& i);
        int ValidateQueryParentVolumeResponse(Json::Value &data, std::string volumeName,
                                                std::string &parentVolName, std::string &parentVolUuid,
                                                std::string &parentSnapshot);
        int CheckCloneFieldsPartOne(Json::Value &data, int index);
        int CheckCloneFieldsPartTwo(Json::Value &data, int index);
        int CheckCloneFields(Json::Value &data, int index, bool checkFields);

        int SendRequest(HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode);
        int SendRequestOnce(HttpRequest req, Json::Value &data, std::string &errorDes, int &errorCode);
        int SendRequestEx(HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode);
        int ParseResponse(const std::string &json_data, Json::Value &data, std::string &errorDes, int &errorCode);
        int ResponseSuccessHandle(HttpRequest req, std::shared_ptr<IHttpResponse>& rsp,
                                    Json::Value &data, std::string &errorDes, int &errorCode);
        int SendHttpReq(std::shared_ptr<IHttpResponse> &rsp,
                            const HttpRequest &req, std::string &errorDes, int& errorCode);
        int Base64Encryption(const std::string& userName, const std::string& password);
        int Base64Decryption(std::string encryptedkey, std::string &plainKey);
        void DelayTimeSendRequest(const std::string &delayForJobName);
        int DestroyDeviceSession();

    protected:
        std::string m_encryptedKey {};
        std::string m_vserverName {};
        std::string m_vserverUuid {};
        std::string m_volumeName {};
        std::string m_volumeUuid {};
        std::string m_resourceName {};
        IHttpClient *fs_pHttpCLient = NULL;
        std::string NetAppIP = "";
        std::string NetAppPort = "";
        std::string NetAppUsername = "";
        std::string NetAppPassword = "";
        std::string NetAppServiceIp = "";
        int NetAppPoolId = -1;
        unsigned long long Capacity = 0;
        std::string curl_http = "https://";
        ShareParam m_shareParam;
    };
}
#endif  // NETAPP_NAS_H
