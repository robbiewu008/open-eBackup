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
#include "VhdHandler.h"
#include <comutil.h>
#include <cstdio>
#include <log/Log.h>
#include <boost/filesystem.hpp>
#include "securec.h"
#include "common/utils/Utils.h"

namespace {
    const std::string MODULE_NAME = "VhdHandler";
    const int FILE_D_TYPE = 8;
}

namespace VirtPlugin {
bool VhdHandler::OpenVHDDisk(const std::string &fileName)
{
    VIRTUAL_STORAGE_TYPE storageType;
    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    OPEN_VIRTUAL_DISK_PARAMETERS params;
    memset_s(&params, sizeof(params), 0, sizeof(params));
    params.Version = OPEN_VIRTUAL_DISK_VERSION_2;
    params.Version2.GetInfoOnly = FALSE;
    DWORD ret = OpenVirtualDisk(&storageType, _com_util::ConvertStringToBSTR(fileName.c_str()),
        VIRTUAL_DISK_ACCESS_NONE, OPEN_VIRTUAL_DISK_FLAG_NONE, &params, &m_vhdHandle);
    if (ret != ERROR_SUCCESS || m_vhdHandle == INVALID_HANDLE_VALUE) {
        ERRLOG("OpenVirtualDisk  failed, ret = %d, error code: %d.", ret, GetLastError());
        return false;
    }
    return true;
}

bool VhdHandler::AttachVHDDisk(ATTACH_VIRTUAL_DISK_FLAG attachFlags)
{
    ATTACH_VIRTUAL_DISK_PARAMETERS attachParameters;
    memset_s(&attachParameters, sizeof(attachParameters), 0, sizeof(attachParameters));
    attachParameters.Version = ATTACH_VIRTUAL_DISK_VERSION_1;
    DWORD ret = AttachVirtualDisk(m_vhdHandle, NULL, attachFlags, 0, &attachParameters, NULL);
    if (ret != ERROR_SUCCESS) {
        ERRLOG("AttachVirtualDisk failed, ret = %d, error code: %d.", ret, GetLastError());
        return false;
    }
    return true;
}

int32_t VhdHandler::Open(const std::string &fileName, const std::string &mode)
{
    if (!OpenVHDDisk(fileName)) {
        ERRLOG("Open disk(%s) failed.", fileName.c_str());
        return FAILED;
    }
    ATTACH_VIRTUAL_DISK_FLAG attachFlags;
    if (mode == "r") {
        attachFlags = ATTACH_VIRTUAL_DISK_FLAG_NO_LOCAL_HOST | ATTACH_VIRTUAL_DISK_FLAG_READ_ONLY;
    } else {
        attachFlags = ATTACH_VIRTUAL_DISK_FLAG_NO_LOCAL_HOST;
    }
    if (!AttachVHDDisk(attachFlags)) {
        ERRLOG("Attach disk failed.");
        return FAILED;
    }
    INFOLOG("Attach %s success.", fileName.c_str());
    return SUCCESS;
}

int32_t VhdHandler::Truncate(const uint64_t &size)
{
    return SUCCESS;
}

int32_t VhdHandler::Close()
{
    if (m_vhdHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_vhdHandle);
    }
    return SUCCESS;
}

size_t VhdHandler::Read(std::shared_ptr<uint8_t[]> buf, size_t count)
{
    LARGE_INTEGER offset;
    offset.QuadPart = m_offset;
    OVERLAPPED overlapped;
    memset_s(&overlapped, sizeof(overlapped), 0, sizeof(overlapped));
    overlapped.Offset = offset.LowPart;
    overlapped.OffsetHigh = offset.HighPart;
    DWORD bytesRead;
    INFOLOG("BEGIN READBLOCKS offset %d, offsetHight %d.", overlapped.Offset, overlapped.OffsetHigh);
    BOOL ret = ReadFile(m_vhdHandle, (LPVOID)buf.get(), count, &bytesRead, &overlapped);
    if (ret == TRUE) {
        return bytesRead;
    }
    ret = GetLastError();
    if (ret != ERROR_IO_PENDING) {
        ERRLOG("Read file failed, error: %d.", ret);
        return 0;
    }
    if (!GetOverlappedResult(m_vhdHandle, &overlapped, &bytesRead, TRUE)) {
        ERRLOG("Get over lapped failed, %llu, %llu, ret: %d.", m_offset, count, GetLastError());
        return 0;
    }
    return bytesRead;
}

size_t VhdHandler::Read(std::string &buf, size_t count)
{
    return SUCCESS;
}

size_t VhdHandler::Write(const std::shared_ptr<uint8_t[]> &buf, size_t count)
{
    LARGE_INTEGER offset;
    offset.QuadPart = m_offset;
    OVERLAPPED overlapped;
    memset_s(&overlapped, sizeof(overlapped), 0, sizeof(overlapped));
    overlapped.Offset = offset.LowPart;
    overlapped.OffsetHigh = offset.HighPart;
    DWORD bytesWritten;
    INFOLOG("BEGIN READBLOCKS offset %d, offsetHight %d.", overlapped.Offset, overlapped.OffsetHigh);
    BOOL ret = WriteFile(m_vhdHandle, (LPVOID)buf.get(), count, &bytesWritten, &overlapped);
    if (ret == TRUE) {
        INFOLOG("WriteFile success, %d, %d.", bytesWritten, GetLastError());
        return bytesWritten;
    }
    ret = GetLastError();
    if (ret != ERROR_IO_PENDING) {
        ERRLOG("Read file failed, error: %d.", ret);
        return 0;
    }
    if (!GetOverlappedResult(m_vhdHandle, &overlapped, &bytesWritten, TRUE)) {
        ERRLOG("Get over lapped failed, %llu, %llu, ret: %d.", m_offset, count, GetLastError());
        return FAILED;
    }
    return bytesWritten;
}

size_t VhdHandler::Write(const std::string &str)
{
    return SUCCESS;
}

size_t VhdHandler::Append(std::shared_ptr<uint8_t[]> buf, size_t count)
{
    return SUCCESS;
}

int64_t VhdHandler::Tell()
{
    return SUCCESS;
}

int64_t VhdHandler::Seek(size_t offset, int origin)
{
    m_offset = offset;
    return SUCCESS;
}

size_t VhdHandler::FileSize(const std::string &fileName)
{
    return SUCCESS;
}

bool VhdHandler::Flush(bool sync)
{
    return true;
}

bool VhdHandler::Exists(const std::string &fileName)
{
    return true;
}

bool VhdHandler::Rename(const std::string &oldName, const std::string &newName)
{
    return true;
}

bool VhdHandler::CopyFile(const std::string &srcName, const std::string &destName)
{
    return true;
}

bool VhdHandler::IsDirectory(const std::string& path)
{
    return true;
}

bool VhdHandler::IsRegularFile(const std::string& path)
{
    return true;
}

bool VhdHandler::Remove(const std::string &fileName)
{
    return true;
}

bool VhdHandler::RemoveAll(const std::string &dirName)
{
    return true;
}

bool VhdHandler::CreateDirectory(const std::string &dirName)
{
    return true;
}

void VhdHandler::GetFiles(std::string pathName, std::vector <std::string> &files)
{
    return;
}
}
