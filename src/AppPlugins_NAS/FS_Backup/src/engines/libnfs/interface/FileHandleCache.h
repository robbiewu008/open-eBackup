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
#ifndef LIBNFS_FILE_HANDLE_CACHE_H
#define LIBNFS_FILE_HANDLE_CACHE_H

#include <mutex>
#include <map>

class FileHandleCache {
public:
    bool Push(std::string dirName, nfsfh* fileHandle)
    {
        std::lock_guard<std::mutex> lk(mtx);
        auto iter = m_handleMap.find(dirName);
        if (iter == m_handleMap.end()) {
            m_handleMap.emplace(dirName, fileHandle);
            return true;
        }
        return false;
    }

    nfsfh* Get(std::string dirName)
    {
        std::lock_guard<std::mutex> lk(mtx);
        auto iter = m_handleMap.find(dirName);
        if (iter == m_handleMap.end()) {
            return nullptr;
        }
        return iter->second;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lk(mtx);
        for (auto mapEntry : m_handleMap) {
            struct nfsfh *nfsFh = (struct nfsfh *) (mapEntry.second);
            if (nfsFh == nullptr) {
                continue;
            }

            if (nfsFh->fh.val != nullptr) {
                free(nfsFh->fh.val);
            }
            free(nfsFh);
        }
        m_handleMap.clear();
    }

    uint64_t Size()
    {
        std::lock_guard<std::mutex> lk(mtx);
        return m_handleMap.size();
    }

private:
    std::mutex mtx {};
    std::map<std::string, nfsfh*> m_handleMap {};
};

#endif // LIBNFS_FILE_HANDLE_CACHE_H