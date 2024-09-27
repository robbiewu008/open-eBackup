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
#ifndef BACKUP_COMMON_SERVICE_PARAMS_H
#define BACKUP_COMMON_SERVICE_PARAMS_H

#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"
#include "BackupQueue.h"
 
using namespace FS_Backup;
using namespace Module;

/*
 * used for to provide library for host(Win32/Posix) Backup only
 */
#ifdef WIN32
const BackupIOEngine HOST_OS_PLATFORM_IO_ENGINE = BackupIOEngine::WIN32_IO;
#else
const BackupIOEngine HOST_OS_PLATFORM_IO_ENGINE = BackupIOEngine::POSIX;
#endif

/*
 * compounded params used to build Reader of the four stage
 */
struct ReaderParams {
    BackupParams                                backupParams {};
    std::shared_ptr<BackupQueue<FileHandle>>    readQueuePtr { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>>    aggregateQueuePtr { nullptr };
    std::shared_ptr<BackupControlInfo>          controlInfo { nullptr };
    std::shared_ptr<BlockBufferMap>             blockBufferMap { nullptr };
    std::shared_ptr<HardLinkMap>                hardlinkMap { nullptr };        /* only used in hardlink stage */
};
 
/*
 * compounded params used to build Writer of the four stage
 */
struct WriterParams {
    BackupParams                                backupParams {};
    std::shared_ptr<BackupQueue<FileHandle>>    writeQueuePtr { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>>    readQueuePtr { nullptr };
    std::shared_ptr<BackupControlInfo>          controlInfo { nullptr };
    std::shared_ptr<BlockBufferMap>             blockBufferMap { nullptr };
    std::shared_ptr<HardLinkMap>                hardlinkMap { nullptr };        /* only used in hardlink stage */
};
 
/*
 * compounded params used to build Aggregator of the four stage
 */
struct AggregatorParams {
    BackupParams                                backupParams {};
    std::shared_ptr<BackupQueue<FileHandle>>    readQueuePtr { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>>    aggregateQueuePtr { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>>    writeQueuePtr { nullptr };
    std::shared_ptr<BackupControlInfo>          controlInfo { nullptr };
    std::shared_ptr<BlockBufferMap>             blockBufferMap { nullptr };
    std::shared_ptr<HardLinkMap>                hardlinkMap { nullptr };        /* only used in hardlink stage */
};

#endif