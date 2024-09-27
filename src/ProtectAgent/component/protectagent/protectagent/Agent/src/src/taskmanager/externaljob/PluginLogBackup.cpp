#include "taskmanager/externaljob/PluginLogBackup.h"

#include <fstream>
#include "common/Log.h"
#include "common/MpString.h"
#include "common/JsonUtils.h"
#include "apps/appprotect/CommonDef.h"
#include "taskmanager/externaljob/JobStateDB.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"

using namespace std;
namespace {
    const mp_string AGENT_LAST_LOG_BACKUP = ".agentlastlogbackup.meta";
    const mp_string DME_LAST_LOG_BACKUP = ".dmelastlogbackup.meta";
    const mp_string LOG_BACKUP = "logBackup";
    const mp_string LATEST_DATA_COPY_ID = "latestDataCopyId";
    const mp_string LATEST_LOG_COPY_NAME = "latestLogCopyName";
    const mp_string TYPE = "type";
    const mp_string PATH = "path";
    const mp_string BEGIN_TIME = "beginTime";
    const mp_string END_TIME = "endTime";
    const mp_string BEGIN_SCN = "beginSCN";
    const mp_string END_SCN = "endSCN";
    const mp_string LOG_DIR_NAME = "logDirName";
    const mp_string ASSOCIATED_COPIES = "associatedCopies";
    const mp_string META_DIR = "meta";
}

namespace AppProtect {
mp_int32 PluginLogBackup::LogBackup(PluginJobData &data)
{
    vector<Json::Value> repositories;
    GET_JSON_ARRAY_JSON(data.param, REPOSITORIES, repositories);
    if (repositories.empty()) {
        ERRLOG("Log backup repositories is empty, jobId=%s.", data.mainID.c_str());
        return MP_FAILED;
    }
    auto it = data.mountPoints.find(RepositoryDataType::type::LOG_REPOSITORY);
    if (it == data.mountPoints.end()) {
        ERRLOG("Get log repository mount path failed, jobId=%s.", data.mainID.c_str());
        return MP_FAILED;
    }
    m_logRepoMountPoint = it->second.front();
    if (data.IsSanClientMount()) {
        m_logRepoMountPoint = m_logRepoMountPoint.substr(0, m_logRepoMountPoint.find('#'));
    }
#ifdef WIN32
    m_logRepoMountPoint = m_logRepoMountPoint.substr(0, m_logRepoMountPoint.find('&'));
#endif
    mp_string agentLastLogBackupMetaPath = m_logRepoMountPoint + PATH_SEPARATOR + AGENT_LAST_LOG_BACKUP;
    vector<mp_string> vecOutput;
    if (ReadMetaData(agentLastLogBackupMetaPath, data.mainID, vecOutput) != MP_SUCCESS) {
        ERRLOG("Failed to read %s, jobId=%s.", AGENT_LAST_LOG_BACKUP.c_str(), data.mainID.c_str());
        return MP_FAILED;
    }

    mp_string latestDataCopyId;
    mp_string latestLogCopyName;
    mp_int32 ret = GetExtendInfo(data, latestDataCopyId, latestLogCopyName);
    if (ret != MP_SUCCESS) {
        ERRLOG("Failed to get extendinfo, jobId=%s, ret : %d.", data.mainID.c_str(), ret);
        return ret;
    }
    if (CreateLogDir(vecOutput, latestDataCopyId, latestLogCopyName, data) != MP_SUCCESS) {
        ERRLOG("Failed to create tmp log director, jobId= : %s.", data.mainID.c_str());
        return MP_FAILED;
    }

    mp_string dmeLastLogBackupMetaPath = m_logRepoMountPoint + PATH_SEPARATOR + DME_LAST_LOG_BACKUP;
    ret = WriteDMELastBackupMeta(dmeLastLogBackupMetaPath, latestDataCopyId, latestLogCopyName);
    if (ret != MP_SUCCESS) {
        ERRLOG("Failed to write dme last backup meta, jobId=%s, ret : %d.", data.mainID.c_str(), ret);
        return ret;
    }
   
    return MP_SUCCESS;
}

mp_int32 PluginLogBackup::CreateLogDir(const vector<mp_string> &vecOutput, const mp_string &latestDataCopyId,
    const mp_string &latestLogCopyName, const PluginJobData &data)
{
    if (vecOutput.empty() || (latestLogCopyName == vecOutput.back() && latestDataCopyId == vecOutput.front()) ||
        latestLogCopyName.empty()) {
        return NormalBackup(data);
    } else {
        return CrossDataCopyBackup(data);
    }
    return MP_SUCCESS;
}

mp_int32 PluginLogBackup::NormalBackup(const PluginJobData &data)
{
    mp_string tmpLogBackupDir = m_logRepoMountPoint + PATH_SEPARATOR + data.mainID;
    if (!CMpFile::DirExist(tmpLogBackupDir.c_str())) {
        ERRLOG("Dir not exist %s, jobId=%s.", tmpLogBackupDir.c_str(), data.mainID.c_str());
        return MP_FAILED;
    }
    mp_string agentLastLogBackupMetaPath = m_logRepoMountPoint + PATH_SEPARATOR + AGENT_LAST_LOG_BACKUP;
    std::vector<mp_string> emptyVec;
    if (WriteToFile(agentLastLogBackupMetaPath, emptyVec) != MP_SUCCESS) {
        ERRLOG("Failed to empty %s file, jobId=%s.", AGENT_LAST_LOG_BACKUP.c_str(), data.mainID.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 PluginLogBackup::CrossDataCopyBackup(const PluginJobData &data)
{
    mp_string tmpLogBackupDir = m_logRepoMountPoint + PATH_SEPARATOR + data.mainID;
    if (!CMpFile::DirExist(tmpLogBackupDir.c_str())) {
        ERRLOG("Dir not exist %s, jobId=%s.", tmpLogBackupDir.c_str(), data.mainID.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:ReadMetaData
Description  :读取元数据文件
------------------------------------------------------------- */
mp_int32 PluginLogBackup::ReadMetaData(mp_string &lastLogBackupMetaPath, const mp_string &mainID,
    vector<mp_string> &vecOutput)
{
    if (!CMpFile::FileExist(lastLogBackupMetaPath)) {
        if (CMpFile::CreateFile(lastLogBackupMetaPath) != MP_SUCCESS) {
            ERRLOG("Failed to create %s.", lastLogBackupMetaPath.c_str());
            return MP_FAILED;
        }
    }
    if (CMpFile::ReadFile(lastLogBackupMetaPath, vecOutput) != MP_SUCCESS) {
        ERRLOG("Failed to read %s file.", lastLogBackupMetaPath.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:WriteDMELastBackupMetaData
Description  :记录DME下发数据副本ID和日志目录元数据文件
------------------------------------------------------------- */
mp_int32 PluginLogBackup::WriteDMELastBackupMeta(mp_string &lastLogBackupPath, const mp_string &latestDataCopyId,
    const mp_string &latestLogCopyName)
{
    if (!CMpFile::FileExist(lastLogBackupPath)) {
        if (CMpFile::CreateFile(lastLogBackupPath) != MP_SUCCESS) {
            ERRLOG("Failed to create %s.", lastLogBackupPath.c_str());
            return MP_FAILED;
        }
    }
    vector<mp_string> vecInput;
    vecInput.push_back(latestDataCopyId);
    vecInput.push_back(latestLogCopyName);
    if (WriteToFile(lastLogBackupPath, vecInput) != MP_SUCCESS) {
        ERRLOG("Failed to write %s file", DME_LAST_LOG_BACKUP.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:IsLogBackupJob
Description  :是否日志备份
------------------------------------------------------------- */
mp_bool PluginLogBackup::IsLogBackupJob(const PluginJobData &data)
{
    if (data.mainType != MainJobType::BACKUP_JOB) {
        return MP_FALSE;
    }
    BackupJob jobParam;
    JsonToStruct(data.param, jobParam);
    if (jobParam.jobParam.backupType == BackupJobType::LOG_BAKCUP) {
        return MP_TRUE;
    }
    return MP_FALSE;
}

/* ------------------------------------------------------------
Function Name:CreateMetedata
Description  :获取DME下发的扩展信息内容中数据副本ID和日志目录名
------------------------------------------------------------- */
mp_int32 PluginLogBackup::GetExtendInfo(const PluginJobData &data, mp_string &latestDataCopyId,
    mp_string &latestLogCopyName)
{
    vector<Json::Value> repositories;
    GET_JSON_ARRAY_JSON(data.param, REPOSITORIES, repositories);
    Json::Value tmpRepositorie;
    for (auto iterRep : repositories) {
        if (!iterRep.isObject() || !iterRep.isMember(TYPE)) {
            ERRLOG("Repositorie no type key value, jobId=%s.", data.mainID.c_str());
            return MP_FAILED;
        }
        if (iterRep[TYPE] == RepositoryDataType::type::LOG_REPOSITORY) {
            tmpRepositorie = iterRep;
        }
    }
    if (!tmpRepositorie.isMember(EXTENDINFO)) {
        ERRLOG("Json repositories have no extendInfo key, jobId=%s.", data.mainID.c_str());
        return MP_FAILED;
    }
    if (tmpRepositorie[EXTENDINFO].isNull()) {
        ERRLOG("Json repositories have no extendInfo value, jobId=%s.", data.mainID.c_str());
        return MP_FAILED;
    }
    Json::Value extendInfo = tmpRepositorie[EXTENDINFO];
    if (!extendInfo.isMember(LOG_BACKUP)) {
        ERRLOG("Json extendInfo have no LogBackup key, jobId=%s.", data.mainID.c_str());
        return MP_FAILED;
    }
    Json::Value logBackup = extendInfo[LOG_BACKUP];
    if (!logBackup.isMember(LATEST_DATA_COPY_ID) || !logBackup[LATEST_DATA_COPY_ID].isString()) {
        ERRLOG("Json extendInfo have no LatestDataCopyId key or is not string, jobId=%s.", data.mainID.c_str());
        return MP_FAILED;
    }
    latestDataCopyId = logBackup[LATEST_DATA_COPY_ID].asString();
    if (!logBackup.isMember(LATEST_LOG_COPY_NAME)) {
        latestLogCopyName = "";
        return MP_SUCCESS;
    }
    if (!logBackup[LATEST_LOG_COPY_NAME].isString()) {
        ERRLOG("Json extendInfo LatestLogCopyName is not string, jobId=%s.", data.mainID.c_str());
        return MP_FAILED;
    }
    latestLogCopyName = logBackup[LATEST_LOG_COPY_NAME].asString();
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:AssembleCopyInfo
Description  :日志备份后置任务，组装上报DME的日志副本信息
------------------------------------------------------------- */
mp_int32 PluginLogBackup::AssembleCopyInfo(const mp_string &jobId, Json::Value &jobValue)
{
    AppProtectJobHandler* appProtectJobHandlerInstance = AppProtectJobHandler::GetInstance();
    if (appProtectJobHandlerInstance == nullptr) {
        ERRLOG("new AppProtectJobHandler failed.");
        return MP_FAILED;
    }
    auto job = appProtectJobHandlerInstance->GetPostJobByMainId(jobId);
    if (!job) {
        ERRLOG("Failed to get post job info, jobId=%s.", jobId.c_str());
        return MP_FAILED;
    }
    if (IsLogBackupJob(job->GetData())) {
        auto it = job->GetData().mountPoints.find(RepositoryDataType::type::LOG_REPOSITORY);
        if (it == job->GetData().mountPoints.end()) {
            ERRLOG("Get log repository mount path failed, jobId=%s.", jobId.c_str());
            return MP_FAILED;
        }
        mp_string mountPath = it->second.front();
        if (job->GetData().IsSanClientMount()) {
            mountPath = mountPath.substr(0, mountPath.find('#'));
        }
#ifdef WIN32
        mountPath = mountPath.substr(0, mountPath.find('&'));
#endif
        mp_string agentLastLogBackupPath = mountPath + PATH_SEPARATOR + AGENT_LAST_LOG_BACKUP;
        mp_string dmeLastLogBackupPath = mountPath + PATH_SEPARATOR + DME_LAST_LOG_BACKUP;
        vector<mp_string> vecDmeLastBackupMeta;
        if (ReadMetaData(dmeLastLogBackupPath, jobId, vecDmeLastBackupMeta) != MP_SUCCESS) {
            ERRLOG("Failed to read %s, jobId=%s.", dmeLastLogBackupPath.c_str(), jobId.c_str());
            return MP_FAILED;
        }
        vector<mp_string> vecAgentLastBackupMeta;
        if (ReadMetaData(agentLastLogBackupPath, jobId, vecAgentLastBackupMeta) != MP_SUCCESS) {
            ERRLOG("Failed to read %s, jobId=%s.", agentLastLogBackupPath.c_str(), jobId.c_str());
            return MP_FAILED;
        }
        return AssembleCopyInfo_Ex(vecAgentLastBackupMeta, vecDmeLastBackupMeta, mountPath, jobId, jobValue);
    }
    return MP_SUCCESS;
}

mp_int32 PluginLogBackup::AssembleCopyInfo_Ex(const vector<mp_string> &vecAgentLastBackupMeta,
    const vector<mp_string> &vecDmeLastBackupMeta, const mp_string &mountPath, const mp_string &mainID,
    Json::Value &jobValue)
{
    if (!jobValue.isObject() || !jobValue.isMember(EXTENDINFO)) {
        ERRLOG("Json copy have no extendInfo key, jobId=%s.", mainID.c_str());
        return MP_FAILED;
    }
    set<mp_string> associatedCopiesSet;
    if (jobValue[EXTENDINFO].isObject() && jobValue[EXTENDINFO].isMember(ASSOCIATED_COPIES)) {
        Json::Value assCopyVal = jobValue[EXTENDINFO][ASSOCIATED_COPIES];
        mp_int32 assCopySize = assCopyVal.size();
        for (Json::ArrayIndex index = 0; index < assCopySize; ++index) {
            associatedCopiesSet.insert(assCopyVal[index].asString());
        }
    }
    Json::Value extendInfo = jobValue[EXTENDINFO];
    if (vecDmeLastBackupMeta.empty()) {
        ERRLOG("Dme last backup meta is empty.");
        return MP_FAILED;
    }
    if (!vecAgentLastBackupMeta.empty() && vecAgentLastBackupMeta.front() != vecDmeLastBackupMeta.front()) {
        associatedCopiesSet.insert(vecAgentLastBackupMeta.front());
    }
    mp_string agentLastLogBackupPath = mountPath + PATH_SEPARATOR + AGENT_LAST_LOG_BACKUP;
    vector<mp_string> vecLastBackupMeta;
    vecLastBackupMeta.push_back(vecDmeLastBackupMeta.front());
    vecLastBackupMeta.push_back(mainID);
    extendInfo[LOG_DIR_NAME] = mainID;
    associatedCopiesSet.insert(vecDmeLastBackupMeta.front());
    extendInfo[ASSOCIATED_COPIES] = Json::arrayValue;
    for (const auto &it : associatedCopiesSet) {
        extendInfo[ASSOCIATED_COPIES].append(it);
    }
    jobValue[EXTENDINFO] = std::move(extendInfo);
    return WriteToFile(agentLastLogBackupPath, vecLastBackupMeta);
}

mp_int32 PluginLogBackup::WriteToFile(const mp_string& strFilePath, const std::vector<mp_string>& vecInput)
{
    std::ofstream file(strFilePath, std::ios::out | std::ios::trunc);
    if (file.is_open()) {
        for (const auto &it : vecInput) {
            file << it;
            file << "\n";
        }
    } else {
        ERRLOG("Failed to open file: %s, errno[%d]:%s.", strFilePath.c_str(), errno, strerror(errno));
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
}