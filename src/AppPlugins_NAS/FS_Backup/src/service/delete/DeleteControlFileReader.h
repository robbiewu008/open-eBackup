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
#ifndef DELETE_CONTROL_FILE_READER_H
#define DELETE_CONTROL_FILE_READER_H

#include <queue>
#include <memory>
#include <thread>
#include <string>
#include "FSBackupUtils.h"
#include "Backup.h"
#include "BackupStructs.h"
#include "BackupQueue.h"
#include "BlockBufferMap.h"
#include "DeleteCtrlParser.h"
#include "BackupDeleteCtrl.h"
#include "CommonServiceParams.h"

class DeleteControlFileReader {
public:
    explicit DeleteControlFileReader(const ReaderParams& readerParams);
    DeleteControlFileReader(BackupParams& backupParams,
        std::shared_ptr<BackupQueue<FileHandle>> readQueuePtr,
        std::shared_ptr<BackupControlInfo> controlInfo,
        std::shared_ptr<BlockBufferMap> blockBufferMap);
    ~DeleteControlFileReader();

    virtual BackupRetCode Start();
    virtual BackupRetCode Abort();
    virtual BackupRetCode Enqueue(std::string contrlFile);
    virtual BackupPhaseStatus GetStatus();

private:
    void ThreadFunc();
    int OpenControlFile(const std::string& controlFile);
    int ReadControlFileEntry(Module::DeleteCtrlEntry& dirEntry, std::string& fileName);
    int FillStatsFromControlHeader();
    bool IsAbort();
    bool IsComplete();
    int ReadControlFileEntryAndProcess(Module::DeleteCtrlEntry& deleteEntry, std::string& fileName,
        FileHandle& fileHandle, ParentInfo& parentInfo);
    int ReadControlFileEntryAndProcessV10(BackupDeleteCtrlEntry& deleteEntry, std::string& fileName,
        FileHandle& fileHandle, ParentInfo& parentInfo);
    int OpenControlFileV10(const std::string& controlFile);
    int OpenControlFileV20(const std::string& controlFile);
    int FillStatsFromControlHeaderV10();
    void HandleComplete();

private:
    BackupParams m_backupParams;

    std::thread m_thread;
    std::shared_ptr<BackupQueue<FileHandle>> m_readQueue;

    std::queue<std::string> m_controlFileQueue;
    std::unique_ptr<Module::DeleteCtrlParser> m_deleteCtrlParser { nullptr };
    std::shared_ptr<BackupControlInfo> m_controlInfo;
    std::shared_ptr<BlockBufferMap> m_blockBufferMap;

    // v1.0
    std::unique_ptr<BackupDeleteCtrl> m_scannerDeleteCtrl = nullptr;

    std::atomic<uint64_t> m_dirCount { 0 };
    std::atomic<uint64_t> m_fileCount { 0 };

    bool m_abort { false };
    time_t m_isCompleteTimer { 0 };
    std::string m_metaFileVersion = META_VERSION_V20;
};

#endif // DELETE_CONTROL_FILE_READER_H