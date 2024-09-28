#include "servicecenter/services/device/LogRepository.h"

#include "taskmanager/externaljob/PluginAnyTimeRestore.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
namespace AppProtect {
mp_void LogRepository::QueryRepoSubPath(const PluginJobData &data, std::vector<mp_string> &path)
{
    if (data.mainType != MainJobType::BACKUP_JOB) {
        return; // 仅日志备份
    }
    path.push_back(data.mainID);
    path.push_back("meta");
    return;
}

mp_bool LogRepository::isNeedOracleMigrate(const PluginJobData &data)
{
    // 兼容oracle 1.2.1版本数据，1.2.1版本日志仓不进行处理
    auto isOracleMigrate = false;
    for (Json::ArrayIndex index = 0; index < data.param["copies"].size(); ++index) {
        if (!data.param["copies"][index].isMember("extendInfo")) {
            continue;
        }
        if (!data.param["copies"][index]["extendInfo"].isMember("migrate")) {
            continue;
        }
        if (!data.param["copies"][index]["extendInfo"]["migrate"].isString()) {
            continue;
        }
        if (data.param["copies"][index]["extendInfo"]["migrate"].asString() == "oracle-migrate") {
            return MP_TRUE;
            break;
        }
    }
    return MP_FALSE;
}

mp_bool LogRepository::isSkipLogRepoCompose(const Json::Value& jobParam)
{
    // 如果存在skipLogRepoCompose这个key则代表不需要组装log仓,由PM插件决定是否要传递此参数
    if (!jobParam.isMember("extendInfo")) {
        ERRLOG("is me ex");
        return MP_FALSE;
    }
    Json::Value jValueExtendinfo = jobParam["extendInfo"];
    if (!jValueExtendinfo.isMember("skipLogRepoCompose")) {
        ERRLOG("is me skip");
        return MP_FALSE;
    }
    ERRLOG("is skip");
    return MP_TRUE;
}

mp_int32 LogRepository::AssembleRepository(const PluginJobData &data, StorageRepository &stRep,
    Json::Value &jsonRep_new)
{
    AssembleLogMetaPath(stRep, jsonRep_new);

    // 任意时间点恢复
    if (!isNeedOracleMigrate(data) && !isSkipLogRepoCompose(data.param)) {
        if (data.mainType == MainJobType::RESTORE_JOB) {
            std::vector<mp_string> tmpMountPoint = m_mountPoint;
            PluginAnyTimeRestore anytimerestore;
            if (data.IsSanClientMount()) {
                auto iter = data.mountPoints.find(RepositoryDataType::type::CACHE_REPOSITORY);
                tmpMountPoint = (iter == data.mountPoints.end() ? tmpMountPoint : iter->second);
            }
            if (anytimerestore.ComputerAnyTimeRestoreLogPath(data.param, tmpMountPoint, stRep, jsonRep_new)
                != MP_SUCCESS) {
                ERRLOG("Failed to assemble log repository, jobId=%s, subJobId=%s.", data.mainID.c_str(),
                    data.subID.c_str());
                return MP_FAILED;
            }
            return MP_SUCCESS;
        }
    }
        
    AssembleLogPath(data, stRep, jsonRep_new);
    return MP_SUCCESS;
}

mp_void LogRepository::AssembleLogMetaPath(StorageRepository &stRep, Json::Value &jsonRep_new)
{
    // 日志元数据仓
    Json::Value metaRepJson;
    StructToJson(stRep, metaRepJson);
    metaRepJson["path"] = Json::arrayValue;
    for (auto mountPt : m_mountPoint) {
#ifdef WIN32
        std::vector<mp_string> mountInfoVec;
        CMpString::StrSplit(mountInfoVec, mountPt, '&');
        mp_string metaMopuntPoint = mountInfoVec.front() + PATH_SEPARATOR + "meta";
#else
        mp_string metaMopuntPoint = mountPt + PATH_SEPARATOR + "meta";
#endif
        metaRepJson["path"].append(metaMopuntPoint);
    }
    metaRepJson["type"] = RepositoryDataType::type::META_REPOSITORY;
    jsonRep_new.append(std::move(metaRepJson));
}

mp_void LogRepository::AssembleLogPath(const PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new)
{
    Json::Value logRepJson;
    StructToJson(stRep, logRepJson);
    logRepJson["path"] = Json::arrayValue;
    // 日志备份
    if (data.mainType == MainJobType::BACKUP_JOB) {
        for (auto mountPt : m_mountPoint) {
#ifdef WIN32
            std::vector<mp_string> mountInfoVec;
            CMpString::StrSplit(mountInfoVec, mountPt, '&');
            mp_string logMopuntPoint = mountInfoVec.front() + PATH_SEPARATOR + data.mainID;
#else
            mp_string logMopuntPoint = mountPt + PATH_SEPARATOR + data.mainID;
#endif
            logRepJson["path"].append(logMopuntPoint);
        }
        jsonRep_new.append(std::move(logRepJson));
        return;
    }
    for (auto mountPt : m_mountPoint) {
#ifdef WIN32
        std::vector<mp_string> mountInfoVec;
        CMpString::StrSplit(mountInfoVec, mountPt, '&');
        logRepJson["path"].append(mountInfoVec.front());
#else
        logRepJson["path"].append(mountPt);
#endif
    }
    jsonRep_new.append(std::move(logRepJson));
}
}
