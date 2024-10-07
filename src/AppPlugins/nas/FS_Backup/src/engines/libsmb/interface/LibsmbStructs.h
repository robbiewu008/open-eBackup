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
#ifndef LIBSMB_STRUCTS_H
#define LIBSMB_STRUCTS_H

#include <memory>
#include "Libsmb.h"
#include "LibsmbCommon.h"
#include "BlockBufferMap.h"
#include "Backup.h"
#include "BackupQueue.h"
#include "BackupStructs.h"
#include "libsmb_ctx/SmbContextWrapper.h"
#include "BackupTimer.h"
#include "PacketStats.h"

struct SmbReaderCommonData {
    FileHandle fileHandle;
    LibsmbEvent event;
    LibsmbParams params;
    BackupTimer *timer { nullptr };

    std::shared_ptr<Module::SmbContextWrapper> readSmbContext;
    std::shared_ptr<BackupQueue<FileHandle>> readQueue;
    std::shared_ptr<BackupQueue<FileHandle>> partialReadQueue;
    std::shared_ptr<BackupQueue<FileHandle>> aggregateQueue;
    std::shared_ptr<BlockBufferMap> blockBufferMap;
    std::shared_ptr<BackupControlInfo> controlInfo;
    std::shared_ptr<PacketStats> pktStats;
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder;

    // ADS
    std::atomic<uint64_t> *adsFileCnt;
};

struct SmbWriterCommonData {
    FileHandle fileHandle;
    LibsmbEvent event;

    std::shared_ptr<Module::SmbContextWrapper> writeSmbContext;
    std::shared_ptr<Module::SmbContextWrapper> mkdirSmbContext;
    std::shared_ptr<BackupQueue<FileHandle>> writeQueue;
    std::shared_ptr<BackupQueue<FileHandle>> dirQueue;
    std::shared_ptr<BlockBufferMap> blockBufferMap;
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder;
    LibsmbParams params;

    BackupTimer* timer;
    std::shared_ptr<BackupControlInfo> controlInfo;
    std::shared_ptr<PacketStats> pktStats;

    // mkdir
    std::string path;

    // hardlink
    std::shared_ptr<HardLinkMap> hardlinkMap;
    std::string linkTargetPath;

    // ADS
    std::atomic<bool> *isBlockAdsOpen;
};

struct SmbMkdirParams {
    FileHandle fileHandle;
    std::shared_ptr<SmbWriterCommonData> writerCommonData;
    std::shared_ptr<Module::SmbContextWrapper> mkdirSmbContext;
    std::shared_ptr<BackupQueue<FileHandle>> writeQueue;
    std::string path;
};

struct SmbDeleteParams {
    std::atomic<uint64_t> *m_deleteDir = nullptr;
    std::atomic<uint64_t> *m_deleteFailedDir = nullptr;
    std::atomic<uint64_t> *m_deleteFile = nullptr;
    std::atomic<uint64_t> *m_deleteFailedFile = nullptr;
    std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue = nullptr;
    std::shared_ptr<Module::SmbContextWrapper> m_SmbContext = nullptr;
    std::shared_ptr<BackupTimer> m_writeTimer;
};

#endif // LIBSMB_STRUCTS_H
