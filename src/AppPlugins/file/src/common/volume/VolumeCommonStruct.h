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
#ifndef VOLUME_COMMONSTRUCT_H
#define VOLUME_COMMONSTRUCT_H

#include <string>
#include "ApplicationProtectFramework_types.h"
#include "PluginConstants.h"
#include "JsonHelper.h"

namespace FilePlugin {

struct PluginReportInfo {
    AppProtect::JobLogLevel::type   logLevel    { AppProtect::JobLogLevel::TASK_LOG_INFO };
    AppProtect::SubJobStatus::type  jobStatus   { AppProtect::SubJobStatus::RUNNING };
    int                 jobProgress { PROGRESS0 };
    std::string         logLabel    {};
    int64_t             errCode     { INITIAL_ERROR_CODE };
};

struct VolumeBackupJobExtend {
    std::string systemBackupFlag = "false";
    std::string snapshotPercent;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(systemBackupFlag, system_backup_flag)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(snapshotPercent, snapshot_size_percent)
    END_SERIAL_MEMEBER
};

enum class VolumeType {
    SNAPSHOT = 1,
    PART,
    LVM
};

struct VolumeInfo {
    std::string     volumePath;
    std::string     oriVolumePath;
    VolumeType      type;
    uint64_t        size;
    std::string     volumeName;
};

struct VolumeLivemountDetail {
    std::string volumeName;
    std::string dstPath;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeName, volume_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dstPath, dst_path)
    END_SERIAL_MEMEBER
};

struct VolumeLivemountFileSystemShareInfoAdvanceParam {
    int domainType {0};
    std::vector<VolumeLivemountDetail> livemountDetail;
    std::string shareName;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(domainType, domainType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(livemountDetail, livemount_detail)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(shareName, shareName)
    END_SERIAL_MEMEBER
};

struct VolumeLivemountFileSystemShareInfo {
    int accessPermission {0};
    VolumeLivemountFileSystemShareInfoAdvanceParam advanceParams;
    std::string fileSystemName {0};
    int type {0};
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(accessPermission, accessPermission)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(advanceParams, advanceParams)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fileSystemName, fileSystemName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(type, type)
    END_SERIAL_MEMEBER
};

struct VolumeLivemountExtend {
    std::string dstPath;
    std::vector<VolumeLivemountFileSystemShareInfo> fileSystemShareInfo;
    std::string fibreChannel;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dstPath, dstPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fileSystemShareInfo, fileSystemShareInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fibreChannel, fibreChannel)
    END_SERIAL_MEMEBER
};

struct VolumeIndexDetail {
    std::string volumeName;
    std::string dstPath;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeName, volume_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dstPath, dst_path)
    END_SERIAL_MEMEBER
};

struct VolumeIndexFileSystemShareInfoAdvanceParam {
    int domainType {0};
    std::string shareName;
    std::vector<VolumeIndexDetail> indexDetail;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(domainType, domainType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(indexDetail, index_detail)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(shareName, shareName)
    END_SERIAL_MEMEBER
};

struct VolumeIndexFileSystemShareInfo {
    int accessPermission {0};
    int type {0};
    VolumeIndexFileSystemShareInfoAdvanceParam advanceParams;
    std::string fileSystemName {0};
    
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(accessPermission, accessPermission)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(advanceParams, advanceParams)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fileSystemName, fileSystemName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(type, type)
    END_SERIAL_MEMEBER
};

struct VolumeIndexExtend {
    std::string dstPath;
    std::string fibreChannel;
    std::vector<VolumeIndexFileSystemShareInfo> fileSystemShareInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dstPath, dstPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fileSystemShareInfo, fileSystemShareInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fibreChannel, fibreChannel)
    END_SERIAL_MEMEBER
};

struct VolumeMountEntry {
    std::string mountTypeVersion;
    std::string mountType;
    std::string mountOptions;
    std::string mountTargetPath;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountTypeVersion, mountTypeVersion)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountType, mountType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountOptions, mountOptions)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountTargetPath, mountTargetPath)
    END_SERIAL_MEMEBER
};

struct VolumeMountEntries {
    std::string volumePath;
    std::vector<VolumeMountEntry> mountEntries;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumePath, volumePath);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountEntries, mountEntries);
    END_SERIAL_MEMEBER
};

struct VolumeMountRecordJsonCommon {
    std::string mountTargetPath;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountTargetPath, mountTargetPath);
    END_SERIAL_MEMEBER
};

struct VolumeInfomation {
    std::string volumePath {};
    uint64_t    size {};
    uint32_t volumeType {};
    std::string mountOption {};
    std::string mountPoint {};
    std::string mountType {};
    std::string volumeName {};
    std::string uuid {};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumePath, volumePath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountOption, mountOption)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountPoint, mountPoint)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountType, mountType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeName, volumeName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeType, volumeType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(uuid, uuid)
    END_SERIAL_MEMEBER
};

struct DiskInfomation {
    std::string diskName;
    uint64_t    diskSize;
    std::string diskId;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diskName, diskName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diskSize, diskSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diskId, diskId)
    END_SERIAL_MEMEBER
};

struct VolumeInfoSet {
    std::vector<VolumeInfomation> volumeInfoSet;
    std::vector<DiskInfomation> diskInfoSet;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeInfoSet, volumeInfoSet)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diskInfoSet, diskInfoSet)
    END_SERIAL_MEMEBER
};

struct VolumeBackupCopy {
    std::string systemBackupFlag = "false";
    uint32_t blockSize {0};
    uint64_t sessionSize {0};
    std::vector<std::string> protectedVolumePaths;
    VolumeInfoSet volumeInfoSet;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(systemBackupFlag, system_backup_flag)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(blockSize, blockSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(sessionSize, sessionSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(protectedVolumePaths, protectedVolumePaths)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeInfoSet, volumeInfoSet)
    END_SERIAL_MEMEBER
};

struct VolumeNativeGeneral {
    time_t jobStartTime {};
    uint64_t backupPhaseStartTime {};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(jobStartTime, jobStartTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(backupPhaseStartTime, backupPhaseStartTime)
    END_SERIAL_MEMEBER
};

struct VolumeBackupSubJob {
    std::string volumePath;
    std::string volumeName;
    int type {};
    uint32_t subTaskType {};  // CopyVolume
    std::string ext;          // any parameters required in future

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumePath, volumePath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeName, volumeName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(type, type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(subTaskType, subTaskType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(ext, ext)
    END_SERIAL_MEMEBER
};

struct VolumeFileGraunlarRestoreInfo {
    std::vector<std::string> paths;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(paths, paths)
    END_SERIAL_MEMEBER
};

struct VolumeScanStatistics {
    time_t   scanStartTime = 0;           /* Scan start time */
    time_t   scanEndTime = 0;             /* Scan end time */
    uint64_t scanDuration = 0;            /* Total scan duration (in seconds) */
    uint64_t totalVolume = 0;             /* Total num of dir detected in NAS Share */
    uint64_t totalSize = 0;               /* Total size of the NAS Share */
    uint64_t totalVolumeToBackup = 0;     /* Total num of dir (new/modified) to backup */
    uint64_t totalSizeToBackup = 0;       /* Total size to backup */
    bool     scanStarted = 0;             /* Status-Whether sanner has been started */

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanStartTime, scanStartTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanEndTime, scanEndTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanDuration, scanDuration)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(totalVolume, totalVolume)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(totalSize, totalSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(totalVolumeToBackup, totalVolumeToBackup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(totalSizeToBackup, totalSizeToBackup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanStarted, scanStarted)
    END_SERIAL_MEMEBER
};

struct VolumeReportStatistic {
    uint64_t bytesWritten {0};
    uint64_t bytesToWrite {0};
    int   volumeCount {0};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(bytesWritten, bytesWritten)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(bytesToWrite, bytesToWrite)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeCount, volumeCount)
    END_SERIAL_MEMEBER
};

enum class VolumeJobType {
    BACKUP,
    RESTORE
};

struct VolumeBackupStatistic {
    uint64_t noOfVolumeToBackup  = 0;        /* No of files to be backed up */
    uint64_t noOfBytesToBackup  = 0;        /* No of bytes (in KB) to be backed up */
    uint64_t noOfVolumeCopied    = 0;        /* No of files copied */
    uint64_t noOfBytesCopied    = 0;        /* No of bytes (in KB) copied */
    uint64_t noOfVolumeFailed      = 0;        /* No of directories failed to be copied/deleted */
    uint64_t backupspeed        = 0;        /* Backup speed (in KBps) */
    time_t   startTime          = 0;        /* Start time of backup phase */
    time_t   lastLogReportTime  = 0;        /* Last time (epoch seconds) when we report log to PM */

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfVolumeToBackup, noOfVolumeToBackup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfBytesToBackup, noOfBytesToBackup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfVolumeCopied, noOfVolumeCopied)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfBytesCopied, noOfBytesCopied)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfVolumeFailed, noOfVolumeFailed)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(backupspeed, backupspeed)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(startTime, startTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(lastLogReportTime, lastLogReportTime)
    END_SERIAL_MEMEBER
};

struct VolumeDataLayoutExtend {
    std::string m_autoIndex {};
    std::string m_metadataBackupType {};
    std::string m_proxyHostMode {};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_autoIndex, auto_index)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_metadataBackupType, permissions_and_attributes)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_proxyHostMode, proxy_host_mode)
    END_SERIAL_MEMEBER
};

// ===== Structs Used For Granular Restore
struct RestoreAdvancedParamters {
    std::string failedScript;
    std::string postScript;
    std::string preScript;
    std::string restoreOption;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(failedScript, failed_script)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(postScript, post_script)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(preScript, pre_script)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(restoreOption, restoreOption)
    END_SERIAL_MEMEBER
};

struct VolumeFileGranularRestoreInfo {
    uint32_t subTaskType;
    std::string volumeName;
    std::string ctrlFilePath;
    std::string metaDirPath;
    std::string dstRootPath;
    std::string srcRootPath;
    std::string trimDstRootPathPrefix;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeName, volumeName);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(subTaskType, subTaskType);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(ctrlFilePath, ctrlFilePath);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(metaDirPath, metaDirPath);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dstRootPath, dstRootPath);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(srcRootPath, srcRootPath);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(trimDstRootPathPrefix, trimDstRootPathPrefix);
    END_SERIAL_MEMEBER
};

}
#endif // VOLUME_COMMONSTRUCT_H