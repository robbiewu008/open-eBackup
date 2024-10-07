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
#ifndef HARDLINK_CONTROL_FILE_READER_H
#define HARDLINK_CONTROL_FILE_READER_H

#include <memory>
#include <queue>
#include <memory>
#include <thread>
#include <string>
#include <securec.h>
#include "Backup.h"
#include "BackupStructs.h"
#include "BackupQueue.h"
#include "BlockBufferMap.h"
#include "HardlinkCtrlParser.h"
#include "MetaParser.h"
#include "XMetaParser.h"
#include "FSBackupUtils.h"
#include "ScannerHardLinkCtrl.h"
#include "ScannerBackupMeta.h"
#include "CommonServiceParams.h"

class HardlinkControlFileReader {
public:
    explicit HardlinkControlFileReader(const ReaderParams& readerParams);
    HardlinkControlFileReader(BackupParams& backupParams,
        std::shared_ptr<BackupQueue<FileHandle>> readQueuePtr,
        std::shared_ptr<BackupControlInfo> controlInfo,
        std::shared_ptr<BlockBufferMap> blockBufferMap,
        std::shared_ptr<HardLinkMap> hardlinkMap);
    ~HardlinkControlFileReader();

    virtual BackupRetCode Start();
    virtual BackupRetCode Abort();
    virtual BackupRetCode Enqueue(std::string contrlFile);
    virtual BackupPhaseStatus GetStatus();

private:
    void ThreadFunc();
    int OpenControlFile(const std::string& controlFile);
    int ReadControlFileEntry(Module::HardlinkCtrlEntry& linkEntry, Module::HardlinkCtrlInodeEntry& inodeEntry);
    int ProcessFileEntry(
        ParentInfo& parentInfo, const Module::HardlinkCtrlEntry& linkEntry, FileHandle& fileHandle);
    void ProcessHardlink(ParentInfo& parentInfo, FileHandle& fileHandle);

    int OpenMetaControlFile(std::string metaFile);
    int OpenXMetaControlFile(std::string metaFile);
    int ReadFileMeta(Module::FileMeta& fileMeta, uint64_t offset);
    int ReadFileXMeta(FileHandle& fileHandle, const Module::FileMeta& fileMeta);
    void FillFileMetaData(FileHandle& fileHandle, const Module::FileMeta& fileMeta);
    std::string GetMetaFile(std::string metaFileName);
    std::string GetXMetaFile(uint64_t xMetaFileIndex);
    std::string GetOpenedMetaFileName() const;
    std::string GetOpenedXMetaFileName() const;
    int FillStatsFromControlHeader() const;
    bool IsAbort() const;
    bool IsComplete();
    int OpenControlFileV10(const std::string& controlFile);
    int OpenControlFileV20(const std::string& controlFile);
    int FillStatsFromControlHeaderV10() const;
    int OpenMetaControlFileV10(const std::string& metaFile);
    std::string GetOpenedMetaFileNameV10() const;
    int ProcessFileEntryV10(ParentInfo& parentInfo, ScannerHardLinkCtrl::LinkEntry& linkEntry,
        FileHandle& fileHandle);
    int FillFileMetaDataV10(FileHandle& fileHandle, const NasScanner::FileMeta& fileMeta);
    int ReadControlFileEntryAndProcessV10(ScannerHardLinkCtrl::LinkEntry& linkEntry,
        ScannerHardLinkCtrl::InodeEntry& inodeEntry, ParentInfo& parentInfo, FileHandle& fileHandle);
    int ReadControlFileEntryAndProcess(Module::HardlinkCtrlEntry& linkEntry,
        Module::HardlinkCtrlInodeEntry& inodeEntry, ParentInfo& parentInfo, FileHandle& fileHandle);

private:
    BackupParams m_backupParams;

    std::thread m_thread;
    std::shared_ptr<BackupQueue<FileHandle>> m_readQueue;

    std::queue<std::string> m_controlFileQueue;
    std::unique_ptr<Module::HardlinkCtrlParser> m_hardlinkCtrlParser { nullptr };
    std::unique_ptr<Module::MetaParser> m_metaParser { nullptr };
    std::unique_ptr<Module::XMetaParser> m_xMetaParser = nullptr;

    // v1.0
    std::unique_ptr<ScannerHardLinkCtrl::CtrlFile> m_scannerHardLinkCtrl = nullptr;
    std::unique_ptr<NasScanner::ScannerBackupMeta> m_scannerBackupMeta = nullptr;

    std::shared_ptr<BackupControlInfo> m_controlInfo { nullptr };
    std::shared_ptr<BlockBufferMap> m_blockBufferMap { nullptr };
    std::shared_ptr<HardLinkMap> m_hardlinkMap { nullptr };

    uint64_t m_dirCount { 0 };

private:
    bool m_abort { false };
    uint64_t m_noOfFilesToBackup { 0 };
    uint64_t m_noOfBytesToBackup { 0 };
    time_t m_isCompleteTimer { 0 };
    std::string m_metaFileVersion = META_VERSION_V20;
};

#endif // HARDLINK_CONTROL_FILE_READER_H