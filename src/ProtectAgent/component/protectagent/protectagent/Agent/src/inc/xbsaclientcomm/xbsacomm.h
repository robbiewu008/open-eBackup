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

#ifndef XBSACOMM
#define XBSACOMM

#include <time.h>
#include "xbsa_struct.h"

#define XBSA_EXPORT_API __attribute__ ((visibility("default")))
#ifdef __cplusplus
extern "C" {
#endif
/* Function Prototypes
 */
namespace xbsacomm {
// long bsaHandle，BSAInit的时候生成，唯一会话ID，后续系列调用都需要使用同一个bsaHandle
XBSA_EXPORT_API int PbBSABeginTxn(long bsaHandle);

XBSA_EXPORT_API int PbBSACreateObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr,
    BSA_DataBlock32 *dataBlockPtr);

XBSA_EXPORT_API int PbBSADeleteObject(long bsaHandle, BSA_UInt64 copyId);

XBSA_EXPORT_API int PbBSAEndData(long bsaHandle);

XBSA_EXPORT_API int PbBSAEndTxn(long bsaHandle, BSA_Vote vote);

XBSA_EXPORT_API int PbBSAGetData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr);

XBSA_EXPORT_API int PbBSAGetEnvironment(long bsaHandle, BSA_ObjectOwner *objectOwner, char **ptr);

XBSA_EXPORT_API int PbBSAGetLastError(BSA_UInt32 *sizePtr, char *errorCodePtr);

XBSA_EXPORT_API int PbBSAGetNextQueryObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr);

XBSA_EXPORT_API int PbBSAGetObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr,
    BSA_DataBlock32 *dataBlockPtr);

XBSA_EXPORT_API int PbBSAInit(long *bsaHandlePtr, BSA_SecurityToken *tokenPtr, BSA_ObjectOwner *objectOwnerPtr,
    char **environmentPtr, const int32_t appType);

XBSA_EXPORT_API int PbBSAQueryApiVersion(BSA_ApiVersion *apiVersionPtr);

XBSA_EXPORT_API int PbBSAQueryObject(long bsaHandle, BSA_QueryDescriptor *queryDescriptorPtr,
    BSA_ObjectDescriptor *objectDescriptorPtr);

XBSA_EXPORT_API int PbBSAQueryServiceProvider(BSA_UInt32 *sizePtr, char *delimiter, char *providerPtr);

XBSA_EXPORT_API int PbBSASendData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr);

XBSA_EXPORT_API int PbBSATerminate(long bsaHandle);

XBSA_EXPORT_API int PbNBBSAGetErrorString(int ErrCode, BSA_UInt32 *sizePtr, char *errorCodePtr);

XBSA_EXPORT_API int PbNBBSAGetServerError(long bsaHandle, int *ServerStatus, BSA_UInt32 sizePtr, char *ServerStatusStr);

XBSA_EXPORT_API int PbNBBSASetEnv(long bsaHandle, char *EnvVar, char *EnvVal);

XBSA_EXPORT_API bool PbNBIsInformix11();
}

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif