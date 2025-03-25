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
#include "CommonBackupJob.h"
#include "ReportJobDetailHandler.h"
#include "common/File.h"
#include "common/Utils.h"
#include "common/MpString.h"
#include "common/Thread.h"
#include "config_reader/ConfigIniReader.h"
#include "common/Path.h"

using namespace GeneralDB;
using namespace AppProtect;

namespace {
    const mp_string MODULE = "CommonBackupJob";
    const mp_string BACKUP_HANDLE_MAP = "BackupHandleMap";
    const mp_string SCAN_DIR_AND_FILE = "ScanDirAndFile";
    using defer = std::shared_ptr<void>;
    const mp_string QUERY_COPY = "queryCopy";
    constexpr int64_t DATA_REPO_TYPE = 1;
    constexpr int64_t META_REPO_TYPE = 0;
    constexpr int64_t LOG_REPO_TYPE = 3;
    const mp_string RECORD_FILE_NAME = "RecordFile.txt"; // 保存扫描出来的文件
    const mp_string RECORD_DIR_NAME = "RecordDir.txt";   // 保存扫描出来的目录
    const mp_int32 WRITE_SCAN_RESULT_PARAM_NUMBER = 3;
    const mp_string DATA_DIR_NAME = "Data";
    const mp_string META_DIR_NAME = "Meta";
    constexpr auto QUERY_SCAN_REPOSITORIES = "QueryScanRepositories";
    static mp_string SYSTEM_DISK_NAME = "";
    const mp_string CFG_MOUNT_SECTION = "Mount";
    const mp_string CFG_WIN_MOUNT_PUBLIC_PATH_VALUE = "win_mount_public_path";
    const mp_string CFG_SYSTEM_SECTION = "System";
    const mp_string CFG_WIN_SYSTEM_DISK_VALUE = "win_system_disk";
    const mp_string SCAN_REPOS_CMD = PATH_SEPARATOR + "bin" + PATH_SEPARATOR + "scan_repos.py";
    constexpr int32_t SCAN_KEEP_ALIVE_REPORT_INTERVAL = 30; // 30s
    constexpr int32_t SCAN_KEEP_ALIVE_SLEEP_INTERVAL = 1;
    constexpr int64_t FOLDER_STRUCTURE_BACKUP = 1;
#ifdef WIN32
    static const mp_int32 PATH_SEPARATOR_COUNT_THREE = 3;
    static const mp_int32 PATH_SEPARATOR_COUNT_TWO = 2;
    static const mp_int32 NUMBER_2 = 2;
#endif
} // namespace

int CommonBackupJob::PrerequisiteJob()
{
    defer _(nullptr, [&](...) {
        SetJobToFinish();
    });

    std::shared_ptr<AppProtect::BackupJob> backupJobPtr =
        std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (backupJobPtr == nullptr) {
        HCP_Log(ERR, MODULE) << "Prerequisite job failed for get backup failed." << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value backupJobStr;
    StructToJson(*backupJobPtr, backupJobStr);

    Json::Value jsValue;
    jsValue["job"] = backupJobStr;
    Json::Value retValue;
    Param param = {jsValue, (*backupJobPtr).protectObject.subType, "BackupPrerequisite", (*backupJobPtr).jobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir((*backupJobPtr).protectObject.subType,
        backupJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, "", SubJobStatus::FAILED,
            LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << (*backupJobPtr).jobId << HCPENDLOG;
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

int CommonBackupJob::GenerateSubJobManually()
{
    std::shared_ptr<AppProtect::BackupJob> backupJobPtr =
        std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (backupJobPtr == nullptr) {
        HCP_Log(ERR, MODULE) << "GenerateSubJob failed for get backup failed." << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value backupJobStr;
    StructToJson(*backupJobPtr, backupJobStr);
    Json::Value jsValue;
    jsValue["job"] = backupJobStr;
    Param param = {jsValue, (*backupJobPtr).protectObject.subType, "BackupGenSubJob", (*backupJobPtr).jobId, "",
        MP_FALSE};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir((*backupJobPtr).protectObject.subType,
        backupJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, m_manualResult, shared_from_this()) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, "", SubJobStatus::FAILED,
            LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << (*backupJobPtr).jobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonBackupJob::GenerateSubJob()
{
    auto jobInfo = std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (jobInfo == nullptr) {
        HCP_Log(ERR, MODULE) << "GenerateSubJob failed for get backupJob Info failed." << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value backupJobStr;
    StructToJson(*jobInfo, backupJobStr);
    bool isNeedScan = false;
    GetScanDirAndFileJobSwitch(backupJobStr, isNeedScan);

    mp_string appType = jobInfo->protectObject.subType;
    Json::Value extendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(jobInfo->protectEnv.extendInfo, extendInfo)) {
        HCP_Log(ERR, MODULE) << "JsonStringToJsonValue Failed! " << DBG(m_jobId) << HCPENDLOG;
        return MP_FAILED;
    }
    if (GenSubJob(BACKUP_HANDLE_MAP, appType, extendInfo, jobInfo->protectEnv.nodes, isNeedScan) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Failed to generate subjob, jobId=" << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

void CommonBackupJob::GetScanDirAndFileJobSwitch(Json::Value backupJobStr, bool &isNeedScan)
{
    std::map<Json::ArrayIndex, std::vector<Json::Value>> mapJsonRep;
    std::vector<Json::Value> vecJsonBackupRep;
    Module::CJsonUtils::GetJsonArrayJson(backupJobStr, "repositories", vecJsonBackupRep);
    mapJsonRep.insert(std::make_pair(0, vecJsonBackupRep));

    for (Json::ArrayIndex index = 0; index < backupJobStr["copies"].size(); ++index) {
        std::vector<Json::Value> vecJsonCopyRep;
        Module::CJsonUtils::GetJsonArrayJson(backupJobStr["copies"][index], "repositories", vecJsonCopyRep);
        mapJsonRep.insert(std::make_pair(index + 1, vecJsonCopyRep));
    }

    for (auto iter = mapJsonRep.begin(); iter != mapJsonRep.end(); ++iter) {
        std::vector<Json::Value> vecJsonRep = iter->second;
        for (auto jsonRep : vecJsonRep) {
            StorageRepository stRep;
            JsonToStruct(jsonRep, stRep);
            AnalazeScanParam(stRep, jsonRep, isNeedScan);
        }
    }
    return;
}

void CommonBackupJob::AnalazeScanParam(const StorageRepository &stRep, const Json::Value &jsonRep, bool &isNeedScan)
{
    if (stRep.repositoryType == RepositoryDataType::type::DATA_REPOSITORY ||
        stRep.repositoryType == RepositoryDataType::type::LOG_REPOSITORY) {
        if (!jsonRep["extendInfo"].isMember("isAgentNeedScan")) {
            return;
        }
        HCP_Log(INFO, MODULE) << "The value of isAgentNeedScan is "
                              << jsonRep["extendInfo"]["isAgentNeedScan"].asString() << HCPENDLOG;
        isNeedScan = jsonRep["extendInfo"]["isAgentNeedScan"].asString() == "true" ? true : false;
    }
}

int CommonBackupJob::ExecuteSubJob()
{
    defer _(nullptr, [&](...) {
        SetJobToFinish();
    });

    std::shared_ptr<AppProtect::BackupJob> backupJobPtr =
        std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (backupJobPtr == nullptr || m_subJobInfo == nullptr) {
        ERRLOG("Get backup job info failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
        return MP_FAILED;
    }
    JobLogDetail startDetail = {(*backupJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::RUNNING,
        LOG_LABEL_TYPE::START_EXEC_SUBJOB};
    ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(startDetail);
    DBGLOG("JobName=%s, jobId=%s, subJobId=%s.", m_subJobInfo->jobName.c_str(), m_parentJobId.c_str(),
        m_jobId.c_str());
    // 查询副本子任务，调QueryBackupCopy接口，暂时为必拆分备份子任务，且为最低优先级
    if (m_subJobInfo->jobName == QUERY_COPY) {
        return ExeQueryBackupCopySubJob(backupJobPtr);
    } else if (m_subJobInfo->jobName == SCAN_DIR_AND_FILE) {
        return ExecuteScanAndRecordFileSubJob(backupJobPtr);
    }

    Json::Value backupJobStr;
    Json::Value subJobStr;
    StructToJson(*backupJobPtr, backupJobStr);
    StructToJson(*m_subJobInfo, subJobStr);
    Json::Value jsValue;
    jsValue["job"] = backupJobStr;
    jsValue["subJob"] = subJobStr;
    Json::Value retValue;
    Param param = {jsValue, (*backupJobPtr).protectObject.subType, "Backup", (*backupJobPtr).jobId,
        (*m_subJobInfo).subJobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir((*backupJobPtr).protectObject.subType,
        backupJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::FAILED,
            LOG_LABEL_TYPE::EXEC_BACKUP_SUBJOB_FAIL, JOB_INTERNAL_ERROR};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        ERRLOG("Exec failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonBackupJob::ExeQueryBackupCopySubJob(std::shared_ptr<AppProtect::BackupJob> &backupJobPtr)
{
    if (QueryBackupCopy() != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::FAILED,
            LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        ERRLOG("Query backup copy failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
        return MP_FAILED;
    }
    JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::COMPLETED,
        LOG_LABEL_TYPE::UNDEFIND_LABEL};
    ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
    INFOLOG("Report backup copy success, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
    return MP_SUCCESS;
}

int CommonBackupJob::ExecuteScanAndRecordFileSubJob(std::shared_ptr<AppProtect::BackupJob> &backupJobPtr)
{
    if (ScanAndRecordFile(backupJobPtr) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::FAILED,
            LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        ERRLOG("Scan dir and file failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
        return MP_FAILED;
    }
    JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::COMPLETED,
        LOG_LABEL_TYPE::UNDEFIND_LABEL};
    ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
    INFOLOG("Scan dir and file success, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
    return MP_SUCCESS;
}

int CommonBackupJob::QueryBackupCopy()
{
    std::shared_ptr<AppProtect::BackupJob> backupJobPtr =
        std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (backupJobPtr == nullptr) {
        HCP_Log(ERR, MODULE) << "QueryBackupCopy job failed for get backup failed." << HCPENDLOG;
        return MP_FAILED;
    }

    // 判断QueryBackupCopy查询副本接口是否配置，不配置则不执行，适配后面应用插件调用RPC工具进行副本上报
    mp_string actionScript;
    mp_string processScript;
    Param param;
    param.appType = (*backupJobPtr).protectObject.subType;
    param.cmdType = "QueryBackupCopy";
    if (ParseConfigFile::GetInstance()->GetExectueCmd(param, actionScript, processScript) != MP_SUCCESS) {
        HCP_Log(WARN, MODULE) << "Get script from config failed. cmdType = QueryBackupCopy, subType = " <<
            (*backupJobPtr).protectObject.subType << HCPENDLOG;
        return MP_SUCCESS;
    }

    Json::Value backupJobStr;
    StructToJson(*backupJobPtr, backupJobStr);
    Json::Value jsValue;
    jsValue["job"] = backupJobStr;

    Json::Value retValue;
    param = {jsValue, (*backupJobPtr).protectObject.subType, "QueryBackupCopy", (*backupJobPtr).jobId, "",
        MP_FALSE};
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << (*backupJobPtr).jobId << HCPENDLOG;
        return MP_FAILED;
    }

    Copy image;
    JsonToStruct(retValue, image);
    ActionResult reportResult;
    JobService::ReportCopyAdditionalInfo(reportResult, (*backupJobPtr).jobId, image);
    if (reportResult.code != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "ReportCopyAdditionalInfo Failed, jobId=" << (*backupJobPtr).jobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonBackupJob::PostJob()
{
    defer _(nullptr, [&](...) {
        SetJobToFinish();
    });

    std::shared_ptr<AppProtect::BackupJob> backupJobPtr =
        std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (backupJobPtr == nullptr || m_subJobInfo == nullptr) {
        HCP_Log(ERR, MODULE) << "PostJob job failed for get backup failed." << HCPENDLOG;
        return MP_FAILED;
    }

    Json::Value backupJobStr;
    Json::Value subJobStr;
    StructToJson(*backupJobPtr, backupJobStr);
    StructToJson(*m_subJobInfo, subJobStr);
    Json::Value jsValue;
    jsValue["job"] = backupJobStr;
    jsValue["subJob"] = subJobStr;
    jsValue["backupJobResult"] = m_jobResult;
    Json::Value retValue;
    Param param = {jsValue, (*backupJobPtr).protectObject.subType, "BackupPostJob", (*backupJobPtr).jobId,
        (*m_subJobInfo).subJobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir((*backupJobPtr).protectObject.subType,
        backupJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::FAILED,
            LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << (*backupJobPtr).jobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

bool CommonBackupJob::IsScriptExist(const mp_string &appType, const mp_string &cmdType)
{
    mp_string actionScript;
    mp_string processScript;
    Param param;
    param.appType = appType;
    param.cmdType = cmdType;
    if (ParseConfigFile::GetInstance()->GetExectueCmd(param, actionScript, processScript) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Get script from config failed.cmdType=" << cmdType << HCPENDLOG;
        return false;
    }
    return true;
}

void CommonBackupJob::QueryScanRepositories(std::shared_ptr<AppProtect::BackupJob> &job,
                                            AppProtect::ScanRepositories &scanRepositories)
{
    if (!IsScriptExist(job->protectObject.subType, QUERY_SCAN_REPOSITORIES)) {
        WARNLOG("QueryScanRepositories config is not exist, appType=%s.", job->protectObject.subType.c_str());
        return;
    }
    Json::Value backupJobStr;
    StructToJson(*job, backupJobStr);
    Json::Value jsValue;
    jsValue["job"] = backupJobStr;
    // 调用执行器
    Json::Value retValue;
    Param param = {jsValue, job->protectObject.subType, QUERY_SCAN_REPOSITORIES, job->jobId, "", MP_FALSE};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(job->protectObject.subType, backupJobStr["protectObject"],
                                                         param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        ERRLOG("Exec query scan repositories failed, appType=%s.", job->protectObject.subType.c_str());
        return;
    }
    JsonToStruct(retValue, scanRepositories);
    return;
}

mp_int32 CommonBackupJob::ScanAndRecordFile(std::shared_ptr<AppProtect::BackupJob> &job)
{
    AppProtect::ScanRepositories scanRepositories;
    QueryScanRepositories(job, scanRepositories);
    if (scanRepositories.scanRepoList.empty()) {
        ERRLOG("QueryScanRepositories Failed or the path is empty, enter the NormalGetScanRepositories.");
        if (NormalGetScanRepositories(job, scanRepositories) != MP_SUCCESS) {
            ERRLOG("Can not get the scan plugins path, record file failed");
            return MP_FAILED;
        }
    }
    StartKeepAliveThread();
    if (GetScanResult(job, scanRepositories) != MP_SUCCESS) {
        ERRLOG("Get Scan result failed.");
        StopKeepAliveThread();
        return MP_FAILED;
    }
    StopKeepAliveThread();
    INFOLOG("Scan finish.");
    return MP_SUCCESS;
}

mp_int32 CommonBackupJob::NormalGetScanRepositories(std::shared_ptr<AppProtect::BackupJob> &job,
                                                    AppProtect::ScanRepositories &scanRepositories)
{
    Json::Value jsonData;
    StructToJson(*job, jsonData);
    
    for (Json::ArrayIndex index = 0; index < jsonData["repositories"].size(); ++index) {
        DBGLOG("NormalScanAndRecordFile, index is %d, the jsondata type is %d, path size is %d",
            index, jsonData["repositories"][index]["type"].asInt(), jsonData["repositories"][index]["path"].size());
        if ((jsonData["repositories"][index]["type"] != META_REPO_TYPE &&
                jsonData["repositories"][index]["type"] != DATA_REPO_TYPE) ||
            jsonData["repositories"][index]["path"].empty()) {
            continue;
        }
        AppProtect::RepositoryPath repoPath;
        for (Json::ArrayIndex index1 = 0; index1 < jsonData["repositories"][index]["path"].size(); ++index1) {
            mp_string path = jsonData["repositories"][index]["path"][index1].asString();
            INFOLOG("Scan path is:%s.", path.c_str());
            if (!Module::CFile::DirExist(path.c_str())) {
                continue;
            }
            repoPath.scanPath = path;
            if (jsonData["repositories"][index]["type"] == META_REPO_TYPE) {
                repoPath.repositoryType = RepositoryDataType::type::META_REPOSITORY;
                GetScanSavePath(path, scanRepositories);
            } else {
                repoPath.repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
            }
            break;
        }
        if (!repoPath.scanPath.empty()) {
            scanRepositories.scanRepoList.push_back(repoPath);
            INFOLOG("The repoPath with repositoryType %lld, scanPath %s has been added.",
                repoPath.repositoryType, repoPath.scanPath.c_str());
        } else {
            ERRLOG("The mount point %s can not be accessed.",
                jsonData["repositories"][index]["path"][0].asString().c_str());
        }
    }
    if (scanRepositories.scanRepoList.size() == 0) {
        ERRLOG("Can not get the scan plugins path.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

void CommonBackupJob::GetScanSavePath(const mp_string &path, AppProtect::ScanRepositories &scanRepositories)
{
    if (scanRepositories.savePath.empty()) {
        scanRepositories.savePath = path;
    }
    return;
}

mp_int32 CommonBackupJob::GetScanResult(std::shared_ptr<AppProtect::BackupJob> &job,
                                        const AppProtect::ScanRepositories &scanRepositories)
{
    INFOLOG("Scan result save path is %s", scanRepositories.savePath.c_str());
    for (int i = 0; i < scanRepositories.scanRepoList.size(); ++i) {
        AppProtect::RepositoryPath repoPath = scanRepositories.scanRepoList[i];
        INFOLOG("Will scan repo type %d, scan path %s.", repoPath.repositoryType, repoPath.scanPath.c_str());
        int64_t rType = repoPath.repositoryType;
        if ((rType != META_REPO_TYPE && rType != DATA_REPO_TYPE && rType != LOG_REPO_TYPE) ||
            repoPath.scanPath.size() == 0) {
            continue;
        }
        if (DoScan(job, scanRepositories.savePath, repoPath) != MP_SUCCESS) {
            ERRLOG("Failed to scan dirs the result.");
            return MP_FAILED;
        }
        if (DoGenerateRecord(job, scanRepositories.savePath, repoPath.scanPath, repoPath.repositoryType) != MP_SUCCESS) {
            ERRLOG("Failed to save the result.");
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 CommonBackupJob::DoScan(std::shared_ptr<AppProtect::BackupJob> &job, const mp_string& savePath, AppProtect::RepositoryPath &repoPath)
{
    Json::Value backupJobStr;
    StructToJson(*job, backupJobStr);
    mp_string scanPath = repoPath.scanPath;
    mp_string savePreDir = savePath;
    if (job->jobParam.backupType == AppProtect::BackupJobType::LOG_BAKCUP) {
        savePreDir = savePreDir + PATH_SEPARATOR + "meta";
        INFOLOG("Job is Log Backup mode, the savePreDir is %s", savePreDir.c_str());
    }
    // 存储生成的目录记录文件和文件记录文件的位置
    mp_string saveCtrlDir = savePreDir + PATH_SEPARATOR + job->jobId + "_ctrl";
    Param param;
    param.appType = job->protectObject.subType;
    param.cmdType = "DoScan";
    param.jobId = job->jobId;
    param.scriptPath = Module::CPath::GetInstance().GetRootPath() + SCAN_REPOS_CMD;
#ifdef WIN32
    replace(param.scriptPath.begin(), param.scriptPath.end(), '\\', '/');
    replace(saveCtrlDir.begin(), saveCtrlDir.end(), '\\', '/');
    replace(scanPath.begin(), scanPath.end(), '\\', '/');
#endif
    // 下发扫描仓库的命令 jobId 扫描结果存放位置 需要扫描的目录
    param.scriptParams = {"scanRepo", m_jobId, saveCtrlDir, scanPath};
    
    Json::Value retValue;
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        ERRLOG("DoScan Exec failed, jobId=", job->jobId);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CommonBackupJob::DoGenerateRecord(std::shared_ptr<AppProtect::BackupJob> &job,
    const mp_string& savePath, const mp_string& scanPath, int64_t repositoryType)
{
    Json::Value backupJobStr;
    StructToJson(*job, backupJobStr);

    mp_string savePreDir = savePath;
    bool isLogNotMeta = false;
    if (job->jobParam.backupType == AppProtect::BackupJobType::LOG_BAKCUP) {
        savePreDir = savePreDir + PATH_SEPARATOR + "meta";
        INFOLOG("Job is Log Backup mode, the savePreDir is %s", savePreDir.c_str());
        isLogNotMeta = (repositoryType != META_REPO_TYPE);
    }
    // 存储生成的目录记录文件和文件记录文件的位置
    mp_string saveDir = savePreDir + PATH_SEPARATOR + job->jobId;
    INFOLOG("Scan result save to %s", saveDir.c_str());
    // 决定生成的文件头为Data还是Meta
    mp_string repoType = DATA_DIR_NAME;
    if (repositoryType == META_REPO_TYPE) {
        repoType = META_DIR_NAME;
    }
    // 生成控制文件的路径
    mp_string saveCtrlDir = saveDir + "_ctrl";

    // 截掉挂载目录部分
    mp_string truncateScanPath = scanPath;
    TruncateScanPath(job, truncateScanPath, repositoryType, isLogNotMeta);
    INFOLOG("After TruncateScanPath, the truncateScanPath is %s", truncateScanPath.c_str());
    
    Param param;
    param.appType = job->protectObject.subType;
    param.cmdType = "GenerateRecords";
    param.jobId = job->jobId;
    param.scriptPath = Module::CPath::GetInstance().GetRootPath() + SCAN_REPOS_CMD;
    // 处理扫描结果: 任务id 仓库类型 扫描结果存放位置 处理结果存放位置 扫描后结果应该添加的文件前缀
#ifdef WIN32
    replace(param.scriptPath.begin(), param.scriptPath.end(), '\\', '/');
    replace(saveCtrlDir.begin(), saveCtrlDir.end(), '\\', '/');
    replace(saveDir.begin(), saveDir.end(), '\\', '/');
    replace(truncateScanPath.begin(), truncateScanPath.end(), '\\', '/');
#endif
    param.scriptParams = {"generateRecords", m_jobId, repoType, saveCtrlDir, saveDir, truncateScanPath};
    
    Json::Value retValue;
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        ERRLOG("DoGenerateRecord Exec failed, jobId=", job->jobId);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}


mp_int32 CommonBackupJob::ScanDirAndFile(
    mp_string &rootPath, std::vector<mp_string> &rootfolderpath, std::vector<mp_string> &rootfilepath)
{
    // 获取当前目录下的所有的文件全路径
    std::vector<mp_string> vecfilePath;
    int iRet = GetFilePath(rootPath, vecfilePath);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Scan file path failed.iRet is:%d.", iRet);
        return iRet;
    }
    rootfilepath.insert(rootfilepath.end(), vecfilePath.begin(), vecfilePath.end());
 
    // 获取当前目录下的所有的子目录全路径
    std::vector<mp_string> vecfolderPath;
    iRet = GetFolderPath(rootPath, vecfolderPath);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Scan Folder path failed.iRet is:%d.", iRet);
        return iRet;
    }
    rootfolderpath.insert(rootfolderpath.end(), vecfolderPath.begin(), vecfolderPath.end());
 
    // 递归扫描子目录下的文件和目录
    for (mp_string folderPath : vecfolderPath) {
        iRet = ScanDirAndFile(folderPath, rootfolderpath, rootfilepath);
        if (iRet != MP_SUCCESS) {
            ERRLOG("ScanDirAndFile failed.iRet is:%d.", iRet);
            return iRet;
        }
    }
    return MP_SUCCESS;
}

mp_int32 CommonBackupJob::GetFolderPath(mp_string &strFolder, std::vector<mp_string> &vecFolderPath)
{
    std::vector<mp_string> folderNameList;
    folderNameList.clear();
    mp_int32 iRet = Module::CFile::GetFolderDir(strFolder, folderNameList);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    for (mp_string strName : folderNameList) {
        // 快照可见场景下，.snapshot为应用快照目录，非应用目录，过滤掉，不必放到待扫描目录中
        if (strName == ".snapshot") {
            INFOLOG(".snapshot of %s don't need to scan.", strFolder.c_str());
            continue;
        }
        vecFolderPath.push_back(strFolder + PATH_SEPARATOR + strName);
    }
    return MP_SUCCESS;
}
 
mp_int32 CommonBackupJob::GetFilePath(mp_string &strFolder, std::vector<mp_string> &vecFolderPath)
{
    std::vector<mp_string> folderNameList;
    mp_int32 iRet = Module::CFile::GetFolderFile(strFolder, folderNameList);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    for (mp_string strName : folderNameList) {
        vecFolderPath.push_back(strFolder + PATH_SEPARATOR + strName);
    }
    return MP_SUCCESS;
}

#ifdef WIN32
mp_string CommonBackupJob::GetMountPublicPath()
{
    mp_string mountPublicPath = Module::ConfigReader::getStringFromAgentXml(CFG_MOUNT_SECTION, CFG_WIN_MOUNT_PUBLIC_PATH_VALUE);
    if (mountPublicPath == "") {
        WARNLOG("Failed to get CFG_WIN_MOUNT_PUBLIC_PATH.");
        mountPublicPath = "C:\\mnt\\databackup\\";
    }
    INFOLOG("mountPublicPath from cfg file: %s.", mountPublicPath.c_str());
    if (Module::CFile::DirExist(mountPublicPath.c_str())) {
        // 路径存在则直接使用
        return mountPublicPath;
    }
    // 路径不存在，检查对应盘符是否存在，否则转为系统盘符再使用
    mp_string targetDiskName = mountPublicPath.substr(0, 1);
    targetDiskName = targetDiskName + ":\\";
    if (Module::CFile::DirExist(targetDiskName.c_str())) {
        // 盘符存在则直接使用
        return mountPublicPath;
    }
    // 盘符不存在，先修改为C盘并改为系统盘符后使用
    mountPublicPath[0] = 'C';
    mountPublicPath = GetSystemDiskChangedPathInWin(mountPublicPath);
    INFOLOG("mountPublicPath is: %s.", mountPublicPath.c_str());
    return mountPublicPath;
}

mp_string CommonBackupJob::GetSystemDiskChangedPathInWin(const mp_string& oriPath)
{
    if (SYSTEM_DISK_NAME == "") {
        SYSTEM_DISK_NAME = Module::ConfigReader::getStringFromAgentXml(CFG_SYSTEM_SECTION, CFG_WIN_SYSTEM_DISK_VALUE);
        if (SYSTEM_DISK_NAME == "") {
            SYSTEM_DISK_NAME = "C";
            WARNLOG("Get system disk name from config failed, use default system disk name: C.");
        }
    }
    if (SYSTEM_DISK_NAME == "C" || SYSTEM_DISK_NAME == "c") {
        DBGLOG("The default system disk name is C, no need to change.");
        return oriPath;
    }
    mp_string changePath = SYSTEM_DISK_NAME + oriPath.substr(1, oriPath.length() - 1);
    DBGLOG("System disk name is %s, the oripath %s has been changed to %s.", SYSTEM_DISK_NAME.c_str(),
        oriPath.c_str(), changePath.c_str());
    return changePath;
}
#endif

mp_int32 CommonBackupJob::CheckPathIsValid(const mp_string &filePath)
{
    // E6000设备即时挂载，需要扫描副本目录用于创建文件克隆，写入扫描结果时，需要提权到root执行
    // root执行会校验目录，修改时需要同步修改PluginSubPostJob::ScanAndRecordFile()和PrepareFileSystem定义
#ifdef WIN32
    mp_string MOUNT_PUBLIC_PATH = GetMountPublicPath();  // 挂载目录的前置
#else
    mp_string MOUNT_PUBLIC_PATH = "/mnt/databackup/";
#endif
    mp_string realPath = filePath;
    // FormattingPath的目录不存在会返回失败，当前函数只归一化目录，不判断返回值
    (mp_void) Module::CMpString::FormattingPath2(realPath);
    INFOLOG("Format path is %s, file path %s.", realPath.c_str(), filePath.c_str());
    
    if (realPath.find(MOUNT_PUBLIC_PATH) != 0) {
        ERRLOG("realPath %s isn't begin with %s.", realPath.c_str(), MOUNT_PUBLIC_PATH.c_str());
        return MP_FAILED;
    }

    mp_string fileName = Module::BaseFileName(realPath);
    if ((fileName != DATA_DIR_NAME + RECORD_FILE_NAME) && (fileName != DATA_DIR_NAME + RECORD_DIR_NAME) &&
        (fileName != META_DIR_NAME + RECORD_FILE_NAME) && (fileName != META_DIR_NAME + RECORD_DIR_NAME)) {
        ERRLOG("File name %s isn't fix file name.", fileName.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

size_t CommonBackupJob::FindStrPos(const mp_string str, char c, int n)
{
    size_t pos = str.find(c);
    for (int i = 1; i < n; i++) {
        pos = str.find(c, pos + 1);
        if (pos == std::string::npos) {
            return std::string::npos;
        }
    }
    return pos;
}

#ifndef WIN32
mp_int32 CommonBackupJob::TruncateScanPath(std::shared_ptr<AppProtect::BackupJob> &job, mp_string &scanPath,
    int64_t repositoryType, bool isLogNotMeta)
{
    Json::Value jsonData;
    StructToJson(*job, jsonData);
    SubJob subJobParam;
    JsonToStruct(jsonData, subJobParam);
    mp_string appType = job->protectObject.subType;
    mp_string truncateStr =
        "/mnt/databackup/" + appType + PATH_SEPARATOR + subJobParam.jobId.c_str() + PATH_SEPARATOR + "data";
    mp_string truncateStrMeta =
        "/mnt/databackup/" + appType + PATH_SEPARATOR + subJobParam.jobId.c_str() + PATH_SEPARATOR + "meta";
    mp_string truncateStrLog =
        "/mnt/databackup/" + appType + PATH_SEPARATOR + subJobParam.jobId.c_str() + PATH_SEPARATOR + "log";
    size_t size = scanPath.size();
    if (scanPath.find(truncateStr) != 0 && scanPath.find(truncateStrMeta) != 0) {
        if (scanPath.find(truncateStrLog) != 0) {
            ERRLOG("folderPath is invalid:%s.", scanPath.c_str());
            return MP_FAILED;
        }
        scanPath = scanPath.substr(truncateStrLog.length(), size);
        return MP_SUCCESS;
    }
    scanPath = scanPath.substr(truncateStr.length(), size);
    return MP_SUCCESS;
}
#else

void CommonBackupJob::PreHandleMountPublicPath(std::string &mount_public_path)
{
    // 去除连续的反斜杠
    std::string::size_type pos = 0;
    while ((pos = mount_public_path.find("\\\\", pos)) != std::string::npos) {
        mount_public_path.replace(pos, NUMBER_2, "\\");
        pos += 1;
    }
    // 去除末尾的反斜杠
    char tmp = '\\';
    if (mount_public_path.back() == tmp) {
        mount_public_path.pop_back();
    }
}

void CommonBackupJob::TruncateStorageName(std::shared_ptr<AppProtect::BackupJob> &job, mp_string &StorageName,
    int &copy_format)
{
    Json::Value jsonData;
    StructToJson(*job, jsonData);

    StorageName = jsonData["repositories"][0]["remotePath"].asString();
    // copy_format为0时为快照格式备份，为1时为目录格式
    INFOLOG("StorageName before is %s.", StorageName.c_str());
    AppProtect::BackupJob jobParam;
    JsonToStruct(jsonData, jobParam);
    if (job->jobParam.backupType != AppProtect::BackupJobType::LOG_BAKCUP) {
        size_t pos;
        if (StorageName.find("InnerDirectory") == mp_string::npos) {
            pos = FindStrPos(StorageName, '/', PATH_SEPARATOR_COUNT_THREE);
        } else {
            pos = FindStrPos(StorageName, '/', PATH_SEPARATOR_COUNT_TWO);
            copy_format = FOLDER_STRUCTURE_BACKUP;
        }
        StorageName = StorageName.substr(0, pos);
    }
    INFOLOG("StorageName after is %s, copy_format is %d", StorageName.c_str(), copy_format);
}

mp_int32 CommonBackupJob::TruncateScanPath(std::shared_ptr<AppProtect::BackupJob> &job, mp_string &scanPath,
    int64_t repositoryType, bool isLogNotMeta)
{
    mp_string StorageName;
    int copy_format = -1;
    TruncateStorageName(job, StorageName, copy_format);
    
    char tmp = '\\';
    if (scanPath.back() != tmp) {
        scanPath += '\\';
    }
    std::string mount_public_path = GetMountPublicPath();  // 挂载目录的前置

    PreHandleMountPublicPath(mount_public_path);

    int count = std::count(mount_public_path.begin(), mount_public_path.end(), '\\');
    int backSlashCount1 = count + PATH_SEPARATOR_COUNT_TWO;
    int backSlashCount2 = count + PATH_SEPARATOR_COUNT_THREE;
    size_t pathPos1 = FindStrPos(scanPath, '\\', backSlashCount1);
    if (pathPos1 == mp_string::npos) {
        ERRLOG("scanPath is invalid:%s.", scanPath.c_str());
        return MP_FAILED;
    }

    size_t pathPos2 = FindStrPos(scanPath, '\\', backSlashCount2);
    if (pathPos2 == mp_string::npos) {
        ERRLOG("scanPath is invalid:%s.", scanPath.c_str());
        return MP_FAILED;
    }
        
    std::string logicIpStr = scanPath.substr(pathPos1 + 1, pathPos2 - pathPos1 - 1);
    scanPath = scanPath.substr(pathPos2);
    replace(scanPath.begin(), scanPath.end(), '\\', '/');

    size_t pathPos3 = FindStrPos(scanPath, '/', PATH_SEPARATOR_COUNT_TWO);
    if (pathPos3 == mp_string::npos) {
        scanPath = StorageName + scanPath;
        ERRLOG("scanPath is invalid:%s.", scanPath.c_str());
        return MP_FAILED;
    }
    std::string str1 = scanPath.substr(pathPos3);
    std::string str2 = scanPath.substr(0, pathPos3);
    INFOLOG("TruncateScanPath str1 is %s, str2 is %s.", str1.c_str(), str2.c_str());
    // 目录格式的非meta仓或者快照格式日志备份的log仓
    if (isLogNotMeta || (copy_format == FOLDER_STRUCTURE_BACKUP && repositoryType != META_REPO_TYPE)) {
        scanPath = StorageName + str1 + logicIpStr + str2;
    } else {
        scanPath = StorageName + str2 + "/" + logicIpStr + str1;
    }
    return MP_SUCCESS;
}
#endif

mp_void CommonBackupJob::ReportSubJobRunning()
{
    DBGLOG("Keep alive thread start, jobId=%s.", m_jobId.c_str());
    int count = SCAN_KEEP_ALIVE_REPORT_INTERVAL;

    std::shared_ptr<AppProtect::BackupJob> backupJobPtr =
        std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    
    while (!m_stopScanKeepAliveTheadFlag.load()) {
        count++;
        if (count <= SCAN_KEEP_ALIVE_REPORT_INTERVAL) {
            Module::SleepFor(std::chrono::seconds(SCAN_KEEP_ALIVE_SLEEP_INTERVAL));
            continue;
        }
        count = 0;
        JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, m_subJobInfo->subJobId, SubJobStatus::RUNNING, LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
    }
    DBGLOG("Keep alive thread end, jobId=%s.", m_jobId.c_str());
}

mp_void CommonBackupJob::StartKeepAliveThread()
{
    if (m_subJobInfo != nullptr && m_subJobInfo->jobName == SCAN_DIR_AND_FILE) {
        DBGLOG("Start keep alive thread, jobId=%s.", m_jobId.c_str());
        m_stopScanKeepAliveTheadFlag.store(false);
        m_scanKeepAliveTh = std::make_shared<std::thread>([this]() { return ReportSubJobRunning(); });
    }
}


mp_void CommonBackupJob::StopKeepAliveThread()
{
    m_stopScanKeepAliveTheadFlag.store(true);
    if (m_scanKeepAliveTh) {
        m_scanKeepAliveTh->join();
        m_scanKeepAliveTh.reset();
        DBGLOG("Keep alive thread reset");
    }
}