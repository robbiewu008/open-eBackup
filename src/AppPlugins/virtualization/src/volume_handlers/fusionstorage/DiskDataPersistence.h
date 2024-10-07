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
#ifndef DISK_DATA_PERSISTENCE_H
#define DISK_DATA_PERSISTENCE_H

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <json/json.h>
#include "FusionStorageDef.h"
#include "FusionStorageCleanFile.h"
#include "common/Structs.h"

namespace VirtPlugin {
using std::shared_ptr;

class DiskDataPersistence {
public:
    DiskDataPersistence();
    ~DiskDataPersistence();

    void AddObject(const std::string &key, const std::string &value);

    void RemoveObject(const std::string &key);

    std::string GetObject(const std::string &key);

    void AppendArrayElement(const std::string &key, const std::string &value);

    void RemoveArrayElement(const std::string &key, const std::string &value);

    std::string GetFirstArrayElement(const std::string &key);

    bool ToJsonValue(const std::string &jsonStr);

    std::string ToJsonString();

    std::string GetCachedUUID();

    void Finish();

private:
    DiskDataPersistence(const DiskDataPersistence &);

    const DiskDataPersistence &operator=(const DiskDataPersistence &);

    bool IsCachedataUsefull();

    void ChangeDiskDataPersistence();

private:
    Json::Value m_cachedObjects;
    std::string m_cacheUUID;
    std::shared_ptr<FusionStorageCleanFile> m_cleanFile;
};
}

#endif  // DISK_DATA_PERSISTENCE_H