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
#ifndef _CACHE_REPOSITORY_H
#define _CACHE_REPOSITORY_H

#include <vector>
#include "common/Types.h"
#include "servicecenter/services/device/Repository.h"

namespace AppProtect {

class CacheRepository : public Repository {
public:
    CacheRepository() : Repository()
    {}
    ~CacheRepository()
    {}

    mp_int32 Umount(
        const std::vector<mp_string> &mountPoints, const mp_string &jobID, const bool &isFileClientMount = false);

protected:
    mp_int32 AssembleRepository(const PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new);
    mp_void QueryRepoSubPath(const PluginJobData &data, std::vector<mp_string> &path);
};
}
#endif