/* *
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @Description XBSA client data type conversion.
 * @Create 2021-05-21
 * @Author machenglin mwx1011302
 */
#include "xbsaclientcomm/DataConversion.h"

#include <cstdio>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include "securec.h"
#include "common/Log.h"
#include "common/MpString.h"

int DataConversion::CopyStrToChar(const std::string &src, char *dst, uint32_t dstSize)
{
    return strncpy_s(dst, dstSize, src.c_str(), src.size());
}

void DataConversion::ConvertStrToTime(const std::string &src, struct tm &dst)
{
    time_t time = (time_t)atoi(src.c_str());
    if (gmtime_r(&time, &dst) == NULL) {
        ERRLOG("gmtime_r failed! src=%s", src.c_str());
    }
}

void DataConversion::ConvertObjectDescriptorIn(BSA_ObjectDescriptor *src, BsaObjectDescriptor &dst)
{
    if (src == nullptr) {
        return;
    }
    dst.copyId.left = (int64_t)src->copyId.left;
    dst.copyId.right = (int64_t)src->copyId.right;

    dst.copyType = src->copyType;
    dst.createTime.utcTime = "123"; // req中没有用到但是thrift接口是必填参数，随便赋个值
    dst.estimatedSize.left = (int64_t)src->estimatedSize.left;
    dst.estimatedSize.right = (int64_t)src->estimatedSize.right;

    dst.objectDescription = src->objectDescription;
    dst.objectInfo = (char *)src->objectInfo;
    dst.objectName.objectSpaceName = src->objectName.objectSpaceName;
    mp_string str_realpath(src->objectName.pathName);
    dst.objectName.pathName = CMpString::RealPath(str_realpath);
    dst.objectOwner.appObjectOwner = src->objectOwner.app_ObjectOwner;
    dst.objectOwner.bsaObjectOwner = src->objectOwner.bsa_ObjectOwner;

    dst.objectStatus = (int32_t)src->objectStatus;
    dst.objectType = (int32_t)src->objectType;

    dst.resourceType = src->resourceType;
    dst.restoreOrder.left = (int64_t)src->restoreOrder.left;
    dst.restoreOrder.right = (int64_t)src->restoreOrder.right;
    dst.__set_rsv2(src->rsv2);
}

bool DataConversion::ConvertObjectDescriptorOut(BsaObjectDescriptor &src, BSA_ObjectDescriptor *dst)
{
    // 副本id没有实际作用，但是informix要求非0
    dst->copyId.left = 1;
    dst->copyId.right = 1;
    dst->copyType = (BSA_CopyType)src.copyType;

    ConvertStrToTime(src.createTime.utcTime, dst->createTime);
    dst->estimatedSize.left = (BSA_UInt32)src.estimatedSize.left;
    dst->estimatedSize.right = (BSA_UInt32)src.estimatedSize.right;

    if (CopyStrToChar(src.objectDescription, dst->objectDescription, BSA_MAX_DESCRIPTION) != 0) {
        ERRLOG("copy objectDescription failed!");
        return false;
    }
    if (CopyStrToChar(src.objectInfo, (char *)dst->objectInfo, BSA_MAX_OBJECTINFO) != 0) {
        ERRLOG("copy objectInfo failed!");
        return false;
    }
    if (CopyStrToChar(src.objectName.objectSpaceName, dst->objectName.objectSpaceName, BSA_MAX_OBJECTSPACENAME) != 0) {
        ERRLOG("copy objectSpaceName failed!");
        return false;
    }
    if (CopyStrToChar(CMpString::RealPath(src.objectName.pathName), dst->objectName.pathName, BSA_MAX_PATHNAME) != 0) {
        ERRLOG("copy pathName failed!");
        return false;
    }
    if (CopyStrToChar(src.objectOwner.appObjectOwner, dst->objectOwner.app_ObjectOwner, BSA_MAX_APPOBJECT_OWNER) != 0) {
        ERRLOG("copy app_ObjectOwner failed!");
        return false;
    }
    if (CopyStrToChar(src.objectOwner.bsaObjectOwner, dst->objectOwner.bsa_ObjectOwner, BSA_MAX_BSAOBJECT_OWNER) != 0) {
        ERRLOG("copy bsa_ObjectOwner failed!");
        return false;
    }
    if (CopyStrToChar(src.resourceType, dst->resourceType, BSA_MAX_RESOURCETYPE) != 0) {
        ERRLOG("copy resourceType failed!");
        return false;
    }

    dst->restoreOrder.left = (BSA_UInt32)src.restoreOrder.left;
    dst->restoreOrder.right = (BSA_UInt32)src.restoreOrder.right;

    dst->objectType = (BSA_ObjectType)src.objectType;
    dst->objectStatus = (BSA_ObjectStatus)src.objectStatus;
    return true;
}

void DataConversion::ConvertdataBlockOut(BsaDataBlock32 &src, BSA_DataBlock32 *dst)
{
    if (dst == nullptr) {
        return;
    }
    dst->bufferLen = (BSA_UInt32)src.bufferLen;
    dst->bufferPtr = NULL; // 创建对象时bufferPtr没有用到
    dst->headerBytes = (BSA_UInt32)src.headerBytes;
    dst->numBytes = (BSA_UInt32)src.numBytes;
    dst->shareOffset = (BSA_UInt32)src.shareOffset;
    dst->shareId = (BSA_Int16)src.shareId;
}

void DataConversion::ConvertQueryObjectIn(BSA_QueryDescriptor *src, BsaQueryDescriptor &dst)
{
    if (src == nullptr) {
        return;
    }
    dst.copyType = (int32_t)src->copyType;
    mp_string str_realpath(src->objectName.pathName);
    dst.objectName.pathName = CMpString::RealPath(str_realpath);
    dst.objectName.objectSpaceName = src->objectName.objectSpaceName;
    dst.objectOwner.appObjectOwner = src->objectOwner.app_ObjectOwner;
    dst.objectOwner.bsaObjectOwner = src->objectOwner.bsa_ObjectOwner;
    dst.objectStatus = (int32_t)src->objectStatus;
    dst.objectType = (int32_t)src->objectType;
    dst.__set_rsv5(src->rsv5);
}

void DataConversion::ConvertdataBlockIn(BSA_DataBlock32 *src, BsaDataBlock32 &dst)
{
    dst.bufferLen = (int64_t)src->bufferLen;
    dst.headerBytes = (int64_t)src->headerBytes;
    dst.numBytes = (int64_t)src->numBytes;
    dst.shareOffset = (int64_t)src->shareOffset;
    dst.shareId = (int64_t)src->shareId;
    dst.bufferPtr = "123"; // req中没有用到但是thrift接口是必填参数，随便赋个值
}

void DataConversion::U64ToBsaU64(unsigned long long u64, BsaUInt64 &b64)
{
    const unsigned int offset = 32;
    b64.left = (u64 >> offset) & 0xFFFFFFFF;
    b64.right = u64 & 0xFFFFFFFF;
}
