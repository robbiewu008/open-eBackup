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
#include "DiskDeviceFile.h"
#include <securec.h>
#include <fcntl.h>
#include <boost/filesystem.hpp>
#include "system/System.hpp"
#include "log/Log.h"
#include "common/Constants.h"
#include "common/Macros.h"
#include "config_reader/ConfigIniReader.h"
#include "common/Timer.h"

namespace {
const std::string MODULE_NAME = "DiskDeviceFile";
}

namespace VirtPlugin {
DiskDeviceFile::DiskDeviceFile() : m_diskSizeInBytes(0), m_readOnly(true), m_errDesc("internal error.")
{}

DiskDeviceFile::~DiskDeviceFile()
{}

static void CloseFile(int *fd)
{
    DBGLOG("Enter close file.");
    if (fd != nullptr) {
        DBGLOG("Close file, fd:%d", *fd);
        (void)close(*fd);
    }
}

DISK_DEVICE_RETURN_CODE DiskDeviceFile::Open(const std::string &fileName, int owMode, const uint64_t &volumeSize)
{
    INFOLOG("Open disk fileName: %s, mode: %d", fileName.c_str(), owMode);
    int32_t ret;
    TP_START("TP_OpenDisk", 1, &ret);
    TP_END
    if (ret == FAILED) {
        return DISK_DEVICE_OPEN_DISK_FAILED;
    }
    std::string realPath;
    try {
        boost::system::error_code errcode;
        realPath = boost::filesystem::canonical(boost::filesystem::path(fileName), errcode).string();
        if (errcode.value() != 0) {
            ERRLOG("Get real path failed, file name:%s, errcode:%d", fileName.c_str(), errcode.value());
            return DISK_DEVICE_OPEN_DISK_FAILED;
        }
    } catch (const boost::filesystem::filesystem_error &errExp) {
        ERRLOG("Get real path failed. file name:%s, errcode:%s", fileName.c_str(), errExp.what());
        return DISK_DEVICE_OPEN_DISK_FAILED;
    }
    DBGLOG("Open disk real path: %s", realPath.c_str());
    int fd = open(realPath.c_str(), owMode);
    if (fd == -1) {
        int errNo = errno;
        m_errDesc = HcpErrStr(errNo);
        ERRLOG("Open disk real path:%s, mode:%d, err code:%d, err desc:%s",
            realPath.c_str(), owMode, errNo, m_errDesc.c_str());
        return DISK_DEVICE_OPEN_DISK_FAILED;
    }

    m_diskHandle = std::shared_ptr<int>(new (std::nothrow) int(fd), CloseFile);
    if (m_diskHandle.get() == nullptr) {
        ERRLOG("Allocate memory for disk handle failed!");
        (void)close(fd);
        return DISK_DEVICE_OPEN_DISK_FAILED;
    }

    DBGLOG("Open volume: %s,fd: %d successfully.", realPath.c_str(), *m_diskHandle);
    m_diskSizeInBytes = volumeSize;
    m_diskDevicePath = realPath;
    m_readOnly = (owMode != O_WRONLY);
    DBGLOG("Vol path:%s, ReadOnly:%d", m_diskDevicePath.c_str(), m_readOnly);
    return DISK_DEVICE_OK;
}

void DiskDeviceFile::ReleaseBufferCache(const uint64_t &offsetInBytes, const uint64_t &readLen)
{
    std::shared_ptr<int> spHandle = m_diskHandle;
    DBGLOG("Begin release buffer cache, start:%lld, len:%lld", offsetInBytes, readLen);
    if (!posix_fadvise(*spHandle, (off_t)offsetInBytes, (off_t)readLen, POSIX_FADV_DONTNEED)) {
        DBGLOG("release one block buffer failed, don't worry, buffer will be release after disk closed");
    }
}

DISK_DEVICE_RETURN_CODE DiskDeviceFile::Close()
{
    std::shared_ptr<int> spHandle = m_diskHandle;
    m_diskHandle.reset();
    if (spHandle.get() == nullptr) {
        return DISK_DEVICE_OK;
    }
    DBGLOG("Begin to sync %s.", m_diskDevicePath.c_str());
    if (!m_readOnly) {
        int retryTimes = 10;
        int sleepInterval = 10;
        while (retryTimes-- > 0) {
            if (fsync(*spHandle) != 0) {
                ERRLOG("Sync volume file[%s]failed , error message is %s", m_diskDevicePath.c_str(), strerror(errno));
                std::this_thread::sleep_for(std::chrono::seconds(sleepInterval));
            } else {
                break;
            }
        }
        if (retryTimes <= 0) {
            return DISK_DEVICE_SYNC_FAILED;
        }
    }
    DBGLOG("End to sync %s.", m_diskDevicePath.c_str());
    if (posix_fadvise(*spHandle, (off_t)0, (off_t)0, POSIX_FADV_DONTNEED) != 0) {
        int errNo = errno;
        m_errDesc = HcpErrStr(errNo);
        ERRLOG("Release buffer cache from file failed ! disk name=%s.error msg is %s, error code is %d",
            m_diskDevicePath.c_str(), m_errDesc.c_str(), errNo);
    }
    return DISK_DEVICE_OK;
}

std::string DiskDeviceFile::GetErrString()
{
    return m_errDesc;
}

std::string DiskDeviceFile::GetDiskDevicePath()
{
    return m_diskDevicePath;
}

DISK_DEVICE_RETURN_CODE DiskDeviceFile::DoRead(const uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
    const std::shared_ptr<uint8_t[]> &buffer)
{
    size_t readLen = static_cast<size_t>(bufferSizeInBytes);
    if (static_cast<size_t>(offsetInBytes) + static_cast<size_t>(bufferSizeInBytes) > m_diskSizeInBytes) {
        readLen = m_diskSizeInBytes - offsetInBytes;
    }
    size_t readedLen = 0;
    while (readLen > readedLen) {
        ssize_t retLen = pread(*m_diskHandle, buffer.get() + readedLen, readLen - readedLen, offsetInBytes + readedLen);
        if (retLen <= 0) {
            WARNLOG("Read file failed. len:%d", retLen);
            break;
        }
        readedLen = readedLen + retLen;
    }
    if (readedLen != readLen) {
        int errNo = errno;
        m_errDesc = HcpErrStr(errNo);
        ERRLOG("Read from file failed ! fd:%d, offset:%lld, readLen:%d, readedLen:%d.\
            error msg is %s, error code is %d",
            *m_diskHandle, offsetInBytes, readLen, readedLen, m_errDesc.c_str(), errNo);
        return DISK_DEVICE_READ_FAILED;
    }
    bufferSizeInBytes = readedLen;
    return DISK_DEVICE_OK;
}

#ifndef WIN32
DISK_DEVICE_RETURN_CODE DiskDeviceFile::DoAioRead(uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
                                                  std::shared_ptr<uint8_t[]> &buffer, io_context_t aioCtx)
{
    struct iocb *io = (struct iocb *)malloc(sizeof(struct iocb));
    if (io == nullptr) {
        ERRLOG("malloc struct iocb failed");
        return DISK_DEVICE_AIO_READ_FAILED;
    }
    memset_s(io, sizeof(struct iocb), 0, sizeof(struct iocb));
    int srcfd = *m_diskHandle;
    io_prep_pread(io, srcfd, buffer.get(), bufferSizeInBytes, offsetInBytes);
    int rc = io_submit(aioCtx, 1, &io);
    if (rc < 0) {
        ERRLOG("aio send read one block failed, offsetInBytes:%lld", offsetInBytes);
        free(io);
        return DISK_DEVICE_AIO_READ_FAILED;
    } else {
        DBGLOG("aio send read one block success, offsetInBytes:%lld", offsetInBytes);
        return DISK_DEVICE_OK;
    }
    return DISK_DEVICE_OK;
}

DISK_DEVICE_RETURN_CODE DiskDeviceFile::AioRead(uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
    std::shared_ptr<uint8_t[]> &buffer, io_context_t aioCtx)
{
    DBGLOG("AioReadBlock, offset:%lld,data len%lld, disk size:%lld", offsetInBytes,
        bufferSizeInBytes, m_diskSizeInBytes);
    if (m_diskHandle.get() == nullptr) {
        ERRLOG("The volume file is not open.");
        return DISK_DEVICE_READ_FAILED;
    }
    if (offsetInBytes >= m_diskSizeInBytes) {
        WARNLOG("Read end of the file.");
        return DISK_DEVICE_OVER_FLOW;
    }
    return DoAioRead(offsetInBytes, bufferSizeInBytes, buffer, aioCtx);
}
#endif

DISK_DEVICE_RETURN_CODE DiskDeviceFile::Read(uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
    std::shared_ptr<uint8_t[]> &buffer)
{
    if (m_diskHandle.get() == nullptr) {
        ERRLOG("The volume file is not open.");
        return DISK_DEVICE_READ_FAILED;
    }
    if (offsetInBytes >= m_diskSizeInBytes) {
        WARNLOG("Read end of the file, offsetInBytes: %lld, m_diskSizeInBytes: %lld", offsetInBytes, m_diskSizeInBytes);
        return DISK_DEVICE_OVER_FLOW;
    }
    int rereadTryTimes = Module::ConfigReader::getInt(Module::MS_CFG_BACKUPNODE_SECTION, "rereadTryTimes");
    while (rereadTryTimes-- > 0) {
        if (DoRead(offsetInBytes, bufferSizeInBytes, buffer) == DISK_DEVICE_OK) {
            return DISK_DEVICE_OK;
        } else if (rereadTryTimes > 0) {
            int backupRereadInteval =
                Module::ConfigReader::getInt(Module::MS_CFG_BACKUPNODE_SECTION, "backupRereadInteval");
            DBGLOG("get retry sleep timeout = %d", backupRereadInteval);
            std::this_thread::sleep_for(std::chrono::seconds(backupRereadInteval));
        } else {
            break;
        }
    }
    ERRLOG("Read file failed:%s", m_diskDevicePath.c_str());
    return DISK_DEVICE_READ_FAILED;
}

DISK_DEVICE_RETURN_CODE DiskDeviceFile::DoWrite(uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
    const std::shared_ptr<uint8_t[]> &buffer)
{
    size_t writeLen = static_cast<size_t>(bufferSizeInBytes);
    if (static_cast<size_t>(offsetInBytes) + static_cast<size_t>(bufferSizeInBytes) > m_diskSizeInBytes) {
        writeLen = static_cast<size_t>(m_diskSizeInBytes - offsetInBytes);
    }
    ssize_t retLen = pwrite(*m_diskHandle, buffer.get(), writeLen, offsetInBytes);
    if ((retLen < 0) || (writeLen != static_cast<size_t>(retLen))) {
        int errNo = errno;
        m_errDesc = strerror(errNo);
        ERRLOG("Write to file failed:%s, write len:%d, offset:%lld,ret len:%d. error msg is %s, error code is %d",
            m_diskDevicePath.c_str(), writeLen, offsetInBytes, retLen, m_errDesc.c_str(), errNo);
        return DISK_DEVICE_WRITE_FAILED;
    }
    return DISK_DEVICE_OK;
}

DISK_DEVICE_RETURN_CODE DiskDeviceFile::Write(uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
    std::shared_ptr<uint8_t[]> &buffer)
{
    if (m_diskHandle.get() == nullptr) {
        ERRLOG("The volume file is not open:%s", m_diskDevicePath.c_str());
        return DISK_DEVICE_WRITE_FAILED;
    }
    if (offsetInBytes >= m_diskSizeInBytes) {
        ERRLOG("Write end of the file:%s", m_diskDevicePath.c_str());
        return DISK_DEVICE_OVER_FLOW;
    }
    int restoreTryTimes = Module::ConfigReader::getInt(Module::MS_CFG_BACKUPNODE_SECTION, "restoreTryTimes");
    while (restoreTryTimes-- > 0) {
        if (DoWrite(offsetInBytes, bufferSizeInBytes, buffer) == DISK_DEVICE_OK) {
            return DISK_DEVICE_OK;
        } else if (restoreTryTimes > 0) {
            int restoreRewriteInteval =
                Module::ConfigReader::getInt(Module::MS_CFG_BACKUPNODE_SECTION, "restoreRewriteInteval");
            DBGLOG("Get retry sleep timeout namestore:%d", restoreRewriteInteval);
            std::this_thread::sleep_for(std::chrono::seconds(restoreRewriteInteval));
        } else {
            break;
        }
    }
    ERRLOG("Write end of the file:%s", m_diskDevicePath.c_str());
    return DISK_DEVICE_WRITE_FAILED;
}

DISK_DEVICE_RETURN_CODE DiskDeviceFile::Flush()
{
    if (!m_readOnly) {
        if (0 != fsync(*m_diskHandle)) {
            m_errDesc = strerror(errno);
            ERRLOG("Sync volume file[%s] failed , error message is %s", m_diskDevicePath.c_str(), m_errDesc.c_str());
            return DISK_DEVICE_SYNC_FAILED;
        }
        DBGLOG("Flush End.");
    }
    return DISK_DEVICE_OK;
}
}
