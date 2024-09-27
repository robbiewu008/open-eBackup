/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleNativeBackupPlugin.cpp
 * @brief  Implementation of the Class OracleNativeBackupPlugin
 * @version 1.0.0
 * @date 2019-11-15
 * @author wangguitao 00510599
 */
#include <vector>
#include <map>
#include "plugins/oraclenative/OracleNativeBackupPlugin.h"

#include "jsoncpp/include/json/json.h"
#include "jsoncpp/include/json/value.h"
#include "common/Defines.h"
#include "common/ErrorCode.h"
#include "common/Log.h"
#include "message/tcp/CDppMessage.h"
#include "apps/oracle/OracleDefines.h"
#include "taskmanager/TaskManager.h"

using std::map;
using std::vector;

static OracleNativeRedoTaskRegister g_regOracleRedoTask;

REGISTER_PLUGIN(OracleNativeBackupPlugin);
OracleNativeBackupPlugin::OracleNativeBackupPlugin()
{
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_GETSTOR_INFO, &OracleNativeBackupPlugin::GetDBStorInfo);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_PREPARE_MEDIA, &OracleNativeBackupPlugin::PrepareMedia);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_QUERY_BACKUPLEVEL, &OracleNativeBackupPlugin::QueryBackupLevel);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_BACKUP_DATA, &OracleNativeBackupPlugin::BackupDataFile);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_BACKUP_LOG, &OracleNativeBackupPlugin::BackupLogFile);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_RESTORE, &OracleNativeBackupPlugin::RestoreOracleDB);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_LIVEMOUNT, &OracleNativeBackupPlugin::LiveMountOracleDB);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_CLIVEMOUNT, &OracleNativeBackupPlugin::CancelLiveMountOracleDB);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_INSTANT_RESTORE, &OracleNativeBackupPlugin::InstanceRestore);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_EXPIRE_COPY, &OracleNativeBackupPlugin::ExpireCopy);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_DISMOUNT_MEDIA, &OracleNativeBackupPlugin::DisMountMedium);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_STOP_TASK, &OracleNativeBackupPlugin::StopTask);

    // Query host's role of oracle ARC cluster
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_ORACLE_QUERY_HOST_ROLE, &OracleNativeBackupPlugin::QueryHostRoleInCluster);

    // Redo task
    mp_int32 iRet = TaskManager::GetInstance()->CreateRedoTask();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init CreateRedoTask failed.");
    }

    if (oracleNativeBackup.CreateCheckMountTask() != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init CreateCheckMountTask failed.");
    }
}

OracleNativeBackupPlugin::~OracleNativeBackupPlugin()
{}

/**
 * 注册命令和函数的对应关系，后面命令分配时，插件管理器查询插件是否支持该命令，找到对应的插件并调用对应的函数
 */
mp_int32 OracleNativeBackupPlugin::Init(vector<mp_uint32>& cmds)
{
    // 初始化支持的命令列表
    cmds.push_back(MANAGE_CMD_NO_ORACLE_GETSTOR_INFO);
    cmds.push_back(MANAGE_CMD_NO_ORACLE_PREPARE_MEDIA);
    cmds.push_back(MANAGE_CMD_NO_ORACLE_QUERY_BACKUPLEVEL);
    cmds.push_back(MANAGE_CMD_NO_ORACLE_BACKUP_DATA);
    cmds.push_back(MANAGE_CMD_NO_ORACLE_BACKUP_LOG);
    cmds.push_back(MANAGE_CMD_NO_ORACLE_RESTORE);
    cmds.push_back(MANAGE_CMD_NO_ORACLE_LIVEMOUNT);
    cmds.push_back(MANAGE_CMD_NO_ORACLE_CLIVEMOUNT);
    cmds.push_back(MANAGE_CMD_NO_ORACLE_INSTANT_RESTORE);
    cmds.push_back(MANAGE_CMD_NO_ORACLE_EXPIRE_COPY);
    cmds.push_back(MANAGE_CMD_NO_ORACLE_DISMOUNT_MEDIA);
    cmds.push_back(MANAGE_CMD_NO_ORACLE_CHECK_AUTH);
    cmds.push_back(MANAGE_CMD_NO_ORACLE_STOP_TASK);
    return MP_SUCCESS;
}

mp_int32 OracleNativeBackupPlugin::DoAction(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    DO_DPP_ACTION(OracleNativeBackupPlugin, reqMsg, rspMsg);
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::GetDBStorInfo(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    LOGGUARD("");
    // get request message
    Json::Value reqBodyParams;
    CHECK_NOT_OK(reqMsg.GetManageBody(reqBodyParams));
    if (!reqBodyParams.isMember(MANAGECMD_KEY_BODY)) {
        COMMLOG(OS_LOG_ERROR, "dpp message have no body.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    Json::Value reqBody = reqBodyParams[MANAGECMD_KEY_BODY];

    oracle_db_info_t stDBInfo;
    GET_JSON_STRING(reqBody, PARAM_KEY_DBNAME, stDBInfo.strDBName);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strDBName, 0, ORACLE_PLUGIN_MAX_DBNAME));
    GET_JSON_STRING(reqBody, PARAM_KEY_INSTNAME, stDBInfo.strInstName);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strInstName, 0, ORACLE_PLUGIN_MAX_DBINSTANCENAME));
    GET_JSON_STRING(reqBody, PARAM_KEY_DBUSER, stDBInfo.strDBUsername);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strDBUsername, 0, ORACLE_PLUGIN_MAX_NSERNAME));
    GET_JSON_STRING(reqBody, PARAM_KEY_DBPWD, stDBInfo.strDBPassword);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strDBPassword, 0, ORACLE_PLUGIN_MAX_STRING));
    GET_JSON_STRING(reqBody, PARAM_KEY_ASMINST, stDBInfo.strASMInstance);
    if (!stDBInfo.strASMInstance.empty()) {
        CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strASMInstance, 1, ORACLE_PLUGIN_MAX_DBINSTANCENAME));
    }
    GET_JSON_STRING(reqBody, PARAM_KEY_ASMUSER, stDBInfo.strASMUserName);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strASMUserName, 0, ORACLE_PLUGIN_MAX_NSERNAME));
    GET_JSON_STRING(reqBody, PARAM_KEY_ASMPWD, stDBInfo.strASMPassword);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strASMPassword, 0, ORACLE_PLUGIN_MAX_STRING));

    Json::Value rspBody;
    Json::Value dbInfo;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_GETSTOR_INFO_ACK;
    mp_int32 iRet = oracleNativeBackup.GetDBStorInfo(stDBInfo, dbInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "get database storage info failed, iRet %d.", iRet);
        return iRet;
    }

    rspBody[MANAGECMD_KEY_BODY] = std::move(dbInfo);
    rspMsg.SetMsgBody(rspBody);

    stDBInfo.strDBUsername.replace(0, stDBInfo.strDBUsername.length(), "");
    stDBInfo.strDBPassword.replace(0, stDBInfo.strDBPassword.length(), "");
    return iRet;
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::QueryBackupLevel(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value reqBodyParams;
    CHECK_NOT_OK(reqMsg.GetManageBody(reqBodyParams));
    if (!reqBodyParams.isMember(MANAGECMD_KEY_BODY)) {
        COMMLOG(OS_LOG_ERROR, "dpp message have no body.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    Json::Value reqBody = reqBodyParams[MANAGECMD_KEY_BODY];

    Json::Value rspBody;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_QUERY_BACKUPLEVEL_ACK;
    mp_int32 iRet = oracleNativeBackup.QueryBackupLevel(reqBody, rspBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "query backup level failed, iRet %d.", iRet);
        return iRet;
    }
    rspMsg.SetMsgBody(rspBody);
    return iRet;
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::BackupDataFile(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value rspBody;
    mp_string taskId;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_BACKUP_DATA_ACK;

    mp_int32 iRet = oracleNativeBackup.BackupData(reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), taskId);
    rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
    Json::Value taskBody;
    taskBody[MANAGECMD_KEY_TASKID] = taskId;
    Task* pTask = TaskManager::GetInstance()->FindTaskEx(taskId, "OracleNativeBackupTask");
    if (pTask != NULL) {
        taskBody[PARAM_KEY_TASKRESULT] = pTask->GetTaskResult();
    }
    rspBody[MANAGECMD_KEY_BODY] = std::move(taskBody);
    rspMsg.SetMsgBody(rspBody);

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Backup database data file failed, iRet %d.", iRet);
    }
    return iRet;
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::BackupLogFile(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value rspBody;
    mp_string taskId;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_BACKUP_LOG_ACK;

    mp_int32 iRet = oracleNativeBackup.BackupLog(reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), taskId);
    rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
    Json::Value taskBody;
    taskBody[MANAGECMD_KEY_TASKID] = taskId;
    Task* pTask = TaskManager::GetInstance()->FindTaskEx(taskId, "OracleNativeBackupTask");
    if (pTask != NULL) {
        taskBody[PARAM_KEY_TASKRESULT] = pTask->GetTaskResult();
    }
    rspBody[MANAGECMD_KEY_BODY] = std::move(taskBody);
    rspMsg.SetMsgBody(rspBody);

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Backup database log file failed, iRet %d.", iRet);
    }
    return iRet;
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::RestoreOracleDB(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value rspBody;
    mp_string taskId;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_RESTORE_ACK;

    mp_int32 iRet = oracleNativeBackup.RestoreDB(reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), taskId);
    rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
    Json::Value taskBody;
    taskBody[MANAGECMD_KEY_TASKID] = taskId;
    Task* pTask = TaskManager::GetInstance()->FindTaskEx(taskId, "OracleNativeRestoreTask");
    if (pTask != NULL) {
        taskBody[PARAM_KEY_TASKRESULT] = pTask->GetTaskResult();
    }
    rspBody[MANAGECMD_KEY_BODY] = std::move(taskBody);
    rspMsg.SetMsgBody(rspBody);

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Restore database failed, iRet %d.", iRet);
    }
    return iRet;
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::LiveMountOracleDB(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value rspBody;
    mp_string taskId;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_LIVEMOUNT_ACK;

    mp_int32 iRet = oracleNativeBackup.LiveMount(reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), taskId);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Livemount database failed, iRet %d.", iRet);
        return iRet;
    }

    rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
    Json::Value taskBody;
    taskBody[MANAGECMD_KEY_TASKID] = taskId;
    rspBody[MANAGECMD_KEY_BODY] = std::move(taskBody);
    rspMsg.SetMsgBody(rspBody);
    return iRet;
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::CancelLiveMountOracleDB(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value rspBody;
    mp_string taskId;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_CLIVEMOUNT_ACK;

    mp_int32 iRet = oracleNativeBackup.CancelLiveMount(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), taskId);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Cancle Livemount database failed, iRet %d.", iRet);
        return iRet;
    }
    rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
    Json::Value taskBody;
    taskBody[MANAGECMD_KEY_TASKID] = taskId;
    rspBody[MANAGECMD_KEY_BODY] = std::move(taskBody);
    rspMsg.SetMsgBody(rspBody);
    return iRet;
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::InstanceRestore(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value rspBody;
    mp_string taskId;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_INSTANT_RESTORE_ACK;

    mp_int32 iRet = oracleNativeBackup.InstanceRestore(
        reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), taskId);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "instance recover database failed, iRet %d.", iRet);
        return iRet;
    }
    rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
    Json::Value taskBody;
    taskBody[MANAGECMD_KEY_TASKID] = taskId;
    rspBody[MANAGECMD_KEY_BODY] = std::move(taskBody);
    rspMsg.SetMsgBody(rspBody);
    return iRet;
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::ExpireCopy(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value rspBody;
    mp_string taskId;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_EXPIRE_COPY_ACK;
    rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
    rspMsg.SetMsgBody(rspBody);
    return MP_SUCCESS;

    mp_int32 iRet = oracleNativeBackup.ExpireCopy(reqMsg.GetBuffer(), reqMsg.GetIpAddr(), reqMsg.GetPort(), taskId);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Expire copy failed, iRet %d.", iRet);
        return iRet;
    }
    rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
    Json::Value taskBody;
    taskBody[MANAGECMD_KEY_TASKID] = taskId;
    rspBody[MANAGECMD_KEY_BODY] = taskBody;
    rspMsg.SetMsgBody(rspBody);
    return iRet;
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::DisMountMedium(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value reqBodyParams;
    CHECK_NOT_OK(reqMsg.GetManageBody(reqBodyParams));
    if (!reqBodyParams.isMember(MANAGECMD_KEY_BODY)) {
        COMMLOG(OS_LOG_ERROR, "Dismount task dpp message have no body.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    Json::Value& reqBody = reqBodyParams[MANAGECMD_KEY_BODY];
    // tasktype 0 -clean, 1- dismount
    mp_int32 taskType;
    GET_JSON_INT32(reqBody, keyTaskType, taskType);

    mp_string taskId;
    GET_JSON_STRING(reqBody, KEY_TASKID, taskId);

    Json::Value rspBody;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_DISMOUNT_MEDIA_ACK;
    rspBody[MANAGECMD_KEY_TASKID] = taskId;

    mp_int32 iRet = oracleNativeBackup.DisMountMedium(reqMsg.GetBuffer(), taskType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Dismount backup or restore medium failed, iRet %d.", iRet);
    }
    if (iRet == MP_SUCCESS) {
        rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
    }

    rspMsg.SetMsgBody(rspBody);
    return iRet;
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::StopTask(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value rspBody;
    mp_string taskId;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_STOP_TASK_ACK;

    mp_int32 iRet = oracleNativeBackup.StopTask(reqMsg.GetBuffer(), taskId);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "stop task failed, iRet %d.", iRet);
        return iRet;
    }
    rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
    Json::Value taskBody;
    taskBody[MANAGECMD_KEY_TASKID] = taskId;
    rspBody[MANAGECMD_KEY_BODY] = std::move(taskBody);
    rspMsg.SetMsgBody(rspBody);
    return iRet;
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::PrepareMedia(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    LOGGUARD("");
    Json::Value reqBodyParams;
    CHECK_NOT_OK(reqMsg.GetManageBody(reqBodyParams));
    if (!reqBodyParams.isMember(MANAGECMD_KEY_BODY)) {
        COMMLOG(OS_LOG_ERROR, "dpp message have no body.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    Json::Value& reqBody = reqBodyParams[MANAGECMD_KEY_BODY];

    mp_int32 taskType, storType;
    GET_JSON_INT32(reqBody, keyTaskType, taskType);
    CHECK_FAIL_EX(CheckParamInteger32(taskType, 0, ORACLE_PLUGIN_MAX_TASKTYPE));

    if (!reqBody.isMember(keyStorage)) {
        COMMLOG(OS_LOG_ERROR, "dpp message have no key storage.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    Json::Value& mediaInfo = reqBody[keyStorage];
    if (mediaInfo.isArray()) {
        COMMLOG(OS_LOG_WARN, "storage is array.");
        ERROR_COMMON_INVALID_PARAM;
    }

    if (!mediaInfo.isMember(keyStorType)) {
        COMMLOG(OS_LOG_WARN, "dpp message have no key storage storType.");
        storType = ORA_STORTYPE_FC;
    } else {
        GET_JSON_INT32(mediaInfo, keyStorType, storType);
        CheckParamInteger32(storType, ORA_STORTYPE_NAS, ORA_STORTYPE_FC);
    }

    Json::Value respMsg;
    mp_int32 iRet = oracleNativeBackup.PrepareMedia(reqMsg.GetBuffer(), taskType, storType, respMsg);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "prepare backup or restore medium failed, iRet %d.", iRet);
        return iRet;
    }
    Json::Value rspBody;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_PREPARE_MEDIA_ACK;
    rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
    rspBody[MANAGECMD_KEY_BODY] = respMsg;
    COMMLOG(OS_LOG_INFO, "respMsg is %s", respMsg.toStyledString().c_str()) ;
    rspMsg.SetMsgBody(rspBody);
    COMMLOG(OS_LOG_INFO, "Exec prepare media succ.");
    return iRet;
}

EXTER_ATTACK mp_int32 OracleNativeBackupPlugin::QueryHostRoleInCluster(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value reqBodyParams;
    CHECK_NOT_OK(reqMsg.GetManageBody(reqBodyParams));
    if (!reqBodyParams.isMember(MANAGECMD_KEY_BODY)) {
        COMMLOG(OS_LOG_ERROR, "dpp message have no body.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    Json::Value& reqBody = reqBodyParams[MANAGECMD_KEY_BODY];

    mp_int32 clusterType;
    oracle_db_info_t stDBinfo;
    GET_JSON_INT32(reqBody, PARAM_KEY_CLUSTER_TYPE, clusterType);
    GET_JSON_STRING(reqBody, PARAM_KEY_DBNAME, stDBinfo.strDBName);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBinfo.strDBName, 0, ORACLE_PLUGIN_MAX_DBNAME));

    Json::Value rspBody;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_ORACLE_QUERY_HOST_ROLE_ACK;

    vector<mp_string> storHosts;
    vector<mp_string> dbHosts;
    mp_int32 iRet = oracleCommon.QueryHostRoleInCluster(stDBinfo, storHosts, dbHosts);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "get database storage info failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value hostInfo;
    Json::Value storInfo;
    Json::Value dbInfo;
    for (vector<mp_string>::iterator iter = storHosts.begin(); iter != storHosts.end(); ++iter) {
        storInfo.append(*iter);
    }

    for (vector<mp_string>::iterator iter = dbHosts.begin(); iter != dbHosts.end(); ++iter) {
        dbInfo.append(*iter);
    }

    hostInfo["storHost"] = std::move(storInfo);
    hostInfo["dbHost"] = std::move(dbInfo);
    rspBody[MANAGECMD_KEY_BODY] = std::move(hostInfo);
    rspMsg.SetMsgBody(rspBody);
    return MP_SUCCESS;
}
