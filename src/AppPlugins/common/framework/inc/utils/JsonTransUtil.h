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
#ifndef TR_JSON_WITH_STRUCT_H
#define TR_JSON_WITH_STRUCT_H

#include "define/Types.h"
#include "common/JsonHelper.h"
#include "ApplicationProtectBaseDataType_types.h"
#include "ApplicationProtectPlugin_types.h"
#include "ApplicationProtectFramework_types.h"

static void StructToJson(const ApplicationPlugin& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    Module::JsonHelper::TypeToJsonValue(st.endPoint, jsonValue["endPoint"]);
    Module::JsonHelper::TypeToJsonValue(st.port, jsonValue["port"]);
    Module::JsonHelper::TypeToJsonValue(st.processId, jsonValue["processId"]);
}


static void StructToJson(const Authentication& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(int(st.authType), jsonValue["authType"]);
    Module::JsonHelper::TypeToJsonValue(st.authkey, jsonValue["authkey"]);
    if (!st.extendInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const ApplicationResource& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    Module::JsonHelper::TypeToJsonValue(st.subType, jsonValue["subType"]);
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    Module::JsonHelper::TypeToJsonValue(st.parentId, jsonValue["parentId"]);
    Module::JsonHelper::TypeToJsonValue(st.parentName, jsonValue["parentName"]);
    if (!st.extendInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const Application& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    Module::JsonHelper::TypeToJsonValue(st.subType, jsonValue["subType"]);
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    Module::JsonHelper::TypeToJsonValue(st.parentId, jsonValue["parentId"]);
    Module::JsonHelper::TypeToJsonValue(st.parentName, jsonValue["parentName"]);
    StructToJson(st.auth, jsonValue["auth"]);
    if (!st.extendInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const ApplicationEnvironment& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    Module::JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    Module::JsonHelper::TypeToJsonValue(st.subType, jsonValue["subType"]);
    Module::JsonHelper::TypeToJsonValue(st.endpoint, jsonValue["endpoint"]);
    Module::JsonHelper::TypeToJsonValue(st.port, jsonValue["port"]);
    StructToJson(st.auth, jsonValue["auth"]);
    for (auto node : st.nodes) {
        Json::Value temp;
        StructToJson(node, temp);
        jsonValue["nodes"].append(temp);
    }
    if (!st.extendInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const QueryByPage& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.pageNo, jsonValue["pageNo"]);
    Module::JsonHelper::TypeToJsonValue(st.pageSize, jsonValue["pageSize"]);
    for (auto order : st.orders) {
        jsonValue["orders"].append(order);
    }
    Module::JsonHelper::TypeToJsonValue(st.conditions, jsonValue["conditions"]);
}

static void StructToJson(const AppProtect::ListResourceRequest& st, Json::Value& jsonValue)
{
    StructToJson(st.appEnv, jsonValue["appEnv"]);
    for (auto application : st.applications) {
        Json::Value temp;
        StructToJson(application, temp);
        jsonValue["applications"].append(temp);
    }
    StructToJson(st.condition, jsonValue["condition"]);
}

static void StructToJson(const SubJob& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["jobId"]);
    Module::JsonHelper::TypeToJsonValue(st.subJobId, jsonValue["subJobId"]);
    Module::JsonHelper::TypeToJsonValue(int(st.jobType), jsonValue["jobType"]);
    Module::JsonHelper::TypeToJsonValue(st.jobName, jsonValue["jobName"]);
    Module::JsonHelper::TypeToJsonValue(st.jobPriority, jsonValue["jobPriority"]);
    Module::JsonHelper::TypeToJsonValue(int(st.policy), jsonValue["policy"]);
    Module::JsonHelper::TypeToJsonValue(st.ignoreFailed, jsonValue["ignoreFailed"]);
    Module::JsonHelper::TypeToJsonValue(st.execNodeId, jsonValue["execNodeId"]);
    if (!st.jobInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.jobInfo, jsonValue["jobInfo"]);
}

static void StructToJson(const std::vector<AppProtect::SubJob>& jobs, Json::Value& jsonValue)
{
    for (const auto& job : jobs) {
        Json::Value value;
        StructToJson(job, value);
        jsonValue.append(value);
    }
}

static void StructToJson(const HostAddress& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.ip, jsonValue["ip"]);
    Module::JsonHelper::TypeToJsonValue(st.port, jsonValue["port"]);
    Module::JsonHelper::TypeToJsonValue(st.supportProtocol, jsonValue["supportProtocol"]);
}

static void StructToJson(const StorageRepository& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(int(st.repositoryType), jsonValue["repositoryType"]);
    Module::JsonHelper::TypeToJsonValue(int(st.role), jsonValue["role"]);
    Module::JsonHelper::TypeToJsonValue(st.isLocal, jsonValue["isLocal"]);
    for (auto item : st.path) {
        jsonValue["path"].append(item);
    }
    Module::JsonHelper::TypeToJsonValue(int(st.protocol), jsonValue["protocol"]);
    StructToJson(st.auth, jsonValue["auth"]);
    StructToJson(st.endpoint, jsonValue["endpoint"]);
    Module::JsonHelper::TypeToJsonValue(st.remotePath, jsonValue["remotePath"]);
    Module::JsonHelper::TypeToJsonValue(st.remoteName, jsonValue["remoteName"]);
    for (std::vector<HostAddress>::const_iterator it = st.remoteHost.begin();
         it != st.remoteHost.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["remoteHost"].append(temp);
    }
    StructToJson(st.extendAuth, jsonValue["extendAuth"]);
    if (!st.extendInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const Snapshot& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(st.parentName, jsonValue["parentName"]);
}

static void StructToJson(const Copy& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(int(st.formatType), jsonValue["formatType"]);
    Module::JsonHelper::TypeToJsonValue(int(st.dataType), jsonValue["dataType"]);
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
    if (!st.extendInfo.empty()) {
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
    }
}

static void StructToJson(const AppProtect::Qos& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.bandwidth, jsonValue["bandwidth"]);
    Module::JsonHelper::TypeToJsonValue(st.protectIops, jsonValue["protectIops"]);
    Module::JsonHelper::TypeToJsonValue(st.backupIops, jsonValue["backupIops"]);
}

static void StructToJson(const AppProtect::ResourceFilter& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.filterBy, jsonValue["filterBy"]);
    Module::JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    Module::JsonHelper::TypeToJsonValue(st.rule, jsonValue["rule"]);
    Module::JsonHelper::TypeToJsonValue(st.mode, jsonValue["mode"]);
    Module::JsonHelper::TypeToJsonValue(st.values, jsonValue["values"]);
}

static void StructToJson(const AppProtect::BuildIndexJobParam& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.preCopyId, jsonValue["preCopyId"]);
    Module::JsonHelper::TypeToJsonValue(st.indexPath, jsonValue["indexPath"]);
}

static void StructToJson(const AppProtect::DataLayout& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.encryption, jsonValue["encryption"]);
    Module::JsonHelper::TypeToJsonValue(st.deduption, jsonValue["deduption"]);
    Module::JsonHelper::TypeToJsonValue(st.compression, jsonValue["compression"]);
    Module::JsonHelper::TypeToJsonValue(st.nativeData, jsonValue["nativeData"]);
    if (!st.extendInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const AppProtect::JobScripts& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.preScript, jsonValue["preScript"]);
    Module::JsonHelper::TypeToJsonValue(st.postScript, jsonValue["postScript"]);
    Module::JsonHelper::TypeToJsonValue(st.failPostScript, jsonValue["failPostScript"]);
}

static void StructToJson(const AppProtect::BackupJobParam& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(int(st.backupType), jsonValue["backupType"]);
    StructToJson(st.dataLayout, jsonValue["dataLayout"]);
    StructToJson(st.qos, jsonValue["qos"]);
    for (std::vector<AppProtect::ResourceFilter>::const_iterator it = st.filters.begin();
         it != st.filters.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["filters"].append(temp);
    }
    StructToJson(st.scripts, jsonValue["scripts"]);
    if (!st.advanceParams.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.advanceParams, jsonValue["advanceParams"]);
}

static void StructToJson(const AppProtect::BackupJob& st, Json::Value& jsonValue)
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
    StructToJson(st.copy, jsonValue["copy"]);
    if (!st.extendInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const AppProtect::RestoreJobParam& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(int(st.restoreType), jsonValue["restoreType"]);
    Module::JsonHelper::TypeToJsonValue(st.restoreMode, jsonValue["restoreMode"]);
    for (std::vector<AppProtect::ResourceFilter>::const_iterator it = st.filters.begin();
         it != st.filters.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["filters"].append(temp);
    }
    StructToJson(st.qos, jsonValue["qos"]);
    StructToJson(st.scripts, jsonValue["scripts"]);
    if (!st.advanceParams.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.advanceParams, jsonValue["advanceParams"]);
}

static void StructToJson(const AppProtect::RestoreJob& st, Json::Value& jsonValue)
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
    if (!st.extendInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const AppProtect::BuildIndexJob& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["jobId"]);
    StructToJson(st.jobParam, jsonValue["jobParam"]);
    StructToJson(st.indexEnv, jsonValue["indexEnv"]);
    for (std::vector<Copy>::const_iterator it = st.copies.begin();
        it != st.copies.end(); ++it) {
            Json::Value temp;
            StructToJson(*it, temp);
            jsonValue["copies"].append(temp);
    }
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
    if (!st.extendInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const AppProtect::LivemountJobParam& st, Json::Value& jsonValue)
{
    StructToJson(st.scripts, jsonValue["scripts"]);
    Module::JsonHelper::TypeToJsonValue(st.advanceParams, jsonValue["advanceParams"]);
}

static void StructToJson(const AppProtect::LivemountJob& st, Json::Value& jsonValue)
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
    StructToJson(st.copy, jsonValue["copy"]);
    Module::JsonHelper::TypeToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const AppProtect::CancelLivemountJobParam& st, Json::Value& jsonValue)
{
    StructToJson(st.scripts, jsonValue["scripts"]);
    Module::JsonHelper::TypeToJsonValue(st.advanceParams, jsonValue["advanceParams"]);
}
 
static void StructToJson(const AppProtect::CancelLivemountJob& st, Json::Value& jsonValue)
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
    StructToJson(st.copy, jsonValue["copy"]);
    Module::JsonHelper::TypeToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const AppProtect::DelCopyJob& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["jobId"]);
    StructToJson(st.protectEnv, jsonValue["protectEnv"]);
    StructToJson(st.protectObject, jsonValue["protectObject"]);
    for (std::vector<AppProtect::StorageRepository>::const_iterator it = st.repositories.begin();
         it != st.repositories.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["repositories"].append(temp);
    }
    for (std::vector<AppProtect::Copy>::const_iterator it = st.copies.begin();
         it != st.copies.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["copies"].append(temp);
    }
    if (!st.extendInfo.empty()) {
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
    }
}

static void StructToJson(const AppProtect::CheckCopyJob& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    Module::JsonHelper::TypeToJsonValue(st.jobId, jsonValue["jobId"]);
    StructToJson(st.protectEnv, jsonValue["protectEnv"]);
    StructToJson(st.protectObject, jsonValue["protectObject"]);
    for (std::vector<AppProtect::StorageRepository>::const_iterator it = st.repositories.begin();
         it != st.repositories.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["repositories"].append(temp);
    }
    for (std::vector<AppProtect::Copy>::const_iterator it = st.copies.begin();
         it != st.copies.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["copies"].append(temp);
    }
    if (!st.extendInfo.empty()) {
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
    }
}

static void StructToJson(const AppProtect::OracleDBInfo& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.oracleUser,    jsonValue["oracleUser"]);
    Module::JsonHelper::TypeToJsonValue(st.dbName,        jsonValue["dbName"]);
    Module::JsonHelper::TypeToJsonValue(st.instanceName,  jsonValue["instanceName"]);
    Module::JsonHelper::TypeToJsonValue(st.dbUser,        jsonValue["dbUser"]);
    Module::JsonHelper::TypeToJsonValue(st.gridUser,      jsonValue["gridUser"]);
    Module::JsonHelper::TypeToJsonValue(st.asmName,       jsonValue["asmName"]);
    Module::JsonHelper::TypeToJsonValue(st.asmUser,       jsonValue["asmUser"]);
    Module::JsonHelper::TypeToJsonValue(st.archThreshold, jsonValue["archThreshold"]);
}

static void StructToJson(const std::vector<AppProtect::OracleDBInfo> &infos, Json::Value &jsonValue)
{
    for (const auto& info : infos) {
        Json::Value value;
        StructToJson(info, value);
        jsonValue.append(value);
    }
}

/* api with return Json value, needs to be put at the end */
template <class T>
Json::Value StructToJson(const T& t)
{
    Json::Value jv;
    StructToJson(t, jv);
    return jv;
}
#endif