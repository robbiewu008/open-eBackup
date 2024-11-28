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
#ifndef APPPLUGIN_NAS_PLUGINCONSTANTS_H
#define APPPLUGIN_NAS_PLUGINCONSTANTS_H

namespace {
#ifdef WIN32
    const std::string  dir_sep = "\\";
    const std::string METAFILE_ZIP_NAME = "metafile.zip";
#else
    const std::string  dir_sep = "/";
    const std::string METAFILE_TAR_NAME = "metafile.tar";
    const std::string METAFILE_ZIP_NAME = "metafile.tar.gz";
#endif
    const std::string SMB_PROTOCOL = "0";
    const std::string NFS_PROTOCOL = "1";
    const std::string SMB_VERSION_3_1_1 = "3.1.1";
    const std::string SMB_VERSION_3_02 = "3.02";
    const std::string SMB_VERSION_3_0 = "3.0";
    const std::string SMB_VERSION_2_1 = "2.1";
    const std::string SMB_VERSION_2_0 = "2.0";
    const std::string SMB_ENCRYPTION = "1";
    const std::string KRB5CCNAMEPREFIX = "/DataBackup/ProtectClient/ProtectClient-E/tmp/tkt_";
    const std::string KRB5CCNAMEENVPREFIX = "FILE:";
    const std::string KRB5KEYTABPREFIX = "/DataBackup/ProtectClient/ProtectClient-E/tmp/krb5_";
    const std::string KRB5KEYTABPOSTFIX = ".keytab";
    const std::string KRB5CONFIGPREFIX = "/DataBackup/ProtectClient/ProtectClient-E/tmp/krb5_";
    const std::string KRB5CONFIGPOSTFIX = ".conf";
    const std::string KRB5AUTHMODE = "5";

    // used for resource access
    const std::string RESOURCE_NATIVE_FILE_TYPE  = "native";
    const std::string RESOURCE_VOLUME_TYPE       = "volume";

    // default value for hcpconf.ini
    const std::string EXCLUDE_FILESYSTEM_LIST_DEFAULT = "";
    const std::string WIN32_EXCLUDE_PATH_LIST_DEFAULT = "";
    const std::string LINUX_EXCLUDE_PATH_LIST_DEFAULT = "/dev,/proc,/run,/sys";
    const std::string AIX_EXCLUDE_PATH_LIST_DEFAULT = "/dev,/proc,/run,/sys";
    const std::string SOLARIS_EXCLUDE_PATH_LIST_DEFAULT = "/dev,/proc,/run,/sys";
    const std::string WIN32_SNAPSHOT_PARENT_PATH_DEFAULT = R"(C:\vss_snapshots)";
    const std::string LINUX_SNAPSHOT_PARENT_PATH_DEFAULT = "/opt/lvm_snapshots";
    const std::string AIX_SNAPSHOT_PARENT_PATH_DEFAULT = "/opt/jfs_snapshots";
    const std::string SOLARIS_SNAPSHOT_PARENT_PATH_DEFAULT = "/opt/lvm_snapshots";

    constexpr uint32_t RETRYTIMES3 = 3;

    constexpr int PROGRESS0 = 0;
    constexpr int PROGRESS50 = 50;
    constexpr int PROGRESS100 = 100;

    constexpr uint64_t INITIAL_ERROR_CODE = 0;

    constexpr uint32_t BACKUP_REPORT_CIRCLE_TIME_IN_SEC = 120;
    constexpr uint32_t BACKUP_PRINT_CIRCLE_TIME_IN_SEC = 10;

    constexpr int EXECUTE_SUBTASK_MONITOR_DUR_IN_SEC = 10;
    constexpr int GENERATE_SUBTASK_MONITOR_DUR_IN_SEC = 10;
    constexpr int SUBTASK_WAIT_FOR_SCANNER_READY_IN_SEC = 1;

    constexpr uint32_t SUBJOB_TYPE_SETUP_PHASE = 1;
    constexpr uint32_t SUBJOB_TYPE_DATACOPY_COPY_PHASE = 2;
    constexpr uint32_t SUBJOB_TYPE_DATACOPY_DELETE_PHASE = 3;
    constexpr uint32_t SUBJOB_TYPE_DATACOPY_HARDLINK_PHASE = 4;
    constexpr uint32_t SUBJOB_TYPE_DATACOPY_DIRMTIME_PHASE = 5;
    constexpr uint32_t SUBJOB_TYPE_TEARDOWN_PHASE = 6;
    constexpr uint32_t SUBJOB_TYPE_COPYMETA_PHASE = 7;
    constexpr uint32_t SUBJOB_TYPE_CHECK_SUBJOB_PHASE = 9;

    constexpr uint32_t SUBJOB_TYPE_SETUP_PHASE_PRIO = 10;
    constexpr uint32_t SUBJOB_TYPE_DATACOPY_COPY_PHASE_PRIO = 20;
    constexpr uint32_t SUBJOB_TYPE_DATACOPY_DELETE_PHASE_PRIO = 30;
    constexpr uint32_t SUBJOB_TYPE_DATACOPY_HARDLINK_PHASE_PRIO = 40;
    constexpr uint32_t SUBJOB_TYPE_DATACOPY_DIRMTIME_PHASE_PRIO = 50;
    constexpr uint32_t SUBJOB_TYPE_COPYMETA_PHASE_PRIO = 60;
    constexpr uint32_t SUBJOB_TYPE_TEARDOWN_PHASE_PRIO = 70;
    constexpr uint32_t SUBJOB_TYPE_AGGREGATE_BASE_PRIO = 200;
    // UBC优先级最大值为65535
    constexpr uint32_t SUBJOB_TYPE_CHECK_SUBJOB_PHASE_PRIO = 60000;

    // volume backup task type
    constexpr uint32_t SUBJOB_TYPE_DATACOPY_VOLUME = 2;
    constexpr uint32_t SUBJOB_TYPE_TEARDOWN_VOLUME = 3;

    // volume backup type priority
    constexpr uint32_t SUBJOB_TYPE_DATACOPY_VOLUME_PRIO = 20;
    constexpr uint32_t SUBJOB_TYPE_TEARDOWN_VOLUME_PRIO = 100;

    // volume granular restore
    constexpr uint32_t SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_COPY = 3000;
    constexpr uint32_t SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_HARDLINK = 3001;
    constexpr uint32_t SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_MTIME = 3002;
    constexpr uint32_t SUBJOB_TYPE_VOLUME_GRANULAR_TEARDOWN = 3004;
    const std::string SUBJOB_NAME_VOLUME_GRANULAR_RESTORE_COPY = "VolumeGranular_Restore_Copy";
    const std::string SUBJOB_NAME_VOLUME_GRANULAR_RESTORE_HARDLINK = "VolumeGranular_Restore_Hardlink";
    const std::string SUBJOB_NAME_VOLUME_GRANULAR_RESTORE_MTIME = "VolumeGranular_Restore_Mtime";
    const std::string SUBJOB_NAME_VOLUME_GRANULAR_TEARDOWN = "VolumeGranular_Teardown";
    constexpr uint32_t SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_COPY_PRIO = 20;
    constexpr uint32_t SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_HARDLINK_PRIO = 40;
    constexpr uint32_t SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_MTIME_PRIO = 50;
    constexpr uint32_t SUBJOB_TYPE_VOLUME_GRANULAR_TEARDOWN_PRIO = 100;

    const std::string SUBJOB_TYPE_VOLUME_JOBNAME = "VolumeBackup_CopyVolume";
    const std::string SUBJOB_TYPE_VOLUME_TEARDOWN = "VolumeBackup_TearDown";

    const std::string JOB_CTRL_PHASE_ALLOWLOCALNODE = "AllowBackupInLocalNode";
    const std::string JOB_CTRL_PHASE_CHECKBACKUPJOBTYPE = "CheckBackupJobType";
    const std::string JOB_CTRL_PHASE_PREJOB = "PrerequisiteJob";
    const std::string JOB_CTRL_PHASE_GENSUBJOB = "GenerateSubJob";
    const std::string JOB_CTRL_PHASE_EXECSUBJOB = "ExecuteSubJob";
    const std::string JOB_CTRL_PHASE_POSTJOB = "PostJob";

    const std::string SUBJOB_TYPE_TEARDOWN_JOBNAME = "NasBackup_Teardown";
    const std::string SUBJOB_TYPE_CHECK_SUBJOB_JOBNAME = "Fileset_CheckSubJob";
    const std::string SUBJOB_TYPE_COPYMETA_JOBNAME = "NasBackup_CopyMeta";
    const std::string SUBJOB_TYPE_ARCHIVE_REPORT_START_LABEL = "Fileset_Archive_ReportStartLabel";

    constexpr auto BACKUP_COPY_METAFILE = "backup-copy-meta.json";
    constexpr auto SCANNER_STAT = "/scanner_status.json";
    constexpr auto BACKUP_JOBCNT_METAFILE = "_backup-jobcnt-meta.json";
    constexpr auto VOLUME_COPY_METAFILE = "volumecopy.meta.json";
    constexpr auto BACKUP_JOBSTAT_DIR = "/backup-job/backup/stats/";
    constexpr auto RESTORE_JOBSTAT_DIR = "/backup/stats/";

    const std::string FILTER_TYPE_FILE = "File";
    const std::string FILTER_TYPE_DIR  = "Dir";

    const std::string FILTER_MODEL_INCLUDE = "INCLUDE";
    const std::string FILTER_MODEL_EXCLUDE = "EXCLUDE";
    const std::string METAFILE_PARENT_DIR = "filemeta";
    const std::string TMP_DIR = "/tmp/";
    constexpr int ONE_GB = 1024 * 1024 * 1024;
    constexpr int HALF_GB = ONE_GB / 2;
    constexpr int SCAN_CTRL_FILE_TIMES_SEC = 5;
    constexpr int SCAN_CTRL_MAX_ENTRIES_FULL_BACKUP = 100000;
    constexpr int SCAN_CTRL_MAX_ENTRIES_INCBKUP = 10000;
    constexpr uint32_t SCAN_CTRL_MIN_ENTRIES_FULL_BKUP = 100000;
    constexpr uint32_t SCAN_CTRL_MIN_ENTRIES_INC_BKUP = 10000;
    constexpr uint32_t SCAN_CTRL_MAX_QUEUE_SIZE = 10000;
    constexpr uint32_t PROGRESS_COMPLETE = 100;
    constexpr uint32_t MAX_OPEN_DIR_REQ_COUNT = 4000;
    constexpr uint32_t SLEEP_TEN_SECONDS = 10;
    const std::string META = "meta";
    const std::string SCAN = "scan";
    const std::string CTRL = "ctrl";
    const std::string DIRCACHE_FILE_NAME = "dircache";
    const std::string BACKUP = "backup";
    const std::string RESTORE = "restore";
    const std::string STATISTICS = "statistics";
    const std::string HARDLINK_CTRL_PREFIX = "hardlink";
    const std::string MTIME_CTRL_PREFIX = "mtime";
    const std::string DELETE_CTRL_PREFIX = "delete";
    const std::string CONTROL_CTRL_PREFIX = "control";
    const std::string SOURCE_PRIMARY_VOLUME = "sourcePrimaryVolume";
    const std::string SOURCE_PRIMARY_NFS_VOLUME = "sourcePrimaryNFSVolume";
    const std::string SNAPSHOT_PRIMARY_VOLUME = "snapshotPrimaryVolume";
    const std::string SUB_VOLUM_PREFIX = "SubVolume_";
    const std::string VOLUM_PREFIX = "Volume_";
    constexpr uint32_t SEND_ADDNEWJOB_RETRY_TIMES = 30;
    constexpr uint32_t SEND_ADDNEWJOB_RETRY_INTERVAL = 10; // 10s delay
    const std::string PLUGIN_ATT_JSON = "plugin_attribute_1.0.0.json";
    const std::string ZFS_SNAPSHOT_DIR = "/.zfs/snapshot";
}
#endif // APPPLUGIN_NAS_PLUGINCONSTANTS_H