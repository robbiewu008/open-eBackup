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
#ifndef DIR_CONTROL_FILE_READER_H
#define DIR_CONTROL_FILE_READER_H

#include <memory>
#include <queue>
#include <memory>
#include <thread>
#include <string>
#include "Backup.h"
#include "BackupStructs.h"
#include "BackupQueue.h"
#include "BlockBufferMap.h"
#include "MtimeCtrlParser.h"
#include "BackupMtimeCtrl.h"
#include "CommonServiceParams.h"

class DirControlFileReader {
public:
    explicit DirControlFileReader(const ReaderParams& readerParams);
    DirControlFileReader(BackupParams& backupParams,
        std::shared_ptr<BackupQueue<FileHandle>> readQueuePtr,
        std::shared_ptr<BackupControlInfo> controlInfo,
        std::shared_ptr<BlockBufferMap> blockBufferMap);
    ~DirControlFileReader();

    virtual BackupRetCode Start();
    virtual BackupRetCode Abort();
    virtual BackupRetCode Enqueue(std::string contrlFile);
    virtual BackupPhaseStatus GetStatus();

private:
    void ThreadFunc();
    int OpenControlFile(const std::string& controlFile);
    int ReadControlFileEntry(Module::MtimeCtrlEntry& dirEntry);
    int ProcessDirEntry(const Module::MtimeCtrlEntry& dirEntry, const FileHandle& fileHandle);
    int FillStatsFromControlHeader();
    bool IsAbort();
    bool IsComplete();
    int ReadControlFileEntryAndProcess(Module::MtimeCtrlEntry& dirEntry, FileHandle& fileHandle);
    int ReadControlFileEntryAndProcessV10(BackupMtimeCtrlEntry& dirEntry, FileHandle& fileHandle);
    int OpenControlFileV10(const std::string& controlFile);
    int OpenControlFileV20(const std::string& controlFile);
    int FillStatsFromControlHeaderV10();
    int ProcessDirEntryV10(const BackupMtimeCtrlEntry& dirEntry, FileHandle& fileHandle);

private:
    BackupParams m_backupParams;

    std::thread m_thread;
    std::shared_ptr<BackupQueue<FileHandle>> m_readQueue;

    std::queue<std::string> m_controlFileQueue;
    std::unique_ptr<Module::MtimeCtrlParser> m_mtimeCtrlParser = nullptr;
    // v1.0
    std::unique_ptr<BackupMtimeCtrl> m_scannerMtimeCtrl = nullptr;

    std::shared_ptr<BackupControlInfo> m_controlInfo;
    std::shared_ptr<BlockBufferMap> m_blockBufferMap;

    std::atomic<uint64_t> m_dirCount { 0 };
    time_t m_isCompleteTimer { 0 };

private:
    bool m_abort { false };
    std::string m_metaFileVersion = META_VERSION_V20;
};

#endif // DIR_CONTROL_FILE_READER_H