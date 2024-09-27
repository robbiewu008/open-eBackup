#include "apps/dws/XBSAServer/HcsTaskManage.h"
#include <vector>
#include "common/Path.h"
#include "apps/dws/XBSAServer/DwsTaskInfoParser.h"
#include "apps/dws/XBSAServer/BsaDb.h"
#include "apps/dws/XBSAServer/BsaMountManager.h"
#include "common/ConfigXmlParse.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"
#include "apps/dws/XBSACom/TSSLSocketFactoryPassword.h"

mp_int32 HcsTaskManage::GetTaskInfoLockedInner()
{
    DwsTaskInfoParser parser;
    if (parser.ParseTaskInfo(m_cacheInfo, m_taskInfo) != MP_SUCCESS) {
        ERRLOG("Parse task info failed.");
        return MP_FAILED;
    }

    if (IsBackupTask() && CreateBsaDb() != MP_SUCCESS) {
        ERRLOG("Create bsa db file or db table failed.");
        return MP_FAILED;
    }

    if (IsRestoreTask() && GetFsRelation() != MP_SUCCESS) {
        ERRLOG("Parse filesystem relations failed.");
        return MP_FAILED;
    }

    // 归档到云副本走TCP恢复，不使用挂载文件系统
    if (!IsCloudArchiveCopy(m_taskInfo.copyType)) {
        INFOLOG("Set SetRepository");
        BsaMountManager::GetInstance().SetRepository(m_cacheInfo.taskId, m_taskInfo.repositories);
    }

    return MP_SUCCESS;
}

mp_int32 HcsTaskManage::UpdateTaskWhenCreateObject(const BsaObjectDescriptor &objDesc)
{
    BsaQueryDescriptor queryDesc;
    queryDesc.objectName.pathName = objDesc.objectName.pathName;
    return UpdateTaskWhenQueryObject(queryDesc);
}
mp_int32 HcsTaskManage::UpdateTaskWhenQueryObject(const BsaQueryDescriptor &objDesc)
{
    mp_string hcsPathName = objDesc.objectName.pathName;
    INFOLOG("Update task when query object, %s.", hcsPathName.c_str());
    std::vector<mp_string> pathInfovec;
    CMpString::StrSplit(pathInfovec, hcsPathName, '/');
    mp_int32 size = pathInfovec.size();
    static const mp_int32 pathInfoparamsminsize = 2;
    if (size < pathInfoparamsminsize) {
        ERRLOG("hcsPathName param split string failed, %s.", hcsPathName.c_str());
        return MP_FAILED;
    }
    mp_string instanceId;
    if (hcsPathName.substr(0, 1) == "/") {
        // 全量增量备份时，路径格式为/{instance_id}/xxx
        instanceId = pathInfovec[1];
    } else {
        // 日志备份时，路径格式为{instance_id}/xxx/xxx
        instanceId = pathInfovec[0];
    }
    mp_string hcsCacheInfoFile = "";
    hcsCacheInfoFile = "xbsa_cacheInfo_info_hcs_" + instanceId + ".txt";
    INFOLOG("BSA cache info file name, %s.", hcsCacheInfoFile.c_str());
    mp_string cacheFilePath = CPath::GetInstance().GetStmpFilePath(hcsCacheInfoFile);
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

mp_void HcsTaskManage::AllocFilesystem(BsaObjInfo &objInfo)
{
    // 未分配文件系统时为此主机分配一个新文件系统
    BsaMountManager::GetInstance().AllocFilesystem(m_taskId, objInfo.fsDeviceId, objInfo.fsId, objInfo.fsName);

    INFOLOG("Add new host.fsDeviceId=%s,fsName=%s,fsId=%s.",
            objInfo.fsDeviceId.c_str(), objInfo.fsName.c_str(), objInfo.fsId.c_str());
}

mp_bool HcsTaskManage::FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, QueryObjectResult &rsp)
{
    return FillQuryRspCommonHcs(bsaHandle, queryReslt, rsp);
}
mp_bool HcsTaskManage::FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, GetNextQueryObjectResult &rsp)
{
    return FillQuryRspCommonHcs(bsaHandle, queryReslt, rsp);
}

template<typename T>
mp_bool HcsTaskManage::FillQuryRspCommonHcs(mp_long bsaHandle, const BsaObjInfo &queryReslt, T &rsp)
{
    if (!IsCloudArchiveCopy(m_taskInfo.copyType)) {
        FsKeyInfo newFs(queryReslt.fsId, queryReslt.fsName, queryReslt.fsDeviceId);
        bool isRestore = false;
        DwsTaskInfoParser parser;
        XbsaBusinessConfig busConfig;

        if (parser.ParseBusConfig(m_cacheInfo.cacheRepoPath, m_cacheInfo.copyId, busConfig) != MP_SUCCESS) {
            ERRLOG("Fail to read business config.Default value: full restore.");
            return MP_FAILED;
        }
        if ((busConfig.jobType == static_cast<mp_uint32>(XbsaJobType::FULL_RESTORE)) ||
            (busConfig.jobType == static_cast<mp_uint32>(XbsaJobType::DIFF_RESTORE))) {
            isRestore = true;
        }
        if (isRestore && !TransFilesystem(queryReslt, newFs)) {
            ERRLOG("Get transfered filesystem name failed.bsaHandle=%lld,fsDeviceId(%s),fsName(%s)",
                bsaHandle, queryReslt.fsDeviceId.c_str(), queryReslt.fsName.c_str());
            return MP_FALSE;
        }

        DBGLOG("Transfer filesystem success.bsaHandle=%lld,newFsName(%s),oldFsName(%s)",
            bsaHandle, newFs.fsName.c_str(), queryReslt.fsName.c_str());
        mp_string mountPath = BsaMountManager::GetInstance().GetMountPath(m_taskId, newFs.fsDeviceId, newFs.fsName);
        if (mountPath.empty()) {
            ERRLOG("Get MountPath fail! bsaHandle=%lld,fsDeviceId(%s),newFsName(%s),oldFsName(%s), taskId(%s)",
                bsaHandle, newFs.fsDeviceId.c_str(), newFs.fsName.c_str(), queryReslt.fsName.c_str(), m_taskId.c_str());
            return MP_FALSE;
        }
        rsp.storePath = mountPath + "/" + queryReslt.storePath;
        rsp.getDataType = BSA_GET_DATA_FROM_NAS;
    } else {
        rsp.storePath = queryReslt.storePath;
        rsp.archiveBackupId = m_cacheInfo.copyId;
        rsp.fsID = GetCloudCopyFsId(queryReslt);
        rsp.archiveServerIp = GetArchiveServerIp();
        rsp.archiveServerPort = m_taskInfo.fileServers[0].port;
        rsp.archiveOpenSSL = m_taskInfo.fileServers[0].sslEnabled;
        rsp.getDataType = BSA_GET_DATA_FROM_ARCHIVE;
        DBGLOG("BackupId:%s. fsId:%s. Ip:%s. Port:%d. OpenSSL:%d.", rsp.archiveBackupId.c_str(), rsp.fsID.c_str(),
            rsp.archiveServerIp.c_str(), rsp.archiveServerPort, rsp.archiveOpenSSL);
    }

    return MP_TRUE;
}
