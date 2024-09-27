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
#ifndef TR_STRUCT_WITH_JSON_H
#define TR_STRUCT_WITH_JSON_H

#include <algorithm>
#include "define/Types.h"
#include "common/JsonHelper.h"
#include "common/Defines.h"
#include "ApplicationProtectBaseDataType_types.h"
#include "ApplicationProtectPlugin_types.h"
#include "ApplicationProtectFramework_types.h"

static mp_void StructToJson(const ApplicationPlugin& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    Module::JsonHelper::TypeToJsonValue(st.endPoint, jsonValue["endPoint"]);
    Module::JsonHelper::TypeToJsonValue(st.port, jsonValue["port"]);
    Module::JsonHelper::TypeToJsonValue(st.processId, jsonValue["processId"]);
}

static mp_void StructToJson(const Authentication& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.authType), jsonValue["authType"]);
    Module::JsonHelper::TypeToJsonValue(st.authkey, jsonValue["authKey"]);
    Module::JsonHelper::TypeToJsonValue(st.authPwd, jsonValue["authPwd"]);
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const ApplicationResource& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    Module::JsonHelper::TypeToJsonValue(st.subType, jsonValue["subType"]);
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    Module::JsonHelper::TypeToJsonValue(st.parentId, jsonValue["parentId"]);
    Module::JsonHelper::TypeToJsonValue(st.parentName, jsonValue["parentName"]);
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const ActionResult& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.code), jsonValue["code"]);
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.bodyErr), jsonValue["bodyErr"]);
    Module::JsonHelper::JsonStringToJsonValue(st.message, jsonValue["message"]);
}

static mp_void StructToJson(const QueryByPage& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.pageNo, jsonValue["pageNo"]);
    Module::JsonHelper::TypeToJsonValue(st.pageSize, jsonValue["pageSize"]);
    Module::JsonHelper::JsonStringToJsonValue(st.conditions, jsonValue["conditions"]);
}

static mp_void StructToJson(const Application& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    Module::JsonHelper::TypeToJsonValue(st.subType, jsonValue["subType"]);
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    Module::JsonHelper::TypeToJsonValue(st.parentId, jsonValue["parentId"]);
    Module::JsonHelper::TypeToJsonValue(st.parentName, jsonValue["parentName"]);
    StructToJson(st.auth, jsonValue["auth"]);
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const ApplicationEnvironment& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    Module::JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    Module::JsonHelper::TypeToJsonValue(st.subType, jsonValue["subType"]);
    Module::JsonHelper::TypeToJsonValue(st.endpoint, jsonValue["endpoint"]);
    Module::JsonHelper::TypeToJsonValue(st.port, jsonValue["port"]);
    StructToJson(st.auth, jsonValue["auth"]);
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
    for (const auto& node : st.nodes) {
        Json::Value value;
        Module::JsonHelper::TypeToJsonValue(node.id, value["id"]);
        Module::JsonHelper::TypeToJsonValue(node.name, value["name"]);
        Module::JsonHelper::TypeToJsonValue(node.type, value["type"]);
        Module::JsonHelper::TypeToJsonValue(node.subType, value["subType"]);
        Module::JsonHelper::TypeToJsonValue(node.endpoint, value["endpoint"]);
        Module::JsonHelper::TypeToJsonValue(node.port, value["port"]);
        StructToJson(node.auth, value["auth"]);
        Module::JsonHelper::JsonStringToJsonValue(node.extendInfo, value["extendInfo"]);
        jsonValue["nodes"].append(value);
    }
}

static mp_void StructToJson(const SubJob& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["jobId"]);
    Module::JsonHelper::TypeToJsonValue(st.subJobId, jsonValue["subJobId"]);
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.jobType), jsonValue["jobType"]);
    Module::JsonHelper::TypeToJsonValue(st.jobName, jsonValue["jobName"]);
    Module::JsonHelper::TypeToJsonValue(st.jobPriority, jsonValue["jobPriority"]);
    Module::JsonHelper::TypeToJsonValue(st.ignoreFailed, jsonValue["ignoreFailed"]);
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.policy), jsonValue["policy"]);
    jsonValue["jobInfo"] = st.jobInfo;
}

static const mp_void StructToJson(const std::vector<AppProtect::SubJob>& jobs, Json::Value& jsonValue)
{
    for (const auto& job : jobs) {
        Json::Value value;
        StructToJson(job, value);
        jsonValue.append(value);
    }
}

static mp_void StructToJson(const HostAddress& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.ip, jsonValue["ip"]);
    Module::JsonHelper::TypeToJsonValue(st.port, jsonValue["port"]);
    Module::JsonHelper::TypeToJsonValue(st.supportProtocol, jsonValue["supportProtocol"]);
}

static mp_void StructToJson(const StorageRepository& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.repositoryType), jsonValue["repositoryType"]);
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.role), jsonValue["role"]);
    for (auto item : st.path) {
        jsonValue["path"].append(item);
    }
    Module::JsonHelper::TypeToJsonValue(st.isLocal, jsonValue["isLocal"]);
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.protocol), jsonValue["protocol"]);
    StructToJson(st.auth, jsonValue["auth"]);
    StructToJson(st.extendAuth, jsonValue["extendAuth"]);
    StructToJson(st.endpoint, jsonValue["endpoint"]);
    for (std::vector<HostAddress>::const_iterator it = st.remoteHost.begin();
        it != st.remoteHost.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["remoteHost"].append(temp);
    }
    Module::JsonHelper::TypeToJsonValue(st.remotePath, jsonValue["remotePath"]);
    Module::JsonHelper::TypeToJsonValue(st.remoteName, jsonValue["remoteName"]);
    if (!st.extendInfo.empty()) {
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
    }
}

static mp_void StructToJson(const Snapshot& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(st.parentName, jsonValue["parentName"]);
}

static mp_void StructToJson(const Copy& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.formatType), jsonValue["format"]);
    auto iter = std::find_if(TransferMap_CopyDataType.begin(), TransferMap_CopyDataType.end(),
        [&st](const std::pair<std::string, AppProtect::CopyDataType>& item) -> bool {
        return item.second == st.dataType;
    });
    if (iter != TransferMap_CopyDataType.end()) {
        jsonValue["type"] = iter->first;
    }
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    Module::JsonHelper::TypeToJsonValue(st.timestamp, jsonValue["timestamp"]);
    Module::JsonHelper::TypeToJsonValue(st.transactionNo, jsonValue["transactionNo"]);
    StructToJson(st.protectEnv, jsonValue["protectEnv"]);
    StructToJson(st.protectObject, jsonValue["protectObject"]);
    for (std::vector<ApplicationResource>::const_iterator it = st.protectSubObjects.begin();
        it != st.protectSubObjects.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["protectSubObjects"].append(temp);
    }
    for (std::vector<StorageRepository>::const_iterator it = st.repositories.begin();
        it != st.repositories.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["repositories"].append(temp);
    }
    for (std::vector<Snapshot>::const_iterator it = st.snapshots.begin();
        it != st.snapshots.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["snapshots"].append(temp);
    }
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::Qos& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.bandwidth, jsonValue["bandwidth"]);
    Module::JsonHelper::TypeToJsonValue(st.protectIops, jsonValue["protectIops"]);
    Module::JsonHelper::TypeToJsonValue(st.backupIops, jsonValue["backupIops"]);
}

static mp_void StructToJson(const AppProtect::ResourceFilter& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.filterBy, jsonValue["filterBy"]);
    Module::JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    Module::JsonHelper::TypeToJsonValue(st.rule, jsonValue["rule"]);
    Module::JsonHelper::TypeToJsonValue(st.mode, jsonValue["mode"]);
    Module::JsonHelper::TypeToJsonValue(st.values, jsonValue["values"]);
}

static mp_void StructToJson(const AppProtect::DataLayout& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.encryption, jsonValue["encryption"]);
    Module::JsonHelper::TypeToJsonValue(st.deduption, jsonValue["deduption"]);
    Module::JsonHelper::TypeToJsonValue(st.compression, jsonValue["compression"]);
    Module::JsonHelper::TypeToJsonValue(st.nativeData, jsonValue["nativeData"]);
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::JobScripts& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.preScript, jsonValue["preScript"]);
    Module::JsonHelper::TypeToJsonValue(st.postScript, jsonValue["postScript"]);
    Module::JsonHelper::TypeToJsonValue(st.failPostScript, jsonValue["failPostScript"]);
}

static mp_void StructToJson(const AppProtect::BackupJobParam& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.backupType), jsonValue["backupType"]);
    StructToJson(st.dataLayout, jsonValue["dataLayout"]);
    StructToJson(st.qos, jsonValue["qos"]);
    for (std::vector<AppProtect::ResourceFilter>::const_iterator it = st.filters.begin();
        it != st.filters.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["filters"].append(temp);
    }
    StructToJson(st.scripts, jsonValue["scripts"]);
    Module::JsonHelper::JsonStringToJsonValue(st.advanceParams, jsonValue["advanceParams"]);
}

static mp_void StructToJson(const AppProtect::LivemountJobParam& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.advanceParams, jsonValue["advanceParams"]);
    StructToJson(st.scripts, jsonValue["scripts"]);
}

static mp_void StructToJson(const AppProtect::CancelLivemountJobParam& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.advanceParams, jsonValue["advanceParams"]);
    StructToJson(st.scripts, jsonValue["scripts"]);
}

static mp_void StructToJson(const AppProtect::BackupJob& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["jobId"]);
    StructToJson(st.jobParam, jsonValue["jobParam"]);
    StructToJson(st.protectEnv, jsonValue["protectEnv"]);
    StructToJson(st.protectObject, jsonValue["protectObject"]);
    for (std::vector<ApplicationResource>::const_iterator it = st.protectSubObject.begin();
        it != st.protectSubObject.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["protectSubObject"].append(temp);
    }
    for (std::vector<StorageRepository>::const_iterator it = st.repositories.begin();
        it != st.repositories.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["repositories"].append(temp);
    }
    {
        Json::Value temp;
        StructToJson(st.copy, temp);
        jsonValue["copy"].append(temp);
    }
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::RestoreJobParam& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.restoreType), jsonValue["restoreType"]);
    for (std::vector<AppProtect::ResourceFilter>::const_iterator it = st.filters.begin();
        it != st.filters.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["filters"].append(temp);
    }
    StructToJson(st.qos, jsonValue["qos"]);
    StructToJson(st.scripts, jsonValue["scripts"]);
    Module::JsonHelper::JsonStringToJsonValue(st.advanceParams, jsonValue["advanceParams"]);
}


static mp_void StructToJson(const AppProtect::RestoreJob& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["jobId"]);
    StructToJson(st.jobParam, jsonValue["jobParam"]);
    StructToJson(st.targetEnv, jsonValue["targetEnv"]);
    StructToJson(st.targetObject, jsonValue["targetObject"]);
    for (std::vector<ApplicationResource>::const_iterator it = st.restoreSubObjects.begin();
        it != st.restoreSubObjects.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["restoreSubObjects"].append(temp);
    }
    for (std::vector<Copy>::const_iterator it = st.copies.begin();
        it != st.copies.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["copies"].append(temp);
    }
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::LivemountJob& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["jobId"]);
    StructToJson(st.jobParam, jsonValue["jobParam"]);
    StructToJson(st.targetEnv, jsonValue["targetEnv"]);
    StructToJson(st.targetObject, jsonValue["targetObject"]);
    for (std::vector<ApplicationResource>::const_iterator it = st.targetSubObjects.begin();
        it != st.targetSubObjects.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["targetSubObjects"].append(temp);
    }
    {
        Json::Value temp;
        StructToJson(st.copy, temp);
        jsonValue["copy"].append(temp);
    }
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::CancelLivemountJob& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["jobId"]);
    StructToJson(st.jobParam, jsonValue["jobParam"]);
    StructToJson(st.targetEnv, jsonValue["targetEnv"]);
    StructToJson(st.targetObject, jsonValue["targetObject"]);
    for (std::vector<ApplicationResource>::const_iterator it = st.targetSubObjects.begin();
        it != st.targetSubObjects.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["targetSubObjects"].append(temp);
    }
    {
        Json::Value temp;
        StructToJson(st.copy, temp);
        jsonValue["copy"].append(temp);
    }
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::BuildIndexJobParam& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.preCopyId, jsonValue["preCopyId"]);
    Module::JsonHelper::TypeToJsonValue(st.indexPath, jsonValue["indexPath"]);
}

static mp_void StructToJson(const AppProtect::BuildIndexJob& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["jobId"]);
    StructToJson(st.jobParam, jsonValue["jobParam"]);
    StructToJson(st.indexEnv, jsonValue["indexEnv"]);
    StructToJson(st.indexProtectObject, jsonValue["indexProtectObject"]);
    for (std::vector<ApplicationResource>::const_iterator it = st.indexProtectSubObject.begin();
        it != st.indexProtectSubObject.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["indexProtectSubObject"].append(temp);
    }
    for (std::vector<StorageRepository>::const_iterator it = st.repositories.begin();
        it != st.repositories.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["repositories"].append(temp);
    }
    for (std::vector<Copy>::const_iterator it = st.copies.begin();
        it != st.copies.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["copies"].append(temp);
    }
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::LogDetail& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.level), jsonValue["logLevel"]);
    Module::JsonHelper::TypeToJsonValue(st.description, jsonValue["logInfo"]);
    Module::JsonHelper::TypeToJsonValue(st.params, jsonValue["logInfoParam"]);
    Module::JsonHelper::TypeToJsonValue(st.timestamp, jsonValue["logTimestamp"]);
    if (st.errorCode != 0) {
        Module::JsonHelper::TypeToJsonValue(st.errorCode, jsonValue["logDetail"]);
        Module::JsonHelper::TypeToJsonValue(st.errorParams, jsonValue["logDetailParam"]);
        Module::JsonHelper::TypeToJsonValue(st.additionalDesc, jsonValue["logDetailInfo"]);
    }
}

static mp_void StructToJson(const AppProtect::SubJobDetails& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["taskId"]);
    Module::JsonHelper::TypeToJsonValue(st.subJobId, jsonValue["subTaskId"]);
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.jobStatus), jsonValue["taskStatus"]);
    Module::JsonHelper::TypeToJsonValue(st.additionalStatus, jsonValue["additionalStatus"]);
    for (std::vector<AppProtect::LogDetail>::const_iterator it = st.logDetail.begin();
        it != st.logDetail.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["logDetail"].append(temp);
    }
    Module::JsonHelper::TypeToJsonValue(st.progress, jsonValue["progress"]);
    if (st.__isset.dataSize) {
        Module::JsonHelper::TypeToJsonValue(st.dataSize, jsonValue["dataSize"]);
    } else {
        Module::JsonHelper::TypeToJsonValue(UNSET_DATA_SIZE, jsonValue["dataSize"]);
    }
    Module::JsonHelper::TypeToJsonValue(st.speed, jsonValue["speed"]);
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}


static mp_void StructToJson(const AppProtect::JobPermission& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.user, jsonValue["user"]);
    Module::JsonHelper::TypeToJsonValue(st.group, jsonValue["group"]);
    Module::JsonHelper::TypeToJsonValue(st.fileMode, jsonValue["fileMode"]);
    Module::JsonHelper::TypeToJsonValue(st.isMount, jsonValue["isMount"]);
    Module::JsonHelper::TypeToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::DelCopyJob& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["jobId"]);
    StructToJson(st.protectEnv, jsonValue["protectEnv"]);
    StructToJson(st.protectObject, jsonValue["protectObject"]);
    for (std::vector<Copy>::const_iterator it = st.copies.begin();
        it != st.copies.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["copies"].append(temp);
    }
    for (std::vector<StorageRepository>::const_iterator it = st.repositories.begin();
        it != st.repositories.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["repositories"].append(temp);
    }
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::Resource& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.scope), jsonValue["scope"]);
    Module::JsonHelper::TypeToJsonValue(st.scopeKey, jsonValue["scopeKey"]);
    Module::JsonHelper::TypeToJsonValue(st.resourceKey, jsonValue["resourceKey"]);
    Module::JsonHelper::TypeToJsonValue(st.resourceValue, jsonValue["resourceValue"]);
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.sharedNum), jsonValue["sharedNum"]);
    Module::JsonHelper::TypeToJsonValue(st.lockType, jsonValue["lockType"]);
}

static mp_void StructToJson(const AppProtect::ResourceStatus& st, Json::Value& jsonValue)
{
    StructToJson(st.resource, jsonValue["resource"]);
    Module::JsonHelper::TypeToJsonValue(mp_int32(st.lockNum), jsonValue["lockNum"]);
}

static mp_void StructToJson(const AppProtect::CheckCopyJob& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["jobId"]);
    StructToJson(st.protectEnv, jsonValue["protectEnv"]);
    StructToJson(st.protectObject, jsonValue["protectObject"]);
    for (std::vector<StorageRepository>::const_iterator it = st.repositories.begin();
        it != st.repositories.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["repositories"].append(std::move(temp));
    }
    for (std::vector<Copy>::const_iterator it = st.copies.begin();
        it != st.copies.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["copies"].append(std::move(temp));
    }
    Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::OracleDBInfo& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.oracleUser, jsonValue["oracleUser"]);
    Module::JsonHelper::TypeToJsonValue(st.dbName, jsonValue["dbName"]);
    Module::JsonHelper::TypeToJsonValue(st.instanceName, jsonValue["instanceName"]);
    Module::JsonHelper::TypeToJsonValue(st.dbUser, jsonValue["dbUser"]);
    Module::JsonHelper::TypeToJsonValue(st.dbPassword, jsonValue["dbPassword"]);
    Module::JsonHelper::TypeToJsonValue(st.gridUser, jsonValue["gridUser"]);
    Module::JsonHelper::TypeToJsonValue(st.asmName, jsonValue["asmName"]);
    Module::JsonHelper::TypeToJsonValue(st.asmUser, jsonValue["asmUser"]);
    Module::JsonHelper::TypeToJsonValue(st.asmPassword, jsonValue["asmPassword"]);
    Module::JsonHelper::TypeToJsonValue(st.archThreshold, jsonValue["archThreshold"]);
    Module::JsonHelper::TypeToJsonValue(st.runUserPwd, jsonValue["runUserPwd"]);
    Module::JsonHelper::TypeToJsonValue(st.accessOracleBase, jsonValue["accessOracleBase"]);
    Module::JsonHelper::TypeToJsonValue(st.accessOracleHome, jsonValue["accessOracleHome"]);
}
#endif