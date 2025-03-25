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
#ifndef APPPLUGIN_NAS_WIN32HANDLER_H
#define APPPLUGIN_NAS_WIN32HANDLER_H

#include <vector>
#include <string>
#include "ApplicationManager.h"
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

namespace FilePlugin {
class Win32Handler : public ApplicationManager {
public:
    Win32Handler() = default;

    virtual ~Win32Handler() noexcept
    {}

    void ListNativeResource(
        FilePlugin::FileResourceInfo &resourceInfo, const FilePlugin::ListResourceParam &listResourceParam) override;

    void ListAggregateResource(
        FilePlugin::FileResourceInfo &resourceInfo, const FilePlugin::ListResourceParam &listResourceParams) override;

    void ListVolumeResource(
        FilePlugin::FileResourceInfo &resourceInfo, const FilePlugin::ListResourceParam &listResourceParam) override;
    std::vector<WinVolumeInfo> GetAllVolumes();
    StringVolumeInfo ConvertVolumeInfo(const WinVolumeInfo &info);
    WinVolumeInfo ConvertStringVolumeInfo(const StringVolumeInfo &info);
    void LogStringVolumeInfo(const StringVolumeInfo &info);
    std::wstring GetVolumeLabel(const std::wstring &volumePath);
    DWORD GetPartitionNumber(const std::wstring& drive);

private:
    void GetReourceList(FileResourceInfo &resourceInfo, int pageNo, int pageSize, const std::string &convertPath);

    void BrowseFolderByPage(
        FilePlugin::FileResourceInfo &resourceInfo, const std::string &parentPath, int pageNo, int pageSize);

    int getVolumeType(const WinVolumeInfo &info);
    bool IsDriverBackupable(const std::string &driverPath);
    bool GetVolumeDiskExtents(const std::wstring &volumePath, std::vector<VOLUME_DISK_EXTENTS> &extents);
    bool IsPathValid(const std::wstring &path);
    bool IsSafePath(const std::wstring &path);
    bool IsSystemVolume(const std::wstring &driveLetter);
    void GetVolumeSpaceInfo(const std::wstring &volumePath, ULONGLONG &totalSize, ULONGLONG &freeSize);
    void PopulateVolumeInfo(WinVolumeInfo &info, const std::wstring &volumePath);
    void PrintPartitionInfo(std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX> &driveLayout);
    void HandleDeviceIoControlError(DWORD errorCode, std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX> &driveLayout);
    void TransformResultForVolume(
        AppProtect::ResourceResultByPage &returnValue, const FileResourceInfo &resourceInfo) override;
    std::wstring FindEFIPartition(const std::string& bcdOutput);
    std::wstring FindRecoveryPartition(const std::string& bcdOutput);
    std::wstring NormalizePath(const std::wstring &path);
    std::wstring GetSafeVolumePath(const std::wstring &volumePath);
    std::wstring GetFileSystemType(const std::wstring &volumePath);
    std::wstring GetVolumeSerialNumber(const std::wstring &volumePath);
    std::wstring GetDriveTypeString(UINT driveType);
    std::wstring GetDriveLetterFromGUID(const std::wstring &volumePath);
    std::wstring GetVolumeName(const std::wstring volumeName);
    std::wstring ExtractPartition(const std::string& line, const std::string& key);
    std::wstring CheckPartitionType(const std::wstring& drive);
    std::wstring GuidToWString(const GUID& guid);
    std::vector<int> GetPhysicalDriveForVolume(const std::wstring &volumePath, LONGLONG &offset);
    std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX> GetDiskLayout(HANDLE hDisk);
    std::string ExecCommand(const std::string& command);

    bool IsEqualGUIDString(const GUID& guid, const std::wstring& guidStr);
    std::wstring GuidToString(const GUID& guid);
    bool GetPartitionTypeGUID(wchar_t* drivePath, GUID& partitionTypeGuid);
    std::wstring GuidToWString(GUID guid);

    std::unordered_map<LONGLONG, WinVolumeInfo *> m_vMap;  // 卷偏移量和卷信息
    std::unordered_map<LONGLONG, std::wstring> m_pMap;     // 磁盘分区偏移量和分区名
    std::unordered_map<LONGLONG, std::wstring> m_gMap;     // 磁盘分区偏移量和分区GUID
};
}  // namespace FilePlugin

#endif  // APPPLUGIN_NAS_WIN32HANDLER_H
