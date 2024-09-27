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
#ifndef LIBNFS_REQUESTS_INTERFACE_H
#define LIBNFS_REQUESTS_INTERFACE_H

#include "LibnfsStructs.h"
#include "OpenRequest.h"
#include "ReadRequest.h"
#include "CloseRequest.h"
#include "ReadLinkRequest.h"
#include "MkdirRequest.h"
#include "CreateRequest.h"
#include "SetMetaRequest.h"
#include "WriteRequest.h"
#include "SymLinkRequest.h"
#include "MknodRequest.h"
#include "LinkUtimeRequest.h"
#include "LinkDeleteRequest.h"
#include "DirectoryDeleteRequest.h"
#include "LstatRequest.h"
#include "HardLinkRequest.h"
#include "LibnfsCommonMethods.h"

int SendNfsRequest(FileHandle &fileHandle, void *cbData, LibnfsEvent event);
int SendNfsRequest1(FileHandle &fileHandle, void *cbData, LibnfsEvent event);
int SendNfsRequest2(FileHandle &fileHandle, void *cbData, LibnfsEvent event);

int SendOpenRequest(FileHandle &fileHandle, NfsCommonData &commonData);
int SendReadRequest(FileHandle &fileHandle, NfsCommonData &commonData,
    std::shared_ptr<BlockBufferMap> blockBufferMap);
int SendReadLinkRequest(FileHandle &fileHandle, NfsCommonData &commonData,
    std::shared_ptr<BlockBufferMap> blockBufferMap);
int SendSrcCloseRequest(FileHandle &fileHandle, NfsCommonData &commonData);
int SendCreateRequest(FileHandle &fileHandle, struct nfsfh *nfsfh, uint32_t openFlag, NfsCommonData &commonData,
    BackupParams backupParams);
int SendWriteRequest(FileHandle &fileHandle, NfsCommonData &commonData,
    std::shared_ptr<BlockBufferMap> blockBufferMap);
int SendSetMetaRequest(FileHandle &fileHandle, NfsCommonData &commonData);
int SendLstatRequest(FileHandle &fileHandle, struct nfsfh *nfsfh, NfsCommonData &commonData,
    BackupParams backupParams);
int SendSymlinkRequest(FileHandle &fileHandle, struct nfsfh *nfsfh, NfsCommonData &commonData,
    std::shared_ptr<BlockBufferMap> blockBufferMap);
int SendSymlinkUtimeRequest(FileHandle &fileHandle, NfsCommonData &commonData);
int SendHardlinkRequest(FileHandle &fileHandle, std::string targetPath, struct nfsfh *nfsfh, NfsCommonData &commonData,
    BackupParams backupParams);
int SendMknodRequest(FileHandle &fileHandle, struct nfsfh *nfsfh, NfsCommonData &commonData);
int SendDstCloseRequest(FileHandle &fileHandle, NfsCommonData &commonData);
int SendLinkDeleteRequest(FileHandle &fileHandle, NfsCommonData &commonData);
int SendLinkDeleteRequestForRestore(FileHandle &fileHandle, NfsCommonData &commonData);
int SendDirDeleteRequest(FileHandle &fileHandle, NfsCommonData &commonData);

int ReadLink(FileHandle &fileHandle, NfsCommonData &commonData, std::shared_ptr<BlockBufferMap> blockBufferMap);
int LstatFile(FileHandle &fileHandle, NfsCommonData &commonData, BackupParams backupParams,
    std::shared_ptr<FileHandleCache> fileHandleCache);
int CreateSymlink(FileHandle &fileHandle, NfsCommonData &commonData, std::shared_ptr<FileHandleCache> fileHandleCache,
    std::shared_ptr<BlockBufferMap> blockBufferMap);
int CreateHardlink(FileHandle &fileHandle, NfsCommonData &commonData, BackupParams backupParams,
    std::shared_ptr<FileHandleCache> fileHandleCache);
int CreateSpecialFile(FileHandle &fileHandle, NfsCommonData &commonData,
    std::shared_ptr<FileHandleCache> fileHandleCache);
int WriteSymLinkMeta(FileHandle &fileHandle, NfsCommonData &commonData);
int LinkDelete(FileHandle &fileHandle, NfsCommonData &commonData, std::shared_ptr<BlockBufferMap> blockBufferMap);
int LinkDeleteForRestore(FileHandle &fileHandle, NfsCommonData &commonData);
int DirectoryDelete(FileHandle &fileHandle, NfsCommonData &commonData);
int MakeDirectory(FileHandle &fileHandle, NfsCommonData &commonData, std::shared_ptr<FileHandleCache> fileHandleCache);

void PushToAggregator(FileHandle& fileHandle, NfsCommonData &commonData);

void HandleZeroSizeFileRead(FileHandle &fileHandle, NfsCommonData &commonData,
    std::shared_ptr<BlockBufferMap> blockBufferMap);

void HandleSendWriterNfsRequestFailure(FileHandle &fileHandle, NfsCommonData &commonData);
void HandleSendReaderNfsRequestFailure(FileHandle &fileHandle, NfsCommonData &commonData);

#endif // LIBNFS_REQUESTS_INTERFACE_H