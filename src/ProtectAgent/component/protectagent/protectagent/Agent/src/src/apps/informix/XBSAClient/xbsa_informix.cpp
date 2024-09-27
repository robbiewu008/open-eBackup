/* *
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @Description XBSA client external interface module.
 * @Create 2021-05-21
 * @Author machenglin mwx1011302
 */
#include "xbsa/xbsa_informix.h"
#include <unistd.h>
#include "securec.h"
#include "xbsacomm.h"
#include "common/Log.h"
#include "common/Ip.h"
#include "ThriftClientMgr.h"
#include "xbsaclientcomm/DataConversion.h"

using namespace xbsacomm;

void DataBlockInformix2Xbsa16(BSA_DataBlock32 *src, BSA_DataBlock32 *dst)
{
    BsaDataBlock16Iif *ifmx16 = (BsaDataBlock16Iif *)src;
    BsaDataBlock32Iif *ifmx32 = (BsaDataBlock32Iif *)src;

    DBGLOG("DataBlockInformix16: bufferLen=%u, numBytes=%u, bufferPtr=%p",
        ifmx16->bufferLen, ifmx16->numBytes, ifmx16->bufferPtr);
    DBGLOG("DataBlockInformix32: bufferLen=%u, numBytes=%u, bufferPtr=%p",
        ifmx32->bufferLen, ifmx32->numBytes, ifmx32->bufferPtr);

    dst->bufferLen = ifmx16->bufferLen;
    dst->numBytes  = ifmx16->numBytes;
    dst->bufferPtr = ifmx16->bufferPtr;
}

void DataBlockInformix2Xbsa32(BSA_DataBlock32 *src, BSA_DataBlock32 *dst)
{
    BsaDataBlock16Iif *ifmx16 = (BsaDataBlock16Iif *)src;
    BsaDataBlock32Iif *ifmx32 = (BsaDataBlock32Iif *)src;

    DBGLOG("DataBlockInformix16: bufferLen=%u, numBytes=%u, bufferPtr=%p",
        ifmx16->bufferLen, ifmx16->numBytes, ifmx16->bufferPtr);
    DBGLOG("DataBlockInformix32: bufferLen=%u, numBytes=%u, bufferPtr=%p",
        ifmx32->bufferLen, ifmx32->numBytes, ifmx32->bufferPtr);

    dst->bufferLen = ifmx32->bufferLen;
    dst->numBytes  = ifmx32->numBytes;
    dst->bufferPtr = ifmx32->bufferPtr;
}

void DataBlockXbsa2Informix16(BSA_DataBlock32 *src, BSA_DataBlock32 *dst)
{
    BsaDataBlock16Iif *ifmx16 = (BsaDataBlock16Iif *)dst;

    ifmx16->numBytes = src->numBytes;
    DBGLOG("DataBlockInformix16: bufferLen=%u, numBytes=%u, bufferPtr=%p",
        ifmx16->bufferLen, ifmx16->numBytes, ifmx16->bufferPtr);
}

void DataBlockXbsa2Informix32(BSA_DataBlock32 *src, BSA_DataBlock32 *dst)
{
    BsaDataBlock32Iif *ifmx32 = (BsaDataBlock32Iif *)dst;

    ifmx32->numBytes = src->numBytes;
    DBGLOG("DataBlockInformix32: bufferLen=%u, numBytes=%u, bufferPtr=%p",
        ifmx32->bufferLen, ifmx32->numBytes, ifmx32->bufferPtr);
}

int BSABeginTxn(long bsaHandle)
{
    return xbsacomm::PbBSABeginTxn(bsaHandle);
}

int BSACreateObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr, BSA_DataBlock32 *dataBlockPtr)
{
    mp_string envStr;
    if (CIP::GetHostEnv("INFORMIXSERVER", envStr) != MP_SUCCESS) {
        return BSA_RC_INVALID_ENV;
    }
    if (DataConversion::CopyStrToChar(envStr, objectDescriptorPtr->rsv2, BSA_MAX_RESOURCETYPE) != 0) {
        ERRLOG("copy objectInfo failed!");
        return BSA_RC_INVALID_ENV;
    }
    return xbsacomm::PbBSACreateObject(bsaHandle, objectDescriptorPtr, dataBlockPtr);
}

int BSADeleteObject(long bsaHandle, BSA_UInt64 copyId)
{
    return xbsacomm::PbBSADeleteObject(bsaHandle, copyId);
}

int BSAEndData(long bsaHandle)
{
    return xbsacomm::PbBSAEndData(bsaHandle);
}

int BSAEndTxn(long bsaHandle, BSA_Vote vote)
{
    return xbsacomm::PbBSAEndTxn(bsaHandle, vote);
}

int BSAGetData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    int ret = 0;
    if (xbsacomm::PbNBIsInformix11()) {
        BSA_DataBlock32 dataBlock = {0};
        DataBlockInformix2Xbsa16(dataBlockPtr, &dataBlock);
        ret = xbsacomm::PbBSAGetData(bsaHandle, &dataBlock);
        DataBlockXbsa2Informix16(&dataBlock, dataBlockPtr);
    } else {
        BSA_DataBlock32 dataBlock = {0};
        DataBlockInformix2Xbsa32(dataBlockPtr, &dataBlock);
        ret = xbsacomm::PbBSAGetData(bsaHandle, &dataBlock);
        DataBlockXbsa2Informix32(&dataBlock, dataBlockPtr);
    }
    return ret;
}

int BSAGetEnvironment(long bsaHandle, BSA_ObjectOwner* objectOwner, char** ptr)
{
    return xbsacomm::PbBSAGetEnvironment(bsaHandle, objectOwner, ptr);
}

int BSAGetLastError(BSA_UInt32 *sizePtr, char *errorCodePtr)
{
    return xbsacomm::PbBSAGetLastError(sizePtr, errorCodePtr);
}

int BSAGetNextQueryObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr)
{
    return xbsacomm::PbBSAGetNextQueryObject(bsaHandle, objectDescriptorPtr);
}

int BSAGetObject(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr, BSA_DataBlock32 *dataBlockPtr)
{
    BSA_QueryDescriptor* queryDesc = new BSA_QueryDescriptor;
    mp_int32 ret = strcpy_s(queryDesc->objectOwner.app_ObjectOwner, BSA_MAX_APPOBJECT_OWNER,
                            objectDescriptorPtr->objectOwner.app_ObjectOwner);
    if (ret != 0) {
        delete queryDesc;
        ERRLOG("BsaHandle=%ld, Failed to copy and assign values.", bsaHandle);
        return BSA_RC_NULL_ARGUMENT;
    }
    ret = strcpy_s(queryDesc->objectOwner.bsa_ObjectOwner, BSA_MAX_BSAOBJECT_OWNER,
                   objectDescriptorPtr->objectOwner.bsa_ObjectOwner);
    if (ret != 0) {
        delete queryDesc;
        ERRLOG("BsaHandle=%ld, Failed to copy and assign values.", bsaHandle);
        return BSA_RC_NULL_ARGUMENT;
    }
    ret = strcpy_s(queryDesc->objectName.objectSpaceName, BSA_MAX_OBJECTSPACENAME,
                   objectDescriptorPtr->objectName.objectSpaceName);
    if (ret != 0) {
        delete queryDesc;
        ERRLOG("BsaHandle=%ld, Failed to copy and assign values.", bsaHandle);
        return BSA_RC_NULL_ARGUMENT;
    }
    ret = strcpy_s(queryDesc->objectName.pathName, BSA_MAX_PATHNAME, objectDescriptorPtr->objectName.pathName);
    if (ret != 0) {
        delete queryDesc;
        ERRLOG("BsaHandle=%ld, Failed to copy and assign values.", bsaHandle);
        return BSA_RC_NULL_ARGUMENT;
    }
    queryDesc->copyType = objectDescriptorPtr->copyType;
    queryDesc->objectType = objectDescriptorPtr->objectType;
    queryDesc->objectStatus = objectDescriptorPtr->objectStatus;
    BSAQueryObject(bsaHandle, queryDesc, objectDescriptorPtr);
    delete queryDesc;
    BSA_DataBlock32 dataBlock = {0};
    DataBlockInformix2Xbsa32(dataBlockPtr, &dataBlock);
    ret = xbsacomm::PbBSAGetObject(bsaHandle, objectDescriptorPtr, &dataBlock);
    return ret;
}

int BSAInit(long *bsaHandlePtr, BSA_SecurityToken *tokenPtr, BSA_ObjectOwner *objectOwnerPtr, char **environmentPtr)
{
    return xbsacomm::PbBSAInit(bsaHandlePtr, tokenPtr, objectOwnerPtr, environmentPtr, BSA_AppType::BSA_INFORMIX);
}

int BSAQueryApiVersion(BSA_ApiVersion *apiVersionPtr)
{
    return xbsacomm::PbBSAQueryApiVersion(apiVersionPtr);
}

int BSAQueryObject(long bsaHandle, BSA_QueryDescriptor *queryDescriptorPtr, BSA_ObjectDescriptor *objectDescriptorPtr)
{
    mp_string envStr;
    if (CIP::GetHostEnv("INFORMIXSERVER", envStr) != MP_SUCCESS) {
        return BSA_RC_INVALID_ENV;
    }
    if (DataConversion::CopyStrToChar(envStr, queryDescriptorPtr->rsv5, BSA_MAX_RESOURCETYPE) != 0) {
        ERRLOG("copy objectInfo failed!");
        return BSA_RC_INVALID_ENV;
    }
    return xbsacomm::PbBSAQueryObject(bsaHandle, queryDescriptorPtr, objectDescriptorPtr);
}

int BSAQueryServiceProvider(BSA_UInt32 *sizePtr, char *delimiter, char *providerPtr)
{
    return ThriftClientMgr::GetInstance().IifBSAQueryServiceProviderMgr(sizePtr, delimiter, providerPtr);
}

int BSASendData(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    BSA_DataBlock32 dataBlock = {0};
    int ret = 0;
    if (xbsacomm::PbNBIsInformix11()) {
        BSA_DataBlock32 dataBlock = {0};
        DataBlockInformix2Xbsa16(dataBlockPtr, &dataBlock);
        ret = xbsacomm::PbBSASendData(bsaHandle, &dataBlock);
    } else {
        BSA_DataBlock32 dataBlock = {0};
        DataBlockInformix2Xbsa32(dataBlockPtr, &dataBlock);
        ret = xbsacomm::PbBSASendData(bsaHandle, &dataBlock);
    }
    return ret;
}

int BSATerminate(long bsaHandle)
{
    return xbsacomm::PbBSATerminate(bsaHandle);
}

int NBBSAGetErrorString(int errCode, BSA_UInt32 *sizePtr, char *errorCodePtr)
{
    return xbsacomm::PbNBBSAGetErrorString(errCode, sizePtr, errorCodePtr);
}

int NBBSAGetServerError(long bsaHandle, int *serverStatus, BSA_UInt32 sizePtr, char *serverStatusStr)
{
    return xbsacomm::PbNBBSAGetServerError(bsaHandle, serverStatus, sizePtr, serverStatusStr);
}

int NBBSASetEnv(long bsaHandle, char *envKey, char *envVal)
{
    return xbsacomm::PbNBBSASetEnv(bsaHandle, envKey, envVal);
}