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