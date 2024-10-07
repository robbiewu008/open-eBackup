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
#ifndef COPY_CONTROL_FILE_READER_H
#define COPY_CONTROL_FILE_READER_H

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
#include "CommonServiceParams.h"

class CopyControlFileReader {
public:
    explicit CopyControlFileReader(const ReaderParams& readerParams);

    CopyControlFileReader(BackupParams& backupParams,
        std::shared_ptr<BackupQueue<FileHandle>> readQueuePtr,
        std::shared_ptr<BackupQueue<FileHandle>> aggregateQueuePtr,
        std::shared_ptr<BackupControlInfo> controlInfo,
        std::shared_ptr<BlockBufferMap> blockBufferMap);
    ~CopyControlFileReader();

    virtual BackupRetCode Start();
    virtual BackupRetCode Abort();
    virtual BackupRetCode Enqueue(std::string contrlFile);
    virtual BackupPhaseStatus GetStatus();

private:
    void ThreadFunc();
    int ProcessControlFileEntry(Module::CopyCtrlDirEntry& dirEntry, Module::CopyCtrlFileEntry& fileEntry,
        ParentInfo &parentInfo);
    int OpenControlFile(const std::string& controlFile);
    int ReadControlFileEntryAndProcess(Module::CopyCtrlFileEntry& fileEntry, Module::CopyCtrlDirEntry& dirEntry,
        ParentInfo& parentInfo);
    int OpenMetaControlFile(const std::string& metaFile);
    int OpenXMetaControlFile(const std::string& metaFile);
    int ReadDirectoryMeta(Module::DirMeta& dirMeta, uint64_t offset);
    int ReadFileMeta(Module::FileMeta& fileMeta, uint64_t offset);
    int ReadDirectoryXMeta(FileHandle& fileHandle, const Module::DirMeta& dirMeta);
    int ReadFileXMeta(FileHandle& fileHandle, const Module::FileMeta& fileMeta);
    std::string GetMetaFile(std::string metaFileName);
    std::string GetXMetaFile(uint64_t xMetaFileIndex);
    std::string GetOpenedMetaFileName();
    std::string GetOpenedXMetaFileName();
    int ProcessDirEntry(
        ParentInfo& parentInfo, const Module::CopyCtrlDirEntry& dirEntry, FileHandle& fileHandle);
    int ProcessFileEntry(
        ParentInfo& parentInfo, const Module::CopyCtrlFileEntry& fileEntry, FileHandle& fileHandle);
    void FillDirMetaData(FileHandle& fileHandle, const Module::DirMeta& dirMeta);
    void FillFileMetaData(FileHandle& fileHandle, const Module::FileMeta& fileMeta, const CopyCtrlFileEntry& fileEntry);
    int FillStatsFromControlHeader();
    int FillStatsFromControlHeaderV10();
    bool IsAbort();
    bool IsComplete();
    int ReadControlFileEntryAndProcessV10(ScannerBackupCtrlFileEntry& fileEntry,
        ScannerBackupCtrlDirEntry& dirEntry, ParentInfo& parentInfo);
    int PushFileHandleToReader(const int& ret, FileHandle& fileHandle);
    int PushDirToAggregator(const int& ret, FileHandle& fileHandle);
    void PushFileHandleToAggregator(FileHandle& fileHandle);
    int OpenControlFileV10(const std::string& controlFile);
    int OpenControlFileV20(const std::string& controlFile);
    int ProcessControlFileEntryV10(ScannerBackupCtrlDirEntry& dirEntry,
        ScannerBackupCtrlFileEntry& fileEntry, ParentInfo& parentInfo);
    int OpenMetaControlFileV10(const std::string& metaFile);
    int ProcessDirEntryV10(ParentInfo &parentInfo, const ScannerBackupCtrlDirEntry& dirEntry,
        FileHandle& fileHandle);
    int ProcessFileEntryV10(ParentInfo& parentInfo, const ScannerBackupCtrlFileEntry& fileEntry,
        FileHandle& fileHandle);
    std::string GetOpenedMetaFileNameV10();
    void FillDirMetaDataV10(FileHandle& fileHandle, const NasScanner::DirectoryMeta& dirMeta);
    int FillFileMetaDataV10(FileHandle& fileHandle, const NasScanner::FileMeta& fileMeta);
    bool IsWindowsStyleEngine(const BackupIOEngine& engine) const;
    std::string ParseAcl(const std::vector<Module::XMetaField> &xmetalist);

private:
    BackupParams m_backupParams;

    std::thread m_thread;
    std::shared_ptr<BackupQueue<FileHandle>> m_readQueue;
    std::shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue;

    std::queue<std::string> m_controlFileQueue;
    std::unique_ptr<Module::CopyCtrlParser> m_copyCtrlParser = nullptr;
    std::unique_ptr<Module::MetaParser> m_metaParser = nullptr;
    std::unique_ptr<Module::XMetaParser> m_xMetaParser = nullptr;

    // v1.0
    std::unique_ptr<ScannerBackupCtrl> m_scannerBackupCtrl = nullptr;
    std::unique_ptr<NasScanner::ScannerBackupMeta> m_scannerBackupMeta = nullptr;

    std::shared_ptr<BackupControlInfo> m_controlInfo;
    std::shared_ptr<BlockBufferMap> m_blockBufferMap;

    bool m_abort { false };
    time_t m_isCompleteTimer { 0 };
    std::string m_metaFileVersion = META_VERSION_V20;

    uint64_t m_noOfFileEntriesReaded = 0;
    uint64_t m_noOfDirEntriedReaded = 0;
};

#endif // COPY_CONTROL_FILE_READER_H