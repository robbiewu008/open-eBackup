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
    mp_int32 ParseRestoreMetaInfo(mp_string& metapath, bool is_metapath = true);
    mp_int32 GetTimeStampLogDirList(std::vector<mp_string>& logdirlist);
    mp_int32 GetSCNLogDirList(std::vector<mp_string>& logdirlist);
    mp_int32 ComposeNewRepository(const std::vector<mp_string>& mountPoint, StorageRepository& stRep,
        Json::Value& jsonRep_new, mp_string associateds);
    void GenerateMultiMap(bool is_metapath, std::vector<mp_string>& loginfo);
    mp_string GetAssociatedCopies(const Json::Value& jValueExtendinfo);

    std::multimap<mp_time, Json::Value> m_timeMultiMap;
    std::multimap<mp_long, Json::Value> m_scnMultiMap;
    mp_string m_taskID;
    mp_int32 m_restoreType;
    mp_string m_restoreCopyid;
    mp_uint64 m_restorePoint;
};
}
#endif