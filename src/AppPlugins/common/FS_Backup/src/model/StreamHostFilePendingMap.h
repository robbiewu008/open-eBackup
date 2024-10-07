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
#ifndef STREAM_HOST_FILE_PENDING_MAP_H
#define STREAM_HOST_FILE_PENDING_MAP_H

#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <mutex>

class StreamHostFilePendingMap {
public:
    // <filepath in posix stype, (is host file backuped, sub stream remain remain not backuped)>
    using InnerMapType = std::unordered_map<std::string, std::pair<bool, uint64_t>>;

    StreamHostFilePendingMap() = default;
    void MarkHostWriteComplete(const std::string& filepath);
    void IncStreamPending(const std::string& streampath);
    void DecStreamPending(const std::string& streampath);
    bool IsHostWriteComplete(const std::string& streampath);
    uint64_t PendingStreamNum(const std::string& streampath);
    void MarkHostWriteFailed(const std::string& filepath);
    bool IsHostWriteFailed(const std::string& streampath);
private:
    // lock used for StreamHostFilePendingMapType mutual access
    std::mutex      m_mutex;

    InnerMapType    m_map;
    std::set<std::string> m_failedAdsHost;
};

#endif