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
#include "Win32Handler.h"
#include <windows.h>
#include <iostream>
#include <sys/stat.h>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <locale>
#include <codecvt>
#include <winioctl.h>
#include <rpcdce.h>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include "Module/src/common/JsonHelper.h"
#include "ProtectPluginFactory.h"
#include "ApplicationServiceDataType.h"
#include "common/FileSystemUtil.h"
#include "PluginConstants.h"
#include "PluginNasTypes.h"
#include "PluginUtilities.h"
#include "ErrorCode.h"


using namespace std;
using namespace Module;
using namespace Module::FileSystemUtil;

namespace {
    constexpr auto WKERNEL_SPACE_DEVICE_PATH_PREFIX = LR"(\Device)";
    constexpr auto WUSER_SPACE_DEVICE_PATH_PREFIX = LR"(\\.)";
    const int NUMBER2 = 2;
    const int NUMBER3 = 3;
    const int NUMBER39 = 39;
    const int NUMBER1024 = 1024;
    // EFI 分区 GUID
    const GUID
        EFI_PARTITION_GUID = { 0xC12A7328, 0xF81F, 0x11D2, { 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B } };
    // Recovery 分区 GUID
    const GUID
        RECOVERY_PARTITION_GUID = { 0xDE94BBA4, 0x06D1, 0x4D40, { 0xA1, 0x6A, 0xBF, 0xD5, 0x01, 0x79, 0xD6, 0xAC } };
}

namespace {
    constexpr auto MODULE = "Win32Handler";
    constexpr auto WIN32_MAX_PATH_LEN = 4096;
    constexpr auto BACKSLASH = "\\";
    constexpr auto SLASH = "/";
    const std::string FILE_ITEM_TYPE = "f";
    const std::string DIRECTORY_ITEM_TYPE = "d";
    const std::string UNC_PATH_PREFIX = R"(\\?\)";
    const std::wstring UNKNOWN = L"Unknown";
}
namespace FilePlugin {
    static AutoRegAppManager<Win32Handler> g_autoReg{ ResourceType::WINDOWS };

    void Win32Handler::ListNativeResource(FileResourceInfo& resourceInfo, const ListResourceParam& listResourceParam)
    {
        int pageNo = listResourceParam.pageNo;
        int pageSize = listResourceParam.pageSize;
        string convertPath = listResourceParam.path;

        if (convertPath == SLASH || convertPath == BACKSLASH) {   // 兼容PM传来的"/"
            convertPath = "";
        }
        if (convertPath.empty()) {
            /* browse drivers */
            vector<std::string> drivers = FileSystemUtil::GetWin32DriverList();   // 返回驱动器
            for (const std::string &driver : drivers) {
                if (!IsDriverBackupable(driver)) {
                    continue;
                }
                NasShareResourceInfo resourceDetailInfo;
                resourceDetailInfo.path = driver;
                resourceDetailInfo.type = "d"; // show driver path as directory
                resourceDetailInfo.hasChildren = true;
                resourceInfo.resourceDetailVec.push_back(resourceDetailInfo);
                ++resourceInfo.totalCount;
            }
            return;
        }
        /* browse files */
        GetReourceList(resourceInfo, pageNo, pageSize, convertPath);  // 返回指定路径的资源
    }

    void Win32Handler::GetReourceList(
        FileResourceInfo& resourceInfo, int pageNo, int pageSize, const std::string& dirPath)
    {
        DBGLOG("list item in directory: %s", dirPath.c_str());
        auto startTime = chrono::steady_clock::now();
        BrowseFolderByPage(resourceInfo, dirPath, pageNo, pageSize);
        auto endTime = chrono::steady_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
        DBGLOG("GetReourceList, duration = %d", duration.count());
        return;
    }

    void Win32Handler::ListAggregateResource(
        FileResourceInfo& resourceInfo,
        const ListResourceParam& listResourceParam)
    {
        ERRLOG("Win32Handler ListAggregateResource not implemented!");
        return;
    }

    // 获取标签
    std::wstring Win32Handler::GetVolumeLabel(const std::wstring& volumePath)
    {
        wchar_t volumeLabel[MAX_PATH];
        if (GetVolumeInformationW(
            volumePath.c_str(),
            volumeLabel,
            MAX_PATH,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            0)) {
            return std::wstring(volumeLabel);
        }
        return UNKNOWN;
    }

    // 获取文件系统类型
    std::wstring Win32Handler::GetFileSystemType(const std::wstring& volumePath)
    {
        wchar_t fileSystemName[MAX_PATH];
        if (GetVolumeInformationW(
            volumePath.c_str(),
            nullptr,
            0,
            nullptr,
            nullptr,
            nullptr,
            fileSystemName,
            MAX_PATH)) {
            return std::wstring(fileSystemName);
        }
        return UNKNOWN;
    }

    // 获取卷序列号
    std::wstring Win32Handler::GetVolumeSerialNumber(const std::wstring& volumePath)
    {
        DWORD volumeSerialNumber;
        if (GetVolumeInformationW(
            volumePath.c_str(),
            nullptr,
            0,
            &volumeSerialNumber,
            nullptr,
            nullptr,
            nullptr,
            0)) {
            std::wstringstream ss;
            ss << std::hex << volumeSerialNumber;
            return ss.str();
        }
        return UNKNOWN;
    }

    // 获取驱动类型
    std::wstring Win32Handler::GetDriveTypeString(const uint32_t driveType)
    {
        switch (driveType) {
            case DRIVE_UNKNOWN:     return UNKNOWN;
            case DRIVE_NO_ROOT_DIR: return L"No Root Directory";
            case DRIVE_REMOVABLE:   return L"Removable";
            case DRIVE_FIXED:       return L"Fixed";
            case DRIVE_CDROM:       return L"CD-ROM";
            case DRIVE_RAMDISK:     return L"RAM Disk";
            default: return UNKNOWN;
        }
    }

    // 从GUID获取驱动器字母
    std::wstring Win32Handler::GetDriveLetterFromGUID(const std::wstring& volumePath)
    {
        WCHAR logicalDrives[MAX_PATH];
        bool ret = GetLogicalDriveStringsW(MAX_PATH, logicalDrives);
        if (!ret) {
            ERRLOG("Failed to get DriveLetter!");
            return UNKNOWN;
        }
        WCHAR* drive = logicalDrives;
        while (*drive) {
            WCHAR volumeName[MAX_PATH];
            if (GetVolumeNameForVolumeMountPointW(drive, volumeName, ARRAYSIZE(volumeName))) {
                if (volumePath == volumeName) {
                    return std::wstring(drive);
                }
            }
            drive += wcslen(drive) + 1;
        }
        return UNKNOWN;
    }

    // 获取物理磁盘
    std::vector<int> Win32Handler::GetPhysicalDriveForVolume(const std::wstring& volumePath, LONGLONG& offset)
    {
        std::string tmp = FileSystemUtil::Utf16ToUtf8(volumePath);
        std::vector<int> diskNumbers;
        HANDLE hVolume = CreateFileW(
            volumePath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            nullptr);
        if (hVolume == INVALID_HANDLE_VALUE) {
            ERRLOG("Failed to open volume:%s, %d", tmp.c_str(), GetLastError());
            return diskNumbers;
        }
        VOLUME_DISK_EXTENTS extents;
        DWORD bytesReturned;
        if (!DeviceIoControl(
            hVolume,
            IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
            nullptr,
            0,
            &extents,
            sizeof(extents),
            &bytesReturned,
            nullptr)) {
            ERRLOG("Failed to get disk extents for volume: %s, Error: %d", tmp.c_str(), GetLastError());
            CloseHandle(hVolume);
            return diskNumbers;
        }
        for (DWORD i = 0; i < extents.NumberOfDiskExtents; ++i) {
            INFOLOG("Disk: %d , start offset: %lld , extend length: %lld",
                extents.Extents[i].DiskNumber,
                extents.Extents[i].StartingOffset.QuadPart,
                extents.Extents[i].ExtentLength.QuadPart);
            diskNumbers.push_back(extents.Extents[i].DiskNumber);
            offset = extents.Extents[i].StartingOffset.QuadPart;
        }
        CloseHandle(hVolume);
        return std::move(diskNumbers);
    }

    // 打开物理磁盘
    HANDLE Win32Handler::OpenDisk(const std::wstring& physicalDrivePath)
    {
        std::string tmp = FileSystemUtil::Utf16ToUtf8(physicalDrivePath);
        INFOLOG("Enter GetDriveLayout: %s.", tmp.c_str());
        HANDLE hDisk = CreateFileW(
            physicalDrivePath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);
        if (hDisk == INVALID_HANDLE_VALUE) {
            ERRLOG("Failed to open physical drive: %s, %d", tmp.c_str(), GetLastError());
        }
        return hDisk;
    }

    using ErrorHandler = std::function<void(std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX>&)>;

    void Win32Handler::HandleDeviceIoControlError(DWORD errorCode,
        std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX>& driveLayout)
    {
        std::unordered_map<DWORD, ErrorHandler> errorHandlers = {
            {ERROR_INSUFFICIENT_BUFFER, [](std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX>&p) {
                ERRLOG("Out of buffer, resizing...");
            }},
            {ERROR_ACCESS_DENIED, [](std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX>&p) {
                ERRLOG("Access denied. Check permissions.");
                p.reset();
            }},
            {ERROR_INVALID_PARAMETER, [](std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX>&p) {
                ERRLOG("Invalid argument passed to DeviceIoControl.");
                p.reset();
            }},
            {ERROR_NOT_SUPPORTED, [](std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX>&p) {
                ERRLOG("The device does not support this operation.");
                p.reset();
            }},
            {ERROR_INVALID_HANDLE, [](std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX>&p) {
                ERRLOG("Invalid handle.");
                p.reset();
            }},
            {ERROR_IO_DEVICE, [](std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX>&p) {
                ERRLOG("Device I/O error.");
                p.reset();
            }}
        };
        auto it = errorHandlers.find(errorCode);
        if (it != errorHandlers.end()) {
            it->second(driveLayout);
        } else {
            ERRLOG("DeviceIoControl failed with error code: %d", errorCode);
            driveLayout.reset();
        }
    }

    std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX> Win32Handler::GetDiskLayout(HANDLE hDisk)
    {
        DWORD bytesReturned = 0;
        DWORD bufferSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX);
        std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX> driveLayout(
            (DRIVE_LAYOUT_INFORMATION_EX*)malloc(bufferSize),
            free);

        if (!driveLayout) {
            ERRLOG("Memory allocation failed.");
            return nullptr;
        }

        BOOL result = FALSE;
        while (!(result = DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
            nullptr,
            0,
            driveLayout.get(),
            bufferSize,
            &bytesReturned,
            nullptr))) {
            DWORD errorCode = GetLastError();
            if (errorCode == ERROR_INSUFFICIENT_BUFFER) {
                // 调整缓冲区大小
                if (bytesReturned > 0) {
                    bufferSize = bytesReturned;
                } else {
                    bufferSize *= NUMBER2;  // 如果 bytesReturned 为 0，倍增缓冲区大小
                }
                ERRLOG("Buffer too small, reallocating to %d bytes...", bufferSize);
                driveLayout.reset((DRIVE_LAYOUT_INFORMATION_EX*)malloc(bufferSize), free);
                if (!driveLayout) {
                    ERRLOG("Failed to reallocate memory.");
                    return nullptr;
                }
            } else {
                HandleDeviceIoControlError(errorCode, driveLayout);
                if (driveLayout == nullptr) {
                    return nullptr;
                }
            }
        }
        // 检查 DeviceIoControl 成功与否
        if (!result) {
            ERRLOG("Failed to get drive layout. Error: %d", GetLastError());
            return nullptr;
        }
        return driveLayout;
    }

    void Win32Handler::PrintPartitionInfo(std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX>& driveLayout)
    {
        INFOLOG("Number of Partition: %d", driveLayout->PartitionCount);
        for (DWORD i = 0; i < driveLayout->PartitionCount; ++i) {
            PARTITION_INFORMATION_EX partition = driveLayout->PartitionEntry[i];
            INFOLOG("Partition: %d, Partition style: %s",
                (i + 1), (partition.PartitionStyle == PARTITION_STYLE_MBR ? "MBR" : "GPT"));
            if (partition.PartitionStyle == PARTITION_STYLE_MBR) {
                INFOLOG("MBR");
                WARNLOG("MBR partition name to be implement.");
            } else if (partition.PartitionStyle == PARTITION_STYLE_GPT) {
                INFOLOG("GPT");
                Win32Handler::m_pMap.emplace(partition.StartingOffset.QuadPart, partition.Gpt.PartitionType);
            }
            INFOLOG("Partition offset : %lld, length: %lld, number : %u",
                partition.StartingOffset.QuadPart,
                partition.PartitionLength.QuadPart,
                partition.PartitionNumber);
        }
    }

    // 获取并输出磁盘分区布局
    void Win32Handler::GetDriveLayout(const std::wstring& physicalDrivePath)
    {
        // 打开物理磁盘
        HANDLE hDisk = Win32Handler::OpenDisk(physicalDrivePath);
        if (hDisk == INVALID_HANDLE_VALUE) {
            return;
        }
        std::shared_ptr<DRIVE_LAYOUT_INFORMATION_EX> driveLayout = Win32Handler::GetDiskLayout(hDisk);

        // 输出磁盘分区信息
        if (driveLayout != nullptr) {
            PrintPartitionInfo(driveLayout);
        }
        CloseHandle(hDisk);
        return;
    }

    std::wstring Win32Handler::CheckPartitionType(const std::wstring& drive)
    {
        // 打开分区
        auto hDevice = std::unique_ptr<void, decltype(&CloseHandle)>(
            CreateFile(drive.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL),
            CloseHandle);
        if (hDevice.get() == INVALID_HANDLE_VALUE) {
            ERRLOG("Failed to open device:%d", GetLastError());
            return UNKNOWN;
        }

        // 获取分区信息
        PARTITION_INFORMATION_EX partitionInfo;
        DWORD bytesReturned;
        if (DeviceIoControl(hDevice.get(), IOCTL_DISK_GET_PARTITION_INFO_EX, NULL, 0, &partitionInfo,
            sizeof(partitionInfo), &bytesReturned, NULL)) {
            if (partitionInfo.PartitionStyle == PARTITION_STYLE_GPT) {
                GUID partitionGuid = partitionInfo.Gpt.PartitionType;

                // 判断是否是 EFI 分区
                if (partitionGuid == EFI_PARTITION_GUID) {
                    INFOLOG("%s is an EFI System Partition.", Utf16ToUtf8(drive).c_str());
                    return L"EFI";
                } else if (partitionGuid == RECOVERY_PARTITION_GUID) {
                    INFOLOG("%s is a Microsoft Recovery Partition.", Utf16ToUtf8(drive).c_str());
                    return L"Recovery";
                } else {
                    INFOLOG("%s is not EFI or Recovery Partition.", Utf16ToUtf8(drive).c_str());
                    return UNKNOWN;
                }
            } else {
                INFOLOG("%s is not a GPT partition.", Utf16ToUtf8(drive).c_str());
                return UNKNOWN;
            }
        } else {
            INFOLOG("Failed to get partition info for:%s", Utf16ToUtf8(drive).c_str());
            return UNKNOWN;
        }
    }

    // 判断给定的分区是否是系统启动分区
    bool Win32Handler::IsSystemVolume(const std::wstring& driveLetter)
    {
        wchar_t systemPath[MAX_PATH];
        if (GetSystemDirectoryW(systemPath, MAX_PATH) == 0) {
            ERRLOG("Failed to get system directory.driveLetter is: %s",
                FileSystemUtil::Utf16ToUtf8(driveLetter).c_str());
            return false;
        }
        std::wstring systemDrive = std::wstring(systemPath).substr(0, 1);

        return systemDrive == driveLetter.substr(0, 1);
    }

    int Win32Handler::getVolumeType(const WinVolumeInfo& info)
    {
        if (info.partitionNameType == L"EFI") {
            return 0; // EFI system partition
        } else if (info.partitionNameType == L"Recovery") {
            return 1; // recovery
        } else if (IsSystemVolume(info.driveLetter)) {
            return NUMBER2; // 系统盘
        } else {
            return NUMBER3; // 普通卷
        }
    }

    StringVolumeInfo Win32Handler::ConvertVolumeInfo(const WinVolumeInfo& info)
    {
        StringVolumeInfo stringInfo;
        try {
            std::string tempVolumeName = FileSystemUtil::Utf16ToUtf8(info.volumeName);
            stringInfo.volumeName = tempVolumeName.substr(tempVolumeName.find("Volume{"),
                tempVolumeName.find("}") - tempVolumeName.find("Volume{") + 1);
        } catch (const std::out_of_range& e) {
            ERRLOG("Out of range error: %s", e.what());
        }
        stringInfo.label = FileSystemUtil::Utf16ToUtf8(info.label);
        stringInfo.fileSystem = FileSystemUtil::Utf16ToUtf8(info.fileSystem);
        stringInfo.volumeSerialNumber = FileSystemUtil::Utf16ToUtf8(info.volumeSerialNumber);
        stringInfo.driveType = FileSystemUtil::Utf16ToUtf8(info.driveType);
        stringInfo.driveLetter = FileSystemUtil::Utf16ToUtf8(info.driveLetter);
        stringInfo.isHealthy = info.isHealthy;
        stringInfo.totalSize = info.totalSize;
        stringInfo.freeSpace = info.freeSpace;
        stringInfo.displayName = stringInfo.label + "(" + stringInfo.driveLetter + ")";
        stringInfo.drivePath = FileSystemUtil::Utf16ToUtf8(info.drivePath);
        std::string tmp = FileSystemUtil::Utf16ToUtf8(info.partitionName);
        stringInfo.partitionName = FileSystemUtil::Utf16ToUtf8(info.partitionName);
        stringInfo.partitionGuid = FileSystemUtil::Utf16ToUtf8(info.partitionGuid);
        stringInfo.partitionNameType = FileSystemUtil::Utf16ToUtf8(info.partitionNameType);
        INFOLOG("transfer info: %s, %s", stringInfo.partitionName.c_str(), tmp.c_str());
        stringInfo.volumeType = getVolumeType(info);
        return stringInfo;
    }

    WinVolumeInfo Win32Handler::ConvertStringVolumeInfo(const StringVolumeInfo& info)
    {
        WinVolumeInfo volumeInfo;
        volumeInfo.isHealthy = info.isHealthy;
        volumeInfo.partitionType = info.partitionType;
        volumeInfo.partitionNumber = info.partitionNumber;
        volumeInfo.volumeType = info.volumeType;
        volumeInfo.totalSize = info.totalSize;
        volumeInfo.freeSpace = info.freeSpace;
        volumeInfo.partitionOffset = info.partitionOffset;
        volumeInfo.partitionLength = info.partitionLength;
        volumeInfo.volumeName = FileSystemUtil::Utf8ToUtf16(info.volumeName);
        volumeInfo.label = FileSystemUtil::Utf8ToUtf16(info.label);
        volumeInfo.fileSystem = FileSystemUtil::Utf8ToUtf16(info.fileSystem);
        volumeInfo.volumeSerialNumber = FileSystemUtil::Utf8ToUtf16(info.volumeSerialNumber);
        volumeInfo.driveType = FileSystemUtil::Utf8ToUtf16(info.driveType);
        volumeInfo.driveLetter = FileSystemUtil::Utf8ToUtf16(info.driveLetter);
        volumeInfo.drivePath = FileSystemUtil::Utf8ToUtf16(info.drivePath);
        volumeInfo.partitionGuid = FileSystemUtil::Utf8ToUtf16(info.partitionGuid);
        volumeInfo.partitionName = FileSystemUtil::Utf8ToUtf16(info.partitionName);
        volumeInfo.partitionNameType = FileSystemUtil::Utf8ToUtf16(info.partitionNameType);
        return volumeInfo;
    }

    std::wstring Win32Handler::GetVolumeName(const std::wstring volumeName)
    {
        std::wstring volumePath(volumeName);
        WCHAR szDrive[MAX_PATH] = { 0 };
        if (GetVolumeNameForVolumeMountPointW(volumePath.c_str(), szDrive, ARRAYSIZE(szDrive))) {
            return std::wstring(szDrive);
        } else {
            return UNKNOWN;
        }
    }

    void Win32Handler::PopulateVolumeInfo(WinVolumeInfo& info, const std::wstring& volumePath)
    {
        info.label = GetVolumeLabel(volumePath);
        info.fileSystem = GetFileSystemType(volumePath);
        info.volumeSerialNumber = GetVolumeSerialNumber(volumePath);
        info.driveType = GetDriveTypeString(GetDriveTypeW(volumePath.c_str()));
        info.driveLetter = GetDriveLetterFromGUID(volumePath);
        info.isHealthy = true;
        std::wstring wVolumeParam = volumePath.substr(4);
        wVolumeParam.pop_back();
        WCHAR wDeviceNameBuffer[MAX_PATH] = L"";
        DWORD charCount = ::QueryDosDeviceW(wVolumeParam.c_str(), wDeviceNameBuffer, ARRAYSIZE(wDeviceNameBuffer));
        if (charCount == 0) {
            info.isQuery = 1;
            WARNLOG("Query Dos Device Information failed: %d", GetLastError());
        }
        std::wstring wVolumeDevicePath = wDeviceNameBuffer;
        auto pos = wVolumeDevicePath.find(WKERNEL_SPACE_DEVICE_PATH_PREFIX);
        if (pos == 0) {
            wVolumeDevicePath = WUSER_SPACE_DEVICE_PATH_PREFIX +
                wVolumeDevicePath.substr(std::wstring(WKERNEL_SPACE_DEVICE_PATH_PREFIX).length());
        }

        std:string tempVolumePath = FileSystemUtil::Utf16ToUtf8(wVolumeDevicePath);
        INFOLOG("Volume Path: %s", tempVolumePath.c_str());
        info.drivePath = wVolumeDevicePath;
        info.isQuery = 0;
    }

    std::vector<WinVolumeInfo> Win32Handler::GetAllVolumes()
    {
        std::vector<WinVolumeInfo> volumes;
        WCHAR volumeName[MAX_PATH];
        HANDLE hVolume = FindFirstVolumeW(volumeName, ARRAYSIZE(volumeName));
        if (hVolume == INVALID_HANDLE_VALUE) {
            ERRLOG("FindFirstVolumeW failed (Error: %d)", GetLastError());
            return volumes;
        }

        do {
            WinVolumeInfo info;
            info.volumeName = GetVolumeName(volumeName);
            ULARGE_INTEGER freeBytesAvailable;
            ULARGE_INTEGER totalNumberOfBytes;
            ULARGE_INTEGER totalNumberOfFreeBytes;
            if (GetDiskFreeSpaceExW(
                info.volumeName.c_str(),
                &freeBytesAvailable,
                &totalNumberOfBytes,
                &totalNumberOfFreeBytes)) {
                info.totalSize = totalNumberOfBytes.QuadPart;
                info.freeSpace = totalNumberOfFreeBytes.QuadPart;
            }
            PopulateVolumeInfo(info, info.volumeName);
            info.partitionNameType = CheckPartitionType(info.drivePath);
            volumes.push_back(info);
        } while (FindNextVolumeW(hVolume, volumeName, ARRAYSIZE(volumeName)));

        FindVolumeClose(hVolume);
        return volumes;
    }

    void Win32Handler::LogStringVolumeInfo(const StringVolumeInfo& info)
    {
        std::stringstream logMessage;
        logMessage << "Volume Name: " << info.volumeName << ", "
            << "Label: " << info.label << ", "
            << "File System: " << info.fileSystem << ", "
            << "Total Size: " << info.totalSize / (NUMBER1024 * NUMBER1024 * NUMBER1024) << L" GB, "
            << "Free Space: " << info.freeSpace / (NUMBER1024 * NUMBER1024 * NUMBER1024) << L" GB, "
            << "Health Status: " << (info.isHealthy ? "Healthy" : "Unhealthy") << ", "
            << "Volume Serial Number: " << info.volumeSerialNumber << ", "
            << "Drive Type: " << info.driveType << ", "
            << "Drive Path: " << info.drivePath << ", "
            << "Drive Letter: " << info.driveLetter << ", "
            << "partition name: " << info.partitionName <<", "
            << "partition GUID: " << info.partitionGuid << ", "
            << "partition Name Type: " << info.partitionNameType << ", "
            << "is backupable: " << info.isBackupable;
        std::string logMessageresult = logMessage.str();
        INFOLOG("VolumeInfo: %s", logMessageresult.c_str());
    }

    std::wstring Win32Handler::GuidToWString(const GUID& guid)
    {
        wchar_t guidString[NUMBER39];
        if (StringFromGUID2(guid, guidString, NUMBER39)) {
            return std::wstring(guidString);
        }
        return L"";
    }

    void Win32Handler::ListVolumeResource(FileResourceInfo& resourceInfo, const ListResourceParam& listResourceParam)
    {
        std::vector<WinVolumeInfo> volumes = GetAllVolumes();
        std::unordered_set<int> diskNumbers;
        for (int i = 0; i < volumes.size(); ++i) {
            LONGLONG offset;
            std::vector<int> tmpDiskNumbers = GetPhysicalDriveForVolume(volumes[i].drivePath, offset);
            if (tmpDiskNumbers.empty()) {
                continue;
            }
            for (int diskNumber : tmpDiskNumbers) {
                diskNumbers.emplace(diskNumber);
            }
            volumes[i].partitionOffset = offset;
            m_vMap.emplace(offset, &volumes[i]);
        }

        if (diskNumbers.empty()) {
            ERRLOG("no physical disk found ");
            return;
        }

        for (int diskNumber : diskNumbers) {
            std::wstring physicalDrivePath = L"\\\\.\\PhysicalDrive" + std::to_wstring(diskNumber);
            GetDriveLayout(physicalDrivePath);
        }
        for (auto it = m_vMap.begin(); it != m_vMap.end(); ++it) {
            WinVolumeInfo* v = it->second;
            if (m_pMap.count(it->first) == 0) {
                INFOLOG("not found offset for: %u", v->partitionOffset);
                continue;
            }
            v->partitionName = GuidToWString(m_pMap[it->first]);
        }
        if (volumes.empty()) {
            WARNLOG("Volume not found or an error has occurred.");
        } else {
            for (const auto& info : volumes) {
                StringVolumeInfo stringInfo = ConvertVolumeInfo(info);
                if (info.driveType != L"Fixed") {
                    continue;
                }

                //"1"为可以备份，“0”为不可以
                if ((info.fileSystem == L"NTFS") || (info.volumeType == 0)) {
                    stringInfo.isBackupable = "1";
                } else {
                    stringInfo.isBackupable = "0";
                }

                LogStringVolumeInfo(stringInfo);
                resourceInfo.volumeResourceDetailVec.push_back(stringInfo);
            }
        }
        return;
    }

    void Win32Handler::TransformResultForVolume(AppProtect::ResourceResultByPage& returnValue,
        const FilePlugin::FileResourceInfo& resourceInfo)
    {
        returnValue.total = resourceInfo.totalCount;
        for (auto resourceInfo : resourceInfo.volumeResourceDetailVec) {
            ApplicationResource resource;
            resource.__set_id(std::to_string(returnValue.total));
            std::string extendInfo;
            if (!Module::JsonHelper::StructToJsonString(resourceInfo, extendInfo)) {
                continue;
            }
            resource.__set_extendInfo(extendInfo);
            returnValue.items.push_back(resource);
        }
    }


    /*
        * Symbolic link & junction point treated as link file,
        * while device mount point treated as directory
        */
    static std::string GetSubItemType(const StatResult& statResult)
    {
        if (!statResult.IsReparsePoint()) {
            /* common file */
            return statResult.IsDirectory() ? DIRECTORY_ITEM_TYPE : FILE_ITEM_TYPE;
        }
        if (statResult.IsSymbolicLink() || statResult.IsJunctionPoint()) {
            return FILE_ITEM_TYPE;
        }
        if (statResult.IsMountedDevice()) {
            return DIRECTORY_ITEM_TYPE;
        }
        const std::string fullpath = statResult.CanicalPath();
        WARNLOG("unknown reparse point: %s", fullpath.c_str());
        return statResult.IsDirectory() ? DIRECTORY_ITEM_TYPE : FILE_ITEM_TYPE;
    }

    void Win32Handler::BrowseFolderByPage(FileResourceInfo& resourceInfo,
        const std::string& dirPath, int pageNo, int pageSize)
    {
        int countBegin = pageNo * pageSize - 1;
        int countEnd = countBegin + pageSize;
        int totalNum = 0;
        int error;
        std::optional<OpenDirEntry> openDirEntry = FileSystemUtil::OpenDir(dirPath);
        if (!openDirEntry) {
            error = ::GetLastError();
            uint32_t errorCode = (error == ERROR_ACCESS_DENIED) ? E_RESOURCE_NO_ACCESS_RIGHT : E_RESOURCE_DIR_NOT_EXIST;
            std::string errMsg = (error == ERROR_ACCESS_DENIED) ? "RESOURCE_NO_ACCESS_RIGHT" : "RESOURCE_DIR_NOT_EXIST";
            ERRLOG("open dirent failed, path = %s, error = %d, mssage = %s", dirPath.c_str(), error, errMsg.c_str());
            ThrowAppException(errorCode, errMsg.c_str());
            return;
        }
        do {
            const std::string entryName = openDirEntry->Name();
            DBGLOG("list directory %s find entry %s", dirPath.c_str(), entryName.c_str());
            if (entryName == "." || entryName == "..") {
                continue;
            }
            const std::string fullpath = openDirEntry->FullPath();
            std::optional<StatResult> subStatResult = Stat(openDirEntry->FullPath());
            if (!subStatResult) {
                error = ::GetLastError();
                ERRLOG("stat path %s fail, error = %d", fullpath.c_str(), error);
                continue;
            }
            if (totalNum <= countBegin || totalNum > countEnd) {
                ++totalNum;
                continue;
            }

            NasShareResourceInfo resourceDetailInfo{};
            resourceDetailInfo.path = fullpath;
            resourceDetailInfo.size = openDirEntry->Size();
            resourceDetailInfo.type = GetSubItemType(subStatResult.value());
            resourceDetailInfo.hasChildren = openDirEntry->IsDirectory() ? true : false;
            resourceInfo.resourceDetailVec.push_back(resourceDetailInfo);
            ++totalNum;
        } while (openDirEntry->Next());
        INFOLOG("total num: %d", totalNum);
        resourceInfo.totalCount = totalNum;
        return;
    }

    bool Win32Handler::IsDriverBackupable(const std::string& driverPath)
    {
        /* Only accept driver root path, so ANSI version works */
        UINT driverType = ::GetDriveTypeA(driverPath.c_str());
        if (driverType == DRIVE_UNKNOWN) {
            WARNLOG("driver path %s is an unknown driver, skip.", driverPath.c_str());
            return false;
        }
        if (driverType == DRIVE_CDROM) {
            WARNLOG("driver path %s is a CD-ROM driver, skip.", driverPath.c_str());
            return false;
        }
        return true;
    }
}