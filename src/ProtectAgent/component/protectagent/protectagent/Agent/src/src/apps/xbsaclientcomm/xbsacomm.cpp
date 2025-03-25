/* *
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @Description XBSA client external interface module.
 * @Create 2021-05-21
 * @Author machenglin mwx1011302
 */
#include <unistd.h>

#include "common/Log.h"
#include "xbsaclientcomm/xbsa_struct.h"
#include "xbsaclientcomm/ThriftClientMgr.h"
#include "xbsaclientcomm/xbsacomm.h"

namespace xbsacomm {
int PbBSABeginTxn(long bsaHandle)
{
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSABeginTxnMgr(bsaHandle);
}

int PbBSACreateObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr, BSA_DataBlock32 *dataBlockPtr)
{
    if (objectDescriptorPtr == nullptr) {
        return BSA_RC_NULL_ARGUMENT;
    }
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    if (objectDescriptorPtr->copyType == BSA_CopyType_ANY) {
        return BSA_RC_INVALID_OBJECTDESCRIPTOR;
    }
    if (objectDescriptorPtr->objectType == BSA_ObjectType_ANY) {
        return BSA_RC_INVALID_OBJECTDESCRIPTOR;
    }
    if (strlen(objectDescriptorPtr->objectName.pathName) <= 0) {
        return BSA_RC_INVALID_OBJECTDESCRIPTOR;
    }

    return ThriftClientMgr::GetInstance().BSACreateObjectMgr(bsaHandle, objectDescriptorPtr, dataBlockPtr);
}

int PbBSADeleteObject(long bsaHandle, BSA_UInt64 copyId)
{
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSADeleteObjectMgr(bsaHandle, copyId);
}

int PbBSAEndData(long bsaHandle)
{
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSAEndDataMgr(bsaHandle);
}

int PbBSAEndTxn(long bsaHandle, BSA_Vote vote)
{
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSAEndTxnMgr(bsaHandle, vote);
}

int PbBSAGetData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    if (dataBlockPtr == nullptr) {
        return BSA_RC_NULL_ARGUMENT;
    }
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSAGetDataMgr(bsaHandle, dataBlockPtr);
}

int PbBSAGetEnvironment(long bsaHandle, BSA_ObjectOwner* objectOwner, char** ptr)
{
    (void)bsaHandle;
    (void)objectOwner;
    (void)ptr;
    mp_int32 ret = BSA_RC_SUCCESS;
    return ret;
}

int PbBSAGetLastError(BSA_UInt32 *sizePtr, char *errorCodePtr)
{
    if ((sizePtr == nullptr) || (errorCodePtr == nullptr)) {
        return BSA_RC_NULL_ARGUMENT;
    }
    return ThriftClientMgr::GetInstance().BSAGetLastErrorMgr(sizePtr, errorCodePtr);
}

int PbBSAGetNextQueryObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr)
{
    if (objectDescriptorPtr == nullptr) {
        return BSA_RC_NULL_ARGUMENT;
    }
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSAGetNextQueryObjectMgr(bsaHandle, objectDescriptorPtr);
}

int PbBSAGetObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr, BSA_DataBlock32 *dataBlockPtr)
{
    if ((objectDescriptorPtr == nullptr) || (dataBlockPtr == nullptr)) {
        return BSA_RC_NULL_ARGUMENT;
    }
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSAGetObjectMgr(bsaHandle, objectDescriptorPtr, dataBlockPtr);
}

int PbBSAInit(long *bsaHandlePtr, BSA_SecurityToken *tokenPtr, BSA_ObjectOwner *objectOwnerPtr, char **environmentPtr,
              const int32_t appType)
{
    if ((bsaHandlePtr == nullptr) || (objectOwnerPtr == nullptr) || (environmentPtr == nullptr)) {
        return BSA_RC_NULL_ARGUMENT;
    }
    return ThriftClientMgr::GetInstance().BSAInitMgr(bsaHandlePtr, tokenPtr, objectOwnerPtr, environmentPtr, appType);
}

int PbBSAQueryApiVersion(BSA_ApiVersion *apiVersionPtr)
{
    if (apiVersionPtr == nullptr) {
        return BSA_RC_NULL_ARGUMENT;
    }
    return ThriftClientMgr::GetInstance().BSAQueryApiVersionMgr(apiVersionPtr);
}

int PbBSAQueryObject(long bsaHandle, BSA_QueryDescriptor *queryDescriptorPtr, BSA_ObjectDescriptor *objectDescriptorPtr)
{
    if ((queryDescriptorPtr == nullptr) || (objectDescriptorPtr == nullptr)) {
        return BSA_RC_NULL_ARGUMENT;
    }
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }

    return ThriftClientMgr::GetInstance().BSAQueryObjectMgr(bsaHandle, queryDescriptorPtr, objectDescriptorPtr);
}

int PbBSAQueryServiceProvider(BSA_UInt32 *sizePtr, char *delimiter, char *providerPtr)
{
    if ((sizePtr == nullptr) || (delimiter == nullptr) || (providerPtr == nullptr)) {
        return BSA_RC_NULL_ARGUMENT;
    }
    return ThriftClientMgr::GetInstance().BSAQueryServiceProviderMgr(sizePtr, delimiter, providerPtr);
}

int PbBSASendData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    if (dataBlockPtr == nullptr) {
        return BSA_RC_NULL_ARGUMENT;
    }
    return ThriftClientMgr::GetInstance().BSASendDataMgr(bsaHandle, dataBlockPtr);
}

int PbBSATerminate(long bsaHandle)
{
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSATerminateMgr(bsaHandle);
}

int PbNBBSAGetErrorString(int errCode, BSA_UInt32 *sizePtr, char *errorCodePtr)
{
    (void)errCode;
    (void)sizePtr;
    (void)errorCodePtr;
    mp_int32 ret = BSA_RC_SUCCESS;
    return ret;
}

int PbNBBSAGetServerError(long bsaHandle, int *serverStatus, BSA_UInt32 sizePtr, char *serverStatusStr)
{
    (void)bsaHandle;
    (void)serverStatus;
    (void)sizePtr;
    (void)serverStatusStr;
    mp_int32 ret = BSA_RC_SUCCESS;
    return ret;
}

int PbNBBSASetEnv(long bsaHandle, char *envKey, char *envVal)
{
    (void)bsaHandle;
    (void)envKey;
    (void)envVal;
    mp_int32 ret = BSA_RC_SUCCESS;
    return ret;
}

bool PbNBIsInformix11()
{
    return ThriftClientMgr::GetInstance().IsInfomrix11();
}

}