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
#include "apps/dws/XBSAServer/BsaIntfAdaptor.h"
#include "common/ConfigXmlParse.h"
#include "apps/dws/XBSAServer/BsaSessionManager.h"

mp_uint64 BsaIntfAdaptor::BsaU64ToU64(const BsaUInt64 &b64)
{
    const mp_uint32 offset = 32;
    return (((mp_uint64)b64.left) << offset) + b64.right;
}

void BsaIntfAdaptor::U64ToBsaU64(mp_uint64 u64, BsaUInt64 &b64)
{
    const mp_uint32 offset = 32;
    b64.left = (u64 >> offset) & 0xFFFFFFFF;
    b64.right = u64 & 0xFFFFFFFF;
}

mp_bool BsaIntfAdaptor::HandleValid(const int64_t handle)
{
    return (handle > 0 && handle <= (int64_t)LONG_MAX);
}

mp_bool BsaIntfAdaptor::StringValid(const std::string &str, mp_uint64 maxLen, mp_bool canBeEmpty)
{
    mp_uint64 len = str.length();
    if (len >= maxLen) {
        return MP_FALSE;
    } else if (len == 0) {
        return canBeEmpty;
    }
    return MP_TRUE;
}

mp_bool BsaIntfAdaptor::BsaObjectOwnerValid(const std::string &bsaObjectOwner, mp_bool canBeEmpty)
{
    return StringValid(bsaObjectOwner, BSA_MAX_BSAOBJECT_OWNER, canBeEmpty);
}

mp_bool BsaIntfAdaptor::AppObjectOwnerValid(const std::string &appObjectOwner, mp_bool canBeEmpty)
{
    return StringValid(appObjectOwner, BSA_MAX_APPOBJECT_OWNER, canBeEmpty);
}

mp_bool BsaIntfAdaptor::ObjectSpaceNameValid(const std::string &objectSpaceName, mp_bool canBeEmpty)
{
    return StringValid(objectSpaceName, BSA_MAX_OBJECTSPACENAME, canBeEmpty);
}

mp_bool BsaIntfAdaptor::PathNameValid(const std::string &pathName, mp_bool canBeEmpty)
{
    return StringValid(pathName, BSA_MAX_PATHNAME, canBeEmpty);
}

mp_bool BsaIntfAdaptor::ResourceTypeValid(const std::string &resourceType, mp_bool canBeEmpty)
{
    return StringValid(resourceType, BSA_MAX_RESOURCETYPE, canBeEmpty);
}

mp_bool BsaIntfAdaptor::ObjectDescriptionValid(const std::string &objectDescription, mp_bool canBeEmpty)
{
    return StringValid(objectDescription, BSA_MAX_DESCRIPTION, canBeEmpty);
}

mp_bool BsaIntfAdaptor::ObjectInfoValid(const std::string &objectInfo, mp_bool canBeEmpty)
{
    return StringValid(objectInfo, BSA_MAX_OBJECTINFO, canBeEmpty);
}

mp_bool BsaIntfAdaptor::CopyTypeValid(const int32_t copyType, mp_bool canBeAny)
{
    if (copyType == BSA_CopyType_ARCHIVE || copyType == BSA_CopyType_BACKUP) {
        return MP_TRUE;
    } else if (copyType == BSA_CopyType_ANY) {
        return canBeAny;
    } else if (copyType == 0) {
        // 在HCS for openGauss场景中暂时服务端没有赋值,默认使用BSA_CopyType_BACKUP
        return MP_TRUE;
    }

    return MP_FALSE;
}

mp_bool BsaIntfAdaptor::ObjectTypeValid(const int32_t objectType, mp_bool canBeAny)
{
    if (objectType == BSA_ObjectType_FILE ||
        objectType == BSA_ObjectType_DIRECTORY ||
        objectType == BASBSA_ObjectType_OTHER) {
        return MP_TRUE;
    } else if (objectType == BSA_ObjectType_ANY) {
        return canBeAny;
    } else if (objectType == 0) {
        // 在HCS for openGauss场景中暂时服务端没有赋值,默认使用BSA_ObjectType_FILE
        return MP_TRUE;
    }

    return MP_FALSE;
}

mp_bool BsaIntfAdaptor::ObjectStatusValid(const int32_t objectStatus, mp_bool canBeAny)
{
    if (objectStatus == BSA_ObjectStatus_MOST_RECENT || objectStatus == BSA_ObjectStatus_NOT_MOST_RECENT) {
        return MP_TRUE;
    } else if (objectStatus == BSA_ObjectStatus_ANY) {
        return canBeAny;
    } else if (objectStatus == BSA_OBJECTSTATUS_APPEND_WRITE || objectStatus == BSA_OBJECTSTATUS_OVERWRITE_WRITE) {
        // 在HCS for openGauss场景中BSA_ObjectStatus_APPEND_WRITE： 追加写 BSA_ObjectStatus_OVERWRITE_WRITE：覆盖写
        return MP_TRUE;
    }

    return MP_FALSE;
}

mp_bool BsaIntfAdaptor::VoteValid(const int32_t vote)
{
    return (vote == BSA_Vote_COMMIT || vote == BSA_Vote_ABORT);
}

mp_void BsaIntfAdaptor::ConvertCreateReqObj(const BsaObjectDescriptor &src, BsaObjInfo &dst,
    const BSA_ObjectOwner &sessionOwner)
{
    DBGLOG("srcOwner(%s,%s),seesionOwner(%s,%s)",
        src.objectOwner.bsaObjectOwner.c_str(), src.objectOwner.appObjectOwner.c_str(),
        sessionOwner.bsa_ObjectOwner, sessionOwner.app_ObjectOwner);

    // XBSA协议规定如果会话中后续任何调用中的BSA_ObjectOwner.bsa_ObjectOwner字段为空，则将默认为在会话初始化中指定的值。
    if (src.objectOwner.bsaObjectOwner.empty()) {
        dst.bsaObjectOwner = sessionOwner.bsa_ObjectOwner;
        dst.appObjectOwner = sessionOwner.app_ObjectOwner;
    } else {
        dst.bsaObjectOwner = src.objectOwner.bsaObjectOwner;
        dst.appObjectOwner = src.objectOwner.appObjectOwner;
    }

    dst.objectSpaceName = src.objectName.objectSpaceName;
    dst.objectName = src.objectName.pathName;
    dst.copyType = src.copyType;
    dst.estimatedSize = BsaU64ToU64(src.estimatedSize);
    dst.resourceType = src.resourceType;
    dst.objectType = src.objectType;
    dst.objectDescription = src.objectDescription;
    dst.objectInfo = src.objectInfo;

    // 初始化不需要的参数
    dst.timestamp = "";
    dst.copyId = 0;
    dst.restoreOrder = 0;
    dst.objectStatus = 0;
}

mp_void BsaIntfAdaptor::ConvertCreateRspObj(const BsaObjInfo &src, BsaObjectDescriptor &dst,
    const BSA_ObjectOwner &sessionOwner)
{
    U64ToBsaU64(src.copyId, dst.copyId);
    U64ToBsaU64(src.restoreOrder, dst.restoreOrder);
    dst.objectStatus = src.objectStatus;
    dst.createTime.utcTime = src.timestamp;

    // XBSA协议规定如果会话中后续任何调用中的BSA_ObjectOwner.bsa_ObjectOwner字段为空，则将默认为在会话初始化中指定的值。
    if (dst.objectOwner.bsaObjectOwner.empty()) {
        dst.objectOwner.bsaObjectOwner = sessionOwner.bsa_ObjectOwner;
        dst.objectOwner.appObjectOwner = sessionOwner.app_ObjectOwner;
    }
}

mp_void BsaIntfAdaptor::ConvertQueryReqObj(mp_long sessionId, const BsaQueryDescriptor &src, BsaObjInfo &dst,
    const BSA_ObjectOwner &sessionOwner)
{
    // XBSA协议规定如果会话中后续任何调用中的BSA_ObjectOwner.bsa_ObjectOwner字段为空，则将默认为在会话初始化中指定的值。
    if (src.objectOwner.bsaObjectOwner.empty()) {
        dst.bsaObjectOwner = sessionOwner.bsa_ObjectOwner;
        dst.appObjectOwner = sessionOwner.app_ObjectOwner;
    } else {
        dst.bsaObjectOwner = src.objectOwner.bsaObjectOwner;
        dst.appObjectOwner = src.objectOwner.appObjectOwner;
    }

    dst.objectSpaceName = src.objectName.objectSpaceName;
    dst.objectName = src.objectName.pathName;
    dst.copyType = src.copyType;
    dst.objectType = src.objectType;
    dst.objectStatus = src.objectStatus;

    INFOLOG("bsaHandle=%lld,queryCond:bsaObjectOwner(%s),appObjectOwner(%s),objectSpaceName(%s),pathName(%s),"
        "objectType=%d,copyType=%d,objectStatus=%d",
        sessionId, src.objectOwner.bsaObjectOwner.c_str(), src.objectOwner.appObjectOwner.c_str(),
        src.objectName.objectSpaceName.c_str(), src.objectName.pathName.c_str(),
        src.objectType, src.copyType, src.objectStatus);
}

mp_void BsaIntfAdaptor::ConvertQueryRspObj(const BsaObjInfo &src, BsaObjectDescriptor &dst)
{
    dst.objectOwner.bsaObjectOwner = src.bsaObjectOwner;
    dst.objectOwner.appObjectOwner = src.appObjectOwner;
    dst.objectName.objectSpaceName = src.objectSpaceName;
    dst.objectName.pathName = src.objectName;
    dst.createTime.utcTime = src.timestamp;
    dst.copyType = src.copyType;
    U64ToBsaU64(src.copyId, dst.copyId);
    U64ToBsaU64(src.restoreOrder, dst.restoreOrder);
    U64ToBsaU64(src.estimatedSize, dst.estimatedSize);
    dst.resourceType = src.resourceType;
    dst.objectType = src.objectType;
    dst.objectStatus = src.objectStatus;
    dst.objectDescription = src.objectDescription;
    dst.objectInfo = src.objectInfo;
}
