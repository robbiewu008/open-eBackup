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
#ifndef READER_BASE_H
#define READER_BASE_H

#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"
#include "BackupQueue.h"
#include "BackupTimer.h"
#include "CommonServiceParams.h"

class ReaderBase {
public:
    virtual BackupRetCode Start() = 0;
    virtual BackupRetCode Abort() = 0;
    virtual BackupRetCode Destroy() = 0;
    virtual BackupPhaseStatus GetStatus()
    {
        return FSBackupUtils::GetReaderStatus(m_controlInfo, m_abort, m_controlInfo->m_backupFailReason);
    }

    explicit ReaderBase(const ReaderParams &readerParams)
    {
        m_backupParams = readerParams.backupParams;
        m_readQueue = readerParams.readQueuePtr;
        m_aggregateQueue = readerParams.aggregateQueuePtr;
        m_blockBufferMap = readerParams.blockBufferMap;
        m_controlInfo = readerParams.controlInfo;
        m_hardlinkMap = readerParams.hardlinkMap;
    }
    
    virtual ~ReaderBase()
    {
        m_aggregateQueue.reset();
        m_readQueue.reset();
        m_blockBufferMap.reset();
        m_controlInfo.reset();
        m_hardlinkMap.reset();
        INFOLOG("Destruct ReaderBase");
    }

    virtual int OpenFile(FileHandle& fileHandle) = 0;
    virtual int ReadData(FileHandle& fileHandle) = 0;
    virtual int ReadMeta(FileHandle& fileHandle) = 0;
    virtual int CloseFile(FileHandle& fileHandle) = 0;
    
    virtual void ThreadFunc() = 0;

    std::vector<FileHandle> GetFailedList()
    {
        return m_failedList;
    }

public:
    bool            m_abort { false };
    BackupParams    m_backupParams;
    BackupTimer     m_timer;
    std::shared_ptr<BackupQueue<FileHandle>>    m_readQueue         { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>>    m_aggregateQueue    { nullptr };
    std::shared_ptr<BlockBufferMap>             m_blockBufferMap    { nullptr };
    std::shared_ptr<BackupControlInfo>          m_controlInfo       { nullptr };
    std::shared_ptr<HardLinkMap>                m_hardlinkMap       { nullptr }; // used only for hardlink Reader

    /* record files/directory failed to backup */
    std::shared_ptr<Module::BackupFailureRecorder> m_failureRecorder = nullptr;
    /* key: backup failed files or dirs; value: errno */
    std::vector<FileHandle> m_failedList;
};

#endif