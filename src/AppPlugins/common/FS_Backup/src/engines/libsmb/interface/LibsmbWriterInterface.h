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
#ifndef LIBSMB_WRITER_INTERFACE_H
#define LIBSMB_WRITER_INTERFACE_H

#include <codecvt>
#include "Libsmb.h"
#include "LibsmbCommon.h"
#include "LibsmbStructs.h"
#include "libsmb_ctx/SmbContextWrapper.h"

int SendWriterRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData, LibsmbEvent event);
void WriterCallBack(struct smb2_context *smb2, int status, void *data, void *privateData);
void WriterCallBackHandleEvent(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData);

int SendOpenDstRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData);
int SendWriteRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData);
int SendCloseDstRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData);
int SendSetSdRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData);
int SendSetBasicInfoRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData);
int SendStatRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData);
int SendHardlinkRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData);
int SendUnlinkRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData);
int SendResetAttrRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData);

void SmbFruncateDstCb(struct smb2_context *smb2, int status, void *data, void *pRData);
void SmbOpenDstCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData);
void HandleSmbOpenDstCbError(struct smb2_context *smb2, int status, SmbWriterCommonData *cbData);
void SmbWriteCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData);
void HandleSmbWriteStatusSuccess(SmbWriterCommonData *cbData);
void HandleSmbWriteStatusFailed(struct smb2_context *smb2, void *data, int status, SmbWriterCommonData *cbData);
void HandleSmbWriteStatusRetry(int status, SmbWriterCommonData *cbData);
void SmbCloseDstCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData);
void SmbSetSdCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData);
void SmbSetBasicInfoCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData);
void SmbSetBasicInfoCbForDir(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData);
void SmbStatCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData);
void SmbHardLinkCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData);
void SmbUnlinkCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData);
void SmbResetAttrCb(struct smb2_context *smb2, int status, void* /* data */, SmbWriterCommonData *cbData);

int ProcessRestorePolicy(SmbWriterCommonData *cbData);
void ResetFileAttr(SmbWriterCommonData *cbData);
bool CheckAttrNotReseted(SmbWriterCommonData *cbData);

BackupRetCode DeleteAllFilesInside(FileHandle &fileHandle, SmbDeleteParams *smbDeleteParams);

#endif // LIBSMB_WRITER_INTERFACE_H
