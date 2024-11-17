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
#ifndef FUSION_STORAGE_STRUCTS_H
#define FUSION_STORAGE_STRUCTS_H

#include "json/json.h"
#include "volume_handlers/common/DiskCommDef.h"
#include "FusionStorageApi.h"
#include "FusionStorageRestApiErrorCode.h"
#include "client/FusionStorageRestClient.h"
#include "client/FusionStorageRestApiRequest.h"
#include "protect_engines/hcs/resource_discovery/HcsMessageInfo.h"
#include "common/JsonUtils.h"
#include "common/Constants.h"
#include "volume_handlers/common/ControlDevice.h"
#include "common/cert_mgr/CertMgr.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

enum class FusionStoraeVolumeStatus {
    NORMAL = 0,
    CREATING = 7
};

struct FusionStorageVolume {
    std::string mWwn;
    std::string mName;
    int32_t mId;
    uint32_t mStatus;   // 0：卷处于正常状态,4：卷作为源卷，并正在执行卷复制业务,5：卷作为目标卷，并正在执行快照在复制业务,8：卷正处于创建过程中。
    uint32_t mProtocol;    // 卷连接使用的协议。0：SCSI协议，VBS客户端挂载卷时使用；1：iSCSI协议，主机映射卷时使用；2：未映射
    int32_t mPoolId;
    bool mIsClone = false;
    uint32_t mEncrypted;    // 数据加密开关，0：关闭
    uint32_t mSubLunType;   // 0：普通卷, 1：PE卷, 2：VVol卷
    uint32_t mHealthStatus;    // 卷的健康状态。1：正常；2：故障。
    uint64_t mCapacity;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mWwn, wwn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mName, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mId, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mStatus, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mProtocol, protocol)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPoolId, pool_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIsClone, is_clone)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mEncrypted, encrypted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSubLunType, sub_lun_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mHealthStatus, health_status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCapacity, capacity)
    END_SERIAL_MEMEBER
};

struct Result {
    int32_t mCode;
    std::string mDescription;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCode, code)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mDescription, description)
    END_SERIAL_MEMEBER
};

struct CreateBitmapResponse {
    int32_t mBlockSize;
    int32_t mBitmapSize;
    int32_t mSnapSize;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mBlockSize, block_size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mBitmapSize, bitmap_size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSnapSize, snap_size)
    END_SERIAL_MEMEBER
};

template <typename T>
struct FusionStorageResponse {
    T mData;
    Result mResult;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mData, data)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mResult, result)
    END_SERIAL_MEMEBER
};

struct VersionInfo {
    int32_t mResult;
    std::string mVersion;
    std::string mErrCode;
    std::string mDescription;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mResult, result)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mVersion, version)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mErrCode, errorCode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mDescription, description)
    END_SERIAL_MEMEBER
};

struct VBSNodeInfo {
    std::string mStorageIp;
    std::string mNodeIp = "";
    std::string mUserName;
    std::string mPassWord;
    int32_t mPort;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mStorageIp, storageIp)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mNodeIp, nodeIp)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mUserName, userName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPassWord, passWord)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPort, port)
    END_SERIAL_MEMEBER
};

struct VBSNodeInfoList {
    std::vector<VBSNodeInfo> mVbsInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mVbsInfo, VBSInfo)
    END_SERIAL_MEMEBER
};

struct StoragePool {
    int32_t mPoolId;
    int64_t mTotalCapacity;
    int64_t mUsedCapacity;
    double mUsedCapacityRate;
    int32_t mPoolServerType;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPoolId, poolId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotalCapacity, totalCapacity)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mUsedCapacity, usedCapacity)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mUsedCapacityRate, usedCapacityRate)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPoolServerType, poolServerType)
    END_SERIAL_MEMEBER
};

struct StoragePools {
    int32_t mResult;
    std::vector<StoragePool> mStoragePools;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mResult, result)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mStoragePools, storagePools)
    END_SERIAL_MEMEBER
};

VIRT_PLUGIN_NAMESPACE_END

#endif