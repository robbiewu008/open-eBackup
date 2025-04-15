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
#ifndef ANTI_READER_H
#define ANTI_READER_H

#include <memory>
#include <thread>
#include <unordered_map>
#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"
#include "BackupQueue.h"
#include "BackupTimer.h"

struct AntiReaderParams {
    BackupParams backupParams {};
    std::shared_ptr<BackupQueue<FileHandle>> readQueuePtr { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>> writeQueuePtr { nullptr };
    std::shared_ptr<BackupControlInfo> controlInfo { nullptr };
    std::shared_ptr<BlockBufferMap> blockBufferMap { nullptr };
};

class AntiRansomwareReader {
public:
    explicit AntiRansomwareReader(const AntiReaderParams &antiReaderParams)
    {
        m_backupParams = antiReaderParams.backupParams;
        m_readQueue = antiReaderParams.readQueuePtr;
        m_writeQueue = antiReaderParams.writeQueuePtr;
        m_blockBufferMap = antiReaderParams.blockBufferMap;
        m_controlInfo = antiReaderParams.controlInfo;
    }

    virtual ~AntiRansomwareReader()
    {
        m_readQueue.reset();
        m_writeQueue.reset();
        m_blockBufferMap.reset();
        m_controlInfo.reset();
        INFOLOG("Destruct AntiRansomwareReader");
    }

    virtual BackupRetCode Start() = 0;
    virtual BackupRetCode Abort() = 0;
    virtual BackupPhaseStatus GetStatus() = 0;
    std::vector<FileHandle> GetFailedList()
    {
        return m_failedList;
    }

protected:
    std::vector<FileHandle> m_failedList; /* key: backup failed files or dirs; value: errno */

private:
    virtual void ThreadFunc() = 0;

    virtual int OpenFile(FileHandle& fileHandle) = 0;
    virtual int ReadData(FileHandle& fileHandle) = 0;
    virtual int ReadMeta(FileHandle& fileHandle) = 0;
    virtual int CloseFile(FileHandle& fileHandle) = 0;

public:
    BackupParams m_backupParams;
    BackupTimer m_timer;

    std::shared_ptr<BackupQueue<FileHandle>> m_readQueue;
    std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue;
    std::shared_ptr<BlockBufferMap> m_blockBufferMap;
    std::shared_ptr<BackupControlInfo> m_controlInfo;

public:
    bool m_abort { false };
};

#endif // ANTI_READER_H