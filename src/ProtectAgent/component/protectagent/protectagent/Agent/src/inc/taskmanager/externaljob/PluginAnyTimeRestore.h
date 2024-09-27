/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @brief  Factory for Plugin job
 * @version 1.0.0
 * @date 2022-1-5
 * @author twx949498
 */
#ifndef PLGUIN_ANYTIME_RESTORE
#define PLGUIN_ANYTIME_RESTORE

#include <map>
#include "common/Types.h"
#include "common/JsonUtils.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"


namespace AppProtect {

class PluginAnyTimeRestore {
public:
    PluginAnyTimeRestore();
    ~PluginAnyTimeRestore();

    mp_int32 ComputerAnyTimeRestoreLogPath(const Json::Value& jobinfo, const std::vector<mp_string>& mountPoint,
        StorageRepository& stRep, Json::Value& jsonRep_new);

private:
    mp_int32 ParseRestoreMetaInfo(mp_string& metapath);
    mp_int32 GetTimeStampLogDirList(std::vector<mp_string>& logdirlist);
    mp_int32 GetSCNLogDirList(std::vector<mp_string>& logdirlist);
    mp_int32 ComposeNewRepository(const std::vector<mp_string>& mountPoint, StorageRepository& stRep,
        Json::Value& jsonRep_new);

    std::multimap<mp_time, Json::Value> m_timeMultiMap;
    std::multimap<mp_long, Json::Value> m_scnMultiMap;
    mp_string m_taskID;
    mp_int32 m_restoreType;
    mp_string m_restoreCopyid;
    mp_uint64 m_restorePoint;
};
}
#endif