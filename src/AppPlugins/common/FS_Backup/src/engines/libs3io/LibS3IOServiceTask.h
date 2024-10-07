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
#ifndef LIBS3IO_SERVICE_TASK_H
#define LIBS3IO_SERVICE_TASK_H

#include <memory>
#include "ThreadPool.h"
#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"
#include "BackupS3IO.h"

enum class LibS3IOEvent {
    READ_STRING,
    READ_FILE,
    READ_DATA,
    READ_DATA_BY_OFFSET,
    WRITE_FILE,
    WRITE_DATA,
    DELETE,
    DELETE_ALL,
    INVALID
};

class LibS3IOParams {
public:
    std::string s3PathPrefix;
    std::string s3WorkPath;
    std::string localPath;
    Module::CallBackHandle cbHandle;
    BackupType           backupType { BackupType::UNKNOWN_TYPE };
    BackupDataFormat     backupDataFormat { BackupDataFormat::UNKNOWN_FORMAT };
    RestoreReplacePolicy restoreReplacePolicy { RestoreReplacePolicy::NONE };
};

class LibS3IOServiceTask : public Module::ExecutableItem {
public:
    LibS3IOServiceTask(LibS3IOEvent event, std::shared_ptr<BlockBufferMap> bufferMapPtr,
        FileHandle& fileHandle, const LibS3IOParams& params, std::shared_ptr<Module::BackupS3IO> io)
        : m_event(event), m_bufferMapPtr(bufferMapPtr), m_fileHandle(fileHandle), m_params(params), m_io(io)
    {}
    virtual ~LibS3IOServiceTask() {};
    void Exec() override;

private:
    void HandleReadString();
    void HandleReadFile();
    void HandleReadData();
    void HandleReadDataByOffset();
    void HandleWriteFile();
    void HandleWriteData();
    void HandleDelete();
    void HandleDeleteAll();

private:
    uint64_t m_offset { 0 };
    uint64_t m_length { 0 };
    uint32_t m_seq { 0 };

public:
    LibS3IOEvent m_event { LibS3IOEvent::INVALID };
    std::shared_ptr<BlockBufferMap> m_bufferMapPtr { nullptr };
    FileHandle m_fileHandle;
    LibS3IOParams m_params;
    std::shared_ptr<Module::BackupS3IO> m_io;
};

#endif // LIBS3IO_SERVICE_TASK_H