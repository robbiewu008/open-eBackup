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
#ifndef LIBSMB_COMMON_H
#define LIBSMB_COMMON_H

#include <string>

#include "Libsmb.h"
#include "Backup.h"
#include "BackupQueue.h"
#include "BackupStructs.h"
#include "libsmb_ctx/SmbContextArgs.h"
#include "libsmb_ctx/SmbContextWrapper.h"
#include "BackupTimer.h"
#include "PacketStats.h"

std::shared_ptr<Module::SmbContextWrapper> SmbConnectContext(const Module::SmbContextArgs &args);
int SmbDisconnectContext(std::shared_ptr<Module::SmbContextWrapper> context);
Module::SmbVersion ConvertStringToSmbVersion(std::string versionString);
bool FillContextParams(Module::SmbContextArgs &smbContextArgs, std::shared_ptr<LibsmbBackupAdvanceParams> advParams);
int SmbEnqueueToTimer(BackupTimer *timer, FileHandle fileHandle);
bool IfNeedRetry(int retryCnt, int maxRetryTimes, int status);
std::string GetPathName(const std::string &filepath);
std::string RemoveFirstSeparator(const std::string &filePath);
std::string GetLibsmbEventName(LibsmbEvent event);
void CheckStatusAndIncStat(int status, PKT_TYPE pktType, std::shared_ptr<PacketStats> pktStats);
int HandleConnectionException(std::shared_ptr<Module::SmbContextWrapper> &smbContext,
    Module::SmbContextArgs &contextArgs, int connectRetryTimes);
PKT_TYPE LibsmbEventToPKT_TYPE(LibsmbEvent event);
bool IsFileReadOrWriteFailed(FileHandle &fileHandle);
bool IsBackupTask(BackupType type);
int64_t GetActualOpenedFileHandleCount(std::shared_ptr<PacketStats> pktStats);

inline void ConcatRootPath(std::string& smbPath, const std::string &rootPath)
{
    DBGLOG("rootPath = %s", rootPath.c_str());
    if (smbPath.empty()) {
        smbPath = rootPath;
    } else if (!rootPath.empty()) {
        smbPath = rootPath + "/" + smbPath;
    }
    DBGLOG("smbPath = %s", smbPath.c_str());
}

#endif // LIBSMB_COMMON_H
