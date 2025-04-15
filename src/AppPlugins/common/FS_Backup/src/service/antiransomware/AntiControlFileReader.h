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
#ifndef ANTI_CONTROL_FILE_READER_H
#define ANTI_CONTROL_FILE_READER_H

#include <securec.h>
#include <queue>
#include <memory>
#include <thread>
#include <string>
#include "FSBackupUtils.h"
#include "Backup.h"
#include "BackupStructs.h"
#include "BackupQueue.h"
#include "BlockBufferMap.h"
#include "CopyCtrlParser.h"
#include "MetaParser.h"
#include "XMetaParser.h"
#include "BackupConstants.h"
#include "ScannerBackupCtrl.h"
#include "ScannerBackupMeta.h"

class AntiControlFileReader {
public:
    AntiControlFileReader(BackupParams& backupParams,
        std::shared_ptr<BackupQueue<FileHandle>> readQueuePtr,
        std::shared_ptr<BackupQueue<FileHandle>> writeQueuePtr,
        std::shared_ptr<BackupControlInfo> controlInfo,
        std::shared_ptr<BlockBufferMap> blockBufferMap);
    ~AntiControlFileReader();

    virtual BackupRetCode Start();
    virtual BackupRetCode Abort();
    virtual BackupRetCode Enqueue(std::string contrlFile);
    virtual BackupPhaseStatus GetStatus();
    std::shared_ptr<NfsAntiRansomwareAdvanceParams> m_advParams;


private:
    bool IsAbort();
    bool IsComplete();
    void HandleComplete();
    void ThreadFunc();
    int ReadControlFileEntry(Module::CopyCtrlFileEntry& fileEntry, Module::CopyCtrlDirEntry& dirEntry);
    int ReadControlFileEntryAndProcess(Module::CopyCtrlFileEntry& fileEntry,
        Module::CopyCtrlDirEntry& dirEntry, ParentInfo &parentInfo);
    int ProcessFileEntry(ParentInfo& parentInfo, const Module::CopyCtrlFileEntry& fileEntry,
        FileHandle& fileHandle);
    int PushFileHandleToReader(FileHandle& fileHandle);
    std::string GetOpenedMetaFileName();
    std::string GetMetaFile(std::string metaFileName);
    int ReadFileMeta(Module::FileMeta& fileMeta, uint64_t offset);
    int OpenMetaControlFile(const std::string& metaFile);
    void FillFileMetaData(FileHandle& fileHandle, const Module::FileMeta& fileMeta);
    int OpenControlFile(const std::string& controlFile);
    int FillStatsFromControlHeader();

private:
    BackupParams m_backupParams;

    std::thread m_thread;
    std::shared_ptr<BackupQueue<FileHandle>> m_readQueue;
    std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue;

    std::queue<std::string> m_controlFileQueue;
    std::unique_ptr<Module::CopyCtrlParser> m_copyCtrlParser = nullptr;
    std::unique_ptr<Module::MetaParser> m_metaParser = nullptr;

    std::shared_ptr<BackupControlInfo> m_controlInfo;
    std::shared_ptr<BlockBufferMap> m_blockBufferMap;
    bool m_abort { false };
    time_t m_isCompleteTimer { 0 };

};

#endif // ANTI_CONTROL_FILE_READER_H