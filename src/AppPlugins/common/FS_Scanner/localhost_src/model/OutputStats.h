/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: g00554214
 * Create: 12/7/2020
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
