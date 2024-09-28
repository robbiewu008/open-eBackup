#include "apps/dws/XBSAServer/AppTaskManage.h"
#include "common/Path.h"
#include "apps/dws/XBSAServer/DwsTaskInfoParser.h"
#include "apps/dws/XBSAServer/BsaDb.h"
#include "apps/dws/XBSAServer/BsaMountManager.h"
#include "common/ConfigXmlParse.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"
#include "apps/dws/XBSACom/TSSLSocketFactoryPassword.h"

bool AppTaskManage::IsBackupCopy(mp_uint32 copyType)
{
    return (copyType == CopyDataType::type::FULL_COPY ||
            copyType == CopyDataType::type::INCREMENT_COPY ||
            copyType == CopyDataType::type::DIFF_COPY);
}

bool AppTaskManage::IsReplicationCopy(mp_uint32 copyType)
{
    return (copyType == CopyDataType::type::REPLICATION_COPY);
}

bool AppTaskManage::IsCloudArchiveCopy(mp_uint32 copyType)
{
    return (copyType == CopyDataType::type::CLOUD_STORAGE_COPY);
}

bool AppTaskManage::IsTapeArchiveCopy(mp_uint32 copyType)
{
    return (copyType == CopyDataType::type::TAPE_STORAGE_COPY);
}

mp_bool AppTaskManage::IsBackupTask()
{
    return (m_taskInfo.taskType == static_cast<mp_uint32>(DwsTaskType::BACKUP));
}
mp_bool AppTaskManage::IsRestoreTask()
{
    return (m_taskInfo.taskType == static_cast<mp_uint32>(DwsTaskType::RESTORE));
}

mp_string AppTaskManage::GetBsaDbFilePath()
{
    mp_string dbFile = m_cacheInfo.metaRepoPath + "/meta/" + m_cacheInfo.copyId + "/objectmeta/"
        + m_cacheInfo.hostKey + "/" + m_cacheInfo.hostKey + ".db";
    DBGLOG("Db file path:%s", dbFile.c_str());
    return dbFile;
}

mp_int32 AppTaskManage::CreateBsaDb()
{
    mp_string dbFile = GetBsaDbFilePath();
    if (dbFile.empty()) {
        ERRLOG("Db file empty!");
        return MP_FAILED;
    }

    if (CMpFile::CreateFile(dbFile) != MP_SUCCESS) {
        ERRLOG("Create db file failed!dbFile=%s", dbFile.c_str());
        return MP_FAILED;
    }

    BsaDb db(dbFile);
    if (db.CreateBsaObjTable() != MP_SUCCESS) {
        ERRLOG("Create db table failed!dbFile=%s", dbFile.c_str());
        return MP_FAILED;
    }

    INFOLOG("Create db file success.dbFile=%s.", dbFile.c_str());
    return MP_SUCCESS;
}

mp_int32 AppTaskManage::GetFsRelation()
{
    INFOLOG("Begin GetFsRelation");
    DwsFsRelation relation;
    m_fsRelationMap.clear();
    if (ParseFsRelation(relation)) {
        ERRLOG("Parse filesystem relationship failed.");
        return MP_FAILED;
    }

    for (const auto &iter : relation.relations) {
        FsKeyInfo oldFs(iter.oldFsId, iter.oldFsName, iter.oldEsn);
        FsKeyInfo newFs(iter.newFsId, iter.newFsName, iter.newEsn);
        m_fsRelationMap[oldFs] = std::move(newFs);
    }
    INFOLOG("GetFsRelation succ");
    return MP_SUCCESS;
}

mp_int32 AppTaskManage::ParseFsRelation(DwsFsRelation &relation)
{
    INFOLOG("CopyType=%u.", m_taskInfo.copyType);
    DwsTaskInfoParser parser;
    if (IsBackupCopy(m_taskInfo.copyType) || IsReplicationCopy(m_taskInfo.copyType)) {
        mp_string filePath = m_cacheInfo.cacheRepoPath + "/meta/" + m_cacheInfo.copyId
            + "/replication/filesystemRelationship.txt";
        return parser.ParseFsRelation(filePath, relation);
    } else if (IsCloudArchiveCopy(m_taskInfo.copyType)) {
        mp_string filePath = m_cacheInfo.cacheRepoPath + "/meta/" + m_cacheInfo.copyId
            + "/replication/filesystemRelationship.txt";
        if (CMpFile::FileExist(filePath)) {
            INFOLOG("Relationship file exist.");
            return parser.ParseFsRelation(filePath, relation);
        }
    } else if (IsTapeArchiveCopy(m_taskInfo.copyType)) {
        mp_string filePath = m_cacheInfo.metaRepoPath + "/meta/" + m_cacheInfo.copyId
            + "/archiveDownload/filesystemRelationship.txt";
        return parser.ParseFsRelation(filePath, relation);
    }
    return MP_SUCCESS;
}

mp_bool AppTaskManage::TransFilesystem(const BsaObjInfo &queryReslt, FsKeyInfo &newFsInfo)
{
    if (IsBackupCopy(m_taskInfo.copyType) ||
        IsReplicationCopy(m_taskInfo.copyType) ||
        IsTapeArchiveCopy(m_taskInfo.copyType)) {
        if (!GetNewFsInRelationMap(queryReslt, newFsInfo)) {
            ERRLOG("Not find fs in relation map.fsDeviceId(%s),fsName(%s)",
                queryReslt.fsDeviceId.c_str(), queryReslt.fsName.c_str());
            return false;
        }
        return true;
    }

    ERRLOG("Unkown copyType=%u.", m_taskInfo.copyType);
    return false;
}

mp_bool AppTaskManage::GetNewFsInRelationMap(const BsaObjInfo &queryReslt, FsKeyInfo &newFsInfo)
{
    FsKeyInfo key(queryReslt.fsId, queryReslt.fsName, queryReslt.fsDeviceId);
    auto iter = m_fsRelationMap.find(key);
    if (iter == m_fsRelationMap.end()) {
        return false;
    }
    newFsInfo.fsId = iter->second.fsId;
    newFsInfo.fsDeviceId = iter->second.fsDeviceId;
    newFsInfo.fsName = iter->second.fsName;
    return true;
}

// 从1.2.1版本开始，归档模块适配多备份存储集群将fsId字段扩展为esn_fsId格式.
mp_string AppTaskManage::GetCloudCopyFsId(const BsaObjInfo &queryReslt)
{
    FsKeyInfo newFsInfo;
    if (!GetNewFsInRelationMap(queryReslt, newFsInfo)) { // 备份副本归档到云
        return queryReslt.fsDeviceId + "_" + queryReslt.fsId;
    }
    DBGLOG("Find filesystem in relation map.");
    return newFsInfo.fsDeviceId + "_" + newFsInfo.fsId; // 复制副本归档到云
}

mp_string AppTaskManage::GetArchiveServerIp()
{
    // ipList格式：ip1,ip2,ip3...
    // 解析任务参数时已经对fileServers数组大小做了校验，不会为空
    mp_string ipList = m_taskInfo.fileServers[0].ip;
    for (size_t i = 1; i < m_taskInfo.fileServers.size(); i++) {
        ipList += "," + m_taskInfo.fileServers[i].ip;
    }
    return ipList;
}

template<typename T>
mp_bool AppTaskManage::FillQuryRspCommon(mp_long bsaHandle, const BsaObjInfo &queryReslt, T &rsp)
{
    if (!IsCloudArchiveCopy(m_taskInfo.copyType)) {
        bool isRestore = false;
        DwsTaskInfoParser parser;
        XbsaBusinessConfig busConfig;
        if (parser.ParseBusConfig(m_cacheInfo.cacheRepoPath, m_cacheInfo.copyId, busConfig) != MP_SUCCESS) {
            ERRLOG("Fail to read business config.Default value: full restore.");
            return MP_FALSE;
        }
        if ((busConfig.jobType == static_cast<mp_uint32>(XbsaJobType::FULL_RESTORE)) ||
            (busConfig.jobType == static_cast<mp_uint32>(XbsaJobType::DIFF_RESTORE))) {
            isRestore = true;
        }

        FsKeyInfo newFs(queryReslt.fsId, queryReslt.fsName, queryReslt.fsDeviceId);
        if (isRestore && !TransFilesystem(queryReslt, newFs)) {
            ERRLOG("Get transfered filesystem name failed.bsaHandle=%lld,fsDeviceId(%s),fsName(%s)", bsaHandle,
                queryReslt.fsDeviceId.c_str(), queryReslt.fsName.c_str());
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

mp_bool AppTaskManage::FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, QueryObjectResult &rsp)
{
    return FillQuryRspCommon(bsaHandle, queryReslt, rsp);
}
mp_bool AppTaskManage::FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, GetNextQueryObjectResult &rsp)
{
    return FillQuryRspCommon(bsaHandle, queryReslt, rsp);
}

const DwsCacheInfo &AppTaskManage::GetCacheInfo()
{
    return m_cacheInfo;
}

mp_string AppTaskManage::GetTaskId()
{
    return m_taskId;
}

void AppTaskManage::GenStorePath(BsaObjInfo &objInfo)
{
    if (objInfo.objectName.front() == '/') {
        objInfo.storePath = "/data/" + m_cacheInfo.copyId + "/" + m_cacheInfo.hostKey + objInfo.objectName;
    } else {
        objInfo.storePath = "/data/" + m_cacheInfo.copyId + "/" + m_cacheInfo.hostKey + "/" + objInfo.objectName;
    }
}
