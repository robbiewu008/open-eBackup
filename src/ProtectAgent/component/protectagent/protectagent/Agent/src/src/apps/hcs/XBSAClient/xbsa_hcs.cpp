#include "xbsa/xbsa.h"

#include <unistd.h>

#include "common/Log.h"
#include "xbsaclientcomm/ThriftClientMgr.h"

int BSABeginTxn(long bsaHandle)
{
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSABeginTxnMgr(bsaHandle);
}

int BSACreateObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr, BSA_DataBlock32 *dataBlockPtr)
{
    if ((objectDescriptorPtr == NULL) || (dataBlockPtr == NULL)) {
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

int BSADeleteObject(long bsaHandle, BSA_UInt64 copyId)
{
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSADeleteObjectMgr(bsaHandle, copyId);
}

int BSAEndData(long bsaHandle)
{
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSAEndDataMgr(bsaHandle);
}

int BSAEndTxn(long bsaHandle, BSA_Vote vote)
{
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSAEndTxnMgr(bsaHandle, vote);
}

int BSAGetData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    if (dataBlockPtr == NULL) {
        return BSA_RC_NULL_ARGUMENT;
    }
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSAGetDataMgr(bsaHandle, dataBlockPtr);
}

int BSAGetEnvironment(long bsaHandle, BSA_ObjectOwner* objectOwner, char** ptr)
{
    (void)bsaHandle;
    (void)objectOwner;
    (void)ptr;
    mp_int32 ret = BSA_RC_SUCCESS;
    return ret;
}

int BSAGetLastError(BSA_UInt32 *sizePtr, char *errorCodePtr)
{
    if ((sizePtr == NULL) || (errorCodePtr == NULL)) {
        return BSA_RC_NULL_ARGUMENT;
    }
    return ThriftClientMgr::GetInstance().BSAGetLastErrorMgr(sizePtr, errorCodePtr);
}

int BSAGetNextQueryObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr)
{
    if (objectDescriptorPtr == NULL) {
        return BSA_RC_NULL_ARGUMENT;
    }
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSAGetNextQueryObjectMgr(bsaHandle, objectDescriptorPtr);
}

int BSAGetObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr, BSA_DataBlock32 *dataBlockPtr)
{
    if ((objectDescriptorPtr == NULL) || (dataBlockPtr == NULL)) {
        return BSA_RC_NULL_ARGUMENT;
    }
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSAGetObjectMgr(bsaHandle, objectDescriptorPtr, dataBlockPtr);
}

int BSAInit(long *bsaHandlePtr, BSA_SecurityToken *tokenPtr, BSA_ObjectOwner *objectOwnerPtr, char **environmentPtr)
{
    if ((bsaHandlePtr == NULL) || (objectOwnerPtr == NULL) || (environmentPtr == NULL)) {
        return BSA_RC_NULL_ARGUMENT;
    }
    return ThriftClientMgr::GetInstance().BSAInitMgr(bsaHandlePtr, tokenPtr, objectOwnerPtr, environmentPtr,
                                                     BSA_AppType::BSA_HCS);
}

int BSAQueryApiVersion(BSA_ApiVersion *apiVersionPtr)
{
    if (apiVersionPtr == NULL) {
        return BSA_RC_NULL_ARGUMENT;
    }
    return ThriftClientMgr::GetInstance().BSAQueryApiVersionMgr(apiVersionPtr);
}

int BSAQueryObject(long bsaHandle, BSA_QueryDescriptor *queryDescriptorPtr, BSA_ObjectDescriptor *objectDescriptorPtr)
{
    if ((queryDescriptorPtr == NULL) || (objectDescriptorPtr == NULL)) {
        return BSA_RC_NULL_ARGUMENT;
    }
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    // 如果pathName中不包含实例ID，则返回对象不存在
    mp_string pathName = queryDescriptorPtr->objectName.pathName;
    INFOLOG("BSA query object get pathName, %s.", pathName.c_str());
    if (pathName.substr(START_POSITION, END_POSITION) == "/roach") {
        return BSA_RC_OBJECT_NOT_FOUND;
    }
    return ThriftClientMgr::GetInstance().BSAQueryObjectMgr(bsaHandle, queryDescriptorPtr, objectDescriptorPtr);
}

int BSAQueryServiceProvider(BSA_UInt32 *sizePtr, char *delimiter, char *providerPtr)
{
    if ((sizePtr == NULL) || (delimiter == NULL) || (providerPtr == NULL)) {
        return BSA_RC_NULL_ARGUMENT;
    }
    return ThriftClientMgr::GetInstance().BSAQueryServiceProviderMgr(sizePtr, delimiter, providerPtr);
}

int BSASendData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    if (dataBlockPtr == NULL) {
        return BSA_RC_NULL_ARGUMENT;
    }
    return ThriftClientMgr::GetInstance().BSASendDataMgr(bsaHandle, dataBlockPtr);
}

int BSATerminate(long bsaHandle)
{
    if (bsaHandle <= 0) {
        return BSA_RC_INVALID_HANDLE;
    }
    return ThriftClientMgr::GetInstance().BSATerminateMgr(bsaHandle);
}

int NBBSAGetErrorString(int errCode, BSA_UInt32 *sizePtr, char *errorCodePtr)
{
    (void)errCode;
    (void)sizePtr;
    (void)errorCodePtr;
    mp_int32 ret = BSA_RC_SUCCESS;
    return ret;
}

int NBBSAGetServerError(long bsaHandle, int *serverStatus, BSA_UInt32 sizePtr, char *serverStatusStr)
{
    (void)bsaHandle;
    (void)serverStatus;
    (void)sizePtr;
    (void)serverStatusStr;
    mp_int32 ret = BSA_RC_SUCCESS;
    return ret;
}

int NBBSASetEnv(long bsaHandle, char *envKey, char *envVal)
{
    (void)bsaHandle;
    (void)envKey;
    (void)envVal;
    mp_int32 ret = BSA_RC_SUCCESS;
    return ret;
}