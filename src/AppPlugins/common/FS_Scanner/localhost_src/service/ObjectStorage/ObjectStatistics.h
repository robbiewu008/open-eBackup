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
#ifndef FS_SCANNER_OBJECT_STATISTICS_H
#define FS_SCANNER_OBJECT_STATISTICS_H

#include "StatisticsMgr.h"

struct SubDirUsedStat {
    std::string prefixName;
    std::vector<std::pair<std::string, uint32_t>> prefixSubDir {};
};

struct BucketDir {
    std::string bucketName;
    /*
     * /bucket1/prefix1/dir1, 100
     * /bucket1/prefix1/dir2, 100
     * ......
     */
    std::vector<SubDirUsedStat> subDir;
};

class ObjectStatistics : public ProtocolStatistics {
    std::atomic_uint64_t m_protoStatistics[MAX_OBJECT_STATS_TYPE];
    std::vector<BucketDir>  m_bucketDirStats {};
    std::pair<std::string, uint32_t> curSubDir {};
    std::mutex m_ObjectStatsMutex;
public:
    ObjectStatistics();
    ~ObjectStatistics() override;
    void IncrProtoStatsByType(const int& idx, const uint64_t& incVal = 1) override;
    void SetProtoStatsByType(const int& idx, const uint64_t& newVal) override;
    uint64_t GetProtoStatsByType(const int& idx) override;
};

#endif