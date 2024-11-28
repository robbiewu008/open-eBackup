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
#ifndef LIBNFS_STRUCTS_H
#define LIBNFS_STRUCTS_H

#include <string>
#include <shared_mutex>
#include "nfsc/libnfs.h"
#include "NfsContextContainer.h"
#include "BackupStructs.h"
#include "BackupQueue.h"
#include "BackupTimer.h"
#include "PacketStats.h"

#define IS_FILE_COPY_FAILED(fileHandle) \
((fileHandle).m_file->GetSrcState() == FileDescState::READ_FAILED || \
    (fileHandle).m_file->GetDstState() == FileDescState::WRITE_FAILED || \
    (fileHandle).m_file->GetDstState() == FileDescState::LINK_DEL_FAILED)

#define IS_TO_BE_OPENED(fileHandle) \
(((fileHandle).m_block.m_size == 0 && (fileHandle).m_block.m_seq == 0) || \
    S_ISLNK((fileHandle).m_file->m_mode))

#define IS_INCREMENT_FAIL_COUNT(fileHandle) \
(!IS_FILE_COPY_FAILED(fileHandle) && !((fileHandle).m_file->IsFlagSet(AGGREGATE_GEN_FILE)))

#define IS_INCREMENT_READ_FAIL_COUNT(fileHandle) \
(((fileHandle).m_file->GetSrcState() != FileDescState::READ_FAILED) && !((fileHandle).m_file->IsFlagSet(AGGREGATE_GEN_FILE)))

#define IS_EMPTY_SYMLINK_BLOCK(fileHandle) \
(((fileHandle).m_block.m_size == 0 && (fileHandle).m_block.m_seq == 0) && S_ISLNK((fileHandle).m_file->m_mode))

#define IS_SPECIAL_DEVICE_FILE(fileHandle) \
(S_ISCHR((fileHandle).m_file->m_mode) || S_ISBLK((fileHandle).m_file->m_mode) || S_ISFIFO((fileHandle).m_file->m_mode))

#define IS_NFSSHARE_ACCESS_ERROR(status) ((status) == -EACCES || (status) == -EROFS)

#define IS_ABORTED_OR_FAILED(commonData) \
(*((commonData)->abort) || (commonData)->controlInfo->m_failed || (commonData)->controlInfo->m_controlReaderFailed)

enum class LibnfsEvent {
    OPEN = 0,
    READ = 1,
    READLINK = 2,
    SRC_CLOSE = 3,
    LSTAT = 4,
    CREATE = 5,
    WRITE = 6,
    WRITE_META = 7,
    MKNOD = 8,
    SYMLINK = 9,
    LINK_UTIME = 10,
    HARDLINK = 11,
    MKDIR = 12,
    DIR_DELETE = 13,
    LINK_DELETE = 14,
    LINK_DELETE_FOR_RESTORE = 15,
    DELETE = 16,
    DST_CLOSE = 17,
    INVALID = 18
};

struct NfsCommonData {
    Module::NfsContextContainer *nfsContextContainer { nullptr };
    Module::NfsContextContainer *syncNfsContextContainer { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>> readQueue { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>> writeQueue { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>> writeWaitQueue { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>> aggregateQueue { nullptr };
    std::shared_ptr<HardLinkMap> hardlinkMap { nullptr };
    BackupTimer *timer { nullptr };
    std::shared_ptr<PacketStats> pktStats { nullptr };
    std::shared_ptr<BackupControlInfo> controlInfo { nullptr };
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder { nullptr };
    void *commonObj { nullptr };
    std::function<bool(void*)> IsResumeSendCb;
    std::function<void(void*)> ResumeSendCb;
    bool *abort { nullptr };
    bool skipFailure { true };
    bool writeDisable { false };
    bool writeMeta {true};
};

#endif // LIBNFS_STRUCTS_H