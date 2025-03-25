/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: Object storage statistics.
* Author: w00444223
* Create: 2023-12-04
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