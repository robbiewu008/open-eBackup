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
    constexpr int32_t TIME_INFO_SIZE = 2;
    constexpr int32_t START_TIME_INDEX = 0;
    constexpr int32_t END_TIME_INDEX = 1;
    constexpr int32_t SCN_INFO_SIZE = 2;
    constexpr int32_t START_SCN_INDEX = 0;
    constexpr int32_t END_SCN_INDEX = 1;
    constexpr int32_t LOG_INFO_MINI_SIZE = 2;
    constexpr int32_t LOG_INFO_MAX_SIZE = 3;
    constexpr int32_t TIME_RESTORE = 1;
    constexpr int32_t SCN_RESTORE = 2;
    constexpr int32_t PATH_INDEX = 1;
    constexpr int32_t PATH_SIZE = 3;
    constexpr int32_t NUM_TEN = 10;
    const mp_string ANYTIME_RESTORE_META_SUFFIX = ".meta";
    const mp_string KEY_EXTENDINFO = "extendInfo";
    const mp_string KEY_RESTORECOPYID = "restoreCopyId";
    const mp_string KEY_RESTORETIMESTAMP = "restoreTimestamp";
    const mp_string KEY_RESTORESCN = "restoreScn";
    const mp_string KEY_COPYINFO = "copies";
    const mp_string KEY_REPOSITORIES = "repositories";
    const mp_string KEY_TYPE = "type";
    const mp_string KEY_PATH = "path";
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

mp_int32 PluginAnyTimeRestore::ComputerAnyTimeRestoreLogPath(const Json::Value& jobParam,
    const std::vector<mp_string>& mountPoint, StorageRepository& stRep, Json::Value& jsonRep_new)
{
    LOGGUARD("");
    if (!jobParam.isObject() || !jobParam.isMember("extendInfo") || !jobParam["extendInfo"].isObject()) {
        ERRLOG("Job param is not json object or have no extendInfo key.");
        return MP_FAILED;
    }
    Json::Value jValueExtendinfo = jobParam["extendInfo"];
    if (!jValueExtendinfo.isMember("restoreTimestamp") && !jValueExtendinfo.isMember("restoreScn")) {
        ERRLOG("Job extendInfo have no restoreTimestamp and restoreScn.");
        return MP_FAILED;
    }
    GET_JSON_STRING(jobParam, "taskId", m_taskID);
    auto extendValue = jobParam[KEY_EXTENDINFO];
    GET_JSON_STRING(extendValue, KEY_RESTORECOPYID, m_restoreCopyid);
    mp_string restorePoint;
    if (extendValue.isMember(KEY_RESTORETIMESTAMP)) {
        m_restoreType = TIME_RESTORE;
        GET_JSON_STRING(extendValue, KEY_RESTORETIMESTAMP, restorePoint);
        m_restorePoint = strtoull(restorePoint.c_str(), NULL, NUM_TEN);
    } else if (extendValue.isMember(KEY_RESTORESCN)) {
        m_restoreType = SCN_RESTORE;
        GET_JSON_STRING(extendValue, KEY_RESTORESCN, restorePoint);
        m_restorePoint = strtoull(restorePoint.c_str(), NULL, NUM_TEN);
    } else {
        ERRLOG("Jobparam have not anytime restorekey ");
        return MP_FAILED;
    }
    if (ComposeNewRepository(mountPoint, stRep, jsonRep_new) != MP_SUCCESS) {
        ERRLOG("Anytime restore compose new repository failed");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 PluginAnyTimeRestore::ParseRestoreMetaInfo(mp_string& metapath)
{
    LOGGUARD("");
    if (metapath.empty()) {
        ERRLOG("Restore metapath is empty");
        return MP_FAILED;
    }
    std::vector<mp_string> vecLoglistInfo;
#ifdef WIN32
    CMpFile::ReadFile(metapath, vecLoglistInfo);
#else
    CRootCaller rootCaller;
    if (rootCaller.Exec((mp_int32)ROOT_COMMAND_CAT, metapath, &vecLoglistInfo) != MP_SUCCESS) {
        ERRLOG("Get metadata info failed, path is %s", metapath.c_str());
        return MP_FAILED;
    }
#endif
    for (const auto& logdata : vecLoglistInfo) {
        std::vector<mp_string> loginfo;
        CMpString::StrSplit(loginfo, logdata, ';');
        if (loginfo.size() < LOG_INFO_MINI_SIZE) {
            WARNLOG("Get restore meta log copy info failed loginfo size is %d", loginfo.size());
            continue;
        }
        Json::Value logjson;
        logjson["dirname"] = loginfo[LOG_DIR_INDEX];
        if (m_restoreType == TIME_RESTORE) {
            std::vector<mp_string> timeinfo;
            CMpString::StrSplit(timeinfo, loginfo[LOG_TIME_INDEX], '~');
            if (timeinfo.size() != TIME_INFO_SIZE) {
                WARNLOG("Get restore meta log copy timeinfo failed.");
                continue;
            }
            logjson["starttime"] = std::stol(timeinfo[START_TIME_INDEX]);
            logjson["endtime"] = std::stol(timeinfo[END_TIME_INDEX]);
            m_timeMultiMap.insert(std::make_pair(logjson["starttime"].asInt64(), logjson));
        }
        if (m_restoreType == SCN_RESTORE) {
            std::vector<mp_string> scninfo;
            CMpString::StrSplit(scninfo, loginfo[LOG_SCN_INDEX], '~');
            if (scninfo.size() !=  SCN_INFO_SIZE) {
                ERRLOG("Get restore meta log copy scn failed.");
                continue;
            }
            logjson["startscn"] = std::stol(scninfo[START_SCN_INDEX]);
            logjson["endscn"] = std::stol(scninfo[END_SCN_INDEX]);
            m_scnMultiMap.insert(std::make_pair(logjson["startscn"].asInt64(), logjson));
        }
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
    Json::Value& jsonRep_new)
{
    LOGGUARD("");
#ifdef WIN32
    std::vector<mp_string> mountInfoVec;
    CMpString::StrSplit(mountInfoVec, mountPoint.front(), '&');
    mp_string restoreMetaPath = mountInfoVec.front() + PATH_SEPARATOR + m_restoreCopyid + ANYTIME_RESTORE_META_SUFFIX;
#else
    mp_string restoreMetaPath = mountPoint.front() + PATH_SEPARATOR + m_restoreCopyid + ANYTIME_RESTORE_META_SUFFIX;
#endif
    if (ParseRestoreMetaInfo(restoreMetaPath) != MP_SUCCESS) {
        ERRLOG("Parse anytime resore meta info failed");
        return MP_FAILED;
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