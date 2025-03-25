/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * @file PosixStatistics.h
 * @version 0.1
 * @date 2022-07-08
 * @author h71726
 * @brief This will give statistics interfaces for the Posix.
 */

#ifndef DME_NAS_SCANNER_POSIX_STATISTICS_H
#define DME_NAS_SCANNER_POSIX_STATISTICS_H

#include "StatisticsMgr.h"

class PosixStatistics : public ProtocolStatistics {
    std::atomic_uint64_t m_protoStatistics[MAX_POSIX_STATS_TYPE];
    std::mutex m_posixStatsMutex;
public:
    PosixStatistics();
    ~PosixStatistics() override;
    void IncrProtoStatsByType(const int& idx, const uint64_t& incVal = 1) override;
    void SetProtoStatsByType(const int& idx, const uint64_t& newVal) override;
    uint64_t GetProtoStatsByType(const int& protoStatsType) override;
};
#endif