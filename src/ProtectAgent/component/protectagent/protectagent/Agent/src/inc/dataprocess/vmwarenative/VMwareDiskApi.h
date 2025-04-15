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
#ifndef VMWARE_DISK_API_H__
#define VMWARE_DISK_API_H__
#include <memory>
#include <mutex>
#include "VMwareDiskApiDefine.h"
#include "MessageLoopThread.h"
#include "common/JsonUtils.h"
#include <future>
// 该类是线程不安全的，只能在单线程中使用，如果需要在多线程中使用请自行加锁保护
class VMwareDiskApi {
public:
    VMwareDiskApi(const VMwareDiskOperations &operations, const VixDiskLibConnection &connection,
        MessageLoop * const messageLoop);

    virtual ~VMwareDiskApi();

    virtual VMWARE_DISK_RET_CODE OpenDisk(const std::string &path,
        const std::string &snapshotRef, const uint32_t flags, const uint64_t chunkSize,
        const std::string &transportModes, std::string &selectedtransportMode,
        std::string &errDesc);
    virtual VMWARE_DISK_RET_CODE OpenDiskByLocalDevice(const std::string &path,
        const mp_uint32 &openMode, const std::string &diskType, const mp_uint64 &diskSize);
    virtual VMWARE_DISK_RET_CODE CloseDiskByLocalDevice(const std::string &path);
    EXTER_ATTACK virtual VMWARE_DISK_RET_CODE QueryAllocatedBlocks(Json::Value &bodyMsg);
    virtual VMWARE_DISK_RET_CODE CloseDisk(std::string &errDesc);

    // ************************************
    // Method:    Read
    // FullName:  VMwareDiskApi::Read
    // Access:    virtual public
    // Returns:   VMWARE_DISK_RET_CODE
    // Qualifier: 读vmware磁盘的数据
    // Parameter: const uint64_t & offsetInBytes 开始读的位置 必须是512的倍数
    // Parameter: uint64_t & bufferSizeInBytes 实际读了多少数据
    // Parameter: unsigned char * buffer 存储读取数据的缓冲区地址
    // Parameter: std::string & errDesc 错误描述
    // ************************************
    virtual VMWARE_DISK_RET_CODE Read(
        const uint64_t &offsetInBytes, uint64_t &bufferSizeInBytes, unsigned char *buffer, std::string &errDesc);

    // ************************************
    // Method:    Write
    // FullName:  VMwareDiskApi::Write
    // Access:    virtual public
    // Returns:   VMWARE_DISK_RET_CODE
    // Qualifier:向vmware磁盘写入数据
    // Parameter: const uint64_t & offsetInBytes 写入的起始位置 必须是512的倍数
    // Parameter: uint64_t & bufferSizeInBytes 实际写入的数据 必须是512的倍数
    // Parameter: const unsigned char * buffer 数据缓存
    // Parameter: std::string & errDesc 错误描述
    // ************************************
    virtual VMWARE_DISK_RET_CODE Write(
        const uint64_t &offsetInBytes, uint64_t &bufferSizeInBytes, const unsigned char *buffer, std::string &errDesc);

private:
    VMwareDiskApi(const VMwareDiskApi &src);
    VMwareDiskApi &operator=(const VMwareDiskApi &);

    void Open(const std::string &path, const uint32_t flags,
        std::promise<VMWARE_DISK_RET_CODE> &promise);

    VMWARE_DISK_RET_CODE Close(std::promise<VMWARE_DISK_RET_CODE> &promise);

    std::string GetTransportMode();

    std::string GetErrString(const VMWARE_DISK_RET_CODE code);

    VMWARE_DISK_RET_CODE RetryOp(std::function<VMWARE_DISK_RET_CODE()> internalOp);

    VMWARE_DISK_RET_CODE DoWrite(const uint64_t &offsetInBytes, const uint64_t &parameterBytes,
        uint64_t &bufferSizeInBytes, const unsigned char *buffer, std::string &errDesc);

    VMWARE_DISK_RET_CODE DoRead(const uint64_t &offsetInBytes, const uint64_t &parameterBytes,
        uint64_t &bufferSizeInBytes, unsigned char *buffer, std::string &errDesc);

    void GetAllocatedBlocks(const std::vector<VixDiskLibBlock> &vixBlocks, Json::Value &bodyMsg);
    void GetAllocatedBlocksParams(mp_int32 &max_chunk_number, mp_uint64 &chunkSize);

private:
    static const uint64_t SECTOR_SIZE = 512;
    static const int DEFAULT_RETRY_TIMES = 3;
    static const int SLEEP_TIME_INTERVAL = 3000;

    VMwareDiskOperations m_vddkOperations;  // VDDK function set
    VixDiskLibConnection m_connection;      // connection with remote vCenter/ESXI
    VixDiskLibHandle m_diskHandle;          // VM disk handle
    FILE* m_devHandle;
    std::string m_diskType;
    uint64_t m_diskSize;
    uint64_t m_diskCapacity;
    uint64_t m_chunkSize;
    std::mutex m_mutex;
    MessageLoop *m_messageLoop;
    std::string m_path;
    std::string m_snapshotRef;
    std::string m_transportModes;
    int m_devFileHandle = -1;
    uint32_t m_diskOpenMode; // default value 1 - read only
};
#endif
