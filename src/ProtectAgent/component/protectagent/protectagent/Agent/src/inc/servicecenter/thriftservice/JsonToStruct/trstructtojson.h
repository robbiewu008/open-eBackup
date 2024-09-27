#ifndef TR_STRUCT_WITH_JSON_H
#define TR_STRUCT_WITH_JSON_H
 
#include <algorithm>
#include "common/Types.h"
#include "common/JsonHelper.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"
#include "apps/appprotect/plugininterface/ApplicationProtectPlugin_types.h"
#include "apps/appprotect/plugininterface/ApplicationProtectFramework_types.h"
 
namespace {
    constexpr int64_t UNSET_DATA_SIZE = -1;
}

static mp_void StructToJson(const ApplicationPlugin& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    JsonHelper::TypeToJsonValue(st.endPoint, jsonValue["endPoint"]);
    JsonHelper::TypeToJsonValue(st.port, jsonValue["port"]);
    JsonHelper::TypeToJsonValue(st.processId, jsonValue["processId"]);
}

static mp_void StructToJson(const Authentication& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(mp_int32(st.authType), jsonValue["authType"]);
    JsonHelper::TypeToJsonValue(st.authkey, jsonValue["authKey"]);
    JsonHelper::TypeToJsonValue(st.authPwd, jsonValue["authPwd"]);
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const ApplicationResource& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    JsonHelper::TypeToJsonValue(st.subType, jsonValue["subType"]);
    JsonHelper::TypeToJsonValue(st.id, jsonValue["uuid"]);
    JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    JsonHelper::TypeToJsonValue(st.parentId, jsonValue["parentUuid"]);
    JsonHelper::TypeToJsonValue(st.parentName, jsonValue["parentName"]);
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const Application& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    JsonHelper::TypeToJsonValue(st.subType, jsonValue["subType"]);
    JsonHelper::TypeToJsonValue(st.id, jsonValue["uuid"]);
    JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    JsonHelper::TypeToJsonValue(st.parentId, jsonValue["parentUuid"]);
    JsonHelper::TypeToJsonValue(st.parentName, jsonValue["parentName"]);
    StructToJson(st.auth, jsonValue["auth"]);
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const ApplicationEnvironment& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.id, jsonValue["uuid"]);
    JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    JsonHelper::TypeToJsonValue(st.subType, jsonValue["subType"]);
    JsonHelper::TypeToJsonValue(st.endpoint, jsonValue["endpoint"]);
    JsonHelper::TypeToJsonValue(st.port, jsonValue["port"]);
    for (const auto& node : st.nodes) {
        Json::Value value;
        JsonHelper::TypeToJsonValue(node.id, value["uuid"]);
        JsonHelper::TypeToJsonValue(node.name, value["name"]);
        JsonHelper::TypeToJsonValue(node.type, value["type"]);
        JsonHelper::TypeToJsonValue(node.subType, value["subType"]);
        JsonHelper::TypeToJsonValue(node.endpoint, value["endpoint"]);
        JsonHelper::TypeToJsonValue(node.port, value["port"]);
        StructToJson(node.auth, value["auth"]);
        JsonHelper::JsonStringToJsonValue(node.extendInfo, value["extendInfo"]);
        jsonValue["nodes"].append(std::move(value));
    }
    StructToJson(st.auth, jsonValue["auth"]);
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const SubJob& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.jobId, jsonValue["taskId"]);
    JsonHelper::TypeToJsonValue(mp_int32(st.jobType), jsonValue["taskType"]);
    JsonHelper::TypeToJsonValue(st.jobName, jsonValue["taskName"]);
    JsonHelper::TypeToJsonValue(st.jobPriority, jsonValue["taskPriority"]);
    JsonHelper::TypeToJsonValue(mp_int32(st.policy), jsonValue["policy"]);
    JsonHelper::TypeToJsonValue(st.ignoreFailed, jsonValue["ignoreFailed"]);
    JsonHelper::TypeToJsonValue(st.execNodeId, jsonValue["execNodeId"]);
    jsonValue["taskParams"] = st.jobInfo;
}

static const mp_void StructToJson(const std::vector<AppProtect::SubJob>& jobs, Json::Value& jsonValue)
{
    for (const auto& job : jobs) {
        Json::Value value;
        StructToJson(job, value);
        jsonValue.append(std::move(value));
    }
}

static mp_void StructToJson(const HostAddress& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.ip, jsonValue["ip"]);
    JsonHelper::TypeToJsonValue(st.port, jsonValue["port"]);
    JsonHelper::TypeToJsonValue(st.supportProtocol, jsonValue["supportProtocol"]);
}

static mp_void StructToJson(const StorageRepository& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    JsonHelper::TypeToJsonValue(mp_int32(st.repositoryType), jsonValue["type"]);
    JsonHelper::TypeToJsonValue(mp_int32(st.role), jsonValue["role"]);
    for (auto item : st.path) {
        jsonValue["path"].append(item);
    }
    JsonHelper::TypeToJsonValue(st.isLocal, jsonValue["isLocal"]);
    JsonHelper::TypeToJsonValue(mp_int32(st.protocol), jsonValue["protocol"]);
    StructToJson(st.auth, jsonValue["auth"]);
    StructToJson(st.extendAuth, jsonValue["extendAuth"]);
    StructToJson(st.endpoint, jsonValue["endpoint"]);
    StructToJson(st.auth, jsonValue["cifsAuth"]);
    if (st.remoteHost.empty()) {
        jsonValue["remoteHost"] = Json::Value(Json::arrayValue);
    } else {
        for (std::vector<HostAddress>::const_iterator it = st.remoteHost.begin();
            it != st.remoteHost.end(); ++it) {
            Json::Value temp;
            StructToJson(*it, temp);
            jsonValue["remoteHost"].append(std::move(temp));
        }
    }
    JsonHelper::TypeToJsonValue(st.remotePath, jsonValue["remotePath"]);
    JsonHelper::TypeToJsonValue(st.remoteName, jsonValue["shareName"]);
    if (!st.extendInfo.empty()) {
        JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
    }
}

static mp_void StructToJson(const Snapshot& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    JsonHelper::TypeToJsonValue(st.parentName, jsonValue["parentName"]);
}

static std::map<std::string, CopyDataType::type> TransferMap_CopyDataType = {
    {"full", CopyDataType::type::FULL_COPY},
    {"increment", CopyDataType::type::INCREMENT_COPY},
    {"diff", CopyDataType::type::DIFF_COPY},
    {"log", CopyDataType::type::LOG_COPY},
    {"nativeSnapshot", CopyDataType::type::SNAPSHOT_COPY},
    {"foreverIncrement", CopyDataType::type::PERMANENT_INCREMENTAL_COPY},
    {"replication", CopyDataType::type::REPLICATION_COPY},
    {"s3Archive", CopyDataType::type::CLOUD_STORAGE_COPY},
    {"tapeArchive", CopyDataType::type::TAPE_STORAGE_COPY},
    {"clone", CopyDataType::type::CLONE_COPY},
};

static mp_void StructToJson(const Copy& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(mp_int32(st.formatType), jsonValue["format"]);
    auto iter = std::find_if(TransferMap_CopyDataType.begin(), TransferMap_CopyDataType.end(),
        [&st](const std::pair<std::string, CopyDataType::type>& item) -> bool {
        return item.second== st.dataType;
    });
    if (iter != TransferMap_CopyDataType.end()) {
        jsonValue["type"] = iter->first;
    }
    JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    JsonHelper::TypeToJsonValue(st.timestamp, jsonValue["timestamp"]);
    JsonHelper::TypeToJsonValue(st.transactionNo, jsonValue["transactionNo"]);
    StructToJson(st.protectEnv, jsonValue["protectEnv"]);
    StructToJson(st.protectObject, jsonValue["protectObject"]);
    for (std::vector<ApplicationResource>::const_iterator it = st.protectSubObjects.begin();
        it != st.protectSubObjects.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["protectSubObject"].append(std::move(temp));
    }
    for (std::vector<StorageRepository>::const_iterator it = st.repositories.begin();
        it != st.repositories.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["repositories"].append(std::move(temp));
    }
    for (std::vector<Snapshot>::const_iterator it = st.snapshots.begin();
        it != st.snapshots.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["snapshots"].append(std::move(temp));
    }
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::Qos& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.bandwidth, jsonValue["bandwidth"]);
    JsonHelper::TypeToJsonValue(st.protectIops, jsonValue["protectIops"]);
    JsonHelper::TypeToJsonValue(st.backupIops, jsonValue["backupIops"]);
}

static mp_void StructToJson(const AppProtect::ResourceFilter& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.filterBy, jsonValue["filterBy"]);
    JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    JsonHelper::TypeToJsonValue(st.rule, jsonValue["rule"]);
    JsonHelper::TypeToJsonValue(st.mode, jsonValue["mode"]);
    JsonHelper::TypeToJsonValue(st.values, jsonValue["values"]);
}

static mp_void StructToJson(const AppProtect::DataLayout& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.encryption, jsonValue["encryption"]);
    JsonHelper::TypeToJsonValue(st.deduption, jsonValue["deduption"]);
    JsonHelper::TypeToJsonValue(st.compression, jsonValue["compression"]);
    JsonHelper::TypeToJsonValue(st.nativeData, jsonValue["nativeData"]);
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::JobScripts& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.preScript, jsonValue["preScript"]);
    JsonHelper::TypeToJsonValue(st.postScript, jsonValue["postScript"]);
    JsonHelper::TypeToJsonValue(st.failPostScript, jsonValue["failPostScript"]);
}

static mp_void StructToJson(const AppProtect::BackupJobParam& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(mp_int32(st.backupType), jsonValue["backupType"]);
    StructToJson(st.dataLayout, jsonValue["dataLayout"]);
    StructToJson(st.qos, jsonValue["qos"]);
    for (std::vector<AppProtect::ResourceFilter>::const_iterator it = st.filters.begin();
        it != st.filters.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["filters"].append(std::move(temp));
    }
    StructToJson(st.scripts, jsonValue["scripts"]);
    JsonHelper::JsonStringToJsonValue(st.advanceParams, jsonValue["advanceParams"]);
}

static mp_void StructToJson(const AppProtect::LivemountJobParam& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.advanceParams, jsonValue["advanceParams"]);
    StructToJson(st.scripts, jsonValue["scripts"]);
}

static mp_void StructToJson(const AppProtect::CancelLivemountJobParam& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.advanceParams, jsonValue["advanceParams"]);
    StructToJson(st.scripts, jsonValue["scripts"]);
}

static mp_void StructToJson(const AppProtect::BackupJob& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    JsonHelper::TypeToJsonValue(st.jobId, jsonValue["taskId"]);
    StructToJson(st.jobParam, jsonValue["taskParams"]);
    StructToJson(st.protectEnv, jsonValue["envInfo"]);
    StructToJson(st.protectObject, jsonValue["appInfo"]);
    for (std::vector<ApplicationResource>::const_iterator it = st.protectSubObject.begin();
        it != st.protectSubObject.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["resourceInfo"].append(std::move(temp));
    }
    for (std::vector<StorageRepository>::const_iterator it = st.repositories.begin();
        it != st.repositories.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["repositories"].append(std::move(temp));
    }
    {
        Json::Value temp;
        StructToJson(st.copy, temp);
        jsonValue["copies"].append(std::move(temp));
    }
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::RestoreJobParam& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(mp_int32(st.restoreType), jsonValue["restoreType"]);
    for (std::vector<AppProtect::ResourceFilter>::const_iterator it = st.filters.begin();
        it != st.filters.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["filters"].append(std::move(temp));
    }
    StructToJson(st.qos, jsonValue["qos"]);
    StructToJson(st.scripts, jsonValue["scripts"]);
    JsonHelper::JsonStringToJsonValue(st.advanceParams, jsonValue["advanceParams"]);
}

static mp_void StructToJson(const AppProtect::RestoreJob& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    JsonHelper::TypeToJsonValue(st.jobId, jsonValue["taskId"]);
    StructToJson(st.jobParam, jsonValue["taskParams"]);
    StructToJson(st.targetEnv, jsonValue["envInfo"]);
    StructToJson(st.targetObject, jsonValue["appInfo"]);
    for (std::vector<ApplicationResource>::const_iterator it = st.restoreSubObjects.begin();
        it != st.restoreSubObjects.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["resourceInfo"].append(std::move(temp));
    }
    for (std::vector<Copy>::const_iterator it = st.copies.begin();
        it != st.copies.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["copies"].append(std::move(temp));
    }
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::DelCopyJob& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    JsonHelper::TypeToJsonValue(st.jobId, jsonValue["taskId"]);
    StructToJson(st.protectEnv, jsonValue["envInfo"]);
    StructToJson(st.protectObject, jsonValue["appInfo"]);
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
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::CheckCopyJob& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    JsonHelper::TypeToJsonValue(st.jobId, jsonValue["taskId"]);
    StructToJson(st.protectEnv, jsonValue["envInfo"]);
    StructToJson(st.protectObject, jsonValue["appInfo"]);
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
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::LivemountJob& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    JsonHelper::TypeToJsonValue(st.jobId, jsonValue["taskId"]);
    StructToJson(st.jobParam, jsonValue["taskParams"]);
    StructToJson(st.targetEnv, jsonValue["envInfo"]);
    StructToJson(st.targetObject, jsonValue["appInfo"]);
    for (std::vector<ApplicationResource>::const_iterator it = st.targetSubObjects.begin();
        it != st.targetSubObjects.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["resourceInfo"].append(std::move(temp));
    }
    {
        Json::Value temp;
        StructToJson(st.copy, temp);
        jsonValue["copies"].append(std::move(temp));
    }
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::CancelLivemountJob& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    JsonHelper::TypeToJsonValue(st.jobId, jsonValue["taskId"]);
    StructToJson(st.jobParam, jsonValue["taskParams"]);
    StructToJson(st.targetEnv, jsonValue["envInfo"]);
    StructToJson(st.targetObject, jsonValue["appInfo"]);
    for (std::vector<ApplicationResource>::const_iterator it = st.targetSubObjects.begin();
        it != st.targetSubObjects.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["resourceInfo"].append(std::move(temp));
    }
    {
        Json::Value temp;
        StructToJson(st.copy, temp);
        jsonValue["copies"].append(std::move(temp));
    }
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::BuildIndexJobParam& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.preCopyId, jsonValue["preCopyId"]);
    JsonHelper::TypeToJsonValue(st.indexPath, jsonValue["indexPath"]);
}

static mp_void StructToJson(const AppProtect::BuildIndexJob& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.requestId, jsonValue["requestId"]);
    JsonHelper::TypeToJsonValue(st.jobId, jsonValue["taskId"]);
    StructToJson(st.jobParam, jsonValue["taskParams"]);
    StructToJson(st.indexEnv, jsonValue["envInfo"]);
    StructToJson(st.indexProtectObject, jsonValue["appInfo"]);
    for (std::vector<ApplicationResource>::const_iterator it = st.indexProtectSubObject.begin();
        it != st.indexProtectSubObject.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["resourceInfo"].append(std::move(temp));
    }
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
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::LogDetail& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(mp_int32(st.level), jsonValue["logLevel"]);
    JsonHelper::TypeToJsonValue(st.description, jsonValue["logInfo"]);
    JsonHelper::TypeToJsonValue(st.params, jsonValue["logInfoParam"]);
    JsonHelper::TypeToJsonValue(st.timestamp, jsonValue["logTimestamp"]);
    if (st.errorCode != 0) {
        JsonHelper::TypeToJsonValue(st.errorCode, jsonValue["logDetail"]);
        JsonHelper::TypeToJsonValue(st.errorParams, jsonValue["logDetailParam"]);
        JsonHelper::TypeToJsonValue(st.additionalDesc, jsonValue["logDetailInfo"]);
    }
}
static mp_void StructToJson(const AppProtect::ResourceResultByPage& st, Json::Value& jsonValue)
{
    for (std::vector<AppProtect::ApplicationResource>::const_iterator it = st.items.begin();
        it != st.items.end(); ++it) {
            Json::Value temp;
            StructToJson(*it, temp);
            jsonValue["items"].append(std::move(temp));
        }
    JsonHelper::TypeToJsonValue(st.pageNo, jsonValue["pageNo"]);
    JsonHelper::TypeToJsonValue(st.pageSize, jsonValue["pageSize"]);
    JsonHelper::TypeToJsonValue(st.pages, jsonValue["pages"]);
    JsonHelper::TypeToJsonValue(st.total, jsonValue["total"]);
}

static mp_void StructToJson(const AppProtect::SubJobDetails& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.jobId, jsonValue["taskId"]);
    JsonHelper::TypeToJsonValue(st.subJobId, jsonValue["subTaskId"]);
    JsonHelper::TypeToJsonValue(mp_int32(st.jobStatus), jsonValue["taskStatus"]);
    JsonHelper::TypeToJsonValue(st.additionalStatus, jsonValue["additionalStatus"]);
    for (std::vector<AppProtect::LogDetail>::const_iterator it = st.logDetail.begin();
        it != st.logDetail.end(); ++it) {
        Json::Value temp;
        StructToJson(*it, temp);
        jsonValue["logDetail"].append(std::move(temp));
    }
    JsonHelper::TypeToJsonValue(st.progress, jsonValue["progress"]);
    if (st.__isset.dataSize) {
        JsonHelper::TypeToJsonValue(st.dataSize, jsonValue["dataSize"]);
    } else {
        JsonHelper::TypeToJsonValue(UNSET_DATA_SIZE, jsonValue["dataSize"]);
    }
    JsonHelper::TypeToJsonValue(st.speed, jsonValue["speed"]);
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static mp_void StructToJson(const AppProtect::JobPermission& st, Json::Value& jsonValue)
{
    JsonHelper::TypeToJsonValue(st.user, jsonValue["user"]);
    JsonHelper::TypeToJsonValue(st.group, jsonValue["group"]);
    JsonHelper::TypeToJsonValue(st.fileMode, jsonValue["fileMode"]);
    JsonHelper::TypeToJsonValue(st.isMount, jsonValue["isMount"]);
    JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}
#endif