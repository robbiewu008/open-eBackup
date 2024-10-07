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
#ifndef HOST_SERVICE_TASK_H
#define HOST_SERVICE_TASK_H

#include <memory>
#include "ThreadPool.h"
#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"

enum class HostEvent {
    OPEN_SRC,
    OPEN_DST,
    READ_DATA,
    READ_META,
    WRITE_DATA,
    WRITE_META,
    CLOSE_SRC,
    CLOSE_DST,
    LINK,
    DELETE_ITEM,
    CREATE_DIR,
    INVALID
};

struct HostParams {
    bool            zeroUmask { false }; /* used for posix only */
    bool            writeMeta { true };
    bool            writeAcl { false };
    bool            writeExtendAttribute { false };
    bool            writeSparseFile { false };
    bool            discardReadError { false };
    uint32_t        blockSize { 0 };
    uint32_t        maxBlockNum { 0 };
    std::string     srcRootPath;
    std::string     dstRootPath;
    std::string     linkTarget;
    std::string     srcTrimPrefix;
    std::string     dstTrimPrefix;

    BackupType           backupType { BackupType::UNKNOWN_TYPE };
    BackupDataFormat     backupDataFormat { BackupDataFormat::UNKNOWN_FORMAT };
    RestoreReplacePolicy restoreReplacePolicy { RestoreReplacePolicy::NONE };
};

class HostServiceTask : public Module::ExecutableItem {
public:
    HostServiceTask() {}

    HostServiceTask(
        HostEvent event,
        std::shared_ptr<BlockBufferMap> bufferMapPtr,
        FileHandle& fileHandle,
        const HostParams& params);

    virtual ~HostServiceTask();
    virtual void Exec(); /* template method to execute handleXXX */
    virtual bool IsCriticalError() const;

public:
    HostEvent m_event { HostEvent::INVALID };
    std::shared_ptr<BlockBufferMap> m_bufferMapPtr { nullptr };
    FileHandle m_fileHandle;
    HostParams m_params;
    BackupPhaseStatus m_backupFailReason {BackupPhaseStatus::FAILED};
    std::pair<std::string, uint64_t> m_errDetails;

protected:
    virtual void SetCriticalErrorInfo(uint64_t err) = 0;

    /* implement in HostService and Win32ServiceTask to be invoked by template method Exec() */
    virtual void HandleOpenSrc() = 0;
    virtual void HandleOpenDst() = 0;
    virtual void HandleReadData() = 0;
    virtual void HandleReadMeta() = 0;
    virtual void HandleWriteData() = 0;
    virtual void HandleWriteMeta() = 0;
    virtual void HandleLink() = 0;
    virtual void HandleCloseSrc() = 0;
    virtual void HandleCloseDst() = 0;
    virtual void HandleDelete() = 0;
    virtual void HandleCreateDir() = 0;
};

#endif // HOST_SERVICE_TASK_H