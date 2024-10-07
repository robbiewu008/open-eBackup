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
#ifndef ARCHIVE_SERVICE_TASK_H
#define ARCHIVE_SERVICE_TASK_H

#include <memory>
#ifndef WIN32
#include <unistd.h>
#include <fcntl.h>
#endif
#include "threadpool/ThreadPool.h"

#include "ArchiveClientBase.h"
#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"

enum class ArchiveEvent {
    OPEN_SRC = 0,
    READ_DATA,
    READ_META,
    CLOSE_SRC,
    LINK_EVENT,
    DELETE_EVENT,
    INVALID_EVENT
};

class ArchiveServiceParams {
public:
    std::string srcRootPath;
    std::string dstRootPath;
    std::string linkTarget;
    BackupType backupType { BackupType::UNKNOWN_TYPE };
    BackupDataFormat backupDataFormat { BackupDataFormat::UNKNOWN_FORMAT };
    RestoreReplacePolicy restoreReplacePolicy { RestoreReplacePolicy::NONE };
};

class ArchiveServiceTask : public Module::ExecutableItem {
public:
    ArchiveServiceTask(ArchiveEvent event, std::shared_ptr<BlockBufferMap> bufferMapPtr,
        const FileHandle& fileHandle, const ArchiveServiceParams& params, std::shared_ptr<ArchiveClientBase> client)
        : m_event(event), m_bufferMapPtr(bufferMapPtr), m_fileHandle(fileHandle),
        m_params(params), m_archiveClient(client) {}
    virtual ~ArchiveServiceTask() {}
    void Exec() override;
public:
    ArchiveEvent m_event { ArchiveEvent::INVALID_EVENT };
    std::shared_ptr<BlockBufferMap> m_bufferMapPtr { nullptr };
    FileHandle m_fileHandle;
    ArchiveServiceParams m_params;
    std::shared_ptr<ArchiveClientBase> m_archiveClient { nullptr };

private:
    void HandleOpenFile();
    void HandleReadData();
    void HandleCloseSrc();
    int ProcessReadSoftLinkData();
    int ProcessReadSpecialFileData() const;

    std::string CutPrefixSlash(const std::string& path) const;
private:
    uint64_t m_offset { 0 };
    uint64_t m_length { 0 };
    uint32_t m_seq { 0 };
};

#endif // ARCHIVE_SERVICE_TASK_H