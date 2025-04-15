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
#ifndef BACKUP_TIMER_H
#define BACKUP_TIMER_H

#include <iostream>
#include <map>
#include <vector>
#include "log/Log.h"
#include "FSBackupUtils.h"

class BackupTimer {
public:
    void Insert(FileHandle &fileHandle, int64_t retryTimeInterval)
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        int64_t currentTime = FSBackupUtils::GetMilliSecond();
        try {
            m_map.emplace(currentTime + retryTimeInterval, fileHandle);
        } catch (const std::exception& e) {
            ERRLOG("map emplace(%s) exception: %s", fileHandle.m_file->m_fileName.c_str(), e.what());
        }
    }

    // return value is next expired time, if without next expired event, return INT64_MAX.
    // fileHandles is expired events.
    int64_t GetExpiredEventAndTime(std::vector<FileHandle> &fileHandles)
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        int64_t currentTime = FSBackupUtils::GetMilliSecond();
        for (auto it = m_map.begin(); it != m_map.end();) {
            if (currentTime >= it->first) {
                fileHandles.push_back(it->second);
                m_map.erase(it++);
            } else {
                break;
            }
        }
        if (m_map.size() == 0) {
            return INT64_MAX;
        }
        return m_map.begin()->first - currentTime;
    }

    // return value is next expired time, if without next expired event, return INT64_MAX.
    // fileHandles is expired events.
    int64_t GetExpiredEventAndTime(std::vector<FileHandle> &fileHandles, int expiredCount)
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        int64_t currentTime = FSBackupUtils::GetMilliSecond();
        int count = 0;
        for (auto it = m_map.begin(); it != m_map.end();) {
            if (currentTime >= it->first && count < expiredCount) {
                fileHandles.push_back(it->second);
                it = m_map.erase(it);
                count++;
            } else {
                break;
            }
        }
        if (m_map.size() == 0) {
            return INT64_MAX;
        }
        if (m_map.begin()->first < currentTime) {
            return 0;
        }
        return m_map.begin()->first - currentTime;
    }

    void ClearAllEvents(std::vector<FileHandle> &fileHandles)
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        for (auto it = m_map.begin(); it != m_map.end();) {
            fileHandles.push_back(it->second);
            m_map.erase(it++);
        }
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        m_map.clear();
        INFOLOG("timer clear");
    }

    uint64_t GetCount()
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        return m_map.size();
    }

private:
    std::mutex m_mutex;
    std::multimap<int64_t, FileHandle> m_map;
};

#endif // BACKUP_TIMER_H