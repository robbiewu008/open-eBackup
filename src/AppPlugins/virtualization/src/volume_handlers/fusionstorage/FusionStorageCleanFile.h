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
#ifndef FUSIONSTORAGE_CLEAN_FILE_H
#define FUSIONSTORAGE_CLEAN_FILE_H
#include <string>
#include <vector>
#include "common/Macros.h"
#include "common/Constants.h"
#include "common/JsonUtils.h"

namespace VirtPlugin {
class GuardFileLock {
public:
    explicit GuardFileLock(const std::string &filePath);
    ~GuardFileLock();

private:
    GuardFileLock(const GuardFileLock &copyStructFunc);
    GuardFileLock &operator=(const GuardFileLock &assignFunc);

private:
    int32_t m_fd;
};

class FusionStorageCleanFile {
public:
    FusionStorageCleanFile();
    ~FusionStorageCleanFile();

    int32_t AddOrReplaceItem(const std::string &key, const std::string &item, const bool busy = true);

    int32_t AddOrReplaceItemNoLock(const std::string &key, const std::string &item, const bool busy);

    int32_t GetAllItems(std::vector<std::string> &items);

    int32_t RemoveItem(const std::string &key);

    int32_t RemoveItemNoLock(const std::string &key);

    void SetAllItemIdle();

    const std::string &GetCleanFilePath() const
    {
        return m_cleanFilePath;
    }

    int32_t GetAllItemsNoLock(std::vector<std::string> &items);
    void RemoveSpecificArrayElement(Json::Value &itemJson,
        const std::string &key, const std::string &value);

private:
    int32_t SaveToFile(const std::vector<std::string> &items);
    std::string TrimRight(const std::string &str);

private:
    std::string m_cleanFilePath;
};
}

#endif  // FUSIONSTORAGE_CLEAN_FILE_H