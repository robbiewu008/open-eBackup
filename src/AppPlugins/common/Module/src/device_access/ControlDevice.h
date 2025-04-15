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
#ifndef CONTROL_DEVICE_H
#define CONTROL_DEVICE_H

#include "define/Types.h"
#include "json/json.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include "common/CleanMemPwd.h"

namespace Module {
    const int MP_NOTEXIST = 2;
    const int NUM_32K = 32768;
    const int64_t MIN_CURL_TIME_OUT = 90;
    const int CERTIFACATE_CURL_ERROR_CODE = 60; // 因证书过期、证书被吊销等原因证书认证失败:curl返回的错误码类型
    constexpr uint64_t CERTIFACATE_IS_INVALID = 1677931026; // 因证书过期、证书被吊销等原因证书认证失败:任务上报类型

    enum UNIX_SYMLINK {
        UNIX_SYMLINK_LOCAL = 0, UNIX_SYMLINK_REMOTE = 1
    };
    enum LOCK_POLICY {
        MANDATORY = 0, ADVISORY = 1
    };

    enum STORAGE_ENUM {
        FUSIONSTORAGE = 0, DORADO = 1, OCEANSTOR = 2, NETAPP = 3, OTHERS = 4, DEFAULT = -1
    };
    enum PROTOCOL {
        SAN = 1, NAS = 2, NFS = 3, CIFS = 4, DATATURBO = 5, PROTOCOL_DEFAULT = -1
    };
    enum INITIATOR_TYPE {
        FC = 0, ISCSI = 1
    };
    enum class IP_TYPE {
        IP_V6 = 1,
        IP_V4 = 0
    };
    enum SecurityStyle {
        MIXED = 0, NATIVE = 1, NTFS = 2, UNIX = 3
    };

    struct SnapdiffInfo {
        std::string progress;
        unsigned long long posType;
        unsigned long long posObj;
        unsigned long long posOffset;
        unsigned long long linkPosType;
        unsigned long long linkPos;
        unsigned short numChanges;
    };

    struct SnapdiffMetadataInfo {
        unsigned int inode;
        unsigned int changeType;
        std::string filePath;
        unsigned int fType;
        unsigned int crTime;
        unsigned int cTime;
        unsigned int mTime;
        unsigned int aTime;
        unsigned int owner;
        unsigned int group;
        unsigned int fAttr;
        unsigned int dosAttr;
        unsigned int size;
        unsigned int links;
    };

    struct LunParams {
        std::string volumeName = "";
        bool Compress = true;
        bool Dedup = true;
        int poolid = -1;
        unsigned long long Size = -1;
        unsigned long long usedSize = -1;

        LunParams(std::string vName, bool cpress, bool dd, int pd, unsigned long long sz)
            : volumeName(vName), Compress(cpress), Dedup(dd), poolid(pd), Size(sz) {}

        LunParams() {}
    };

    struct FSSnapshotInfo {
        std::string snapshotName;
        std::string volumeName;
    };

// enum that defines backup type (Full/Incremental/others ??)
    enum SnapType {
        Incremental = 1, Full = 2
    };

    struct SnapshotInfo {
        SnapshotInfo()
            : id(0),
              lunId(0),
              lunSizeInBytes(0),
              needBackup(true),
              hasBackup(false),
              snapType(Incremental),
              snapShotType(0),
              userCapacity(0),
              consumedCapacity(0),
              timeStamp(0) {}

        uint64_t id;
        std::string name;
        std::string wwn;
        uint64_t lunId;
        std::string lunName;
        std::string lunWwn;
        uint64_t lunSizeInBytes;
        std::string snapID;
        bool needBackup;
        bool hasBackup;  // for redo
        SnapType snapType;
        uint64_t snapShotType;
        uint64_t userCapacity;      // in byte
        uint64_t consumedCapacity;  // in byte
        uint64_t timeStamp;
    };

    struct ShareParam {
        /* for cifs */
        bool homeDirectory{false};
        bool opLocks{false};
        bool accessBasedEnumeration{false};
        bool changeNotify{true};
        bool encryption{false};
        UNIX_SYMLINK unixSymlink{UNIX_SYMLINK_LOCAL};
        /* for nfs */
        LOCK_POLICY lockPolicy{MANDATORY};
    };

    struct SnapshotDiffBitmap {
        SnapshotDiffBitmap() : offset(0), size(0), chunkSize(0) {}

        SnapshotDiffBitmap(unsigned long long uOffset, unsigned long long uSize, unsigned long long uChunkSize)
        {
            offset = uOffset;
            size = uSize;
            chunkSize = uChunkSize;
        }

        unsigned long long offset;     // 单位为字节
        unsigned long long size;       // 单位为字节
        unsigned long long chunkSize;  // 单位为字节
        std::string bitmap;
    };

    const std::string CONTROLDEVICEMODULE = "CONTROLDEVICE";
    const int ERRORCODE_NOTEXIST = 404;
    const int MAXNAMELENGTH = 20;

    struct ControlDeviceInfo {
        ControlDeviceInfo() : poolId(0) {}

        ~ControlDeviceInfo()
        {
            Module::CleanMemoryPwd(password);
            Module::FreeContainer(cert);
        }

        /* common */
        std::string deviceName;
        std::string url;       // login device user name
        std::string port;      // login device user name
        std::string userName;  // login device user name
        std::string password;  // login device user password
        std::string cert;
        std::string crl;
        std::string serviceIp;
        std::string nasUserGroup; // user_or_group
        bool compress = true;
        bool dedup = true;
        bool isShowSnapDir = false;
        bool enableProxy = false;
        int sectorSize = NUM_32K;
        std::string vstoreId;
        /* block special */
        int poolId;
        unsigned long long size;
        int storageType;       // storage type
        PROTOCOL storagePro;        // storage mount protocol
        int isCapacityBalanceMode;
        SecurityStyle secStyle{UNIX};
        std::string shareId;
        std::string fileSystemId;
        std::string dtreeId;
        std::string sharePath;
    };

    struct HostInfo {
        HostInfo() {}

        ~HostInfo()
        {
            Module::CleanMemoryPwd(chapPassword);
            Module::FreeContainer(iscsinitor);
        }

        /* bound common */
        std::string hostId;
        std::string hostIp;
        std::string chapAuthName;
        std::string chapPassword;
        std::string hostOsType;
        std::string domainName;
        std::vector<std::string> hostIpList;
        // key : the name of initiator name
        // value : FC = 0, ISCSI = 1
        std::unordered_map<std::string, int> iscsinitor;
    };

    struct DeviceDetails {
        /* common */
        DeviceDetails() {}

        std::string deviceName;
        int deviceId = 0;
        std::string deviceUniquePath;
        bool Compress = true;
        bool Dedup = true;
        bool isShowSnapDir = false;
        unsigned long long totalCapacity = 0;
        unsigned long long usedCapacity = 0;
        int status = 0;
        unsigned long long minSizeOfFileSys = 0;
        std::string owningController; // 归属控制器ID
        unsigned long long totalSaveCapacity = 0; // 总节省空间
        bool fileHandleByteAlignmentSwitch = false;
        SecurityStyle securityStyle{UNIX};
    };

    struct SharedClientInfo {
        std::string ID;
        std::string name;
        std::string type;
        std::string authorityLevel;
    };

    struct LogicPortInfo {
        std::string dataProtocol;
        std::string IP;
        std::string mask;
        std::string gateway;
        std::string runStatus;
        std::string activeStatus;
    };

    struct NasSharedInfo {
        DeviceDetails deviceDetail;
        std::vector<SharedClientInfo> clients;
    };

    struct DeviceMountInfo {
        std::string mountPoint;
        bool autoUmount{false};
        bool useVF{false};
        std::vector<std::string> vfMountPoint;
    };

    struct ReplicationPairInfo {
        std::string pairID;
        int status = 0;
        int progress = 0;
        int secresDataStatus = 0;
        unsigned long long bandWidth = 0;
        std::string remoteResID;
        std::string remoteResName;
        bool isPrimary = false;
    };
    struct LogicalPorts {
        std::string name;
        std::string ipv4Address;
        std::string ipv6Address;
    };

    class ControlDevice {
    public:
        virtual int Bind(HostInfo &host, const std::string &shareId = "") = 0;

        virtual int UnBind(HostInfo host, const std::string &shareId = "") = 0;

        virtual int Mount(DeviceMountInfo mountInfo, const std::string &shareName = "") = 0;

        virtual int UnMount(DeviceMountInfo mountInfo) = 0;

        virtual int UnMount() = 0;

        virtual int Create(unsigned long long size) = 0;

        virtual int Delete() = 0;

        virtual int Query(DeviceDetails &info) = 0;

        virtual std::unique_ptr<ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) = 0;

        virtual std::unique_ptr<ControlDevice> CreateClone(std::string cloneName, int &errorCode) = 0;

        virtual int QueryServiceHost(std::vector<std::string> &iscsiList, IP_TYPE ipType = IP_TYPE::IP_V4) = 0;

        virtual int QueryServiceIpController(
                std::vector<std::pair<std::string, std::string>> &ipControllerList,
                IP_TYPE ipType = IP_TYPE::IP_V4) = 0;

        virtual int ExtendSize(unsigned long long size) = 0;

        virtual int QuerySnapshotList(std::vector<FSSnapshotInfo> &snapshots) = 0;

        virtual int Revert(std::string SnapshotName) = 0;

        virtual int QueryRevertInfo(const std::string &resourceName, std::string &rollbackRate,
                                    std::string &rollbackStatus) = 0;

        virtual int CreateReplication(int localResId, int rResId, std::string rDevId,
                                      int bandwidth, std::string &repId) = 0;

        virtual int ActiveReplication(std::string repId) = 0;

        virtual int QueryReplication(ReplicationPairInfo &replicationPairInfo) = 0;

        virtual int DeleteReplication(std::string pairId) = 0;

        virtual int CreateShare() = 0;

        virtual int DeleteSnapshot(std::string SnapshotName) = 0;

        virtual int QueryContorllerCnt(int &outCnt) = 0;

        virtual void Clean() = 0;

        virtual int QuerySnapshot(std::string SnapshotName, std::string &id)
        {
            return Module::SUCCESS;
        }

        // 获取本文件系统所在的控制器，和非本文件系统的控制器
        virtual int
        GetLifPort(std::vector<std::string> &ownCtlIP, std::vector<std::string> &otherCtlIP, IP_TYPE ipType)
        {
            return Module::SUCCESS;
        }

        virtual int LoginIscsiTarget(const std::string &iscsiIP, std::string &iqnNumber)
        {
            return Module::SUCCESS;
        }

        virtual std::string ScanLunAfterAttach(std::string &lunID)
        {
            std::string VolumePath;
            return VolumePath;
        }

        virtual ~ControlDevice()
        {
            return;
        }

        virtual void SetRetryAttr(int _retryTimes = 3, int _retryIntervalTime = 10) {}

        virtual int StartSnapshotDiffSession(std::string BaseSnapshotName,
                                             std::string IncrementalSnapshotName, std::string &sessionId)
        {
            return Module::SUCCESS;
        }

        virtual int GetSnapshotDiffChanges(std::string sessionId, SnapdiffInfo &SnapdiffInfo,
                                           SnapdiffMetadataInfo metadatInfo[], int metadataListLen)
        {
            return Module::SUCCESS;
        }

        virtual int EndSnapshotDiffSession(std::string sessionId)
        {
            return Module::SUCCESS;
        }

        virtual int SetIsDeleteParentSnapShotFlag(bool flag)
        {
            return Module::FAILED;
        }

        virtual int TestDeviceConnection()
        {
            return Module::FAILED;
        }

        virtual int SetShareParam(ShareParam &param)
        {
            return Module::SUCCESS;
        }

        virtual void SetCurlTimeOut(uint64_t tmpTimeOut = 90) {}

    public:
        void SetResourceName(const std::string &resourceName)
        {
            ResourceName = resourceName;
        }

        std::string GetResourceName()
        {
            return ResourceName;
        }

        std::string GetDeviceWWN()
        {
            return Wwn;
        }

        int GetResourceId()
        {
            return ResourceId;
        }

        bool checkStringIsDigit(const std::string &str)
        {
            return std::all_of(str.begin(), str.end(), ::isdigit);
        }

        void SetIsCapacityBalanceMode(bool flag)
        {
            isCapacityBalanceMode = flag;
        }

        void SetIsFileHandleByteAligment(bool flag)
        {
            isFileHandleByteAligment = flag;
        }

        /* 当证书验证失败时，将libcurl返回的错误码转换为任务上报相关错误码 */
        int CurlError2HomoError(const int curlError)
        {
            return (curlError == CERTIFACATE_CURL_ERROR_CODE) ? CERTIFACATE_IS_INVALID : curlError;
        }

        void SetErrorCode(const int errorCode)
        {
            curErrorCode = CurlError2HomoError(errorCode);
        }

        int GetErrorCode()
        {
            return curErrorCode;
        }

        void SetExtendInfo(const std::string& errorDes)
        {
            extendInfo = errorDes;
        }

        std::string  GetExtendInfo()
        {
            return extendInfo;
        }

        DeviceMountInfo GetMountInfo()
        {
            return MountInfo;
        }

    protected:
        DeviceMountInfo MountInfo;
        std::string ResourceName;
        int ResourceId;
        std::string Wwn;
        std::string certification;
        std::string crl;
        std::string extendInfo;
        bool Compress;
        bool Dedup;
        bool isShowSnapDir;
        int SectorSize;
        bool isCapacityBalanceMode = false;
        bool isFileHandleByteAligment = false;
        int curErrorCode = 0;
    };
}

#endif  // CONTROL_DEVICE_H
