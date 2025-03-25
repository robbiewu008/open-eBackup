/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file xbsa.h
 * @brief  Contains function declarations xbsa
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
/* xbsa.h
 *
 * This is a sample C header file describing the XBSA.
 *
 * This appendix is not a normative part of the
 * specification and is provided for illustrative
 * purposes only.
 *
 * Implementations must ensure that the sizes of integer
 * datatypes match their names, not necessarily the typedefs
 * presented in this example.
 *
 */

#ifndef XBSAIIF
#define XBSAIIF

#include <time.h>
#include "xbsaclientcomm/xbsa_struct.h"
#include "xbsaclientcomm/ThriftClientMgr.h"
#ifdef __cplusplus
extern "C" {
#endif
void struct_trance(BSA_DataBlock32* dataBlockPtr);

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

mp_bool AnalyzeAppType(BSA_AppType& appType, mp_string& envStr);
#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif