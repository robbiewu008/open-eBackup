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
#ifndef MIGRATE_HOSTS_RESPONSE_H
#define MIGRATE_HOSTS_RESPONSE_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {
struct HostDetailRsp {
    double mAllocationMemory;
    std::string mClusterId;
    std::string mClusterName;
    std::string mCpuArchitecture;
    int32_t mCpuCores;
    int32_t mCpuCoresPerSocket;
    int32_t mCpuFrequency;
    std::string mCpuHfrequency;
    std::string mCpuModelName;
    std::string mCpuModelType;
    double mCpuRate;
    int32_t mCpuSockets;
    int32_t mCpuThreads;
    int32_t mCpuThreadsPerCore;
    double mCpuUsageMhz;
    std::string mCpuVirtualization;
    std::string mCpuVirtualizationType;
    std::string mCreateTime;
    int32_t mDefenseStatus;
    double mFreeAllocationMemory;
    std::string mHmemory;
    std::string mHostname;
    std::string mHstorage;
    std::string mId;
    std::string mIp;
    std::string mIpmiIpaddr;
    std::string mIpmiPw;
    std::string mIpmiUser;
    bool mIsConnected = false;
    bool mIsHa = false;
    bool mIsMaintain = false;
    bool mIsSupporting = false;
    std::string mIscsiInitiator;
    std::string mIvVersion;
    int64_t mMemory;
    double mMemoryRate;
    int64_t mMemoryUsage;
    std::string mName;
    std::string mPatchVersion;
    std::string mPoolId;
    std::string mPoolName;
    std::string mPort;
    std::string mRemark;
    std::string mSerialNumber;
    std::string mSrcClusterId;
    std::string mSrcClusterName;
    int64_t mStorage;
    double mStorageRate;
    std::string mSystem;
    double mUsedMemory;
    std::string mUser;
    int32_t mWakeCategory;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mAllocationMemory, allocationMemory)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mClusterId, clusterId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mClusterName, clusterName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuArchitecture, cpuArchitecture)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuCores, cpuCores)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuCoresPerSocket, cpuCoresPerSocket)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuFrequency, cpuFrequency)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuHfrequency, cpuHfrequency)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuModelName, cpuModelName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuModelType, cpuModelType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuRate, cpuRate)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuSockets, cpuSockets)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuThreads, cpuThreads)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuThreadsPerCore, cpuThreadsPerCore)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuUsageMhz, cpuUsageMhz)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuVirtualization, cpuVirtualization)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuVirtualizationType, cpuVirtualizationType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCreateTime, createTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mDefenseStatus, defenseStatus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mFreeAllocationMemory, freeAllocationMemory)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mHmemory, hmemory)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mHostname, hostname)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mHstorage, hstorage)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mId, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIp, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIpmiIpaddr, ipmiIpaddr)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIpmiPw, ipmiPw)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIpmiUser, ipmiUser)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIsConnected, isConnected)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIsHa, isHa)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIsMaintain, isMaintain)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIsSupporting, isSupporting)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIscsiInitiator, iscsiInitiator)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIvVersion, kvVersion)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mMemory, memory)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mMemoryRate, memoryRate)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mMemoryUsage, memoryUsage)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mName, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPatchVersion, patchVersion)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPoolId, poolId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPoolName, poolName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPort, port)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mRemark, remark)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSerialNumber, serialNumber)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSrcClusterId, srcClusterId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSrcClusterName, srcClusterName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mStorage, storage)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mStorageRate, storageRate)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSystem, system)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mUsedMemory, usedMemory)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mUser, user)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mWakeCategory, wakeCategory)
    END_SERIAL_MEMEBER;
};
class MigrateHostsResponse : public VirtPlugin::ResponseModel {
public:
    MigrateHostsResponse() {}
    ~MigrateHostsResponse() {}

    bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_resMigrateHost)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }

    HostDetailRsp GetTaskDetail()
    {
        return m_resMigrateHost;
    }

private:
    HostDetailRsp m_resMigrateHost;
};
};
#endif