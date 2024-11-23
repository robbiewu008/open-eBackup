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
#ifdef WIN32
#include "Win32FileSystemHandler.h"
#include <winioctl.h>
#include <cstdio>
#include <filesystem>
#include <sstream>
#include "securec.h"
#include "common/utils/Win32Utils.h"
#include "common/File.h"

namespace {
    const std::string MODULE_NAME = "Win32FileSystemHandler";
    const DWORD DIR_ALREADY_EXISTS = 183;
}

namespace VirtPlugin {
int32_t Win32FileSystemHandler::Open(const std::string &fileName, const std::string &mode)
{
    DWORD accessMode;
    DWORD creatPolicy;
    if (mode == "r") {
        accessMode = GENERIC_READ;
        creatPolicy = OPEN_EXISTING;
    } else if (mode == "w+") {
        accessMode = GENERIC_WRITE;
        creatPolicy = CREATE_ALWAYS;
    } else if (mode == "a+") {
        accessMode = GENERIC_WRITE;
        creatPolicy = OPEN_ALWAYS;
    } else if (mode == "r+") {
        accessMode = GENERIC_WRITE;
        creatPolicy = OPEN_EXISTING;
    } else {
        ERRLOG("Invalid mode(%s).", mode.c_str());
        return FAILED;
    }
    std::string longPath = "\\\\?\\" +  fileName;  // Windows文件路徑有長度限制，加入長路徑前綴解除限制
    m_fileHandle = CreateFile(
        (LPCTSTR)String2wstring(longPath).c_str(),
        accessMode,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        creatPolicy,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (m_fileHandle == INVALID_HANDLE_VALUE) {
        ERRLOG("CreateFile normal(%s) failed, %d", longPath.c_str(), GetLastError());
        m_fileHandle = CreateFile(
            (LPCTSTR)String2WstringByUtf8(longPath).c_str(), accessMode,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, creatPolicy, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (m_fileHandle == INVALID_HANDLE_VALUE) {
        ERRLOG("CreateFile utf8(%s) failed, %d", fileName.c_str(), GetLastError());
        return FAILED;
    }
    if (accessMode == GENERIC_WRITE && !Flush()) {
        ERRLOG("Flush %s failed.", fileName.c_str());
        return FAILED;
    }
 
    return SUCCESS;
}

int32_t Win32FileSystemHandler::Truncate(const uint64_t &size)
{
    LARGE_INTEGER li;
    li.QuadPart = size;
    if (!SetFilePointerEx(m_fileHandle, li, NULL, FILE_BEGIN)) {
        ERRLOG("SetFilePointerEx failed.");
        return FAILED;
    }
    if (!SetEndOfFile(m_fileHandle)) {
        ERRLOG("SetEndOfFile failed.");
        return FAILED;
    }
    return SUCCESS;
}

int32_t Win32FileSystemHandler::Close()
{
    if (m_fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_fileHandle);
    }
    return SUCCESS;
}

size_t Win32FileSystemHandler::Read(std::shared_ptr<uint8_t[]> buf, size_t count)
{
    DWORD dwRead;
    if (!ReadFile(m_fileHandle, buf.get(), count, &dwRead, NULL)) {
        ERRLOG("ReadFile failed, %d.", GetLastError());
        return FAILED;
    }
    return dwRead;
}

size_t Win32FileSystemHandler::Read(std::string &buf, size_t count)
{
    std::unique_ptr<char[]> tmpBuf = std::make_unique<char[]>(count + 1);
    if (tmpBuf.get() == nullptr) {
        ERRLOG("Allocate with new returns NULL.");
        return FAILED;
    }
    memset_s(tmpBuf.get(), count + 1, 0, count + 1);
    DWORD dwRead;
    if (!ReadFile(m_fileHandle, tmpBuf.get(), count, &dwRead, NULL)) {
        ERRLOG("ReadFile failed, %d.", GetLastError());
        return FAILED;
    }
    buf = std::string(tmpBuf.get(), count);
    return dwRead;
}

size_t Win32FileSystemHandler::Write(const std::shared_ptr<uint8_t[]> &buf, size_t count)
{
    DWORD dwWritten;
    if (!WriteFile(m_fileHandle, buf.get(), count, &dwWritten, NULL)) {
        ERRLOG("WriteFile failed, %d", GetLastError());
        return FAILED;
    }
    return dwWritten;
}

size_t Win32FileSystemHandler::Append(std::shared_ptr<uint8_t[]> buf, size_t count)
{
    return SUCCESS;
}

int64_t Win32FileSystemHandler::Tell()
{
    LARGE_INTEGER liDistanceToMove;
    LARGE_INTEGER lpNewFilePointer;
    liDistanceToMove.QuadPart = 0;
    if (!SetFilePointerEx(m_fileHandle, liDistanceToMove, &lpNewFilePointer, FILE_CURRENT)) {
        return FAILED;
    }
    return lpNewFilePointer.QuadPart;
}

int64_t Win32FileSystemHandler::Seek(size_t offset, int origin)
{
    LARGE_INTEGER liDistanceToMove;
    liDistanceToMove.QuadPart = offset;
    return SetFilePointerEx(m_fileHandle, liDistanceToMove, NULL, FILE_BEGIN) ? SUCCESS : FAILED;
}

size_t Win32FileSystemHandler::Write(const std::string &str)
{
    DWORD dwWritten;
    DWORD cbWrite = str.length();
    if (!WriteFile(m_fileHandle, (LPWSTR)str.c_str(), cbWrite, &dwWritten, NULL)) {
        ERRLOG("WriteFile failed, %d. ", GetLastError());
        return FAILED;
    }
    return dwWritten;
}

size_t Win32FileSystemHandler::FileSize(const std::string &fileName)
{
    LARGE_INTEGER FileSize;
    if (GetFileSizeEx(m_fileHandle, &FileSize) == 0) {
        ERRLOG("Get file size failed. ");
        return FAILED;
    }
    return FileSize.QuadPart;
}

bool Win32FileSystemHandler::Flush(bool sync)
{
    if (m_fileHandle == INVALID_HANDLE_VALUE) {
        ERRLOG("File fd is null.");
        return false;
    }

    return FlushFileBuffers(m_fileHandle);
}

bool Win32FileSystemHandler::Exists(const std::string &fileName)
{
    if (!std::filesystem::exists(std::filesystem::u8path(fileName))) {
        WARNLOG("File/Dir(%s) does not exist.", fileName.c_str());
        return false;
    }
    return true;
}

bool Win32FileSystemHandler::Rename(const std::string &oldName, const std::string &newName)
{
    if (oldName.empty() || newName.empty()) {
        ERRLOG("param invalid.");
        return false;
    }

    if (!Exists(oldName)) {
        ERRLOG("The old file or path: %s not exists.", oldName.c_str());
        return false;
    }

    try {
        std::filesystem::rename(std::filesystem::u8path(oldName), std::filesystem::u8path(newName));
    } catch (const std::filesystem::filesystem_error& e) {
        ERRLOG("Win32FileSystemHandler rename failed: %s", e.what());
        return false;
    }
    return true;
}

bool Win32FileSystemHandler::CopyFile(const std::string &srcName, const std::string &destName)
{
    if (srcName.empty() || destName.empty()) {
        printf("param invalid");
        return false;
    }

    if (!std::filesystem::exists(std::filesystem::u8path(srcName))) {
        printf("The old file or path: %s not exists.", srcName.c_str());
        return false;
    }

    std::filesystem::copy(std::filesystem::u8path(srcName), std::filesystem::u8path(destName),
        std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
    return true;
}

bool Win32FileSystemHandler::IsDirectory(const std::string& path)
{
    std::string realPath = path;
    int compareRet = path.compare(path.size() - 1, 1, ":"); // 处理path="D:"的情况
    if (compareRet == 0) {
        realPath = realPath + "/";
    }
    if (!std::filesystem::is_directory(std::filesystem::u8path(realPath))) {
        WARNLOG("Dir(%s) does not exist.", realPath.c_str());
        return false;
    }
    return true;
}

bool Win32FileSystemHandler::IsRegularFile(const std::string &fileName)
{
    return std::filesystem::is_regular_file(std::filesystem::u8path(fileName));
}

bool Win32FileSystemHandler::Remove(const std::string &fileName)
{
    if (!Exists(fileName)) {
        INFOLOG("removing file, but not exists. filename=[%s]", fileName.c_str());
        return true;
    }

    if (std::filesystem::remove(std::filesystem::u8path(fileName)) != 0) {
        ERRLOG("Remove failed: errno[%d]:[%s]", errno, strerror(errno));
        return false;
    }

    return true;
}

bool Win32FileSystemHandler::RemoveAll(const std::string &dirName)
{
    return std::filesystem::remove_all(std::filesystem::u8path(dirName));
}

bool Win32FileSystemHandler::CreateDirectory(const std::string &dirName)
{
    std::string realDirection = dirName;
    std::string tmpDirection = dirName;
    while ((tmpDirection.compare(tmpDirection.size() - 1, 1, "/") == 0) ||
        (tmpDirection.compare(tmpDirection.size() - 1, 1, "\\") == 0)) {
        realDirection = tmpDirection.substr(0, tmpDirection.size() - 1);
        tmpDirection =  realDirection;
        DBGLOG("Check Direction(%s).", tmpDirection.c_str());
    }
    try {
        if (!std::filesystem::create_directories(std::filesystem::u8path(realDirection))) {
            DWORD errorCode  = GetLastError();
            ERRLOG("create dir(%s) failed: errno[%d].", realDirection.c_str(), errorCode);
            return errorCode == DIR_ALREADY_EXISTS;
        }
    } catch (std::exception &e) {
        ERRLOG("CreateDirectory exception: %s.", WIPE_SENSITIVE(e.what()).c_str());
        return false;
    }
    return true;
}

void Win32FileSystemHandler::GetFiles(std::string pathName, std::vector <std::string> &files)
{
    return;
}
}
#endif