/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @brief  Factory for Plugin job
 * @version 1.0.0
 * @date 2022-1-5
 * @author twx949498
 */
#include "taskmanager/externaljob/PluginAnyTimeRestore.h"
#include <vector>
#include "common/Log.h"
#include "securecom/RootCaller.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
namespace AppProtect {
namespace {
    constexpr int32_t LOG_DIR_INDEX = 0;
    constexpr int32_t LOG_TIME_INDEX = 1;
    constexpr int32_t LOG_SCN_INDEX = 2;
    constexpr int32_t TIME_SCN_INFO_SIZE = 2;
    constexpr int32_t START_TIME_SCN_INDEX = 0;
    constexpr int32_t END_TIME_SCN_INDEX = 1;
    constexpr int32_t LOG_INFO_MINI_SIZE = 2;
    constexpr int32_t LOG_INFO_MAX_SIZE = 3;
    constexpr int32_t TIME_RESTORE = 1;
    constexpr int32_t SCN_RESTORE = 2;
    constexpr int32_t PATH_INDEX = 1;
    constexpr int32_t PATH_SIZE = 3;
    constexpr int32_t NUM_TEN = 10;
    constexpr int32_t BRACE_START = 1;
    constexpr int32_t BRACE_END = 2;
    const mp_string ANYTIME_RESTORE_META_SUFFIX = ".meta";
    const mp_string KEY_EXTENDINFO = "extendInfo";
    const mp_string KEY_RESTORECOPYID = "restoreCopyId";
    const mp_string KEY_RESTORETIMESTAMP = "restoreTimestamp";
    const mp_string KEY_RESTORESCN = "restoreScn";
    const mp_string KEY_COPYINFO = "copies";
    const mp_string KEY_REPOSITORIES = "repositories";
    const mp_string KEY_TYPE = "type";
    const mp_string KEY_PATH = "path";
    const mp_string KEY_ASSOCIATE_COPIES = "associated_log_copies";
}

PluginAnyTimeRestore::PluginAnyTimeRestore()
{
    m_taskID = "";
    m_restoreType = -1;
    m_restoreCopyid = "";
    m_restorePoint = 0;
}

PluginAnyTimeRestore::~PluginAnyTimeRestore()
{
}

mp_string PluginAnyTimeRestore::GetAssociatedCopies(const Json::Value& jValueExtendinfo)
{
    mp_string associated_log_copies = "";
    if (jValueExtendinfo.isMember(KEY_ASSOCIATE_COPIES)) {
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        associated_log_copies = Json::writeString(builder, jValueExtendinfo[KEY_ASSOCIATE_COPIES]);
        // 去除字符串中的空格
        associated_log_copies.erase(std::remove(associated_log_copies.begin(), associated_log_copies.end(), ' '), associated_log_copies.end());
        associated_log_copies.erase(std::remove(associated_log_copies.begin(), associated_log_copies.end(), '\"'), associated_log_copies.end());
        if (associated_log_copies.front() == '{' && associated_log_copies.back() == '}') {
        return associated_log_copies.substr(BRACE_START, associated_log_copies.length() - BRACE_END);
        }
    }
    return associated_log_copies;
}

mp_int32 PluginAnyTimeRestore::ComputerAnyTimeRestoreLogPath(const Json::Value& jobParam,
    const std::vector<mp_string>& mountPoint, StorageRepository& stRep, Json::Value& jsonRep_new)
{
    LOGGUARD("");
    if (!jobParam.isObject() || !jobParam.isMember(KEY_EXTENDINFO) || !jobParam[KEY_EXTENDINFO].isObject()) {
        ERRLOG("Job param is not json object or have no extendInfo key.");
        return MP_FAILED;
    }
    Json::Value jValueExtendinfo = jobParam[KEY_EXTENDINFO];
    if (!jValueExtendinfo.isMember(KEY_RESTORETIMESTAMP) && !jValueExtendinfo.isMember(KEY_RESTORESCN)) {
        ERRLOG("Job extendInfo have no restoreTimestamp and restoreScn.");
        return MP_FAILED;
    }

    GET_JSON_STRING(jobParam, "taskId", m_taskID);
    GET_JSON_STRING(jValueExtendinfo, KEY_RESTORECOPYID, m_restoreCopyid);
    mp_string restorePoint;
    if (jValueExtendinfo.isMember(KEY_RESTORETIMESTAMP)) {
        m_restoreType = TIME_RESTORE;
        GET_JSON_STRING(jValueExtendinfo, KEY_RESTORETIMESTAMP, restorePoint);
        m_restorePoint = strtoull(restorePoint.c_str(), NULL, NUM_TEN);
    } else if (jValueExtendinfo.isMember(KEY_RESTORESCN)) {
        m_restoreType = SCN_RESTORE;
        GET_JSON_STRING(jValueExtendinfo, KEY_RESTORESCN, restorePoint);
        m_restorePoint = strtoull(restorePoint.c_str(), NULL, NUM_TEN);
    } else {
        ERRLOG("Jobparam have not anytime restorekey ");
        return MP_FAILED;
    }
    // 获取associated_log_copies下发的日志副本信息
    mp_string associated_log_copies = GetAssociatedCopies(jValueExtendinfo);
    if (ComposeNewRepository(mountPoint, stRep, jsonRep_new, associated_log_copies) != MP_SUCCESS) {
        ERRLOG("Anytime restore compose new repository failed");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

void PluginAnyTimeRestore::GenerateMultiMap(bool is_metapath, std::vector<mp_string>& loginfo)
{
    Json::Value logjson;
    logjson["dirname"] = loginfo[LOG_DIR_INDEX];
    mp_string str = loginfo[LOG_TIME_INDEX];
    std::vector<mp_string> timescninfo;
    if (m_restoreType == SCN_RESTORE && is_metapath) {
        str = loginfo[LOG_SCN_INDEX];
    }
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
    CMpString::StrSplit(timescninfo, str, '~');
    if (timescninfo.size() != TIME_SCN_INFO_SIZE) {
        WARNLOG("Get restore meta log copy time or scn info failed.");
        return;
    }
    if (m_restoreType == SCN_RESTORE) {
        logjson["startscn"] = std::stol(timescninfo[START_TIME_SCN_INDEX]);
        logjson["endscn"] = std::stol(timescninfo[END_TIME_SCN_INDEX]);
        m_scnMultiMap.insert(std::make_pair(logjson["startscn"].asInt64(), logjson));
    } else {
        logjson["starttime"] = std::stol(timescninfo[START_TIME_SCN_INDEX]);
        logjson["endtime"] = std::stol(timescninfo[END_TIME_SCN_INDEX]);
        m_timeMultiMap.insert(std::make_pair(logjson["starttime"].asInt64(), logjson));
    }
}

mp_int32 PluginAnyTimeRestore::ParseRestoreMetaInfo(mp_string& metapath, bool is_metapath)
{
    LOGGUARD("");
    if (metapath.empty()) {
        ERRLOG("Restore metapath is empty");
        return MP_FAILED;
    }
    mp_char splitCh = ';';
    std::vector<mp_string> vecLoglistInfo;
    if (is_metapath) {
#ifdef WIN32
        CMpFile::ReadFile(metapath, vecLoglistInfo);
#else
        CRootCaller rootCaller;
        if (rootCaller.Exec((mp_int32)ROOT_COMMAND_CAT, metapath, &vecLoglistInfo) != MP_SUCCESS) {
            ERRLOG("Get metadata info failed, path is %s", metapath.c_str());
            return MP_FAILED;
        }
#endif
    } else {
        CMpString::StrSplit(vecLoglistInfo, metapath,  ',');
        splitCh = ':';
    }

    for (const auto& logdata : vecLoglistInfo) {
        std::vector<mp_string> loginfo;
        CMpString::StrSplit(loginfo, logdata, splitCh);
        if (loginfo.size() < LOG_INFO_MINI_SIZE) {
            WARNLOG("Get restore meta log copy info failed loginfo size is %d", loginfo.size());
            continue;
        }
        GenerateMultiMap(is_metapath, loginfo);
    }
    return MP_SUCCESS;
}

mp_int32 PluginAnyTimeRestore::GetTimeStampLogDirList(std::vector<mp_string>& logdirlist)
{
    LOGGUARD("");
    if (m_timeMultiMap.empty()) {
        ERRLOG("timeMultiMap is empty");
        return MP_FAILED;
    }
    auto timestampItem = m_timeMultiMap.begin();
    for (auto item = m_timeMultiMap.begin(); item != m_timeMultiMap.end(); ++item) {
        if (item->first > m_restorePoint) {
            timestampItem = item;
            break;
        }
    }

    if (timestampItem == m_timeMultiMap.begin()) {
        if (m_restorePoint < m_timeMultiMap.begin()->second["starttime"].asInt64()) {
            ERRLOG("Get restore log dir list failed, log can not apply timestamp restore");
            return MP_FAILED;
        }
        mp_bool flag = false;
        for (auto item: m_timeMultiMap) {
            if (item.second["endtime"].asInt64() >= m_restorePoint) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            ERRLOG("Get restore log dir list failed, log can not apply timestamp restore");
            return MP_FAILED;
        }
        timestampItem = m_timeMultiMap.end();
    }

    for (auto iterator = m_timeMultiMap.begin(); iterator != timestampItem; ++iterator) {
        std::vector<mp_string> pathinfo;
        CMpString::StrSplit(pathinfo, iterator->second["dirname"].toStyledString(), '"');
        if (pathinfo.empty() || pathinfo.size() < PATH_SIZE) {
            ERRLOG("Parse resore meta file path file %s", iterator->second["dirname"].toStyledString().c_str());
            return MP_FAILED;
        }
        logdirlist.push_back(pathinfo[PATH_INDEX]);
        DBGLOG("insert logdirlist is %s.", iterator->second["dirname"].toStyledString().c_str());
    }
    return MP_SUCCESS;
}

mp_int32 PluginAnyTimeRestore::GetSCNLogDirList(std::vector<mp_string>& logdirlist)
{
    LOGGUARD("");
    if (m_scnMultiMap.empty()) {
        ERRLOG("scnMultiMap is empty");
        return MP_FAILED;
    }
    auto scnItem = m_scnMultiMap.begin();
    for (auto item = m_scnMultiMap.begin(); item != m_scnMultiMap.end(); ++item) {
        if (item->first > m_restorePoint) {
            scnItem = item;
            break;
        }
    };
    if (scnItem == m_scnMultiMap.begin()) {
        if (m_restorePoint < m_scnMultiMap.begin()->second["startscn"].asInt64()) {
            ERRLOG("Get restore log dir list failed, log can not apply scn restore");
            return MP_FAILED;
        }
        mp_bool flag = false;
        for (auto& item: m_scnMultiMap) {
            if (item.second["endscn"].asInt64() >= m_restorePoint) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            ERRLOG("Get restore log dir list failed, log can not apply timestamp restore");
            return MP_FAILED;
        }
        scnItem = m_scnMultiMap.end();
    }
    for (auto iterator = m_scnMultiMap.begin(); iterator != scnItem; ++iterator) {
        std::vector<mp_string> pathinfo;
        CMpString::StrSplit(pathinfo, iterator->second["dirname"].toStyledString(), '"');
        if (pathinfo.empty() || pathinfo.size() < PATH_SIZE) {
            ERRLOG("Parse resore meta file path file %s", iterator->second["dirname"].toStyledString().c_str());
            return MP_FAILED;
        }
        logdirlist.push_back(pathinfo[PATH_INDEX]);
    }
    return MP_SUCCESS;
}

mp_int32 PluginAnyTimeRestore::ComposeNewRepository(const std::vector<mp_string>& mountPoint, StorageRepository& stRep,
    Json::Value& jsonRep_new, mp_string associateds)
{
    LOGGUARD("");
#ifdef WIN32
    std::vector<mp_string> mountInfoVec;
    CMpString::StrSplit(mountInfoVec, mountPoint.front(), '&');
    mp_string restoreMetaPath = mountInfoVec.front() + PATH_SEPARATOR + m_restoreCopyid + ANYTIME_RESTORE_META_SUFFIX;
#else
    mp_string restoreMetaPath = mountPoint.front() + PATH_SEPARATOR + m_restoreCopyid + ANYTIME_RESTORE_META_SUFFIX;
#endif
    // 若ubc未下发日志副本信息 沿用原本实现 尝试解析.meta文件
    if (associateds.empty()) {
        if (ParseRestoreMetaInfo(restoreMetaPath) != MP_SUCCESS) {
            ERRLOG("Parse anytime restore meta info failed");
            return MP_FAILED;
        }
    } else {
        if (ParseRestoreMetaInfo(associateds, false) != MP_SUCCESS) {
            ERRLOG("Parse anytime restore meta info failed");
            return MP_FAILED;
        }
    }
    mp_int32 iRet = MP_FAILED;
    std::vector<mp_string> logDirlist;
    if (m_restoreType == TIME_RESTORE) {
        iRet = GetTimeStampLogDirList(logDirlist);
    } else if (m_restoreType == SCN_RESTORE) {
        iRet = GetSCNLogDirList(logDirlist);
    }
    if (iRet != MP_SUCCESS || logDirlist.empty()) {
        ERRLOG("Get log dir list failed");
        return MP_FAILED;
    }
    // 拼接每个日志副本在log仓的路径
    for (const auto &iter : logDirlist) {
        Json::Value logRepJson;
        StructToJson(stRep, logRepJson);
        logRepJson["path"] = Json::arrayValue;
        for (auto mountPt : mountPoint) {
#ifdef WIN32
            std::vector<mp_string> mountInfoVec;
            CMpString::StrSplit(mountInfoVec, mountPt, '&');
            mp_string logMountPath = mountInfoVec.front() + PATH_SEPARATOR + iter;
#else
            mp_string logMountPath = mountPt + PATH_SEPARATOR + iter;
#endif
            logRepJson["path"].append(logMountPath);
        }
        jsonRep_new.append(std::move(logRepJson));
    }
    return MP_SUCCESS;
}
}