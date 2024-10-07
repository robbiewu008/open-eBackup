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
#ifndef __COMMON_FILESYSTEM_UTIL_H__
#define __COMMON_FILESYSTEM_UTIL_H__

#include <string>
#include <iostream>
#include <iterator>
#include <optional>
#include <vector>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define UNICODE /* foring using WCHAR on windows */
#include <Windows.h>
#endif

#ifdef __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#endif

#include "define/Defines.h"

namespace Module {

using SparseRangeResult = std::optional<std::vector<std::pair<uint64_t, uint64_t>>>;

#ifdef WIN32
inline uint64_t CombineDWORD(DWORD low, DWORD high)
{
    return (uint64_t)low + ((uint64_t)MAXDWORD + 1) * high;
}

inline uint64_t ConvertWin32TimeToSeconds(DWORD low, DWORD high)
{
    const uint64_t UNIX_TIME_START = 0x019DB1DED53E8000; /* January 1, 1970 (start of Unix epoch) in "ticks" */
    const uint64_t TICKS_PER_SECOND = 10000000; /* a tick is 100ns */
    LARGE_INTEGER li;
    li.LowPart  = low;
    li.HighPart = high;
    /* Convert ticks since 1/1/1970 into seconds */
    return (li.QuadPart - UNIX_TIME_START) / TICKS_PER_SECOND;
}
#endif

/**
* wrap some cross-platform filesystem API for Windows/LINUX
*/
namespace FileSystemUtil {

#ifdef WIN32
/*
 * transform between UTF-8 and UTF-16
 * any method using win32 API passing WCHAR
 */
AGENT_API std::wstring Utf8ToUtf16(const std::string& str);
AGENT_API std::string Utf16ToUtf8(const std::wstring& wstr);
AGENT_API std::wstring GetFullPathW(const std::string& path);
AGENT_API std::string GetFullPath(const std::string& path);
#endif

/**
* wrap stat for LINUX and GetFileInformationByHandle for windows,
* to provide file meta info query service
*/
class AGENT_API StatResult {
public:
#ifdef __linux__
    StatResult(const std::string& path, const struct stat& statbuff);
#endif

#ifdef WIN32
    StatResult(const std::wstring& wPath, const BY_HANDLE_FILE_INFORMATION& handleFileInformation);
#endif
    /**
    * user id and group id of the owner, always set to zero on windows
    */
    uint64_t UserID() const;
    uint64_t GroupID() const;

    /**
    * AccessTime and CreationTime are invalid on FAT32 drives
    */
    uint64_t AccessTime() const;
    uint64_t CreationTime() const;
    uint64_t ModifyTime() const;

    /**
    * UNIX fs hardlink file share the same inode, but inode has no meaning on FAT32/HPFS/NTFS.. fs
    * Windows use "file index" to mark a unique id of a file or a directory in a volume
    */
    uint64_t UniqueID() const;

    uint64_t Size() const; /* size in bytes */
    uint64_t DeviceID() const; /* id of the disk containning the file */
    uint64_t LinksCount() const; /* no of hard links to the file, on WIN32 always set to 1 on non - NTFS fs */

    bool IsDirectory() const;
    std::string CanicalPath() const;

#ifdef WIN32
    /* based on dwFileAttributes field */
    bool IsArchive() const;
    bool IsCompressed() const;
    bool IsEncrypted() const;
    bool IsSparseFile() const;
    bool IsHidden() const;
    bool IsOffline() const;
    bool IsReadOnly() const;
    bool IsSystem() const;
    bool IsTemporary() const;
    bool IsNormal() const;
    bool IsReparsePoint() const;
    uint64_t Attribute() const;

    /* reparse point related methods */
    DWORD ReparseTag() const;
    bool IsSymbolicLink() const;
    bool IsJunctionPoint() const;
    bool IsMountedDevice() const;
    std::optional<std::wstring> MountedDeviceNameW() const;
    std::optional<std::wstring> JunctionsPointTargetPathW() const;
    std::optional<std::wstring> SymlinkTargetPathW() const;
    bool HasReparseSymbolicLinkTag() const;
    bool HasReparseMountPointTag() const;
    bool HasReparseNfsTag() const;
    bool HasReparseOneDriveTag() const;

    /* final path of the symbolic link/junction point */
    std::optional<std::wstring> FinalPathW() const;
    std::wstring CanicalPathW() const;
#endif

#ifdef __linux__
    /* based on st_mode field */
    bool IsRegular() const;
    bool IsPipe() const;
    bool IsCharDevice() const;
    bool IsBlockDevice() const;
    bool IsSymLink() const;
    bool IsSocket() const;
    uint64_t Mode() const;
#endif

private:
#ifdef __linux__
    struct stat m_stat {};
    std::string m_path; /* raw input path */
#endif
#ifdef WIN32
    BY_HANDLE_FILE_INFORMATION m_handleFileInformation{};
    std::wstring m_wPath; /* raw input path */
#endif
};

AGENT_API std::optional<StatResult> Stat(const std::string& path);
#ifdef WIN32
AGENT_API std::optional<StatResult> StatW(const std::wstring& wPath);
#endif

class AGENT_API OpenDirEntry {
public:
#ifdef WIN32
    OpenDirEntry(const std::string& dirPath,
        const WIN32_FIND_DATAW& findFileData, const HANDLE& fileHandle);
    bool IsArchive() const;
    bool IsCompressed() const;
    bool IsEncrypted() const;
    bool IsSparseFile() const;
    bool IsHidden() const;
    bool IsOffline() const;
    bool IsReadOnly() const;
    bool IsSystem() const;
    bool IsTemporary() const;
    bool IsNormal() const;
    bool IsReparsePoint() const;
    uint64_t AccessTime() const;
    uint64_t CreationTime() const;
    uint64_t ModifyTime() const;
    uint64_t Size() const;
    uint64_t Attribute() const;
#endif

#ifdef __linux__
    OpenDirEntry(const std::string& dirPath,
        DIR* dirPtr, struct dirent* direntPtr);
    bool IsUnknown() const;
    bool IsPipe() const;
    bool IsCharDevice() const;
    bool IsBlockDevice() const;
    bool IsSymLink() const;
    bool IsSocket() const;
    bool IsRegular() const;
    uint64_t INode() const;
#endif

    bool IsDirectory() const;
#ifdef WIN32
    std::wstring NameW() const;
    std::wstring FullPathW() const;
#endif
    std::string Name() const;
    std::string FullPath() const;
    bool Next();
    void Close();
    
    /* disable copy/assign construct */
    OpenDirEntry(const OpenDirEntry&) = delete;
    OpenDirEntry operator = (const OpenDirEntry&) = delete;
    ~OpenDirEntry();

private:
#ifdef WIN32
    std::wstring m_dirPath;
    HANDLE m_fileHandle = nullptr;
    WIN32_FIND_DATAW m_findFileData;
#endif

#ifdef __linux__
    std::string m_dirPath;
    DIR* m_dir = nullptr;
    struct dirent* m_dirent = nullptr;
#endif
};

AGENT_API std::optional<OpenDirEntry> OpenDir(const std::string& path);


#ifdef WIN32
/* Sparse File allocate range API */
AGENT_API SparseRangeResult QuerySparseWin32AllocateRangesW(const std::wstring& wPath);

AGENT_API std::vector<std::wstring> GetWin32DriverListW();
AGENT_API std::vector<std::string> GetWin32DriverList();

class AGENT_API Win32VolumesDetail {
public:
    Win32VolumesDetail(const std::wstring& wVolumeName);
    std::wstring VolumeNameW() const;
    std::string VolumeName() const;
    std::optional<std::wstring> GetVolumeDeviceNameW();
    std::optional<std::string> GetVolumeDeviceName();
    std::optional<std::vector<std::wstring>> GetVolumePathListW();
    std::optional<std::vector<std::string>> GetVolumePathList();
    std::optional<std::string> GetVolumeType();

private:
    std::wstring m_wVolumeName;
};

AGENT_API std::optional<std::vector<Win32VolumesDetail>> GetWin32VolumeList();

/* Win32 Security Descriptor related API */
AGENT_API std::optional<std::wstring> GetSecurityDescriptorW(const std::wstring& wPath);

/*
 * Normalize windows path, convert the path to the form like C:\Dir1\Dir2
 * Except root path like C:\, D:\, path won't ends with backslash
 */
std::wstring NormalizeWin32PathW(std::wstring& wPath);

#endif

}

}
#endif
