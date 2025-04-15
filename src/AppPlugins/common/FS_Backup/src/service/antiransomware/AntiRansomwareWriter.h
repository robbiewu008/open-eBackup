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
#ifndef ANTI_WRITER_H
#define ANTI_WRITER_H

#include <memory>
#include <thread>
#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"
#include "BackupQueue.h"
#include "BackupTimer.h"

struct AntiWriterParams {
    BackupParams backupParams {};
    std::shared_ptr<BackupQueue<FileHandle>> writeQueuePtr { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>> readQueuePtr { nullptr };
    std::shared_ptr<BackupControlInfo> controlInfo { nullptr };
    std::shared_ptr<BlockBufferMap> blockBufferMap { nullptr };
};

class AntiRansomwareWriter {
public:
    explicit AntiRansomwareWriter(const AntiWriterParams &antiWriterParams)
    {
        m_backupParams = antiWriterParams.backupParams;
        m_writeQueue = antiWriterParams.writeQueuePtr;
        m_readQueue = antiWriterParams.readQueuePtr;
        m_controlInfo = antiWriterParams.controlInfo;
        m_blockBufferMap = antiWriterParams.blockBufferMap;
    }
    virtual ~AntiRansomwareWriter()
    {
        m_writeQueue.reset();
        m_readQueue.reset();
        m_blockBufferMap.reset();
        m_controlInfo.reset();
        INFOLOG("Destruct AntiRansomwareWriter");
    }

    virtual BackupRetCode Start() = 0;
    virtual BackupRetCode Abort() = 0;
    
    virtual BackupPhaseStatus GetStatus()
    {
        return FSBackupUtils::GetWriterStatus(m_controlInfo, m_abort, m_controlInfo->m_backupFailReason);
    }

    std::vector<FileHandle> GetFailedList()
    {
        return m_failedList;
    }

protected:
    std::vector<FileHandle> m_failedList;

private:
    virtual void ThreadFunc() = 0;

    virtual int OpenFile(FileHandle& fileHandle) = 0;
    virtual int WriteMeta(FileHandle& fileHandle) = 0;
    virtual int WriteData(FileHandle& fileHandle) = 0;
    virtual int CloseFile(FileHandle& fileHandle) = 0;

public:
    BackupParams m_backupParams;
    BackupTimer m_timer;

    std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue;
    std::shared_ptr<BackupQueue<FileHandle>> m_readQueue;
    std::shared_ptr<BlockBufferMap> m_blockBufferMap;
    std::shared_ptr<BackupControlInfo> m_controlInfo;

public:
    bool m_abort { false };
};

#endif // ANTI_WRITER_H