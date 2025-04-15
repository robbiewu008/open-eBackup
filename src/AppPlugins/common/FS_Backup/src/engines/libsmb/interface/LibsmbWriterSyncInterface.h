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
#ifndef LIBSMB_WRITER_SYNC_INTERFACE_H
#define LIBSMB_WRITER_SYNC_INTERFACE_H

#include <codecvt>
#include "LibsmbCommon.h"
#include "LibsmbStructs.h"
#include "libsmb_ctx/SmbContextWrapper.h"

int SendWriterSyncRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData, LibsmbEvent event);

int SmbMkdirRecursive(FileHandle fileHandle, SmbWriterCommonData *mkdirParams);
int SmbRemoveFileAndDirRecursive(FileHandle &fileHandle, SmbMkdirParams rmdirParams);

int SmbMkdirSync(uint16_t retryCnt, SmbWriterCommonData *mkdirParams);
int HandleMkdirSyncReqStatus(int status, std::string curPath,
                             uint16_t retryCnt, SmbWriterCommonData *mkdirParams);
int MakeDirRecursively(const std::string &targetFilePath, SmbWriterCommonData *mkdirParams);
int StatAndMkDir(std::string dirPath, std::string &parentDirPath, SmbWriterCommonData *mkdirParams);
int HandleDirExist(std::string &curPath, uint16_t retryCnt, SmbWriterCommonData *mkdirParams);

int SmbDeleteAll(FileHandle& fileHandle, SmbWriterCommonData *smbDeleteParams);
int DeleteAllFilesInside(std::string& path, SmbWriterCommonData *smbDeleteParams);
int SmbUnlink(SmbWriterCommonData *linkDeleteParams);

#endif // LIBSMB_WRITER_SYNC_INTERFACE_H
