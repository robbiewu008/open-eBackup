/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file ConnectionTable.h
 * @brief  JsonToStruct
 * @version 1.0.0.0
 * @date 2021-10-27
 * @author kWX884906
 */

#ifndef TR_JSON_WITH_STRUCT_H
#define TR_JSON_WITH_STRUCT_H

#include <algorithm>
#include "common/Types.h"
#include "common/JsonHelper.h"
#include "trstructtojson.h"
// json to nomal type
#define JSON_TO_VALUE(key, value) if (jsonValue.isObject() && jsonValue.isMember(#key)) { \
    JsonHelper::JsonValueToType(value, jsonValue[#key]); \
}

// json to nomal type
#define JSON_TO_STRUCT(key, name) if (jsonValue.isObject() && jsonValue.isMember(#key)) { \
    JsonHelper::JsonValueToType(st.name, jsonValue[#key]); \
    st.__set_##name(st.name); \
}

// json to struct
#define JSON_TO_STRUCT_STRUCT(key, name) if (jsonValue.isObject() && jsonValue.isMember(#key) && \
    jsonValue[#key].isObject()) { \
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

static mp_void JsonToStruct(const Json::Value& jsonValue, ApplicationPlugin& st)
{
    JSON_TO_STRUCT(name, name)
    JSON_TO_STRUCT(endPoint, endPoint)
    JSON_TO_STRUCT(port, port)
    JSON_TO_STRUCT(processId, processId)
}

static mp_void JsonToStruct(const Json::Value& jsonValue, Authentication& st)
{
    JSON_TO_STRUCT_ENUM(authType, authType, AuthType::type);
    JSON_TO_STRUCT(authKey, authkey)
    JSON_TO_STRUCT(authPwd, authPwd)
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, ApplicationResource& st)
{
    JSON_TO_STRUCT(type, type)
    JSON_TO_STRUCT(subType, subType)
    JSON_TO_STRUCT(uuid, id)
    JSON_TO_STRUCT(name, name)
    JSON_TO_STRUCT(parentUuid, parentId)
    JSON_TO_STRUCT(parentName, parentName)
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, Application& st)
{
    JSON_TO_STRUCT(type, type)
    JSON_TO_STRUCT(subType, subType)
    JSON_TO_STRUCT(uuid, id)
    JSON_TO_STRUCT(name, name)
    JSON_TO_STRUCT(parentUuid, parentId)
    JSON_TO_STRUCT(parentName, parentName)
    JSON_TO_STRUCT_STRUCT(auth, auth)
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, ApplicationEnvironment& st)
{
    JSON_TO_STRUCT(uuid, id)
    JSON_TO_STRUCT(name, name)
    JSON_TO_STRUCT(type, type)
    JSON_TO_STRUCT(subType, subType)
    JSON_TO_STRUCT(endpoint, endpoint)
    JSON_TO_STRUCT(port, port)
    JSON_TO_STRUCT_STRUCT(auth, auth)
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
    JSON_TO_STRUCT_ARRAY(nodes, nodes, ApplicationEnvironment);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, QueryByPage& st)
{
    JSON_TO_STRUCT(pageNo, pageNo)
    JSON_TO_STRUCT(pageSize, pageSize)
    JSON_TO_STRING_ARRAY(orders, orders);
    JSON_TO_STRUCT_OBJ(conditions, conditions);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, SubJob& st)
{
    JSON_TO_STRUCT(taskId, jobId)
    JSON_TO_STRUCT(subTaskId, subJobId)
    JSON_TO_STRUCT_ENUM(subTaskType, jobType, SubJobType::type);
    JSON_TO_STRUCT(taskName, jobName)
    JSON_TO_STRUCT(subTaskParams, jobInfo)
}


static mp_void JsonToStruct(const Json::Value& jsonValue, HostAddress& st)
{
    JSON_TO_STRUCT(ip, ip)
    JSON_TO_STRUCT(port, port)
    JSON_TO_STRUCT(supportProtocol, supportProtocol)
}

static mp_void JsonToStruct(const Json::Value& jsonValue, StorageRepository& st)
{
    JSON_TO_STRUCT(id, id)
    JSON_TO_STRUCT_ENUM(type, repositoryType, RepositoryDataType::type);
    JSON_TO_STRUCT_ENUM(role, role, RepositoryRole::type);
    JSON_TO_STRING_ARRAY(path, path);
    JSON_TO_STRUCT(isLocal, isLocal)
    JSON_TO_STRUCT_ENUM(protocol, protocol, RepositoryProtocolType::type);
    JSON_TO_STRUCT_STRUCT(auth, auth)
    JSON_TO_STRUCT_STRUCT(extendAuth, extendAuth)
    JSON_TO_STRUCT_STRUCT(endpoint, endpoint)
    JSON_TO_STRUCT_ARRAY(remoteHost, remoteHost, HostAddress);
    JSON_TO_STRUCT(remotePath, remotePath);
    JSON_TO_STRUCT(shareName, remoteName);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
    JSON_TO_STRUCT_STRUCT(cifsAuth, cifsAuth)
}


static mp_void JsonToStruct(const Json::Value& jsonValue, Snapshot& st)
{
    JSON_TO_STRUCT(id, id)
    JSON_TO_STRUCT(parentName, parentName)
}

static mp_void JsonToStruct(const Json::Value& jsonValue, Copy& st)
{
    JSON_TO_STRUCT_ENUM(format, formatType, CopyFormatType::type);
    std::string strType;
    JSON_TO_VALUE(type, strType)
    auto it = TransferMap_CopyDataType.find(strType);
    if (it != TransferMap_CopyDataType.end()) {
        st.__set_dataType(it->second);
    }
    JSON_TO_STRUCT(id, id)
    JSON_TO_STRUCT(name, name)
    JSON_TO_STRUCT(timestamp, timestamp)
    JSON_TO_STRUCT(transactionNo, transactionNo)
    JSON_TO_STRUCT_STRUCT(protectEnv, protectEnv)
    JSON_TO_STRUCT_STRUCT(protectObject, protectObject)
    JSON_TO_STRUCT_ARRAY(protectSubObject, protectSubObjects, ApplicationResource);
    JSON_TO_STRUCT_ARRAY(repositories, repositories, StorageRepository);
    JSON_TO_STRUCT_ARRAY(snapshots, snapshots, Snapshot);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::Qos& st)
{
    JSON_TO_STRUCT(bandwidth, bandwidth)
    JSON_TO_STRUCT(protectIops, protectIops)
    JSON_TO_STRUCT(backupIops, backupIops)
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::ResourceFilter& st)
{
    JSON_TO_STRUCT(filterBy, filterBy)
    JSON_TO_STRUCT(type, type)
    JSON_TO_STRUCT(rule, rule)
    JSON_TO_STRUCT(mode, mode)
    JSON_TO_STRUCT(values, values)
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::DataLayout& st)
{
    JSON_TO_STRUCT(encryption, encryption)
    JSON_TO_STRUCT(deduption, deduption)
    JSON_TO_STRUCT(compression, compression)
    JSON_TO_STRUCT(nativeData, nativeData)
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::JobScripts& st)
{
    JSON_TO_STRUCT(preScript, preScript)
    JSON_TO_STRUCT(postScript, postScript)
    JSON_TO_STRUCT(failPostScript, failPostScript)
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
    JSON_TO_VALUE(backupType, backupType)
    auto it = TransferMap.find(backupType);
    if (it != TransferMap.end()) {
        st.__set_backupType(it->second);
    }
    JSON_TO_STRUCT_STRUCT(dataLayout, dataLayout)
    JSON_TO_STRUCT_STRUCT(qos, qos)
    JSON_TO_STRUCT_ARRAY(filters, filters, AppProtect::ResourceFilter);
    JSON_TO_STRUCT_STRUCT(scripts, scripts)
    JSON_TO_STRUCT_OBJ(advanceParams, advanceParams);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::LivemountJobParam& st)
{
    JSON_TO_STRUCT(advanceParams, advanceParams)
    JSON_TO_STRUCT_STRUCT(scripts, scripts)
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::CancelLivemountJobParam& st)
{
    JSON_TO_STRUCT(advanceParams, advanceParams)
    JSON_TO_STRUCT_STRUCT(scripts, scripts)
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::BackupJob& st)
{
    JSON_TO_STRUCT(requestId, requestId)
    JSON_TO_STRUCT(taskId, jobId)
    JSON_TO_STRUCT_STRUCT(taskParams, jobParam)
    JSON_TO_STRUCT_STRUCT(envInfo, protectEnv)
    JSON_TO_STRUCT_STRUCT(appInfo, protectObject)
    JSON_TO_STRUCT_ARRAY(resourceInfo, protectSubObject, ApplicationResource);
    JSON_TO_STRUCT_ARRAY(repositories, repositories, StorageRepository);
    JSON_ARRAY_TO_SINGLE_STRUCT(copies, copy, Copy);
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
    JSON_TO_VALUE(restoreType, restoreType)
    auto it = TransferMap.find(restoreType);
    if (it != TransferMap.end()) {
        st.__set_restoreType(it->second);
    }
    JSON_TO_STRUCT_ENUM(restoreType, restoreType, AppProtect::RestoreJobType::type);
    JSON_TO_STRUCT_ARRAY(filters, filters, AppProtect::ResourceFilter);
    JSON_TO_STRUCT_STRUCT(qos, qos)
    JSON_TO_STRUCT_STRUCT(scripts, scripts)
    JSON_TO_STRUCT_OBJ(advanceParams, advanceParams);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::RestoreJob& st)
{
    JSON_TO_STRUCT(requestId, requestId)
    JSON_TO_STRUCT(taskId, jobId)
    JSON_TO_STRUCT_STRUCT(taskParams, jobParam)
    JSON_TO_STRUCT_STRUCT(envInfo, targetEnv)
    JSON_TO_STRUCT_STRUCT(appInfo, targetObject)
    JSON_TO_STRUCT_ARRAY(resourceInfo, restoreSubObjects, ApplicationResource);
    JSON_TO_STRUCT_ARRAY(copies, copies, Copy);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::DelCopyJob& st)
{
    JSON_TO_STRUCT(requestId, requestId)
    JSON_TO_STRUCT(taskId, jobId)
    JSON_TO_STRUCT_STRUCT(envInfo, protectEnv)
    JSON_TO_STRUCT_STRUCT(appInfo, protectObject)
    JSON_TO_STRUCT_ARRAY(repositories, repositories, StorageRepository);
    JSON_TO_STRUCT_ARRAY(copies, copies, Copy);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::CheckCopyJob& st)
{
    JSON_TO_STRUCT(requestId, requestId)
    JSON_TO_STRUCT(taskId, jobId)
    JSON_TO_STRUCT_STRUCT(envInfo, protectEnv)
    JSON_TO_STRUCT_STRUCT(appInfo, protectObject)
    JSON_TO_STRUCT_ARRAY(repositories, repositories, StorageRepository);
    JSON_TO_STRUCT_ARRAY(copies, copies, Copy);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::LivemountJob& st)
{
    JSON_TO_STRUCT(requestId, requestId)
    JSON_TO_STRUCT(taskId, jobId)
    JSON_TO_STRUCT_STRUCT(taskParams, jobParam)
    JSON_TO_STRUCT_STRUCT(envInfo, targetEnv)
    JSON_TO_STRUCT_STRUCT(appInfo, targetObject)
    JSON_TO_STRUCT_ARRAY(resourceInfo, targetSubObjects, ApplicationResource);
    JSON_ARRAY_TO_SINGLE_STRUCT(copies, copy, Copy);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::CancelLivemountJob& st)
{
    JSON_TO_STRUCT(requestId, requestId)
    JSON_TO_STRUCT(taskId, jobId)
    JSON_TO_STRUCT_STRUCT(taskParams, jobParam)
    JSON_TO_STRUCT_STRUCT(envInfo, targetEnv)
    JSON_TO_STRUCT_STRUCT(appInfo, targetObject)
    JSON_TO_STRUCT_ARRAY(resourceInfo, targetSubObjects, ApplicationResource);
    JSON_ARRAY_TO_SINGLE_STRUCT(copies, copy, Copy);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::BuildIndexJobParam& st)
{
    JSON_TO_STRUCT(preCopyId, preCopyId)
    JSON_TO_STRUCT(indexPath, indexPath)
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::BuildIndexJob& st)
{
    JSON_TO_STRUCT(requestId, requestId)
    JSON_TO_STRUCT(taskId, jobId)
    JSON_TO_STRUCT_STRUCT(taskParams, jobParam)
    JSON_TO_STRUCT_STRUCT(envInfo, indexEnv)
    JSON_TO_STRUCT_STRUCT(appInfo, indexProtectObject)
    JSON_TO_STRUCT_ARRAY(resourceInfo, indexProtectSubObject, ApplicationResource);
    JSON_TO_STRUCT_ARRAY(repositories, repositories, StorageRepository);
    JSON_TO_STRUCT_ARRAY(copies, copies, Copy);
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::LogDetail& st)
{
    JSON_TO_STRUCT_ENUM(logLevel, level, AppProtect::JobLogLevel::type);
    JSON_TO_STRUCT(logInfo, description)
    JSON_TO_STRUCT(logInfoParam, params)
    JSON_TO_STRUCT(logTimestamp, timestamp)
    JSON_TO_STRUCT(logDetail, errorCode)
    JSON_TO_STRUCT(logDetailParam, errorParams)
    JSON_TO_STRUCT(logDetailInfo, additionalDesc)
}

static mp_void JsonToStruct(const Json::Value& jsonValue, AppProtect::SubJobDetails& st)
{
    JSON_TO_STRUCT(taskId, jobId)
    JSON_TO_STRUCT(subTaskId, subJobId)
    JSON_TO_STRUCT_ENUM(taskStatus, jobStatus, AppProtect::SubJobStatus::type);
    JSON_TO_STRUCT(additionalStatus, additionalStatus)
    JSON_TO_STRUCT_ARRAY(logDetail, logDetail, AppProtect::LogDetail);
    JSON_TO_STRUCT(progress, progress)
    JSON_TO_STRUCT(dataSize, dataSize)
    JSON_TO_STRUCT(speed, speed)
    JSON_TO_STRUCT_OBJ(extendInfo, extendInfo);
}
#endif