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
#ifndef BUILD_NEW_VM_REQUEST_H
#define BUILD_NEW_VM_REQUEST_H

#include <string>
#include <vector>
#include <common/JsonHelper.h>
#include "CNwareRequest.h"
#include "common/Constants.h"

namespace CNwarePlugin {
const uint32_t HIGH_SPEED_NETWORK_CARD = 1;
const uint32_t OS_TYPE_OTHERS = 0;
const uint32_t ARCH_X86 = 2;
const uint32_t MODE_X86 = 1;
const uint32_t DEFAULT_CORE = 2;
const uint32_t CPU_LIMIT_DEFAULT = 100;
const uint32_t SHARE_DEFAULT = 2;    // 调度优先级1.低 2.中 3.高
const uint32_t SOCKET_DEFAULT = 1;
const uint32_t CACHE_DEFAULT_DIRECTSYNC = 1;
const int64_t IO_HANG_TIME_DEFAULT = 1800000;
const uint32_t VOLUME_TYPE_DEFAULT = 2;
const uint32_t MEMORY_LOCKED_DEFAULT = 0;
const uint32_t DEFAULT_BLKIOTUNE_MID = 2;
const uint32_t MIN_CPU_CORES_FOR_QUEUE = 1;

struct VpcRequest {
    std::string mPrivateNetId;
    std::string mVpcId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPrivateNetId, privateNetId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mVpcId, vpcId)
    END_SERIAL_MEMEBER
};

struct AddBridgeInterfaceRequest {
    std::string mIp;
    bool mIsVhostDriver = true;
    std::string mMac;
    uint32_t mModel = HIGH_SPEED_NETWORK_CARD;
    uint32_t mMtu;
    uint32_t mNetworkType;
    bool mNoIpMacSpoofing;
    std::string mPortGroupId;
    uint32_t mQueues;
    std::string mVfName;
    std::string mVip;
    std::string mVlandId;
    VpcRequest mVpc;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIp, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIsVhostDriver, isVhostDriver)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mMac, mac)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mModel, model)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mMtu, mtu)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mNetworkType, networkType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mNoIpMacSpoofing, noIpMacSpoofing)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPortGroupId, portGroupId)
    // SERIAL_MEMBER_TO_SPECIFIED_NAME(mQueues, queues)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mVfName, vfName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mVip, vip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mVlandId, vlanId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mVpc, vpc)
    END_SERIAL_MEMEBER
};

struct DomainCdromDevicesReq {
    std::string mFileName;
    bool mIsMount {false};
    std::string mPoolName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mFileName, file)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIsMount, isMount)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPoolName, poolName)
    END_SERIAL_MEMEBER
};

struct DomainBindCpu {
    std::string mCpus;
    std::string mVcpu;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpus, cpu)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mVcpu, vcpu)
    END_SERIAL_MEMEBER
};

struct DomainCpuReq {
    uint32_t mArch = ARCH_X86;
    std::vector<DomainBindCpu> mBindCpu;
    uint32_t mCore = DEFAULT_CORE;
    uint32_t mCurrent;
    uint32_t mGurantees = 0;
    uint32_t mMode = MODE_X86;
    uint32_t mQuota = CPU_LIMIT_DEFAULT;
    uint32_t mShares = SHARE_DEFAULT;
    uint32_t mSockets = SOCKET_DEFAULT;

    DomainCpuReq& operator =(const DomainCpu& cpuInfo)
    {
        mArch = cpuInfo.m_arch;
        mCore = cpuInfo.m_cores;
        mCurrent = cpuInfo.m_current;
        mGurantees = cpuInfo.m_gurantees;
        mMode = cpuInfo.m_mode;
        mQuota = cpuInfo.m_quota;
        mShares = cpuInfo.m_shares;
        mSockets = cpuInfo.m_sockets;
        mBindCpu = std::vector<DomainBindCpu>{};
        for (const auto &bind : cpuInfo.m_bindCpu) {
            DomainBindCpu bindCpu;
            bindCpu.mCpus = bind.m_cpus;
            bindCpu.mVcpu = bind.m_vcpu;
            mBindCpu.emplace_back(bindCpu);
        }
        return *this;
    }

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mArch, arch)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mBindCpu, bindCpu)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCore, cores)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCurrent, current)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mGurantees, gurantees)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mMode, mode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mQuota, quota)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mShares, shares)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSockets, sockets)
    END_SERIAL_MEMEBER
};

struct DomainDiskDevicesReq {
    uint32_t mBus;
    uint32_t mCache = CACHE_DEFAULT_DIRECTSYNC;
    uint32_t mIoHangTimeout = IO_HANG_TIME_DEFAULT;
    uint64_t mCapacity;
    std::string mOldPool;
    std::string mOldVol;
    std::string mPreallocation = "off";
    bool mShareable {false};
    uint32_t mSource;
    uint32_t mType = VOLUME_TYPE_DEFAULT;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mBus, bus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCache, cache)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIoHangTimeout, ioHangTimeout)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCapacity, capacity)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mOldPool, oldPool)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mOldVol, oldVol)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPreallocation, preallocation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mShareable, shareable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSource, source)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mType, type)
    END_SERIAL_MEMEBER
};

struct DomainMemoryReq {
    bool mAutoMem = true;
    bool mIsHugePages = false;
    uint64_t mLimit = 0;
    uint32_t mLocked = MEMORY_LOCKED_DEFAULT;
    std::string mMode;
    std::string mNodeSet;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mAutoMem, autoMem)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIsHugePages, isHugePages)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mLimit, limit)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mLocked, locked)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mMode, mode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mNodeSet, nodeSet)
    END_SERIAL_MEMEBER
};

struct VmAddPciReq {
    std::string mAddress;
    std::string mDomainId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mAddress, address)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mDomainId, domainId)
    END_SERIAL_MEMEBER
};

struct AddDomainRequest {
    bool mAutoMigrate = false;
    uint32_t mBlkiotune = DEFAULT_BLKIOTUNE_MID;
    std::vector<AddBridgeInterfaceRequest> mBridgeInterfaces;
    std::vector<DomainCdromDevicesReq> mCdromDevices;
    uint32_t mClock = 0;
    DomainCpuReq mCpuInfo;
    std::vector<DomainDiskDevicesReq> mDiskDevices;
    std::string mDomainname;
    std::string mHostId;
    bool mIsHardwareEncrypt = false;
    bool mIsStartNewDomain = false;
    DomainMemoryReq mMemory;
    int64_t mMemorySize = 536870912;
    std::string mName;
    uint32_t mOsType = OS_TYPE_OTHERS;
    std::string mOsVersion = "Others";
    std::vector<VmAddPciReq> mPciDevices;
    std::string mRemark;
    std::string mSecretId;
    std::string mSlot;
    bool mSpiceConsole = false;
    bool mVncConsole = true;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mAutoMigrate, autoMigrate)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mBlkiotune, blkiotune)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mBridgeInterfaces, bridgeInterfaces)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCdromDevices, cdromDevices)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mClock, clock)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCpuInfo, cpu)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mDiskDevices, diskDevices)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mDomainname, domainname)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mHostId, hostId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIsHardwareEncrypt, isHardwareEncrypt)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mIsStartNewDomain, isStartNewDomain)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mMemory, memory)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mMemorySize, memorySize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mName, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mOsType, osType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mOsVersion, osVersion)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPciDevices, pciDevices)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mRemark, remark)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSlot, slot)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSpiceConsole, spiceConsole)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mVncConsole, vncConsole)
    END_SERIAL_MEMEBER
};

class BuildNewVMRequest : public CNwareRequest {
public:
    BuildNewVMRequest() = default;
    virtual ~BuildNewVMRequest() = default;

    void SetDomainInfo(const AddDomainRequest &addDomainRequest)
    {
        m_addDomainRequest = addDomainRequest;
    }

    void SetMachineName(const std::string &name)
    {
        m_addDomainRequest.mName = name;
    }

    AddDomainRequest GetDomainInfo()
    {
        return m_addDomainRequest;
    }

    int32_t AddDomainRequestToJson()
    {
        Json::Value tempBody;
        if (!Module::JsonHelper::StructToJsonValue(m_addDomainRequest, tempBody)) {
            ERRLOG("Convert snapinfo to json string failed!");
            return VirtPlugin::FAILED;
        }
        if (m_addDomainRequest.mCpuInfo.mCurrent > MIN_CPU_CORES_FOR_QUEUE &&
            tempBody.isMember("bridgeInterfaces")) {
            int index {0};
            for (auto &brige : tempBody["bridgeInterfaces"]) {
                brige["queues"] = m_addDomainRequest.mBridgeInterfaces.at(index).mQueues >
                    m_addDomainRequest.mCpuInfo.mCurrent ? m_addDomainRequest.mCpuInfo.mCurrent :
                    m_addDomainRequest.mBridgeInterfaces.at(index).mQueues;
                DBGLOG("Index: %d, core: %d, ori: %d, que: %d", index,
                    m_addDomainRequest.mCpuInfo.mCurrent,
                    m_addDomainRequest.mBridgeInterfaces.at(index).mQueues, brige["queues"].asInt());
                index++;
            }
        }
        Json::FastWriter writer;
        m_body = writer.write(tempBody);
        DBGLOG("Convert AddDomainRequestToJson to json string %s!", m_body.c_str());
        return VirtPlugin::SUCCESS;
    }

private:
    AddDomainRequest m_addDomainRequest;
};
};

#endif