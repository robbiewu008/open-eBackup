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
#ifndef BACKUP_ENUM_DEFINES_H
#define BACKUP_ENUM_DEFINES_H

enum class FileDescState {
    INIT = 0,               /* 0 init */
    LSTAT,                  /* 1 lstat req sent for the file to obtain the attributes */
    SRC_OPENED,             /* 2 file is opened (at source) */
    DST_OPENED,             /* 3 file is opened (at destination) */
    PARTIAL_READED,         /* 4 file data read from src is in-progress, partially read */
    READED,                 /* 5 file data read from src is completed, all blocks are read */
    AGGREGATED,             /* 6 used in aggregate restore */
    META_READED,            /* 7 File meta read from src is completed */
    PARTIAL_WRITED,         /* 8 file data write to src is in-progress, partially written */
    WRITED,                 /* 9 file data write to src is completed, all blocks are written */
    META_WRITED,            /* 10 File metadata write to src is completed */
    SRC_CLOSED,             /* 11 file is closed (at source) */
    DST_CLOSED,             /* 12 file is closed (at destination) */
    READ_OPEN_FAILED,       /* 13 file open at source is failed */
    WRITE_OPEN_FAILED,      /* 14 file open at destination is failed */
    LINK,                   /* 15 */
    DIR_DEL,                /* 16 directory with same name as file, to be deleted */
    DIR_DEL_RESTORE,        /* directory with same name as file, to be deleted. Comes in link delete flow of restore */
    LINK_DEL,               /* 18 symlink/hardlink/failed file to be deleted */
    LINK_DEL_FAILED,        /* 19 normal file to be deleted after copy failure */
    LINK_DEL_FOR_RESTORE,   /* 20 non zero files in restore to be deleted */
    READ_FAILED,            /* 21 file data read at source is failed */
    WRITE_FAILED,           /* 22 file data write at destination is failed */
    WRITE_SKIP,             /* 23 */
    META_WRITE_FAILED,      /* 24 file metadata write at destination is failed */
    REPLACE_DIR,            /* 25 need to remove the dir and send the fileHandle to copy again */
    FILEHANDLE_INVALID,     /* 26 file handle became invalid, need to reopen */
    END                     /* 27 end */
};

enum class LINK_TYPE {
    SYM = 0,        // src -> protected share
    HARD = 1,       // dst -> secondary share
    REGULAR = 2,    // Create flow
    DEVICE_TYPE = 3 // Mknod flow
};

enum class FileType { // 与 Module 中的enum class MetaType对应
    NFS = 1,
    CIFS,
    UNIX,
    WINDOWS,
    OBJECT
};

enum class BackupRetCode {
    DESTROY_IN_PROGRESS               = -4,
    ABORT_IN_PROGRESS                 = -3,
    DESTROY_FAILED_BACKUP_IN_PROGRESS = -2,
    FAILED                            = -1,
    SUCCESS                           =  0,
    SKIP                              =  1,
};

enum class BackupPhaseStatus {
    INPROGRESS                      = 1,
    ABORT_INPROGRESS                = 2,
    ABORTED                         = 3,
    FAILED                          = 4,
    COMPLETED                       = 5,
    FAILED_NOACCESS                 = 6,
    FAILED_NOSPACE                  = 7,
    FAILED_SEC_SERVER_NOTREACHABLE  = 8,
    FAILED_PROT_SERVER_NOTREACHABLE = 9,
    FAILED_NOMEMORY                 = 10,
};

enum class BackupPhase {
    COPY_STAGE     = 1,
    DELETE_STAGE   = 2,
    HARDLINK_STAGE = 3,
    DIR_STAGE      = 4,
    ANTI_STAGE     = 5,
    UNKNOWN_STAGE  = 6,
};

enum class BackupType {
    BACKUP_FULL        = 1,
    BACKUP_INC         = 2,
    RESTORE            = 3,
    FILE_LEVEL_RESTORE = 4,
    UNKNOWN_TYPE       = 5
};

enum class BackupPlatform {
    UNIX             = 1,
    WINDOWS          = 2,
    OBJECT           = 3,
    UNKNOWN_PLATFORM = 4
};

enum class BackupIOEngine {
    LIBNFS = 1,
    LIBSMB = 2,
    POSIX = 3,
    WIN32_IO = 4,
    POSIXAIO = 5,
    WINDOWSAIO = 6,
    LIBAIO = 7,
    LIBS3IO = 8,
    ARCHIVE_CLIENT = 9,
    NFS_ANTI_ANSOMWARE = 10,
    OBJECTSTORAGE = 11,
    UNKNOWN_ENGINE = 12,
};

enum class BackupDataFormat {
    NATIVE         = 1,
    AGGREGATE      = 2,
    UNKNOWN_FORMAT = 3
};

enum class RestoreReplacePolicy {
    IGNORE_EXIST    = 1,
    OVERWRITE       = 2,
    OVERWRITE_OLDER = 3,
    RENAME          = 4,
    NONE            = 5
};

enum class BackupModuleStatus {
    BACKUP_DEFAULT                                  = 0,
    BACKUP_INIT                                     = 1,
    BACKUP_SCAN_COMPLETED                           = 2,
    BACKUP_STOPPED                                  = 3,
    BACKUP_MOUNT_FAILED                             = 4,
    BACKUP_FAILED                                   = 5,
    BACKUP_COPY_PHASE_INPROGRESS                    = 6,
    BACKUP_COPY_PHASE_READ_CONTEXT_FILLED           = 7,
    BACKUP_COPY_PHASE_WRITE_CONTEXT_FILLED          = 8,
    BACKUP_COPY_PHASE_CONTROLFILE_READ_COMPLETE     = 9,
    BACKUP_COPY_PHASE_READ_COMPLETE                 = 10,
    BACKUP_COPY_PHASE_WRITE_COMPLETE                = 11,
    BACKUP_COPY_PHASE_COMPLETE                      = 12,
    BACKUP_DELETE_PHASE_INPROGRESS                  = 13,
    BACKUP_DELETE_PHASE_CONTEXT_FILLED              = 14,
    BACKUP_DELETE_PHASE_CONTROLFILE_READ_COMPLETE   = 15,
    BACKUP_DELETE_PHASE_COMPLETE                    = 16,
    BACKUP_HARDLINK_PHASE_INPROGRESS                = 17,
    BACKUP_HARDLINK_PHASE_READ_CONTEXT_FILLED       = 18,
    BACKUP_HARDLINK_PHASE_WRITE_CONTEXT_FILLED      = 19,
    BACKUP_HARDLINKF_PHASE_READ_COMPLETE            = 20,
    BACKUP_HARDLINKF_PHASE_WRITE_COMPLETE           = 21,
    BACKUP_HARDLINKF_PHASE_COMPLETE                 = 22,
    BACKUP_DIRMTIME_PHASE_INPROGRESS                = 23,
    BACKUP_DIRMTIME_PHASE_CONTEXT_FILLED            = 24,
    BACKUP_DIRMTIME_PHASE_CONTROLFILE_READ_COMPLETE = 25,
    BACKUP_DIRMTIME_PHASE_COMPLETE                  = 26,
    BACKUP_SUCCESS                                  = 27,
};

enum class SpecialFileType {
    REG = 0,        // regular file
    SLINK,          // soft link
    BLK,            // block device
    CHR,            // char device
    FIFO            // pipe
};

enum class BackupAntiType {
    WORM = 0,
    ENTRPY = 1
};

enum class OrderOfRestore {
    OFF = 0,
    ON_LEXICOGRAPHICAL_ORDER = 1,
    ON_REVERSE_LEXICOGRAPHICAL_ORDER = 2
};

enum class AdsProcessType {
    NONE = 0,                           // ignore ads
    ADS_PROCESS_TYPE_FROM_SCANNER = 1,  // read ads stream data from ads metafile generated by scanner
    ADS_PROCESS_TYPE_FROM_BACKUP = 2    // read ads stream data by FS_Backup self
};

#endif