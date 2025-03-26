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
#ifndef DME_NAS_SCANNER_OUTPUT_STATS_H
#define DME_NAS_SCANNER_OUTPUT_STATS_H
#include "ParserStructs.h"

namespace {
    constexpr uint16_t MILLI_SEC = 1000;
}

class OutputStats {
public:
    uint32_t m_dirFileCount = 0;
    uint64_t m_lastBkupTime = 0;
    std::function<void(void*, std::string)> m_scanResultCb;
    std::function<void(void*, std::string)> m_scanHardlinkResultCb;
    uint64_t m_fileDataLength = 0;
    uint64_t m_dirDataLength = 0;
    std::mutex m_mtx {};

    int64_t GetCurTime()
    {
        /* Hint:: This method is dulplicate with Time::Now() in ScannerTimer.h */
#ifdef WIN32
        /* cross-platform milliseconds timestamp api */
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
#else
        timeval curTime {};
        gettimeofday(&curTime, nullptr);
        int64_t milli = (curTime.tv_sec * uint64_t(MILLI_SEC)) + (curTime.tv_usec / MILLI_SEC);
        return milli;
#endif
    }

    void CleanData()
    {
    }

    OutputStats() : m_lastBkupTime()
    {}

    ~OutputStats()
    {}
};

#endif // DME_NAS_SCANNER_OUTPUT_STATS_H
