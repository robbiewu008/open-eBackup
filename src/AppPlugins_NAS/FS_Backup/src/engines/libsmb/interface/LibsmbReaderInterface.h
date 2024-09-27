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
#ifndef LIBSMB_READER_INTERFACE_H
#define LIBSMB_READER_INTERFACE_H

#include <sys/types.h>
#include "Libsmb.h"
#include "LibsmbCommon.h"
#include "LibsmbStructs.h"
#include "libsmb_ctx/SmbContextWrapper.h"

int SendReaderRequest(FileHandle &fileHandle, SmbReaderCommonData *cbData, LibsmbEvent event);
void ReaderCallBack(struct smb2_context *smb2, int status, void *data, void *privateData);

int SendOpenRequest(FileHandle &fileHandle, SmbReaderCommonData *cbData);
int SendReadRequest(FileHandle fileHandle, SmbReaderCommonData *cbData);
int SendCloseRequest(FileHandle &fileHandle, SmbReaderCommonData *cbData);
int SendAdsRequest(FileHandle &fileHandle, SmbReaderCommonData *cbData);

void SmbOpenCb(struct smb2_context *smb2, int status, void *data, SmbReaderCommonData *cbData);
void SmbReadCb(struct smb2_context *smb2, int status, void *data, SmbReaderCommonData *cbData);
void SmbCloseCb(struct smb2_context *smb2, int status, void *data, SmbReaderCommonData *cbData);
void SmbAdsCb(struct smb2_context *smb2, int status, void *data, SmbReaderCommonData *cbData);

void HandleSmbOpenStatusFailed(struct smb2_context *smb2, int status, SmbReaderCommonData *cbData);
void HandleSmbReadStatusSuccess(SmbReaderCommonData *cbData);
void HandleSmbReadStatusFailed(struct smb2_context *smb2, void *data, int status, SmbReaderCommonData *cbData);
void HandleSmbReadStatusRetry(int status, SmbReaderCommonData *cbData);
void HandleSmbAdsStatusFailed(struct smb2_context *smb2, int status, SmbReaderCommonData *cbData);

void SmbOpenCbSendBlock(SmbReaderCommonData *cbData);

void SmbAdsCbDispachFilehandle(SmbReaderCommonData *cbData, uint32_t adsFileCount);

#endif // LIBSMB_READER_INTERFACE_H
