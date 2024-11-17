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
#ifndef XBSA
#define XBSA

#include <time.h>
#include "xbsaclientcomm/xbsa_struct.h"

#define XBSA_EXPORT_API __attribute__ ((visibility("default")))
#ifdef __cplusplus
extern "C" {
#endif

/* Function Prototypes
 */

// long bsaHandle，BSAInit的时候生成，唯一会话ID，后续系列调用都需要使用同一个bsaHandle
XBSA_EXPORT_API int BSABeginTxn(long bsaHandle);

XBSA_EXPORT_API int BSACreateObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr,
                                    BSA_DataBlock32 *dataBlockPtr);

XBSA_EXPORT_API int BSADeleteObject(long bsaHandle, BSA_UInt64 copyId);

XBSA_EXPORT_API int BSAEndData(long bsaHandle);

XBSA_EXPORT_API int BSAEndTxn(long bsaHandle, BSA_Vote vote);

XBSA_EXPORT_API int BSAGetData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr);

XBSA_EXPORT_API int BSAGetEnvironment(long bsaHandle, BSA_ObjectOwner *objectOwner, char **ptr);

XBSA_EXPORT_API int BSAGetLastError(BSA_UInt32 *sizePtr, char *errorCodePtr);

XBSA_EXPORT_API int BSAGetNextQueryObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr);

XBSA_EXPORT_API int BSAGetObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr,
                                 BSA_DataBlock32 *dataBlockPtr);

XBSA_EXPORT_API int BSAInit(long *bsaHandlePtr, BSA_SecurityToken *tokenPtr, BSA_ObjectOwner *objectOwnerPtr,
                            char **environmentPtr);

XBSA_EXPORT_API int BSAQueryApiVersion(BSA_ApiVersion *apiVersionPtr);

XBSA_EXPORT_API int BSAQueryObject(long bsaHandle, BSA_QueryDescriptor *queryDescriptorPtr,
                                   BSA_ObjectDescriptor *objectDescriptorPtr);

XBSA_EXPORT_API int BSAQueryServiceProvider(BSA_UInt32 *sizePtr, char *delimiter, char *providerPtr);

XBSA_EXPORT_API int BSASendData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr);

XBSA_EXPORT_API int BSATerminate(long bsaHandle);

XBSA_EXPORT_API int NBBSAGetErrorString(int ErrCode, BSA_UInt32 *sizePtr, char *errorCodePtr);

XBSA_EXPORT_API int NBBSAGetServerError(long bsaHandle, int *ServerStatus, BSA_UInt32 sizePtr, char *ServerStatusStr);

XBSA_EXPORT_API int NBBSASetEnv(long bsaHandle, char *EnvVar, char *EnvVal);


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif