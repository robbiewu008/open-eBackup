/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: DWS task manager.
 * Create: 2023-10-22
 * Author: jwx966562
*/
#include "apps/dws/XBSAServer/DwsTaskManage.h"
#include "common/Path.h"
#include "apps/dws/XBSAServer/DwsTaskInfoParser.h"
#include "apps/dws/XBSAServer/BsaDb.h"
#include "apps/dws/XBSAServer/BsaMountManager.h"
#include "common/ConfigXmlParse.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"
#include "apps/dws/XBSACom/TSSLSocketFactoryPassword.h"

const mp_uint32 QUERY_LIMIT = 128;
const mp_int32 DEFAUL_MAX_DWS_HOST_NUM = 2048;

mp_string DwsTaskManage::GetDwsHostDbFilePath()
{
    return (m_cacheInfo.metaRepoPath + "/meta/dwsHosts.db");
}

mp_int32 DwsTaskManage::GetDwsHosts()
{
    mp_int32 maxNum = 0;
    mp_int32 ret = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_DWS_HOST_MAX_NUM, maxNum);
    if (ret != MP_SUCCESS || maxNum < DEFAUL_MAX_DWS_HOST_NUM) {
        WARNLOG("Get %s.%s cfg fail!maxNum=%d", CFG_BACKUP_SECTION.c_str(), CFG_DWS_HOST_MAX_NUM.c_str(), maxNum);
        maxNum = DEFAUL_MAX_DWS_HOST_NUM;
    }

    m_dwsHostMap.clear();
    INFOLOG("Max dws host num=%d.", maxNum);

    BsaQueryPageInfo pageInfo(QUERY_LIMIT, 0);
    BsaDb db(GetDwsHostDbFilePath());
    while (1) {
        std::vector<DwsHostInfo> hostList;
        if (db.QueryDwsHosts(pageInfo, hostList) != MP_SUCCESS) {
            ERRLOG("Query dws hosts failed.");
            return MP_FAILED;
        }
        INFOLOG("Query dws host success.size=%d.", hostList.size());
        for (const auto &iter : hostList) {
            FsKeyInfo fsInfo(iter.fsId, iter.fsName, iter.fsDeviceId);
            m_dwsHostMap[iter.hostname] = fsInfo;
            DBGLOG("Query result:hostname=%s,fsName=%s,fsDeviceId=%s.",
                iter.hostname.c_str(), iter.fsName.c_str(), iter.fsDeviceId.c_str());
        }
        if (m_dwsHostMap.size() > maxNum) {
            ERRLOG("Dws host num=%u more than maxNum=%d.", m_dwsHostMap.size(), maxNum);
            return MP_FAILED;
        }
        if (hostList.size() < QUERY_LIMIT) {
            break;
        }
        pageInfo.offset += hostList.size();
    }
    INFOLOG("Query all dws host success.size=%d.", m_dwsHostMap.size());
    return MP_SUCCESS;
}

mp_void DwsTaskManage::UpdateDwsHosts()
{
    for (auto iter = m_dwsHostMap.begin(); iter != m_dwsHostMap.end();) {
        if (!BsaMountManager::GetInstance().IsFsMounted(
            m_taskId, iter->second.fsDeviceId, iter->second.fsId, iter->second.fsName)) {
            WARNLOG("Host=%s did not mount success.oldDeviceId=%s,oldFsName=%s.",
                iter->first.c_str(), iter->second.fsDeviceId.c_str(), iter->second.fsName.c_str());
            m_dwsHostMap.erase(iter++);
        } else {
            DBGLOG("Hostname=%s,fsDeviceId=%s,fsName=%s,fsId=%s.", iter->first.c_str(),
                iter->second.fsDeviceId.c_str(), iter->second.fsName.c_str(), iter->second.fsId.c_str());
            ++iter;
        }
    }
}

mp_int32 DwsTaskManage::GetTaskInfoLockedInner()
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

    if (IsBackupTask() && GetDwsHosts() != MP_SUCCESS) {
        ERRLOG("Get dws hosts failed.");
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

    if (IsBackupTask()) {
        UpdateDwsHosts();
    }

    return MP_SUCCESS;
}

mp_int32 DwsTaskManage::UpdateTaskWhenCreateObject(const BsaObjectDescriptor &objDesc)
{
    BsaQueryDescriptor queryDesc;
    return UpdateTaskWhenQueryObject(queryDesc);
}
mp_int32 DwsTaskManage::UpdateTaskWhenQueryObject(const BsaQueryDescriptor &objDesc)
{
    mp_string cacheFilePath = CPath::GetInstance().GetStmpFilePath("dws_cacheInfo.txt");
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

mp_void DwsTaskManage::AllocFilesystem(BsaObjInfo &objInfo)
{
    mp_string hostname = ParseDwsHostname(objInfo.objectName);
    DBGLOG("Parsed objName=%s,hostname=%s.", objInfo.objectName.c_str(), hostname.c_str());

    if (hostname.empty()) {
        // hostname无法识别时临时分配一个文件系统
        BsaMountManager::GetInstance().AllocFilesystem(m_taskId, objInfo.fsDeviceId, objInfo.fsId, objInfo.fsName);
        return;
    }

    // 先从已分配的文件系统的主机列表中查询
    auto iter = m_dwsHostMap.find(hostname);
    if (iter != m_dwsHostMap.end()) {
        objInfo.fsId = iter->second.fsId;
        objInfo.fsName = iter->second.fsName;
        objInfo.fsDeviceId = iter->second.fsDeviceId;
    } else {
        // 未分配文件系统时为此主机分配一个新文件系统
        BsaMountManager::GetInstance().AllocFilesystem(m_taskId, objInfo.fsDeviceId, objInfo.fsId, objInfo.fsName);

        // 分配后同步更新到map表
        FsKeyInfo fsInfo(objInfo.fsId, objInfo.fsName, objInfo.fsDeviceId);
        m_dwsHostMap[hostname] = std::move(fsInfo);
        INFOLOG("Add new host.hostname=%s,fsDeviceId=%s,fsName=%s,fsId=%s.", hostname.c_str(),
                objInfo.fsDeviceId.c_str(), objInfo.fsName.c_str(), objInfo.fsId.c_str());
    }
}

// 从XBSA对象名称中解析出hostname，已知的路径格式有:
// /roach/20220723_183709/gauss200-211/dn_6001/data_colstore/file_0.rch
// /roach/20220809_120715_test_schema1.test_table2/dws-150/dn_6002/file_0.rch
mp_string DwsTaskManage::ParseDwsHostname(const mp_string &objName)
{
    if (objName.empty()) {
        return "";
    }
    if (!CMpString::EndsWith(objName, ".rch")) {
        return "";
    }

    std::vector<mp_string> strList;
    std::string token;
    std::istringstream tokenStream(objName);
    while (std::getline(tokenStream, token, '/')) {
        strList.push_back(token);
    }
    const int hostNamePos = 3;
    if (strList.size() > hostNamePos) {
        return strList[hostNamePos];
    }
    return "";
}