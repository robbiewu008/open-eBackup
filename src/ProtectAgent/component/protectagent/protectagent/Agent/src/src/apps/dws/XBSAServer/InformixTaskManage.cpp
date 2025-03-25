/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: app task manager.
 * Create: 2023-10-22
 * Author: jwx966562
*/
#include "apps/dws/XBSAServer/InformixTaskManage.h"
#include "common/Path.h"
#include "common/Utils.h"
#include "apps/dws/XBSAServer/DwsTaskInfoParser.h"
#include "apps/dws/XBSAServer/BsaDb.h"
#include "apps/dws/XBSAServer/BsaMountManager.h"
#include "common/ConfigXmlParse.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"
#include "apps/dws/XBSACom/TSSLSocketFactoryPassword.h"


namespace {
    const mp_string IIF_APP = "informix";
    const mp_string TASK_TYPE_BACKUP = "backup";
    const mp_string TASK_TYPE_RESTORE = "restore";
}

mp_int32 InformixTaskManage::UpdateTaskWhenCreateObject(const BsaObjectDescriptor &objDesc)
{
    LOGGUARD("");
    INFOLOG("Input parameter pathName: %s, resourceType: %s, rsv2: %s.", objDesc.objectName.pathName.c_str(),
        objDesc.resourceType.c_str(), objDesc.rsv2.c_str());
    mp_string instanceName;
    if (GetInstanceName(objDesc.objectName.pathName, instanceName)) {
        return MP_FAILED;
    }
    mp_string dstInstanceName = objDesc.rsv2;
    mp_string resourceType = objDesc.resourceType == "L" ? "log":"data";
    mp_string file_name = IIF_APP + "_" + instanceName + "_" + dstInstanceName +"_"
        + TASK_TYPE_BACKUP + "_" + resourceType + ".txt";
    mp_string cacheFilePath = CPath::GetInstance().GetStmpFilePath(file_name);
    DwsTaskInfoParser parser;
    if (parser.ParseCacheInfo(m_cacheInfo, cacheFilePath) != MP_SUCCESS) {
        ERRLOG("Parse cache info failed.");
        return MP_FAILED;
    }

    if (m_taskId != m_cacheInfo.taskId) {
        INFOLOG("Task changed!old taskId=%s,new taskId=%s.cacheRepoPath=%s,metaRepoPath=%s,copyId=%s,hostKey=%s.",
            m_taskId.c_str(), m_cacheInfo.taskId.c_str(), m_cacheInfo.cacheRepoPath.c_str(),
            m_cacheInfo.metaRepoPath.c_str(), m_cacheInfo.copyId.c_str(), m_cacheInfo.hostKey.c_str());
        m_taskId = m_cacheInfo.taskId;
        if (GetTaskInfoLockedInner() != MP_SUCCESS) {
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 InformixTaskManage::UpdateTaskWhenQueryObject(const BsaQueryDescriptor &objDesc)
{
    LOGGUARD("");
    INFOLOG("Input parameter pathName: %s, rsv5: %s.", objDesc.objectName.pathName.c_str(), objDesc.rsv5.c_str());
    std::vector<mp_string> vecTokens;
    CMpString::StrSplit(vecTokens, objDesc.objectName.pathName, '/');
    if (vecTokens.size() < AWK_COL_FIRST_3) {
        ERRLOG("The format is incorrect, pathName: %s.", objDesc.objectName.pathName.c_str());
        return MP_FAILED;
    }
    mp_string instanceName = vecTokens[AWK_COL_FIRST_1];
    mp_string dstInstanceName = objDesc.rsv5;
    mp_string resourceType = CMpString::IsDigits(vecTokens[AWK_COL_FIRST_2]) ? "log":"data";
    mp_string file_name = IIF_APP+ "_" + instanceName + "_" + dstInstanceName
        + "_"+ TASK_TYPE_RESTORE + "_" + resourceType + ".txt";
    mp_string cacheFilePath = CPath::GetInstance().GetStmpFilePath(file_name);
    DwsTaskInfoParser parser;
    if (parser.ParseCacheInfo(m_cacheInfo, cacheFilePath) != MP_SUCCESS) {
        ERRLOG("Parse cache info failed.");
        return MP_FAILED;
    }

    if (m_taskId != m_cacheInfo.taskId) {
        INFOLOG("Task changed!old taskId=%s,new taskId=%s.cacheRepoPath=%s,metaRepoPath=%s,copyId=%s,hostKey=%s.",
            m_taskId.c_str(), m_cacheInfo.taskId.c_str(), m_cacheInfo.cacheRepoPath.c_str(),
            m_cacheInfo.metaRepoPath.c_str(), m_cacheInfo.copyId.c_str(), m_cacheInfo.hostKey.c_str());
        m_taskId = m_cacheInfo.taskId;
        if (GetTaskInfoLockedInner() != MP_SUCCESS) {
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_void InformixTaskManage::AllocFilesystem(BsaObjInfo &objInfo)
{
    LOGGUARD("");
    INFOLOG("objectName: %s, resourceType: %s.", objInfo.objectName.c_str(), objInfo.resourceType.c_str());
    BsaMountManager::GetInstance().AllocFilesystem(m_taskId, objInfo.fsDeviceId, objInfo.fsId, objInfo.fsName);
}

mp_bool InformixTaskManage::FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, QueryObjectResult &rsp)
{
    mp_string mountPath = BsaMountManager::GetInstance().GetMountPath(m_taskId,
                                                                      queryReslt.fsDeviceId,
                                                                      queryReslt.fsName);
    if (mountPath.empty()) {
        ERRLOG("Get MountPath fail! bsaHandle=%lld, fsDeviceId(%s), FsName(%s), taskId(%s)",
            bsaHandle, queryReslt.fsDeviceId.c_str(), queryReslt.fsName.c_str(), m_taskId.c_str());
        return MP_FALSE;
    }
    rsp.storePath = mountPath + "/" + queryReslt.storePath;
    rsp.getDataType = BSA_GET_DATA_FROM_NAS;
    return MP_TRUE;
}

mp_bool InformixTaskManage::FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, GetNextQueryObjectResult &rsp)
{
    mp_string mountPath = BsaMountManager::GetInstance().GetMountPath(m_taskId,
                                                                      queryReslt.fsDeviceId,
                                                                      queryReslt.fsName);
    if (mountPath.empty()) {
        ERRLOG("Get MountPath fail! bsaHandle=%lld, fsDeviceId(%s), FsName(%s), taskId(%s)",
            bsaHandle, queryReslt.fsDeviceId.c_str(), queryReslt.fsName.c_str(), m_taskId.c_str());
        return MP_FALSE;
    }
    rsp.storePath = mountPath + "/" + queryReslt.storePath;
    rsp.getDataType = BSA_GET_DATA_FROM_NAS;
    return MP_TRUE;
}

mp_int32 InformixTaskManage::GetTaskInfoLockedInner()
{
    LOGGUARD("");
    DwsTaskInfoParser parser;
    if (parser.ParseTaskInfo(m_cacheInfo, m_taskInfo) != MP_SUCCESS) {
        ERRLOG("Parse task info failed.");
        return MP_FAILED;
    }

    if (IsBackupTask() && CreateBsaDb() != MP_SUCCESS) {
        ERRLOG("Create bsa db file or db table failed.");
        return MP_FAILED;
    }

    BsaMountManager::GetInstance().SetRepository(m_cacheInfo.taskId, m_taskInfo.repositories);
    return MP_SUCCESS;
}

mp_int32 InformixTaskManage::GetInstanceName(const mp_string& pathName, mp_string& instanceName)
{
    std::vector<mp_string> vecTokens;
    CMpString::StrSplit(vecTokens, pathName, '/');
    if (vecTokens.size() < AWK_COL_FIRST_3) {
        ERRLOG("The format is incorrect, pathName: %s.", pathName.c_str());
        return MP_FAILED;
    }
    instanceName = vecTokens[AWK_COL_FIRST_1];
    INFOLOG("instanceName: %s.", instanceName.c_str());
    return MP_SUCCESS;
}
