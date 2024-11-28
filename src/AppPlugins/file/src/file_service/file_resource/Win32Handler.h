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
#include "ApplicationManager.h"
#include <vector>
#include <string>

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
    void GetDriveLayout(const std::wstring &physicalDrivePath);
    void HandleDeviceIoControlError(DWORD errorCode, std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX> &driveLayout);
    void TransformResultForVolume(
        AppProtect::ResourceResultByPage &returnValue, const FileResourceInfo &resourceInfo) override;
    std::wstring FindEFIPartition(const std::string& bcdOutput);
    std::wstring FindRecoveryPartition(const std::string& bcdOutput);
    std::wstring NormalizePath(const std::wstring &path);
    std::wstring GetSafeVolumePath(const std::wstring &volumePath);
    std::wstring GetVolumeLabel(const std::wstring &volumePath);
    std::wstring GetFileSystemType(const std::wstring &volumePath);
    std::wstring GetVolumeSerialNumber(const std::wstring &volumePath);
    std::wstring GetDriveTypeString(UINT driveType);
    std::wstring GetDriveLetterFromGUID(const std::wstring &volumePath);
    std::wstring GetVolumeName(const std::wstring volumeName);
    std::wstring ConvertDevicePath(const std::wstring& devicePath);
    std::wstring ExtractPartition(const std::string& line, const std::string& key);
    std::wstring TrimTrailingSpaces(const std::wstring& str);
    std::wstring CheckPartitionType(const std::wstring& drive);
    std::wstring GuidToWString(const GUID& guid);
    std::vector<int> GetPhysicalDriveForVolume(const std::wstring &volumePath, LONGLONG &offset);
    std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX> GetDiskLayout(HANDLE hDisk);
    HANDLE OpenDisk(const std::wstring &physicalDrivePath);
    std::string ExecCommand(const std::string& command);

    std::unordered_map<LONGLONG, WinVolumeInfo *> m_vMap;  // 卷偏移量和卷信息
    std::unordered_map<LONGLONG, GUID> m_pMap;     // 磁盘分区偏移量和分区GUID
};
}  // namespace FilePlugin

#endif  // APPPLUGIN_NAS_WIN32HANDLER_H
