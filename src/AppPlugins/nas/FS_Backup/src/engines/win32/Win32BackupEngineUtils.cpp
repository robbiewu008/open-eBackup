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
#include "Win32BackupEngineUtils.h"
#include <cstring>
#include <string>
#include <algorithm>
#include "FileSystemUtil.h"
#include "ParserStructs.h"
#include "Win32PathUtils.h"
#include "log/Log.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
    const std::string BACKSLASH = "\\";
    const std::string SLASH = "/";
    const std::wstring WPATH_PREFIX = LR"(\\?\)";
    const std::string PATH_PREFIX = R"(\\?\)";
    const uint32_t NUM_2 = 2;
    const uint32_t NUM_7 = 7;
    const std::wstring WMAIN_STREAM_NAME = L"::$DATA";
    const std::wstring WSTREAM_NAME_SUFFIX = L":$DATA";
    const int MAX_RENAME_INDEX = 100;
}

std::string Win32BackupEngineUtils::ReverseSlash(const std::string& path)
{
    string tmp = path;
    for (char& c : tmp) {
        if (c == '/') {
            c = '\\';
        }
    }
    return tmp;
}

std::string Win32BackupEngineUtils::PathConcat(
    const std::string& forwardPath,
    const std::string& ctrlEntryPath,
    const std::string& trimPrefixPath)
{
    /* If the forwardPath(rootPath) is not empty, it in the win format; Ex: "X:\abc\" + /c/test.txt
     * or in the posix format; Ex: /c/test.txt
     */
    std::string trailPath = ctrlEntryPath;
    if (!trimPrefixPath.empty()) {
        /*
         * trim prefix path from control file entry path,
         * ex: trimPrefixPath = /c/dir1/dir2, ctrlEntryPath = /c/dir1/dir2/dir3/dir4 => trailPat = /dir3/dir4
         */
        if (ctrlEntryPath.find(trimPrefixPath) == 0) {
            trailPath = ctrlEntryPath.substr(trimPrefixPath.length());
        } else {
            ERRLOG("%s is not the prefix of %s", trimPrefixPath.c_str(), ctrlEntryPath.c_str());
        }
    }
    std::string concatedWin32Path = RemoveExtraSlash(forwardPath + trailPath);
    if (concatedWin32Path.length() < NUM_2) {
        ERRLOG("invalid path: %s", concatedWin32Path.c_str());
        return "";
    }
    // 是windows格式的路径， 就直接把分隔符换一下
    if (concatedWin32Path[0] != '/' && concatedWin32Path[1] == ':') {
        std::replace(concatedWin32Path.begin(), concatedWin32Path.end(), SLASH[0], BACKSLASH[0]);
        DBGLOG("after PathConcat:%s", concatedWin32Path.c_str());
        return concatedWin32Path;
    }
    std::replace(concatedWin32Path.begin(), concatedWin32Path.end(), BACKSLASH[0], SLASH[0]);
    std::string returnString = Win32PathUtil::PosixToWin32(concatedWin32Path);
    DBGLOG("after PathConcat:%s", returnString.c_str());
    return returnString;
}

/* extended unicode path to break MAX_PATH 260 limit */
std::wstring Win32BackupEngineUtils::ExtenedPathW(const std::string& path)
{
    std::wstring wPath = FileSystemUtil::Utf8ToUtf16(path);
    if (wPath.length() > WPATH_PREFIX.length() && wPath.find(WPATH_PREFIX) == 0) {
        /* already have prefix */
        return wPath;
    }
    return WPATH_PREFIX + wPath;
}

bool Win32BackupEngineUtils::RemovePath(const std::string& path)
{
    DBGLOG("remove path:%s", path.c_str());
    std::wstring wpath = FS_Backup::Win32BackupEngineUtils::ExtenedPathW(path);
    if (DeleteFileW(wpath.c_str()) || RemoveDirectoryW(wpath.c_str())) {
        return true;
    }
    ERRLOG("Error code:%d", ::GetLastError());
    return false;
}

/* extended unicode path to break MAX_PATH 260 limit */
std::string Win32BackupEngineUtils::ExtenedPath(std::string path)
{
    if (path.length() > PATH_PREFIX.length() && path.find(PATH_PREFIX) == 0) {
        /* already have prefix */
        return path;
    }
    return PATH_PREFIX + path;
}

bool Win32BackupEngineUtils::CreateDirectoryRecursively(const string& dirPath, const string& rootPath, DWORD& errorCode)
{
    wstring wDirPath = Win32BackupEngineUtils::ExtenedPathW(dirPath);

    // check path existence
    auto result = FileSystemUtil::StatW(wDirPath);
    if (result) {
        if (result->IsDirectory() && (!result->IsSymbolicLink() && !result->IsJunctionPoint())) {
            errorCode = 0;
            return true;  // directory already exists
        }
        // check path is dataRepository SymbolicLink path
        if ((!rootPath.empty()) && result->IsDirectory() && result->IsSymbolicLink() &&
            Win32PathUtil::LowerCase(dirPath).rfind(Win32PathUtil::LowerCase(rootPath), 0) == 0) {
            return true;  // directory is DataRepository
        }
        WARNLOG("%s already have been created as a file/symlink, delete this file!", dirPath.c_str());
        if (!::DeleteFileW(wDirPath.c_str())) {
            errorCode = ::GetLastError();
            ERRLOG("File %s delete failed, errno %d", dirPath.c_str(), errorCode);
            return false;
        }
    }

    string parentDirPath = Win32PathUtil::GetParentDir(dirPath);
    if (parentDirPath.length() == dirPath.length()) {  // reached root driver path and driver not valid
        ERRLOG("RecurseCreateDirectory found driver root %s invalid!", dirPath.c_str());
        return false;
    }
    if (!CreateDirectoryRecursively(parentDirPath, rootPath, errorCode)) {
        return false;
    }
    if (!::CreateDirectoryW(wDirPath.c_str(), NULL)) {  // Win32 API
        errorCode = ::GetLastError();
        if (errorCode == ERROR_ALREADY_EXISTS) {
            errorCode = 0;
            return true;
        }
        ERRLOG("Dir %s create failed, errno %d", dirPath.c_str(), errorCode);
        return false;
    }
    return true;
}

bool Win32BackupEngineUtils::IsFileHandleSymbolicLink(const FileHandle& fileHandle)
{
    /* m_mode is used as a reserved fields for Windows Backup */
    uint32_t reserved = fileHandle.m_file->m_mode;
    return ((reserved & Module::FILEMETA_FLAG_WIN32_SYMBOLIC_LINK) != 0);
}

bool Win32BackupEngineUtils::IsFileHandleJunctionPoint(const FileHandle& fileHandle)
{
    /* m_mode is used as a reserved fields for Windows Backup */
    uint32_t reserved = fileHandle.m_file->m_mode;
    return ((reserved & Module::FILEMETA_FLAG_WIN32_JUNCTION_LINK)  != 0);
}

std::string Win32BackupEngineUtils::GetSymbolicLinkTargetPath(const FileHandle& fileHandle)
{
    using XAttrsType = std::vector<std::pair<std::string, std::string>>;
    const XAttrsType& xMetaList = fileHandle.m_file->m_xattr;
    XAttrsType::const_iterator it = std::find_if(xMetaList.begin(), xMetaList.end(),
        [](const auto& entry) {return entry.first == EXTEND_ATTR_KEY_WIN32_SYMBOLIC_TARGET;});
    return it == xMetaList.end() ? "" : it->second;
}

std::string Win32BackupEngineUtils::GetJunctionPointTargetPath(const FileHandle& fileHandle)
{
    using XAttrsType = std::vector<std::pair<std::string, std::string>>;
    const XAttrsType& xMetaList = fileHandle.m_file->m_xattr;
    XAttrsType::const_iterator it = std::find_if(xMetaList.begin(), xMetaList.end(),
        [](const auto& entry) {return entry.first == EXTEND_ATTR_KEY_WIN32_JUNCTION_TARGET;});
    return it == xMetaList.end() ? "" : it->second;
}

/* winErrorID is a DWORD type, can be obtained by ::GetLastError call */
std::string Win32BackupEngineUtils::ParseErrorMessage(uint32_t winErrorID)
{
    if (winErrorID == 0) {
        return std::string(); /* No error message has been recorded */
    }
    
    LPWSTR messageBuffer = nullptr;
    /* Ask Win32 to give us the string version of that message ID.
     * The parameters we pass in, tell Win32 to create the buffer that holds the message for us
     * (because we don't yet know how long the message string will be).
     */
    size_t size = ::FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        winErrorID,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&messageBuffer,
        0,
        nullptr);
    
    /* Copy the error message into a std::wstring */
    std::wstring wMessage(messageBuffer, size);

    /* Free the Win32's string's buffe */
    ::LocalFree(messageBuffer);

    std::string message = FileSystemUtil::Utf16ToUtf8(wMessage);
    while (!message.empty() && (message.back() == '\n' || message.back() == '\r')) { /* remove new line token at back */
        message.pop_back();
    }
    /* if failed to obtain meaningful error string, using error code instead */
    if (message.empty()) {
        message = std::string("Win32ErrorCode ") + std::to_string(winErrorID);
    }
    return message;
}

void Win32BackupEngineUtils::ConvertUint64ToDword(
    _In_    uint64_t    source,
    _Out_   DWORD&      dwordHigh,
    _Out_   DWORD&      dwordLow)
{
    DWORD *p = (DWORD*)&source;
    dwordLow = *p;
    dwordHigh = *(p+1);
}

std::string Win32BackupEngineUtils::RemoveExtraSlash(const std::string& path)
{
    string temp = path;
    for (auto it = temp.begin(); it + 1 != temp.end();) {
        if (*it == '/' && *(it + 1) == '/') {
            temp.erase(it + 1);
        } else {
            ++it;
        }
    }
    return temp;
}

/*
 * get stream path name file/directory, will return nullopt if it's main stream or it's in invalid format
 * example: :ads1.txt:$DATA => ads1.txt
 */
std::optional<std::wstring> Win32BackupEngineUtils::GetStreamNameW(const std::wstring& wStreamName)
{
    // Hint:: remove later
    std::string streamName = FileSystemUtil::Utf16ToUtf8(wStreamName);
    DBGLOG("stream name = %s", streamName.c_str());

    if (wStreamName.size() <= WMAIN_STREAM_NAME.length()) {
        /* it's invalid stream name or it's a main stream name */
        return std::nullopt;
    }
    auto suffixPos = wStreamName.find(WSTREAM_NAME_SUFFIX);
    if (wStreamName[0] != L':'
        || suffixPos == std::string::npos
        || suffixPos + WSTREAM_NAME_SUFFIX.length() != wStreamName.length()) {
        /* invalid format */
        return std::nullopt;
    }
    return wStreamName.substr(1, wStreamName.length() - WSTREAM_NAME_SUFFIX.length() - 1);
}

static std::wstring GenerateRenameSuffixW(const std::wstring& wpath, int index)
{
    std::wstring indexSuffix = L" (" + std::to_wstring(index) + L")";
    auto slashPos = wpath.rfind(L"\\");
    std::wstring wnewPath = wpath.substr(0, slashPos);
    std::wstring wnewName = wpath.substr(slashPos + 1);
    auto dotPos = wnewName.rfind(L".");
    if (dotPos == std::wstring::npos) {
        wnewName += indexSuffix;
    } else {
        wnewName = wnewName.substr(0, dotPos) + indexSuffix + wnewName.substr(dotPos);
    }
    wnewPath += wnewName;
    return wnewPath;
}

std::wstring Win32BackupEngineUtils::GetUnusedNewPathW(const std::wstring& wpath)
{
    std::wstring wnewPath = wpath;
    int index = 0;
    while (IsFileExistsW(wnewPath) && index < MAX_RENAME_INDEX) {
        wnewPath = GenerateRenameSuffixW(wpath, index++);
    }
    return wnewPath;
}

bool Win32BackupEngineUtils::IsFileExistsW(const std::wstring& wpath)
{
    return ::GetFileAttributesW(wpath.c_str());
}
