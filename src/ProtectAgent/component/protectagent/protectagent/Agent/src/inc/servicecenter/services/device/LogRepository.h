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
#ifndef _LOG_REPOSITORY_H
#define _LOG_REPOSITORY_H

#include <vector>
#include "common/Types.h"
#include "servicecenter/services/device/Repository.h"

namespace AppProtect {

class LogRepository : public Repository {
public:
    LogRepository() : Repository()
    {}
    ~LogRepository()
    {}

protected:
    mp_int32 AssembleRepository(const PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new);
    mp_void QueryRepoSubPath(const PluginJobData &data, std::vector<mp_string> &path);
private:
    mp_bool isNeedOracleMigrate(const PluginJobData &data);
    mp_bool isSkipLogRepoCompose(const Json::Value& jobParam);
    mp_void AssembleLogMetaPath(StorageRepository &stRep, Json::Value &jsonRep_new);
    mp_void AssembleLogPath(const PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new);
};
}
#endif