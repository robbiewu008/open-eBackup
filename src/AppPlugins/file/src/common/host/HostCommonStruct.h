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
#ifndef HOST_COMMONSTRUCT_H
#define HOST_COMMONSTRUCT_H

#include <cstdint>
#include <string>
#include <vector>
#include <ctime>
#include "Module/src/common/JsonHelper.h"
#include "client/ClientInvoke.h"
namespace FilePlugin {
struct HostApplicationExtent {
    std::string m_filters;
    std::string m_isOSBackup;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_filters, filters)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isOSBackup, is_OS_backup)
    END_SERIAL_MEMEBER
};

struct AggCopyExtendInfo {
    std::string isAggregation;
    std::string metaPathSuffix;
    std::string dataPathSuffix;
    std::string maxSizeAfterAggregate;
    std::string maxSizeToAggregate;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isAggregation, isAggregation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(metaPathSuffix, metaPathSuffix)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dataPathSuffix, dataPathSuffix)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(maxSizeAfterAggregate, maxSizeAfterAggregate)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(maxSizeToAggregate, maxSizeToAggregate)
    END_SERIAL_MEMEBER
};

struct HostBackupJobExtend {
    /* PM传的string，需要修改 */
    std::string m_isConsistent;
    std::string m_isEnableAcl;
    std::string m_isCrossFileSystem;
    std::string m_isBackupNfs;
    std::string m_isBackupSMB;
    std::string m_isSparseFileDetection;
    std::string m_isAdsFileDetection;
    std::string m_isContinueOnFailed;
    std::string m_isAggregate;
    std::string m_maxSizeAfterAggregate;
    std::string m_maxSizeToAggregate;
    std::string m_channels;
    std::string m_snapshotSizePercent;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isConsistent, consistent_backup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isEnableAcl, enable_acl)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isCrossFileSystem, cross_file_system)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isBackupNfs, backup_nfs)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isBackupSMB, backup_smb)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isSparseFileDetection, sparse_file_detection)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isAdsFileDetection, ads_file_detection)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isContinueOnFailed, backup_continue_with_files_backup_failed)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isAggregate, small_file_aggregation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_maxSizeAfterAggregate, aggregation_file_size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_maxSizeToAggregate, aggregation_file_max_size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_channels, channels)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotSizePercent, snapshot_size_percent)
    END_SERIAL_MEMEBER
};

struct ProtectedFileset {
    std::set<std::string> m_protectedPaths;
    HostApplicationExtent m_filesetExt;     // 过滤参数
    HostBackupJobExtend m_advParms;   // 高级参数
};

struct HostBackupCopy {
    std::string m_metadataBackupType {};
    std::string m_backupFormat {};
    std::string m_backupFilter {};
    std::string m_isConsistent {};
    uint64_t m_lastBackupTime {};
    std::string m_isArchiveSupportHardlink {};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_metadataBackupType, MetadataBackupType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_backupFormat, backupFormat)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_backupFilter, backupFilter)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isConsistent, isConsistent)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_lastBackupTime, lastBackupTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isArchiveSupportHardlink, isArchiveSupportHardlink)
    END_SERIAL_MEMEBER
};

struct DataLayOutExtend {
    std::string m_autoIndex {};
    std::string m_metadataBackupType {};
    std::string m_proxyHostMode {};
    std::string m_backupFormat {};
    std::string m_fileReplaceStrategy {};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_autoIndex, auto_index)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_metadataBackupType, permissions_and_attributes)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_proxyHostMode, proxy_host_mode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_backupFormat, small_file_aggregation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_fileReplaceStrategy, fileReplaceStrategy)
    END_SERIAL_MEMEBER
};

struct NativeGeneral {
    time_t m_jobStartTime {};
    std::string m_protocolVersion {};
    std::string m_remoteFilesetSnapshotPath;
    std::string m_remoteFilesetSnapshotName;
    uint64_t m_remoteFilesetSnapshotTime {};
    uint64_t m_backupCopyPhaseStartTime {};
    uint64_t m_backupDelPhaseStartTime {};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_jobStartTime, JobStartTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_protocolVersion, ProtocolVersion)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_remoteFilesetSnapshotPath, ProtectShareSnapShotPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_remoteFilesetSnapshotName, ProtectShareSnapShotName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_remoteFilesetSnapshotTime, ProtectShareSnapShotTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_backupCopyPhaseStartTime, FirstBackupCopySubTaskStartTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_backupDelPhaseStartTime, FirstBackupCopyDelTaskStartTime)
    END_SERIAL_MEMEBER
};

struct BackupStatistic {
    uint64_t noOfDirToBackup    = 0;        /* No of directories to be backed up */
    uint64_t noOfFilesToBackup  = 0;        /* No of files to be backed up */
    uint64_t noOfBytesToBackup  = 0;        /* No of bytes (in KB) to be backed up */
    uint64_t noOfDirToDelete    = 0;        /* No of directories to be deleted */
    uint64_t noOfFilesToDelete  = 0;        /* No of files to be deleted */
    uint64_t noOfDirCopied      = 0;        /* No of directories copied */
    uint64_t noOfFilesCopied    = 0;        /* No of files copied */
    uint64_t noOfBytesCopied    = 0;        /* No of bytes (in KB) copied */
    uint64_t skipFileCnt        = 0;        /* No of files skipped */
    uint64_t skipDirCnt         = 0;        /* No of dir skipped */
    uint64_t noOfDirDeleted     = 0;        /* No of directories deleted */
    uint64_t noOfFilesDeleted   = 0;        /* No of files deleted */
    uint64_t noOfDirFailed      = 0;        /* No of directories failed to be copied/deleted */
    uint64_t noOfFilesFailed    = 0;        /* No of files failed to be copied/deleted */
    uint64_t backupspeed        = 0;        /* Backup speed (in KBps) */
    time_t   startTime          = 0;        /* Start time of backup phase */
    uint64_t noOfSrcRetryCount  = 0;        /* No of src side retry count */
    uint64_t noOfDstRetryCount  = 0;        /* No of dst side retry count */
    time_t   lastLogReportTime  = 0;        /* Last time (epoch seconds) when we report log to PM */
    uint64_t noOfFilesWriteSkip = 0;        /* No of files skipped to write (Ignore replace policy) */
    uint64_t noOfFailureRecordsWritten = 0; /* No of backup failure records that have been written to file */

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfDirToBackup, noOfDirToBackup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfFilesWriteSkip, noOfFilesWriteSkip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfFilesToBackup, noOfFilesToBackup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfBytesToBackup, noOfBytesToBackup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfDirToDelete, noOfDirToDelete)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfFilesToDelete, noOfFilesToDelete)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfDirCopied, noOfDirCopied)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfFilesCopied, noOfFilesCopied)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfBytesCopied, noOfBytesCopied)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(skipFileCnt, skipFileCnt)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(skipDirCnt, skipDirCnt)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfDirDeleted, noOfDirDeleted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfFilesDeleted, noOfFilesDeleted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfDirFailed, noOfDirFailed)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfFilesFailed, noOfFilesFailed)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(backupspeed, backupspeed)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(startTime, startTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfSrcRetryCount, noOfSrcRetryCount)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfDstRetryCount, noOfDstRetryCount)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(lastLogReportTime, lastLogReportTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(noOfFailureRecordsWritten, noOfFailureRecordsWritten)
    END_SERIAL_MEMEBER

    BackupStatistic operator + (const BackupStatistic& stats)
    {
        BackupStatistic sumBackupStatistic {};
        sumBackupStatistic.noOfDirToBackup   = stats.noOfDirToBackup    + noOfDirToBackup;
        sumBackupStatistic.noOfFilesToBackup = stats.noOfFilesToBackup  + noOfFilesToBackup;
        sumBackupStatistic.noOfBytesToBackup = stats.noOfBytesToBackup  + noOfBytesToBackup;
        sumBackupStatistic.noOfDirToDelete   = stats.noOfDirToDelete    + noOfDirToDelete;
        sumBackupStatistic.noOfFilesToDelete = stats.noOfFilesToDelete  + noOfFilesToDelete;
        sumBackupStatistic.noOfDirCopied     = stats.noOfDirCopied      + noOfDirCopied;
        sumBackupStatistic.noOfFilesCopied   = stats.noOfFilesCopied    + noOfFilesCopied;
        sumBackupStatistic.noOfBytesCopied   = stats.noOfBytesCopied    + noOfBytesCopied;
        sumBackupStatistic.skipFileCnt       = stats.skipFileCnt        + skipFileCnt;
        sumBackupStatistic.noOfFilesWriteSkip       = stats.noOfFilesWriteSkip        + noOfFilesWriteSkip;
        sumBackupStatistic.skipDirCnt        = stats.skipDirCnt         + skipDirCnt;
        sumBackupStatistic.noOfDirDeleted    = stats.noOfDirDeleted     + noOfDirDeleted;
        sumBackupStatistic.noOfFilesDeleted  = stats.noOfFilesDeleted   + noOfFilesDeleted;
        sumBackupStatistic.noOfDirFailed     = stats.noOfDirFailed      + noOfDirFailed;
        sumBackupStatistic.noOfFilesFailed   = stats.noOfFilesFailed    + noOfFilesFailed;
        sumBackupStatistic.noOfSrcRetryCount = stats.noOfSrcRetryCount  + noOfSrcRetryCount;
        sumBackupStatistic.noOfDstRetryCount = stats.noOfDstRetryCount  + noOfDstRetryCount;
        sumBackupStatistic.noOfFailureRecordsWritten = stats.noOfFailureRecordsWritten + noOfFailureRecordsWritten;
        return sumBackupStatistic;
    }
};

struct HostScanStatistics {
    time_t m_scanStartTime = 0;             /* Scan start time */
    time_t m_remoteNasScanEndTime = 0;      /* Scan End time */
    time_t m_scanEndTime = 0;               /* Scan end time along with metadata write */
    uint64_t m_scanDuration = 0;            /* Total scan duration (in seconds) */
    uint64_t m_totDirs = 0;                 /* Total num of dir detected in NAS Share */
    uint64_t m_totFiles = 0;                /* Total num of files detected in NAS Share */
    uint64_t m_totalSize = 0;               /* Total size of the NAS Share */
    uint64_t m_totDirsToBackup = 0;         /* Total num of dir (new/modified) to backup */
    uint64_t m_totFilesToBackup = 0;        /* Total num of files(new/modified) to backup */
    uint64_t m_totFilesDeleted = 0;         /* Total num of files to be deleted */
    uint64_t m_totDirsDeleted = 0;          /* Total num of dirs to be deleted */
    uint64_t m_totalSizeToBackup = 0;       /* Total size to backup */
    uint64_t m_totalControlFiles = 0;       /* Total Control Files Generated */
    uint64_t m_totFailedDirs = 0;           /* Total num of Failed dir detected in NAS Share */
    uint64_t m_totFailedFiles = 0;          /* Total num of Failed files detected in NAS Share */
    uint64_t mEntriesMayFailedToArchive = 0;/* Total num of file/directories with long path/name that may failed to archive */
    bool m_scanStarted = 0;                 /* Status-Whether sanner has been started */

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_scanStartTime, scanStartTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_remoteNasScanEndTime, remoteNasScanEndTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_scanEndTime, scanEndTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_scanDuration, scanDuration)

    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_totDirs, totDirs)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_totFiles, totFiles)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_totalSize, totalSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_totDirsToBackup, totDirsToBackup)

    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_totFilesToBackup, totFilesToBackup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_totFilesDeleted, totFilesDeleted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_totDirsDeleted, totDirsDeleted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_totalSizeToBackup, totalSizeToBackup)

    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_totalControlFiles, m_totalControlFiles)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_totFailedDirs, m_totFailedDirs)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_totFailedFiles, m_totFailedFiles)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mEntriesMayFailedToArchive, mEntriesMayFailedToArchive)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_scanStarted, scanStarted)
    END_SERIAL_MEMEBER
};

enum class SCANNER_TASK_STATUS {
    INIT        = 0,
    INPROGRESS  = 1,
    SUCCESS     = 2,
    ABORTED     = 3,
    FAILED      = 4
};

struct BackupSubJob {
    std::string controlFile;         // the control file while we receive in the call back of scanner/backupcopy phase
    uint32_t subTaskType {};         // CopyPhase, DelPhase, HardLinkPhase, DirMTimePhase
    std::string prefix {};           // backup prefix
    std::string fsId {};             // archive fs id in S3

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(controlFile, controlFile)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(subTaskType, subTaskType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(prefix, prefix)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fsId, fsId)
    END_SERIAL_MEMEBER
};

struct SubJobInfoSet {
    std::set<std::string> m_subJobInfoSet;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_subJobInfoSet, m_subJobInfoSet)
    END_SERIAL_MEMEBER
};

struct BackupSubJobInfo {
    std::string scanCtrlDir;
    std::string backupCtrlDir;
    std::string prefix;
    std::string incControlDir;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(scanCtrlDir, scanCtrlDir)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(backupCtrlDir, backupCtrlDir)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(prefix, prefix)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(incControlDir, incControlDir)
    END_SERIAL_MEMEBER
};

struct HostSnapResidualInfo {
    std::string jobId;
    std::vector<std::string> snapshotInfos;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(jobId, jobId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(snapshotInfos, snapshotInfos)
    END_SERIAL_MEMEBER
};

struct HostSnapResidualInfoList {
    std::vector<HostSnapResidualInfo> infos;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(infos, infos)
    END_SERIAL_MEMEBER
};
}
#endif