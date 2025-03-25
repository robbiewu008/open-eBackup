/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file AppProtect.h
 * @brief Implement for application protect
 * @version 1.1.0
 * @date 2021-11-23
 * @author youlei 00412658
 */

#include "apps/appprotect/AppProtectService.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/Utils.h"
#include "common/LogRotater.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "apps/appprotect/CommonDef.h"
#include "pluginfx/AutoReleasePlugin.h"
#include "common/Defines.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "securecom/CryptAlg.h"
#include "servicecenter/services/device/PrepareFileSystem.h"
#include "common/CMpTime.h"
#include "common/Ip.h"
#include "common/File.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"

std::shared_ptr<AppProtectService> AppProtectService::g_instance = nullptr;
std::mutex AppProtectService::m_mutex;
std::mutex AppProtectService::m_mutex_write;
std::mutex AppProtectService::m_mutex_lunidlist;
std::mutex AppProtectService::m_mutex_luninfolist;
std::mutex AppProtectService::m_mutex_errorcodelist;
std::mutex AppProtectService::m_mutex_mountpoint;
std::mutex AppProtectService::m_mutex_sanclientprejobfailedset;
namespace {
constexpr int MIN_TASKID_SIZE = 36;
constexpr int KEY_NOT_EXSITS = 0;
constexpr int MAX_TASKID_SIZE = 128;
constexpr int MAX_LUNID_NAME = 256;
const mp_int32 SUPPORT_DATATURBO_PROTOCOL = 1024;
const mp_int32 BACKUP_JOB = 1;
const mp_int32 RESTORE_JOB = 2;
const mp_int32 COPY_VERIFY_JOB = 8;
const mp_int32 DATA_TYPE = 1;
const mp_int32 CACHE_TYPE = 2;
const mp_int32 LOG_TYPE = 3;
const mp_int64 DEFAULT_SIZE = 536870912000;
const mp_int32 DATA_REPOSITORY_POS = 1;
const mp_int32 UNIT_CONVERSION_BYTE_TO_GBYTE = 1024 * 1024 * 1024;
const mp_int32 EXPANSION_SIZE = 5;
const mp_int32 PASS_SEED_NUM_UPPER = 26;
const mp_int32 PASS_SEED_NUM_LOWER = 26;
const mp_int32 PASS_SEED_NUM_DIGIT = 10;
const mp_int32 RANDOM_PASS_LENGTH = 12;
const mp_int32 RANDOM_PASS_DIGIT_LENGTH = 4;
const mp_int32 MAX_RETRY_TIMES = 3;
const mp_int32 DELAY_TIME = 30 * 1000;
const mp_string DATA_SIZE  = "dataSize";
const mp_string ACCESS_LOG_NAME  = "access.log";
}

std::shared_ptr<AppProtectService> AppProtectService::GetInstance()
{
    if (g_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        g_instance.reset(new (std::nothrow) AppProtectService());
        if (g_instance == nullptr) {
            ERRLOG("new AppProtectService failed.");
            return nullptr;
        }

        if (g_instance->Init() != MP_SUCCESS) {
            ERRLOG("initialize AppProtectService failed.");
            return nullptr;
        }
        return g_instance;
    }
    return g_instance;
}

AppProtectService::AppProtectService() {}

AppProtectService::~AppProtectService() {}

mp_int32 AppProtectService::Init()
{
    mp_int32 ret = MP_SUCCESS;
    for (int i = 1; i < MAX_LUNID_NAME; i++) {
        m_lunidList.push_back(i);
    }
    LogRotater::GetInstance().Init(CPath::GetInstance().GetNginxLogsPath(), ACCESS_LOG_NAME);
    return ret;
}

mp_void AppProtectService::SetSanclientFailedPreJob(const mp_string &taskId)
{
    std::lock_guard<std::mutex> lock(m_mutex_sanclientprejobfailedset);
    m_sanclientPreJobFailedSet.insert(taskId);
}

mp_int32 AppProtectService::WakeUpJob(CRequestMsg &req, CResponseMsg &rsp)
{
    LOGGUARD("");
    mp_string strUrl = req.GetURL().GetProcURL();
    INFOLOG("Received wakeupjob request %s", strUrl.c_str());
    std::smatch match = GetTaskIdFromUrl(strUrl);
    if (match.empty()) {
        ERRLOG("WakeUpJob url is illegal. url: %s.", strUrl.c_str());
        return MP_FAILED;
    }
    mp_string strTaskID = match.str();
    CHECK_FAIL_EX(CheckParamStringEnd(strTaskID, MIN_TASKID_SIZE, MAX_TASKID_SIZE));

    mp_string appType;
    const Json::Value &jv = req.GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jv, NOTIFY_APPTYPE, appType);

    AppProtect::AppProtectJobHandler *ins = AppProtect::AppProtectJobHandler::GetInstance();
    if (!ins) {
        ERRLOG("AppProtectJobHandler is null.");
        return MP_FAILED;
    }

    Json::Value& rspData = rsp.GetJsonValueRef();
    AppProtect::WakeupJobResult result;
    ins->WakeUpJob(match.str(), jv, result, rsp);
    rspData["AgentStatus"] = result.agentStatus;
    return MP_SUCCESS;
}

mp_int32 AppProtectService::SanclientJobForUbc(Json::Value &jvReq, CRequestMsg &req)
{
    mp_string strUrl = req.GetURL().GetProcURL();
    std::smatch taskId;
    // parse main taskId
    taskId = GetTaskIdFromUrl(strUrl);
    if (taskId.empty()) {
        ERRLOG("Abortjob url is illegal. url: %s.", strUrl.c_str());
        return MP_FAILED;
    }
    mp_string strTaskID = taskId.str();
    CHECK_FAIL_EX(CheckParamStringEnd(strTaskID, MIN_TASKID_SIZE, MAX_TASKID_SIZE));
    INFOLOG("Taskid is %s", strTaskID.c_str());
    mp_int32 errorCode;
    {
        std::lock_guard<std::mutex> lock(m_mutex_errorcodelist);
        if (m_ErrorcodeList.find(strTaskID) != m_ErrorcodeList.end()) {
            errorCode = m_ErrorcodeList[strTaskID];
            if (errorCode == MP_SUCCESS) {
                std::lock_guard<std::mutex> lock(m_mutex_luninfolist);
                if (m_LuninfoList.find(strTaskID) != m_LuninfoList.end()) {
                    jvReq = m_LuninfoList[strTaskID];
                }
            } else {
                jvReq["code"] = errorCode;
                jvReq["bodyErr"] = errorCode;
            }
        } else {
            std::lock_guard<std::mutex> lock(m_mutex_sanclientprejobfailedset);
            if (m_sanclientPreJobFailedSet.count(strTaskID) != 0) {
                errorCode = ERR_SANCLIENT_PREPAREJOB_FAILED;
                jvReq["code"] = errorCode;
                jvReq["bodyErr"] = errorCode;
            }
        }
    }
    DBGLOG("Task %s jvReq %s", strTaskID.c_str(), jvReq.toStyledString().c_str());
    return MP_SUCCESS;
}

std::smatch AppProtectService::GetTaskIdFromUrl(const mp_string &url)
{
    std::regex reg("[[:alnum:]]{8}-[[:alnum:]]{4}-[[:alnum:]]{4}-[[:alnum:]]{4}-[[:alnum:]]{12}"
        "([_A-Za-z0-9]*){0,1}");
    std::smatch match;
    std::regex_search(url, match, reg);
    return match;
}

mp_int32 AppProtectService::AbortJob(CRequestMsg &req, CResponseMsg &rsp)
{
    mp_string strUrl = req.GetURL().GetProcURL();
    INFOLOG("Receive abort request %s", strUrl.c_str());
    std::smatch taskId;
    // parse main taskId
    taskId = GetTaskIdFromUrl(strUrl);
    if (taskId.empty()) {
        ERRLOG("Abortjob url is illegal. url: %s.", strUrl.c_str());
        return MP_FAILED;
    }
    mp_string strTaskID = taskId.str();
    CHECK_FAIL_EX(CheckParamStringEnd(strTaskID, MIN_TASKID_SIZE, MAX_TASKID_SIZE));
    // parse subtaskId
    std::string subtaskId;
    const std::map<mp_string, mp_string> &param = req.GetURL().GetQueryParam();
    auto iter = param.find("subTaskId");
    if (iter != param.end()) {
        subtaskId = iter->second;
        CHECK_FAIL_EX(CheckParamStringEnd(subtaskId, MIN_TASKID_SIZE, MAX_TASKID_SIZE));
    }
    AppProtect::AppProtectJobHandler *ins = AppProtect::AppProtectJobHandler::GetInstance();
    if (ins == nullptr) {
        ERRLOG("AppProtectJobHandler is null. Abort task(%s, %s) failed.", taskId.str().c_str(), subtaskId.c_str());
        return MP_FAILED;
    }

    return ins->AbortJob(taskId.str(), subtaskId);
}

mp_int32 AppProtectService::DeliverJobStatus(const mp_string &strAppType, CRequestMsg &req, CResponseMsg &rsp)
{
    LOGGUARD("");
    rsp.SetHttpType(CResponseMsg::RSP_JSON_TYPE2);

    auto plugin = ExternalPluginManager::GetInstance().GetPluginByRest(strAppType);
    if (plugin == nullptr) {
        ERRLOG("Get plugin failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    auto pClient = plugin->GetPluginClient();
    if (pClient == nullptr) {
        ERRLOG("Get thrift client failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    AutoReleasePlugin autoRelease(strAppType);
    auto appServiceClient = GetProtectServiceClient(pClient);

    const Json::Value &requestParam = req.GetMsgBody().GetJsonValueRef();
    mp_string status;
    mp_string taskId;
    mp_string script;
    GET_JSON_STRING(requestParam, "status", status);
    GET_JSON_STRING(requestParam, "taskId", taskId);
    GET_JSON_STRING(requestParam, "script", script);
    ActionResult _return;
    try {
        appServiceClient->DeliverTaskStatus(_return, status, taskId, script);
    } catch (apache::thrift::transport::TTransportException &ex) {
        ERRLOG("TTransportException. %s", ex.what());
        _return.code = MP_FAILED;
    } catch (const std::exception &ex) {
        ERRLOG("Standard C++ Exception. %s", ex.what());
        _return.code = MP_FAILED;
    } catch (...) {
        ERRLOG("Unknown exception.");
        _return.code = MP_FAILED;
    }
    Json::Value &jValue = rsp.GetJsonValueRef();
    if (_return.code == MP_SUCCESS) {
        jValue["errorCode"] = "0";
        jValue["errorMessage"] = _return.message;
    } else {
        jValue["code"] = _return.code;
        jValue["bodyErr"] = _return.bodyErr;
        jValue["message"] = _return.message;
    }

    return _return.code;
}

mp_void AppProtectService::GetCopiesID(const Json::Value &jvReq, std::vector<mp_string> &backupCopiesID,
    mp_string &logCopyID)
{
    std::vector<Json::Value> vecJsonCopy;
    CJsonUtils::GetJsonArrayJson(jvReq, "copies", vecJsonCopy);
    for (const auto &jsonCopy : vecJsonCopy) {
        std::string copyType;
        std::string copyId;
        std::string generatedBy;
        if (CJsonUtils::GetJsonString(jsonCopy, "type", copyType) != MP_SUCCESS) {
            ERRLOG("Invalid jsonCopy;");
            continue;
        }
        DBGLOG("Copy type is :%s.", copyType.c_str());
        if (copyType == "s3Archive" || copyType == "tapeArchive") {
            if (!jsonCopy.isMember("extendInfo") || !jsonCopy["extendInfo"].isObject() ||
                !jsonCopy["extendInfo"].isMember("extendInfo") || !jsonCopy["extendInfo"]["extendInfo"].isObject() ||
                !jsonCopy["extendInfo"]["extendInfo"].isMember("copyid") ||
                !jsonCopy["extendInfo"]["extendInfo"]["copyid"].isString()) {
                ERRLOG("Invalid jsonCopy.");
                continue;
            }
            backupCopiesID.push_back(jsonCopy["extendInfo"]["extendInfo"]["copyid"].asString());
        } else if (CJsonUtils::GetJsonString(jsonCopy, "generatedBy", generatedBy) == MP_SUCCESS &&
                   (generatedBy == "normalReplication" || generatedBy == "reverseReplication")) {
            if (!jsonCopy.isMember("originCopyId") || !jsonCopy["originCopyId"].isString()) {
                ERRLOG("OriginCopyId is not exists.");
                continue;
            }
            backupCopiesID.push_back(jsonCopy["originCopyId"].asString());
        } else if (copyType == "log" && CJsonUtils::GetJsonString(jsonCopy, "id", copyId) == MP_SUCCESS) {
            logCopyID = copyId;
        } else if (copyType != "log" && CJsonUtils::GetJsonString(jsonCopy, "id", copyId) == MP_SUCCESS) {
            backupCopiesID.push_back(copyId);
        }
    }
}

mp_int32 AppProtectService::CopyLogMeta(const mp_string &logRepositoryPath, const mp_string &cacheRepositoryPath,
    const std::vector<mp_string> &backupCopiesID)
{
#ifndef WIN32
    if (logRepositoryPath.empty() || cacheRepositoryPath.empty()) {
        ERRLOG("logRepositoryPath or cacheRepositoryPath is empty.");
        return MP_FAILED;
    }
    CRootCaller rootCaller;
    const mp_string srcFile = logRepositoryPath + "/" + backupCopiesID.front() + ".meta";
    const mp_string desFile = cacheRepositoryPath;
    std::ostringstream scriptParam;
    scriptParam << "srcPath=" << srcFile << NODE_COLON << "desPath=" << cacheRepositoryPath << NODE_COLON;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SANCLIENT_COPY_LOG_META, scriptParam.str(), NULL);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Copy .meta file failed, iRet:%d.", iRet);
        return iRet;
    }
#endif
    return MP_SUCCESS;
}

mp_void AppProtectService::GetLogRepAndCacheRep(const AppProtect::FilesystemInfo &fileinfo,
    mp_bool &isLogRepositoryMounted, mp_string &logRepositoryPath, mp_string &cacheRepositoryPath)
{
    if (fileinfo.Filesystemtype == LOG_TYPE) {
        isLogRepositoryMounted = true;
        logRepositoryPath = fileinfo.mountPoints.front();
    }
    if (fileinfo.Filesystemtype == CACHE_TYPE) {
        cacheRepositoryPath = fileinfo.mountPoints.front();
    }
}

mp_int32 AppProtectService::SanclientMount(const Json::Value &jvReq, const mp_string &taskId,
    std::vector<AppProtect::FilesystemInfo> &filesysteminfo, const mp_string &lanFreeSwitch)
{
    AppProtect::MountNasParam param;
    param.jobID = taskId;
    if (SanclientMountParaCheck(jvReq) != MP_SUCCESS) {
        ERRLOG("Invalid SanclientMount parameter.");
        return MP_FAILED;
    }
    mp_int32 taskType = jvReq["taskType"].asInt();
    mp_bool isLogRepositoryMounted = false;
    mp_string logRepositoryPath;
    mp_string cacheRepositoryPath;
    std::vector<Json::Value> vecJsonBackupRep;
    mp_string datasize = "0";
    HandleDatasize(datasize, jvReq); // 获取将要备份文件大小
    CJsonUtils::GetJsonArrayJson(jvReq, "repositories", vecJsonBackupRep);
    std::vector<mp_string> backupCopiesID;
    mp_string logCopyID;
    if (taskType == RESTORE_JOB || taskType == COPY_VERIFY_JOB) {
        GetCopiesID(jvReq, backupCopiesID, logCopyID); // 恢复任务存在多副本时获取所有副本ID
    }
    for (auto &iter : vecJsonBackupRep) {
        std::vector<Json::Value> vecJsonRemotepath;
        CJsonUtils::GetJsonArrayJson(iter, "remotePath", vecJsonRemotepath);
        for (auto &iter1 : vecJsonRemotepath) {
            AppProtect::FilesystemInfo fileinfo;
            if (InitFilesystemInfo(fileinfo, datasize, taskType, logCopyID) != MP_SUCCESS) {
                ERRLOG("Initialize filesystem info failed.");
                return MP_FAILED;
            }
            StorageRepository stRep;
            SanclientMountEX(iter, iter1, param, fileinfo, stRep); // 获取挂载所需信息
            fileinfo.BackupCopiesID.insert(fileinfo.BackupCopiesID.end(), backupCopiesID.begin(), backupCopiesID.end());
            param.isFcOn = jvReq["sanclient"]["openLanFreeSwitch"].asBool();
            mp_int32 iRet = MountType(lanFreeSwitch, param, fileinfo, filesysteminfo); // 调用mount接口挂载文件系统
            if (iRet != MP_SUCCESS) {
                std::lock_guard<std::mutex> lock(m_mutex_errorcodelist);
                m_ErrorcodeList.insert(std::pair<mp_string, mp_int32>(taskId, iRet));
                return iRet;
            }
            // 日志恢复时需要获取日志仓和cache挂载点
            GetLogRepAndCacheRep(fileinfo, isLogRepositoryMounted, logRepositoryPath, cacheRepositoryPath);
        }
    }
    if (taskType == RESTORE_JOB && isLogRepositoryMounted) {
        // 日志恢复时需要将.meta文件从日志仓拷贝到cache仓以适配日志恢复 没有.meta文件使用ubc下发的associated_log_copies
        CopyLogMeta(logRepositoryPath, cacheRepositoryPath, backupCopiesID);
    }
    return MP_SUCCESS;
}

mp_int32 AppProtectService::InitFilesystemInfo(AppProtect::FilesystemInfo &fileInfo, const mp_string &dataSize,
    const mp_int32 &taskType, const mp_string &logCopyID)
{
    fileInfo.JobType = taskType;
    if (StringToLonglong(fileInfo.FilesystemSize, dataSize) != MP_SUCCESS) {
        ERRLOG("Invalid data size.");
        return MP_FAILED;
    }
    fileInfo.FilesystemSize = fileInfo.FilesystemSize == 0 ? DEFAULT_SIZE : fileInfo.FilesystemSize;
    fileInfo.LogCopyID = logCopyID;
    return MP_SUCCESS;
}

mp_int32 AppProtectService::StringToLonglong(mp_int64 &result, const std::string &str)
{
    try {
        result = CMpString::SafeStoll(str, 0);
    } catch (const std::exception& erro) {
        ERRLOG("Invalid str, erro: %s.", erro.what());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 AppProtectService::SanclientMountParaCheck(const Json::Value &jvReq)
{
    if (!jvReq.isObject() || !jvReq.isMember("taskType") || !jvReq["taskType"].isInt()) {
        ERRLOG("Invalid jvReq parameter.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

void AppProtectService::HandleDatasize(mp_string &datasize, const Json::Value &jvReq)
{
    mp_string datasizeForUbc;
    if (!jvReq.isObject() || !jvReq.isMember(DATA_SIZE) || !jvReq[DATA_SIZE].isString()) {
        datasizeForUbc = "";
    } else {
        datasizeForUbc = jvReq[DATA_SIZE].asString();
    }
    if (datasizeForUbc != "" && datasizeForUbc != "None") {
        datasize = jvReq[DATA_SIZE].asString();
    }
}

void AppProtectService::SanclientMountEX(Json::Value &repositories, Json::Value &remotePathJson,
    AppProtect::MountNasParam &param, AppProtect::FilesystemInfo &fileinfo, StorageRepository &stRep)
{
    if (!remotePathJson.isObject() || !remotePathJson.isMember("path") || !remotePathJson["path"].isString() ||
        !remotePathJson.isMember("type") || !remotePathJson["type"].isInt() ||
        !repositories.isObject() || !repositories.isMember("type") || !repositories["type"].isInt()) {
            ERRLOG("Invalid repositories or remotePathJson parameter.");
            return;
        }
    mp_string remotePath = remotePathJson["path"].asString();
    mp_int32 remoteType = repositories["type"].asInt();
    mp_int32 remoteType1 = remotePathJson["type"].asInt();
    JsonToStruct(repositories, stRep);
    for (const auto &host : stRep.remoteHost) {
        if (host.supportProtocol == SUPPORT_DATATURBO_PROTOCOL) {
            param.vecDataturboIP.push_back(host.ip);
        } else {
            param.vecStorageIp.push_back(host.ip);
        }
    }
    if (fileinfo.JobType == RESTORE_JOB || fileinfo.JobType == COPY_VERIFY_JOB) {
        if ((remoteType == RepositoryDataType::type::DATA_REPOSITORY &&
            remoteType1 == RepositoryDataType::type::DATA_REPOSITORY) ||
            remoteType == RepositoryDataType::type::LOG_REPOSITORY) {
            std::vector<mp_string> outputRemotePath;
            CMpString::StrSplit(outputRemotePath, remotePath, CHAR_SLASH);
            if (outputRemotePath.size() > DATA_REPOSITORY_POS) {
                remotePath = CHAR_SLASH + outputRemotePath[DATA_REPOSITORY_POS];
            }
        }
    }
    fileinfo.FilesystemName = remotePath;
    if (remoteType1 == 0) {
        fileinfo.Filesystemtype = remoteType1;
    } else {
        fileinfo.Filesystemtype = remoteType;
    }
    param.storagePath = remotePath;
    param.storageName = stRep.remoteName;
    param.repositoryType = AppProtect::STORAGE_TYPE_ARRAY[mp_int32(stRep.repositoryType)];
    param.authKey = stRep.auth.authkey;
    param.authPwd = stRep.auth.authPwd;
}

mp_int32 AppProtectService::MountType(const mp_string &lanFreeSwitch, AppProtect::MountNasParam &param,
    AppProtect::FilesystemInfo &fileinfo, std::vector<AppProtect::FilesystemInfo> &filesysteminfo)
{
    std::vector<mp_string> mountPoints;
    std::vector<mp_string> dtbMountPoints;
    AppProtect::PrepareFileSystem mountHandler;
    if (lanFreeSwitch == "true") {
        mp_int32 iRet = mountHandler.MountDataturboFileSystem(param, mountPoints, dtbMountPoints);
        if (iRet == MP_SUCCESS) {
            ClearString(param.authPwd);
            INFOLOG("Mount dataturbo file system info success");
            fileinfo.FilesystemMountPath = mountPoints.front();
            fileinfo.mountPoints.insert(fileinfo.mountPoints.end(), mountPoints.begin(), mountPoints.end());
            filesysteminfo.push_back(fileinfo);
            std::lock_guard<std::mutex> lock(m_mutex_mountpoint);
            m_mountPoint[param.jobID].insert(m_mountPoint[param.jobID].end(), mountPoints.begin(), mountPoints.end());
            return iRet;
        }
        ERRLOG("Mount dataturbo file system failed.");
        // dataturbo挂载失败，尝试nas挂载，挂载失败不返回
    }
    mp_int32 ret = mountHandler.MountNasFileSystem(param, mountPoints);
    ClearString(param.authPwd);
    if (ret != MP_SUCCESS) {
        ERRLOG("Mount nas file system failed.");
        return ret;
    }
    fileinfo.FilesystemMountPath = mountPoints.front();
    fileinfo.mountPoints.insert(fileinfo.mountPoints.end(), mountPoints.begin(), mountPoints.end());
    filesysteminfo.push_back(fileinfo);
    std::lock_guard<std::mutex> lock(m_mutex_mountpoint);
    m_mountPoint[param.jobID].insert(m_mountPoint[param.jobID].end(), mountPoints.begin(), mountPoints.end());
    return MP_SUCCESS;
}

mp_int32 AppProtectService::EnvCheck(const mp_string &taskId)
{
    LOGGUARD("");
    INFOLOG("Start check sanclient env, jobid %s", taskId.c_str());
#ifndef WIN32
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SANCLIENT_ENVCHECK, "", NULL);
    if (iRet != MP_SUCCESS) {
        std::lock_guard<std::mutex> lock(m_mutex_errorcodelist);
        ERRLOG("Check env failed, iRet %d.", iRet);
        m_ErrorcodeList.insert(std::pair<mp_string, mp_int32>(taskId, iRet));
        return iRet;
    }
#endif
    return MP_SUCCESS;
}

mp_int32 AppProtectService::GetChapPass(mp_string &unidirectionalAuthPwd)
{
    INFOLOG("Start generating chap certification.");
    mp_char charUppercase[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G',
                               'H', 'I', 'J', 'K', 'L', 'M', 'N',
                               'O', 'P', 'Q', 'R', 'S', 'T',
                               'U', 'V', 'W', 'X', 'Y', 'Z'};
    mp_char charLowercase[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g',
                               'h', 'i', 'j', 'k', 'l', 'm', 'n',
                               'o', 'p', 'q', 'r', 's', 't',
                               'u', 'v', 'w', 'x', 'y', 'z'};
    mp_char charDigits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    mp_uint64 num;
    for (mp_int32 i = 0; i < RANDOM_PASS_LENGTH; i++) {
        if (GetRandom(num, true) == MP_FAILED) {
            return MP_FAILED;
        }
        if (i < RANDOM_PASS_DIGIT_LENGTH) {
            mp_int32 idx = num % PASS_SEED_NUM_UPPER;
            unidirectionalAuthPwd += charUppercase[idx];
        } else if (i >= (RANDOM_PASS_DIGIT_LENGTH + RANDOM_PASS_DIGIT_LENGTH)) {
            mp_int32 idx = num % PASS_SEED_NUM_LOWER;
            unidirectionalAuthPwd += charLowercase[idx];
        } else {
            mp_int32 idx = num % PASS_SEED_NUM_DIGIT;
            unidirectionalAuthPwd += charDigits[idx];
        }
    }
    return MP_SUCCESS;
}

mp_void AppProtectService::StructToJson(const AppProtect::LunInfo &lunInfo, Json::Value &luninfojson)
{
    luninfojson["sanclientWwpn"] = lunInfo.Wwpn;
    luninfojson["lunName"] = lunInfo.LunName;
    luninfojson["lunId"] = lunInfo.LunId;
    luninfojson["path"] = lunInfo.Path;
    Json::Int64 tmFileSize = lunInfo.FilesystemSize;
    luninfojson["filesystemsize"] = tmFileSize;
    luninfojson["agentWwpn"] = lunInfo.AgentWwpn;
    luninfojson["UnidirectionalAuthPwd"] = lunInfo.UnidirectionalAuthPwd;
    luninfojson["port"] = lunInfo.Port;
    luninfojson["fileioName"] = lunInfo.FileioName;
    luninfojson["jobType"] = lunInfo.JobType;
    for (std::vector<mp_string>::const_iterator it = lunInfo.mountPoints.begin(); it != lunInfo.mountPoints.end();
         ++it) {
        luninfojson["mountPoints"].append(*it);
    }
}

mp_int32 AppProtectService::CreateLunISCSI(const mp_string &agentIqn, const mp_string &iqn,
    std::vector<AppProtect::FilesystemInfo> &filesysteminfo, const mp_string &taskid)
{
    LOGGUARD("");
    std::lock_guard<std::mutex> lock(m_mutex_write);
    if (iqn.empty() || filesysteminfo.empty()) {
        ERRLOG("Iqn or filesysteminfo is empty, size of iqn: %d, \
            size of filesysteminfo: %d.", iqn.size(), filesysteminfo.size());
        return MP_FAILED;
    }
    m_lunInfos.clear();
    INFOLOG("Start create lun, jobid %s", taskid.c_str());
    AppProtect::LunInfo luninfo;
    mp_string strPort;
    mp_string strIP;
    if (CIP::GetListenIPAndPort(strIP, strPort) != MP_SUCCESS) { // 获取sanclient的业务IP
        ERRLOG("Get Agent listen IP and port failed.");
        return MP_FAILED;
    }
    luninfo.Wwpn = iqn + "_" + strIP;
    luninfo.AgentWwpn = agentIqn;
    mp_string unidirectionalAuthPwd;
    if (GetChapPass(unidirectionalAuthPwd) != MP_SUCCESS) { // 生成单向认证密码
        ERRLOG("Failed to generate one-way authentication password.");
        return MP_FAILED;
    }
    mp_int32 iRet = CreateLunForIqns(filesysteminfo, taskid, luninfo, strIP, unidirectionalAuthPwd);
    INFOLOG("Start destroying chap certification.");
    ClearString(unidirectionalAuthPwd);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Create Lun by iscsi failed");
        m_lunInfos.clear();
        std::lock_guard<std::mutex> lock(m_mutex_errorcodelist);
        m_ErrorcodeList.insert(std::pair<mp_string, mp_int32>(taskid, ERR_CREATE_FILEIO_FAILED));
        return MP_FAILED;
    }
    Json::Value luninfojson;
    Json::Value jvReq = Json::Value();
    for (const auto &iter : m_lunInfos) {
        StructToJson(iter, luninfojson);
        jvReq["lunInfo"].append(luninfojson);
    }
    ReportLunInfo(taskid, jvReq);
    std::lock_guard<std::mutex> lockForErrorcodeList(m_mutex_errorcodelist);
    m_ErrorcodeList.insert(std::pair<mp_string, mp_int32>(taskid, MP_SUCCESS));
    return MP_SUCCESS;
}

mp_int32 AppProtectService::CreateLun(const std::vector<mp_string> &agentwwpns, const std::vector<mp_string> &wwpns,
    std::vector<AppProtect::FilesystemInfo> &filesysteminfo, const mp_string &taskid)
{
    LOGGUARD("");
    std::lock_guard<std::mutex> lock(m_mutex_write);
    if (wwpns.empty() || filesysteminfo.empty()) {
        ERRLOG("Wwpn or filesysteminfo is empty, size of wwpns: %d, \
            size of filesysteminfo: %d.", wwpns.size(), filesysteminfo.size());
        return MP_FAILED;
    }
    { // ubc重复调情况下，若任务已经创建过lun，直接退出；
        std::lock_guard<std::mutex> lock(m_mutex_luninfolist);
        if (m_LuninfoList.find(taskid) != m_LuninfoList.end()) {
            DBGLOG("Lun info exists, job id:%s", taskid.c_str());
            return MP_SUCCESS;
        }
    }
    m_lunInfos.clear();
    INFOLOG("Start create lun, jobid %s", taskid.c_str());
    bool flag = false;
    AppProtect::LunInfo luninfo;
    for (auto &iter : agentwwpns) { // 下发的同一台agent的所有wwpn轮询创建lun，有一个成功即可
        luninfo.AgentWwpn = iter;
        mp_int32 iRet = CreateLunForWwpns(wwpns, filesysteminfo, taskid, luninfo);
        if (iRet == MP_SUCCESS) {
            flag = true;
        }
    }
    if (!flag) {
        ERRLOG("Create Lun all failed");
        m_lunInfos.clear();
        std::lock_guard<std::mutex> lock(m_mutex_errorcodelist);
        m_ErrorcodeList.insert(std::pair<mp_string, mp_int32>(taskid, ERR_CREATE_FILEIO_FAILED));
        return MP_FAILED;
    }
    Json::Value luninfojson;
    Json::Value jvReq = Json::Value();
    for (const auto &iter : m_lunInfos) {
        InsertLunInfo(iter, luninfojson);
        jvReq["lunInfo"].append(luninfojson);
    }
    DBGLOG("Create lun info: %s", jvReq.toStyledString().c_str());
    {
        std::lock_guard<std::mutex> lock(m_mutex_errorcodelist);
        m_ErrorcodeList.insert(std::pair<mp_string, mp_int32>(taskid, MP_SUCCESS));
    }
    ReportLunInfo(taskid, jvReq);
    return MP_SUCCESS;
}

mp_void AppProtectService::InsertLunInfo(const AppProtect::LunInfo &lunInfo, Json::Value &lunInfoJson)
{
    lunInfoJson["sanclientWwpn"] = lunInfo.Wwpn;
    lunInfoJson["lunName"] = lunInfo.LunName;
    lunInfoJson["lunId"] = lunInfo.LunId;
    lunInfoJson["path"] = lunInfo.Path;
    Json::Int64 tmFileSize = lunInfo.FilesystemSize;
    lunInfoJson["filesystemsize"] = tmFileSize;
    lunInfoJson["agentWwpn"] = lunInfo.AgentWwpn;
    lunInfoJson["fileioName"] = lunInfo.FileioName;
    lunInfoJson["jobType"] = lunInfo.JobType;
    lunInfoJson["port"] = lunInfo.Port;
    for (std::vector<mp_string>::const_iterator it = lunInfo.mountPoints.begin(); it != lunInfo.mountPoints.end();
         ++it) {
        lunInfoJson["mountPoints"].append(*it);
    }
}

void AppProtectService::ReportLunInfo(const mp_string &taskid, Json::Value &jvReq)
{
    std::lock_guard<std::mutex> lock(m_mutex_luninfolist);
    if (m_LuninfoList.count(taskid) == KEY_NOT_EXSITS) {
        m_LuninfoList.insert(std::pair<mp_string, Json::Value>(taskid, jvReq));
    } else {
        ReportLunInfoforCluster(taskid, jvReq);
    }
}

void AppProtectService::ReportLunInfoforCluster(const mp_string &taskid, Json::Value &jvReq)
{
    Json::Value jvReqForNext = m_LuninfoList[taskid];
    for (auto &iter : jvReq["lunInfo"]) {
        for (const auto &iter2 : jvReqForNext["lunInfo"]) {
            // 集群下发到同一个sanclient中转机时，创建lun不只fileioName相同，也需要lunid相同
            if (iter["fileioName"] == iter2["fileioName"]) {
                iter["lunId"] = iter2["lunId"];
            }
        }
        jvReqForNext["lunInfo"].append(iter);
    }
    m_LuninfoList[taskid] = jvReqForNext;
}

mp_int32 AppProtectService::CreateLunForIqnsV2(const mp_string &backupJobID, const AppProtect::FilesystemInfo &fsInfo,
    mp_string &unidirectionalAuthPwd, AppProtect::LunInfo &luninfo, const mp_string &strIP)
{
#ifndef WIN32
    CRootCaller rootCaller;
    mp_int32 lunid = 0;
    mp_string fileioName = backupJobID + "_" + std::to_string(fsInfo.Filesystemtype);
    for (const auto &iter : m_lunInfos) {
        if (iter.FileioName == fileioName && iter.Wwpn == luninfo.Wwpn) {
            lunid = CMpString::SafeStoi(iter.LunId);
            break;
        }
    }
    if (!lunid) {
        AssignmentLunId(lunid);
    }
    mp_string fileioFullPathName = fsInfo.FilesystemMountPath + "/" + backupJobID + "/" + fileioName;
    luninfo.Path = fsInfo.FilesystemMountPath;
    luninfo.FileioName = fileioName;
    luninfo.JobType = std::to_string(RESTORE_JOB);
    std::ostringstream scriptParam;
    scriptParam << "sanclientiqn=" << luninfo.Wwpn << NODE_COLON <<
        "agentiqn=" << luninfo.AgentWwpn << NODE_COLON <<   "fileioname=" << fileioName << NODE_COLON <<
        "fileiofullpathname=" << fileioFullPathName << NODE_COLON <<
        "lunid=" << std::to_string(lunid) << NODE_COLON <<
        "unidirectionalAuthPwd=" << unidirectionalAuthPwd << NODE_COLON <<
        "sanclientIP=" << strIP << NODE_COLON;
    std::vector<mp_string> vecResult;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SANCLIENT_ACTION_ISCSI, scriptParam.str(), &vecResult);
    if (iRet != MP_SUCCESS || vecResult.empty()) {
        std::lock_guard<std::mutex> lock(m_mutex_lunidlist);
        m_lunidList.push_back(lunid);
        ERRLOG("Create Lun Failed, lunname is %s", luninfo.FileioName.c_str());
        return MP_FAILED;
    }
    luninfo.Port = vecResult[0];
    EncryptStr(unidirectionalAuthPwd, luninfo.UnidirectionalAuthPwd);
    luninfo.LunName = std::to_string(fsInfo.Filesystemtype);
    luninfo.LunId =  std::to_string(lunid);
    luninfo.mountPoints = fsInfo.mountPoints;
    m_lunInfos.push_back(luninfo);
#endif
    return MP_SUCCESS;
}

mp_int32 AppProtectService::CreateLunForIqnsV3(const AppProtect::FilesystemInfo &fsInfo,
    mp_string &unidirectionalAuthPwd, AppProtect::LunInfo &luninfo, const mp_string &strIP)
{
    if (fsInfo.Filesystemtype == DATA_TYPE) {
        for (const auto &backupJobID : fsInfo.BackupCopiesID) {
            if (CreateLunForIqnsV2(backupJobID, fsInfo, unidirectionalAuthPwd, luninfo, strIP) != MP_SUCCESS) {
                return MP_FAILED;
            }
        }
    } else {
        if (CreateLunForIqnsV2(fsInfo.LogCopyID, fsInfo, unidirectionalAuthPwd, luninfo, strIP) != MP_SUCCESS) {
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 AppProtectService::CreateLunForIqns(std::vector<AppProtect::FilesystemInfo> &filesysteminfo,
    const mp_string &taskid, AppProtect::LunInfo &luninfo, const mp_string &strIP, mp_string &unidirectionalAuthPwd)
{
    LOGGUARD("");
    for (auto &iter : filesysteminfo) {
        if (iter.Filesystemtype != DATA_TYPE && iter.Filesystemtype != LOG_TYPE) {
            continue;
        }
        if (CreateLunForIqnsV3(iter, unidirectionalAuthPwd, luninfo, strIP) != MP_SUCCESS) {
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 AppProtectService::CreateLunForWwpnsV2(const mp_string &backupJobID, const AppProtect::FilesystemInfo &fsInfo,
    const mp_string &taskid, AppProtect::LunInfo &luninfo, mp_bool &flag)
{
#ifndef WIN32
    CRootCaller rootCaller;
    mp_int32 lunid = 0;
    mp_string jobID = backupJobID;
    mp_string fileioName = jobID + "_" + std::to_string(fsInfo.Filesystemtype);
    // 避免同一个fileio文件赋予了不同的lunID
    for (const auto &iter : m_lunInfos) {
        if (iter.FileioName == fileioName && iter.Wwpn == luninfo.Wwpn) {
            lunid = CMpString::SafeStoi(iter.LunId);
            break;
        }
    }
    if (!lunid) {
        AssignmentLunId(lunid);
    }
    mp_string fileioFullPathName = fsInfo.FilesystemMountPath + "/" + jobID + "/" + fileioName;
    mp_string filesystemsize = std::to_string(fsInfo.FilesystemSize / UNIT_CONVERSION_BYTE_TO_GBYTE + EXPANSION_SIZE);
    std::ostringstream scriptParam;
    scriptParam << "sanclientwwpn=" << luninfo.Wwpn << NODE_COLON << "agentwwpn=" << luninfo.AgentWwpn << NODE_COLON <<
        "fileioname=" << fileioName << NODE_COLON << "filesystemsize=" << filesystemsize << "G" << NODE_COLON <<
        "fileiofullpathname=" << fileioFullPathName << NODE_COLON << "lunid=" << std::to_string(lunid) <<
        NODE_COLON << "filesystemmountpath=" << fsInfo.FilesystemMountPath << NODE_COLON << "taskid=" <<
        taskid << NODE_COLON << "tasktype=" << std::to_string(fsInfo.JobType) << NODE_COLON;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SANCLIENT_ACTION, scriptParam.str(), NULL);
    if (iRet != MP_SUCCESS) {
        flag = false;
        std::lock_guard<std::mutex> lock(m_mutex_lunidlist);
        m_lunidList.push_back(lunid);
        ERRLOG("Create Lun Failed, lunname is %s", fileioName.c_str());
        return iRet;
    }
    LunInfoForAgent(fsInfo, fileioName, luninfo, lunid, filesystemsize);
#endif
    return MP_SUCCESS;
}

mp_int32 AppProtectService::CreateLunForWwpnsV3(const AppProtect::FilesystemInfo &fsInfo,
    const mp_string &taskid, AppProtect::LunInfo &luninfo, mp_bool &flag)
{
    // 数据仓可能有多个副本，日志仓只有一个
    if (fsInfo.Filesystemtype == DATA_TYPE) {
        for (const auto &backupJobID : fsInfo.BackupCopiesID) {
            if (CreateLunForWwpnsV2(backupJobID, fsInfo, taskid, luninfo, flag) != MP_SUCCESS) {
                return MP_FAILED;
            }
        }
    } else {
        if (CreateLunForWwpnsV2(fsInfo.LogCopyID, fsInfo, taskid, luninfo, flag) != MP_SUCCESS) {
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 AppProtectService::CreateLunForWwpnsV4(const AppProtect::FilesystemInfo &fsInfo,
    const mp_string &taskid, AppProtect::LunInfo &luninfo, mp_bool &flag)
{
    if (fsInfo.JobType == RESTORE_JOB || fsInfo.JobType == COPY_VERIFY_JOB) {
        // 恢复任务需要根据下发的副本信息去找到挂载目录下的副本ID，再根据该文件创建lun
        if (CreateLunForWwpnsV3(fsInfo, taskid, luninfo, flag) != MP_SUCCESS) {
            return MP_FAILED;
        }
    } else {
        if (CreateLunForWwpnsV2(taskid, fsInfo, taskid, luninfo, flag) != MP_SUCCESS) {
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 AppProtectService::CreateLunForWwpns(const std::vector<mp_string> &wwpns,
    std::vector<AppProtect::FilesystemInfo> &filesysteminfo, const mp_string &taskid, AppProtect::LunInfo &luninfo)
{
    LOGGUARD("");
    mp_bool createlunflag = false;
    mp_int32 iRet;
    // 下发的sanclient的每个wwpn都要去创建lun，有一个成功即可
    for (const auto &iter1 : wwpns) {
        mp_bool flag = true;
        luninfo.Wwpn = iter1;
        // 数据仓和日志仓需要创建lun，全部成功才为成功
        for (auto &iter2 : filesysteminfo) {
            if (iter2.Filesystemtype != DATA_TYPE && iter2.Filesystemtype != LOG_TYPE) {
                continue;
            }
            if (CreateLunForWwpnsV4(iter2, taskid, luninfo, flag) != MP_SUCCESS) {
                break;
            }
        }
        if (flag == true) {
            createlunflag = true;
        }
    }
    if (!createlunflag) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

void AppProtectService::LunInfoForAgent(const AppProtect::FilesystemInfo &fileinfo,
    const mp_string &fileioName, AppProtect::LunInfo &luninfo, const mp_int32 &lunid, const mp_string &filesystemsize)
{
    luninfo.JobType = std::to_string(fileinfo.JobType);
    luninfo.Path = fileinfo.FilesystemMountPath;
    luninfo.FileioName = fileioName;
    luninfo.FilesystemSize = CMpString::SafeStoll(filesystemsize);
    luninfo.LunName = std::to_string(fileinfo.Filesystemtype);
    luninfo.LunId = std::to_string(lunid);
    luninfo.mountPoints = fileinfo.mountPoints;
    m_lunInfos.push_back(luninfo);
}

void AppProtectService::AssignmentLunId(mp_int32 &lunid)
{
    std::lock_guard<std::mutex> lock(m_mutex_lunidlist);
    lunid = m_lunidList.front();
    m_lunidList.pop_front();
}

mp_int32 AppProtectService::CleanEnv(const mp_string &taskid)
{
    LOGGUARD("");
#ifndef WIN32
    CRootCaller rootCaller;
    Json::Value jvReq;
    mp_int32 retryTimes = 0;
    while (retryTimes < MAX_RETRY_TIMES) {
        if (m_LuninfoList.find(taskid) != m_LuninfoList.end()) {
            jvReq = m_LuninfoList[taskid];
            break;
        }
        ++retryTimes;
        CMpTime::DoSleep(DELAY_TIME);
    }
    mp_int32 iRet = DeleteLunInfo(jvReq);
    std::vector<mp_string> vecMountPath;
    if (m_mountPoint.find(taskid) != m_mountPoint.end()) {
        vecMountPath = m_mountPoint[taskid];
    }
    for (mp_string strPath : vecMountPath) {
        std::ostringstream scriptParam;
        scriptParam << "umountPath=" << strPath << NODE_COLON;
        if (rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_UMOUNT_NAS_FILESYS, scriptParam.str(), NULL) != MP_SUCCESS) {
            WARNLOG("Umount %s failed.", strPath.c_str());
        }
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex_mountpoint);
        m_mountPoint.erase(taskid);
    }
    if (iRet == MP_FAILED) {
        return MP_FAILED;
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex_luninfolist);
        m_LuninfoList.erase(taskid);
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex_errorcodelist);
        m_ErrorcodeList.erase(taskid);
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex_sanclientprejobfailedset);
        m_sanclientPreJobFailedSet.erase(taskid);
    }
#endif
    return MP_SUCCESS;
}

mp_int32 AppProtectService::DeleteLunInfo(const Json::Value &jvReq)
{
    bool flag = true;
#ifndef WIN32
    CRootCaller rootCaller;
    for (auto &iter : jvReq["lunInfo"]) {
        for (auto &iter2 : iter["mountPoints"]) {
            std::ostringstream scriptParam;
            scriptParam << "sanclientwwpn=" << iter["sanclientWwpn"].asString() << NODE_COLON
                        << "agentwwpn=" << iter["agentWwpn"].asString() << NODE_COLON
                        << "fileioname=" << iter["fileioName"].asString() << NODE_COLON
                        << "fileiofullpath=" << iter2.asString() << NODE_COLON << "lunid=" << iter["lunId"].asString()
                        << NODE_COLON << "sanclientPort=" << iter["port"].asString() << NODE_COLON;
            DBGLOG("Clear ScriptParam is:%s", scriptParam.str().c_str());
            if (rootCaller.Exec((mp_int32)ROOT_COMMAND_SANCLIENT_CLEAR, scriptParam.str(), NULL) != MP_SUCCESS) {
                ERRLOG("Clear Lun failed");
                flag = false;
            }
        }
        {
            std::lock_guard<std::mutex> lock(m_mutex_lunidlist);
            m_lunidList.push_back(CMpString::SafeStoi(iter["lunId"].asString(), 0));
        }
    }
#endif
    if (!flag) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

std::shared_ptr<AppProtect::ProtectServiceIf> AppProtectService::GetProtectServiceClient(
    std::shared_ptr<thriftservice::IThriftClient> &pThriftClient)
{
    return pThriftClient->GetConcurrentClientIf<AppProtect::ProtectServiceConcurrentClient>("ProtectService");
}