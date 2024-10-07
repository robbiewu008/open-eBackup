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

#include <algorithm>
#include "define/Types.h"
#include "common/JsonHelper.h"
#include "common/Defines.h"
#include "ApplicationProtectBaseDataType_types.h"
#include "ApplicationProtectPlugin_types.h"
#include "ApplicationProtectFramework_types.h"

// json to nomal type
#define JSON_TO_VALUE(key, value) if (jsonValue.isObject() && jsonValue.isMember(#key)) { \
    Module::JsonHelper::JsonValueToType(value, jsonValue[#key]); \
}

// json to nomal type
#define JSON_TO_STRUCT(key, name) if (jsonValue.isObject() && jsonValue.isMember(#key)) { \
    Module::JsonHelper::JsonValueToType(st.name, jsonValue[#key]); \
    st.__set_##name(st.name); \
}

// json to struct
#define JSON_TO_STRUCT_STRUCT(key, name) if (jsonValue.isObject() && jsonValue.isMember(#key) \
    && jsonValue[#key].isObject()) { \
    JsonToStruct(jsonValue[#key], st.name); \
    st.__set_##name(st.name); \
}

// jsonObj to string
#define JSON_TO_STRUCT_OBJ(key, name) do { \
    if (jsonValue.isObject() && jsonValue.isMember(#key) && jsonValue[#key].isObject()) { \
        st.__set_##name(jsonValue[#key].toStyledString()); \
    } \
} while (0)

// json to enum
#define JSON_TO_STRUCT_ENUM(key, name, T)  do { \
    if (jsonValue.isObject() && jsonValue.isMember(#key) && jsonValue[#key].isInt()) { \
        mp_int32 nType = jsonValue[#key].asInt(); \
        st.__set_##name(T(nType)); \
    } \
} while (0)

// json to array struct
#define JSON_TO_STRUCT_ARRAY(key, name, T)  do { \
    if (jsonValue.isObject() && jsonValue.isMember(#key) && jsonValue[#key].isArray()) { \
        for (Json::ArrayIndex index = 0; index < jsonValue[#key].size(); ++index) { \
            T ele; \
            JsonToStruct(jsonValue[#key][index], ele); \
            st.name.push_back(ele); \
        } \
        st.__set_##name(st.name); \
    } \
} while (0)

// json array to single struct
#define JSON_ARRAY_TO_SINGLE_STRUCT(key, name, T)  do { \
    if (jsonValue.isObject() && jsonValue.isMember(#key) && jsonValue[#key].isArray()) { \
        for (Json::ArrayIndex index = 0; index < jsonValue[#key].size(); ++index) { \
            T ele; \
            JsonToStruct(jsonValue[#key][index], ele); \
            st.__set_##name(ele); \
            break; \
        } \
    } \
} while (0)

// json to array string
#define JSON_TO_STRING_ARRAY(key, name)  do { \
    if (jsonValue.isObject() && jsonValue.isMember(#key) && jsonValue[#key].isArray()) { \
        for (Json::ArrayIndex index = 0; index < jsonValue[#key].size(); ++index) { \
            if (jsonValue[#key][index].isString()) { \
                st.name.push_back(jsonValue[#key][index].asString()); \
            } \
        } \
        st.__set_##name(st.name); \
    } \
} while (0)

namespace {
    constexpr int64_t UNSET_DATA_SIZE = -1;
    enum class NodeRole {
        NONE = 0,
        ACTIVE = 1,
        STANDBY = 2,
        SHARD = 3
    };
}

static mp_void JsonToStruct(const Json::Value& jsonValue, ApplicationPlugin& st)
{
    JSON_TO_STRUCT(name, name);
    JSON_TO_STRUCT(endPoint, endPoint);
    JSON_TO_STRUCT(port, port);
    JSON_TO_STRUCT(processId, processId);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, Authentication& st)
{
    JSON_TO_STRUCT_ENUM(authType, authType, AuthType::type);
    JSON_TO_STRUCT(authKey, authkey);
    JSON_TO_STRUCT(authPwd, authPwd);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, ApplicationResource& st)
{
    JSON_TO_STRUCT(type, type);
    JSON_TO_STRUCT(subType, subType);
    JSON_TO_STRUCT(id, id);
    JSON_TO_STRUCT(name, name);
    JSON_TO_STRUCT(parentId, parentId);
    JSON_TO_STRUCT(parentName, parentName);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, Application& st)
{
    JSON_TO_STRUCT(type, type);
    JSON_TO_STRUCT(subType, subType);
    JSON_TO_STRUCT(id, id);
    JSON_TO_STRUCT(name, name);
    JSON_TO_STRUCT(parentId, parentId);
    JSON_TO_STRUCT(parentName, parentName);
    JSON_TO_STRUCT_STRUCT(auth, auth);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, ActionResult& st)
{
    JSON_TO_STRUCT(code, code);
    JSON_TO_STRUCT(bodyErr, bodyErr);
    JSON_TO_STRUCT(message, message);
    JSON_TO_STRING_ARRAY(bodyErrParams, bodyErrParams);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, ApplicationEnvironment& st)
{
    JSON_TO_STRUCT(id, id);
    JSON_TO_STRUCT(name, name);
    JSON_TO_STRUCT(type, type);
    JSON_TO_STRUCT(subType, subType);
    JSON_TO_STRUCT(endpoint, endpoint);
    JSON_TO_STRUCT(port, port);
    JSON_TO_STRUCT_STRUCT(auth, auth);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
    JSON_TO_STRUCT_ARRAY(nodes, nodes, ApplicationEnvironment);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, SubJob& st)
{
    JSON_TO_STRUCT(jobId, jobId);
    JSON_TO_STRUCT(subJobId, subJobId);
    JSON_TO_STRUCT_ENUM(jobType, jobType, SubJobType::type);
    JSON_TO_STRUCT(jobName, jobName);
    JSON_TO_STRUCT(jobPriority, jobPriority);
    JSON_TO_STRUCT_ENUM(policy, policy, ExecutePolicy::type);
    JSON_TO_STRUCT(ignoreFailed, ignoreFailed);
    JSON_TO_STRUCT(execNodeId, execNodeId);
    JSON_TO_STRUCT(jobInfo, jobInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, HostAddress& st)
{
    JSON_TO_STRUCT(ip, ip);
    JSON_TO_STRUCT(port, port);
    JSON_TO_STRUCT(supportProtocol, supportProtocol);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, StorageRepository& st)
{
    JSON_TO_STRUCT(id, id);
    JSON_TO_STRUCT_ENUM(repositoryType, repositoryType, RepositoryDataType::type);
    JSON_TO_STRUCT_ENUM(role, role, RepositoryRole::type);
    JSON_TO_STRING_ARRAY(path, path);
    JSON_TO_STRUCT(isLocal, isLocal);
    JSON_TO_STRUCT_ENUM(protocol, protocol, RepositoryProtocolType::type);
    JSON_TO_STRUCT_STRUCT(auth, auth);
    JSON_TO_STRUCT_STRUCT(extendAuth, extendAuth);
    JSON_TO_STRUCT_STRUCT(endpoint, endpoint);
    JSON_TO_STRUCT_ARRAY(remoteHost, remoteHost, HostAddress);
    JSON_TO_STRUCT(remotePath, remotePath);
    JSON_TO_STRUCT(remoteName, remoteName);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, Snapshot& st)
{
    JSON_TO_STRUCT(id, id);
    JSON_TO_STRUCT(parentName, parentName);
}

static std::map<std::string, AppProtect::CopyDataType> TransferMap_CopyDataType = {
    {"full", AppProtect::CopyDataType::FULL_COPY},
    {"increment", AppProtect::CopyDataType::INCREMENT_COPY},
    {"diff", AppProtect::CopyDataType::DIFF_COPY},
    {"log", AppProtect::CopyDataType::LOG_COPY},
    {"nativeSnapshot", AppProtect::CopyDataType::SNAPSHOT_COPY},
    {"foreverIncrement", AppProtect::CopyDataType::PERMANENT_INCREMENTAL_COPY},
    {"replication", AppProtect::CopyDataType::REPLICATION_COPY},
    {"s3Archive", AppProtect::CopyDataType::CLOUD_STORAGE_COPY},
    {"tapeArchive", AppProtect::CopyDataType::TAPE_STORAGE_COPY},
    {"clone", AppProtect::CopyDataType::CLONE_COPY},
};

static mp_void JsonToStruct(const Json::Value& jsonValue, Copy& st)
{
    JSON_TO_STRUCT_ENUM(format, formatType, CopyFormatType::type);
    std::string strType;
    JSON_TO_VALUE(type, strType);
    auto it = TransferMap_CopyDataType.find(strType);
    if (it != TransferMap_CopyDataType.end()) {
        st.__set_dataType(it->second);
    }
    JSON_TO_STRUCT(id, id);
    JSON_TO_STRUCT(name, name);
    JSON_TO_STRUCT(timestamp, timestamp);
    JSON_TO_STRUCT(transactionNo, transactionNo);
    JSON_TO_STRUCT_STRUCT(protectEnv, protectEnv);
    JSON_TO_STRUCT_STRUCT(protectObject, protectObject);
    JSON_TO_STRUCT_ARRAY(protectSubObject, protectSubObjects, ApplicationResource);
    JSON_TO_STRUCT_ARRAY(repositories, repositories, StorageRepository);
    JSON_TO_STRUCT_ARRAY(snapshots, snapshots, Snapshot);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::Qos& st)
{
    JSON_TO_STRUCT(bandwidth, bandwidth);
    JSON_TO_STRUCT(protectIops, protectIops);
    JSON_TO_STRUCT(backupIops, backupIops);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::ResourceFilter& st)
{
    JSON_TO_STRUCT(filterBy, filterBy);
    JSON_TO_STRUCT(type, type);
    JSON_TO_STRUCT(rule, rule);
    JSON_TO_STRUCT(mode, mode);
    JSON_TO_STRUCT(values, values);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::DataLayout& st)
{
    JSON_TO_STRUCT(encryption, encryption);
    JSON_TO_STRUCT(deduption, deduption);
    JSON_TO_STRUCT(compression, compression);
    JSON_TO_STRUCT(nativeData, nativeData);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::JobScripts& st)
{
    JSON_TO_STRUCT(preScript, preScript);
    JSON_TO_STRUCT(postScript, postScript);
    JSON_TO_STRUCT(failPostScript, failPostScript);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::BackupJobParam& st)
{
    static std::map<std::string, AppProtect::BackupJobType> TransferMap = {
        {"snapshot", AppProtect::BackupJobType::SNAPSHOT},
        {"fullBackup", AppProtect::BackupJobType::FULL_BACKUP},
        {"incrementBackup", AppProtect::BackupJobType::INCREMENT_BACKUP},
        {"diffBackup", AppProtect::BackupJobType::DIFF_BACKUP},
        {"logBackup", AppProtect::BackupJobType::LOG_BAKCUP},
        {"foreverIncrementBackup", AppProtect::BackupJobType::PERMANENT_INCREMENTAL_BACKUP}
    };

    std::string backupType;
    JSON_TO_VALUE(backupType, backupType);
    auto it = TransferMap.find(backupType);
    if (it != TransferMap.end()) {
        st.__set_backupType(it->second);
    }
    JSON_TO_STRUCT_STRUCT(dataLayout, dataLayout);
    JSON_TO_STRUCT_STRUCT(qos, qos);
    JSON_TO_STRUCT_ARRAY(filters, filters, AppProtect::ResourceFilter);
    JSON_TO_STRUCT_STRUCT(scripts, scripts);
    JSON_TO_STRUCT_OBJ(advanceParams, advanceParams);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::LivemountJobParam& st)
{
    JSON_TO_STRUCT(advanceParams, advanceParams);
    JSON_TO_STRUCT_STRUCT(scripts, scripts);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::CancelLivemountJobParam& st)
{
    JSON_TO_STRUCT(advanceParams, advanceParams);
    JSON_TO_STRUCT_STRUCT(scripts, scripts);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::BackupJob& st)
{
    JSON_TO_STRUCT(requestId, requestId);
    JSON_TO_STRUCT(jobId, jobId);
    JSON_TO_STRUCT_STRUCT(jobParam, jobParam);
    JSON_TO_STRUCT_STRUCT(protectEnv, protectEnv);
    JSON_TO_STRUCT_STRUCT(protectObject, protectObject);
    JSON_TO_STRUCT_ARRAY(protectSubObject, protectSubObject, ApplicationResource);
    JSON_TO_STRUCT_ARRAY(repositories, repositories, StorageRepository);
    JSON_ARRAY_TO_SINGLE_STRUCT(copy, copy, Copy);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::RestoreJobParam& st)
{
    static std::map<std::string, AppProtect::RestoreJobType::type> TransferMap = {
        {"normalRestore", AppProtect::RestoreJobType::type::NORMAL_RESTORE},
        {"instantRestore", AppProtect::RestoreJobType::type::INSTANT_RESTORE},
        {"fineGrainedRestore", AppProtect::RestoreJobType::type::FINE_GRAINED_RESTORE}
    };

    std::string restoreType;
    JSON_TO_VALUE(restoreType, restoreType);
    auto it = TransferMap.find(restoreType);
    if (it != TransferMap.end()) {
        st.__set_restoreType(it->second);
    }
    JSON_TO_STRUCT_ENUM(restoreType, restoreType, AppProtect::RestoreJobType::type);
    JSON_TO_STRUCT_ARRAY(filters, filters, AppProtect::ResourceFilter);
    JSON_TO_STRUCT_STRUCT(qos, qos);
    JSON_TO_STRUCT_STRUCT(scripts, scripts);
    JSON_TO_STRUCT_OBJ(advanceParams, advanceParams);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::RestoreJob& st)
{
    JSON_TO_STRUCT(requestId, requestId);
    JSON_TO_STRUCT(jobId, jobId);
    JSON_TO_STRUCT_STRUCT(jobParam, jobParam);
    JSON_TO_STRUCT_STRUCT(targetEnv, targetEnv);
    JSON_TO_STRUCT_STRUCT(targetObject, targetObject);
    JSON_TO_STRUCT_ARRAY(restoreSubObjects, restoreSubObjects, ApplicationResource);
    JSON_TO_STRUCT_ARRAY(copies, copies, Copy);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::LivemountJob& st)
{
    JSON_TO_STRUCT(requestId, requestId);
    JSON_TO_STRUCT(jobId, jobId);
    JSON_TO_STRUCT_STRUCT(jobParam, jobParam);
    JSON_TO_STRUCT_STRUCT(targetEnv, targetEnv);
    JSON_TO_STRUCT_STRUCT(targetObject, targetObject);
    JSON_TO_STRUCT_ARRAY(targetSubObjects, targetSubObjects, ApplicationResource);
    JSON_ARRAY_TO_SINGLE_STRUCT(copy, copy, Copy);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::CancelLivemountJob& st)
{
    JSON_TO_STRUCT(requestId, requestId);
    JSON_TO_STRUCT(jobId, jobId);
    JSON_TO_STRUCT_STRUCT(jobParam, jobParam);
    JSON_TO_STRUCT_STRUCT(targetEnv, targetEnv);
    JSON_TO_STRUCT_STRUCT(targetObject, targetObject);
    JSON_TO_STRUCT_ARRAY(targetSubObjects, targetSubObjects, ApplicationResource);
    JSON_ARRAY_TO_SINGLE_STRUCT(copy, copy, Copy);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::BuildIndexJobParam& st)
{
    JSON_TO_STRUCT(preCopyId, preCopyId);
    JSON_TO_STRUCT(indexPath, indexPath);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::BuildIndexJob& st)
{
    JSON_TO_STRUCT(requestId, requestId);
    JSON_TO_STRUCT(jobId, jobId);
    JSON_TO_STRUCT_STRUCT(jobParam, jobParam);
    JSON_TO_STRUCT_STRUCT(indexEnv, indexEnv);
    JSON_TO_STRUCT_STRUCT(indexProtectObject, indexProtectObject);
    JSON_TO_STRUCT_ARRAY(indexProtectSubObject, indexProtectSubObject, ApplicationResource);
    JSON_TO_STRUCT_ARRAY(repositories, repositories, StorageRepository);
    JSON_TO_STRUCT_ARRAY(copies, copies, Copy);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::LogDetail& st)
{
    JSON_TO_STRUCT_ENUM(logLevel, level, AppProtect::JobLogLevel::type);
    JSON_TO_STRUCT(logInfo, description);
    JSON_TO_STRUCT(logInfoParam, params);
    JSON_TO_STRUCT(logTimestamp, timestamp);
    JSON_TO_STRUCT(logDetail, errorCode);
    JSON_TO_STRUCT(logDetailParam, errorParams);
    JSON_TO_STRUCT(logDetailInfo, additionalDesc);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::SubJobDetails& st)
{
    JSON_TO_STRUCT(taskId, jobId);
    JSON_TO_STRUCT(subTaskId, subJobId);
    JSON_TO_STRUCT_ENUM(taskStatus, jobStatus, AppProtect::SubJobStatus::type);
    JSON_TO_STRUCT(additionalStatus, additionalStatus);
    JSON_TO_STRUCT_ARRAY(logDetail, logDetail, AppProtect::LogDetail);
    JSON_TO_STRUCT(progress, progress);
    JSON_TO_STRUCT(dataSize, dataSize);
    JSON_TO_STRUCT(speed, speed);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::JobPermission& st)
{
    JSON_TO_STRUCT(user, user);
    JSON_TO_STRUCT(group, group);
    JSON_TO_STRUCT(fileMode, fileMode);
    JSON_TO_STRUCT(isMount, isMount);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::ResourceResultByPage& st)
{
    JSON_TO_STRUCT_ARRAY(resourceList, items, AppProtect::ApplicationResource);
    JSON_TO_STRUCT(pageNo, pageNo);
    JSON_TO_STRUCT(pageSize, pageSize);
    JSON_TO_STRUCT(pages, pages);
    JSON_TO_STRUCT(total, total);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::Resource& st)
{
    JSON_TO_STRUCT_ENUM(scope, scope, AppProtect::ResourceScope::type);
    JSON_TO_STRUCT(scopeKey, scopeKey);
    JSON_TO_STRUCT(resourceKey, resourceKey);
    JSON_TO_STRUCT(resourceValue, resourceValue);
    JSON_TO_STRUCT(sharedNum, sharedNum);
    JSON_TO_STRUCT(lockType, lockType);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::PrepareRepositoryByPlugin& st)
{
    JSON_TO_STRUCT_ARRAY(repository, repository, AppProtect::StorageRepository);
    JSON_TO_STRUCT_STRUCT(permission, permission);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::CheckCopyJob& st)
{
    JSON_TO_STRUCT(requestId, requestId)
    JSON_TO_STRUCT(jobId, jobId)
    JSON_TO_STRUCT_STRUCT(protectEnv, protectEnv)
    JSON_TO_STRUCT_STRUCT(protectObject, protectObject)
    JSON_TO_STRUCT_ARRAY(repositories, repositories, StorageRepository);
    JSON_TO_STRUCT_ARRAY(copies, copies, Copy);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}
#endif