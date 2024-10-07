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
#ifndef DISK_DEVICE_FILE_H
#define DISK_DEVICE_FILE_H
#include <cstdint>
#include <memory>
#include <string.h>
#include <string>
#include <mutex>
#include <map>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#include "libaio.h"
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include "cstring"
#include "cstdio"
#include "cerrno"
#include "cstdlib"
#include "common/Macros.h"
#include "tracepoint/EbkTracePoint.h"

namespace VirtPlugin {
enum DISK_DEVICE_RETURN_CODE {
    DISK_DEVICE_OK,
    DISK_DEVICE_OVER_FLOW,
    DISK_DEVICE_READ_FAILED,
    DISK_DEVICE_AIO_READ_FAILED,
    DISK_DEVICE_WRITE_FAILED,
    DISK_DEVICE_SYNC_FAILED,
    DISK_DEVICE_OPEN_DISK_FAILED,
    DISK_DEVICE_PARAMATER_ERROR,
    DISK_DEVICE_ERROR,
    DISK_DEVICE_NOT_EXIST,
    DISK_DEVICE_RETRY,
    DISK_DEVICE_FUN_THREAD_FAILED,
    DISK_DEVICE_FUN_THREAD_OK
};

class DiskDeviceFile {
public:
    DiskDeviceFile();
    ~DiskDeviceFile();
    DISK_DEVICE_RETURN_CODE Open(const std::string &fileName, int owMode, const uint64_t &volumeSize);
    DISK_DEVICE_RETURN_CODE Read(uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
        std::shared_ptr<uint8_t[]> &buffer);
#ifndef WIN32
    DISK_DEVICE_RETURN_CODE AioRead(uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
        std::shared_ptr<uint8_t[]> &buffer, io_context_t aioCtx);
#endif
    DISK_DEVICE_RETURN_CODE Write(uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
        std::shared_ptr<uint8_t[]> &buffer);
    DISK_DEVICE_RETURN_CODE Close();
    DISK_DEVICE_RETURN_CODE Flush();
    std::string GetErrString();
    std::string GetDiskDevicePath();
    void ReleaseBufferCache(const uint64_t &offsetInBytes, const uint64_t &readLen);
    std::shared_ptr<int> GetDiskHandle()
    {
        return m_diskHandle;
    }

private:
    DiskDeviceFile(const DiskDeviceFile &src);
    DiskDeviceFile &operator=(const DiskDeviceFile &);
    DISK_DEVICE_RETURN_CODE DoRead(const uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
        const std::shared_ptr<uint8_t[]> &buffer);
    DISK_DEVICE_RETURN_CODE DoWrite(uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
        const std::shared_ptr<uint8_t[]> &buffer);
#ifndef WIN32
    DISK_DEVICE_RETURN_CODE DoAioRead(uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
        std::shared_ptr<uint8_t[]> &buffer, io_context_t aioCtx);
#endif

private:
    std::shared_ptr<int> m_diskHandle;
    uint64_t m_diskSizeInBytes;
    bool m_readOnly;
    std::string m_diskDevicePath;
    std::string m_errDesc;
};

const unsigned int HCP_ERROR_SIZE = 1024;
inline std::string HcpErrStr(int errnum)
{
    char buf[HCP_ERROR_SIZE];
    char *msg =
#ifdef WIN32
        strerror(errnum);
#else
        strerror_r(errnum, buf, sizeof(buf));
#endif
    if (msg == nullptr) {
        return std::string("");
    }
    return std::string(msg);
}
}
#endif
