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
#ifndef FS_SCANNER_SCAN_INFO_H
#define FS_SCANNER_SCAN_INFO_H

#include <mutex>
#include <atomic>
#include "StatisticsMgr.h"
class ScanInfo {
public:
    SCANNER_STATUS m_scanStatus = SCANNER_STATUS::INIT;   /* Scan Status */
    time_t m_scanStartTime {};                                /* scan start time */
    time_t m_remoteNasScanEndTime {};                         /* scan end time */
    time_t m_scanEndTime {};                                  /* overall (remote nas scan + control write) end time */
    std::shared_ptr<StatisticsMgr> m_scanStatistics = nullptr;     /* Statistics during Nas share Scan phase */
    std::shared_ptr<StatisticsMgr> m_diffStatistics = nullptr;     /* Statistics during CRC DIFF Phase */
    bool m_readWriteCompleted = false;
    uint32_t m_WriteJobCountExecCnt = 0;
    uint32_t m_ReadJobCountExecCnt = 0;
    uint64_t m_readDirObjects {0};
    bool m_terminateFlag = false;
    bool m_stopScan = false;
    std::string m_finalDcacheFile = "";
    std::atomic_uint32_t m_dirCacheFileIndex = 0;
    std::vector<std::string> m_dirCacheFileList {};           /* Global dircache sorted list */
    std::mutex m_mtxReaddir {};
    std::mutex m_createNewMetaFileLck {};
    std::atomic_uint64_t m_pendingOpenDirReqCnt {0};
    std::atomic_uint64_t m_totalOpendirCount {0};
    std::atomic_uint64_t m_opendirReqCount {0};
    std::atomic_uint64_t m_opendirRespCount {0};
    std::atomic_uint64_t m_opendirResumeCount {0};
    std::atomic_uint64_t m_opendirRetryCount {0};
    std::mutex m_mtx {};
    std::mutex m_mtxDirCacheFileList {};
    std::mutex m_writeTaskMtx {};
    std::atomic_uint32_t m_metaFileCountIterator {0};
    std::atomic_uint32_t m_xMetaFileCountIterator {0};
    std::atomic_uint32_t m_fcacheFileCountIterator {0};
    std::atomic_uint32_t m_protServerFailCnt{0};
    std::atomic_uint64_t m_lastCheckpointTime {};
    bool m_isCheckPointRunning = false;
    std::mutex m_checkpointMtx {};
    std::mutex m_opendirCbMtx {};

    inline void StopScan(SCANNER_STATUS scanStatus)
    {
        m_scanStatus = scanStatus;
        m_stopScan = true;
    }

    int GetDirCaheFileListSize()
    {
        std::lock_guard<std::mutex> lk(m_mtxDirCacheFileList);
        if (m_dirCacheFileList.empty()) {
            return 0;
        }
        return m_dirCacheFileList.size();
    }

    /* Add new file to directory cache list */
    void AddToDirCacheFileList(std::string fname)
    {
        std::lock_guard<std::mutex> lk(m_mtxDirCacheFileList);
        m_finalDcacheFile = fname;
        m_dirCacheFileList.push_back(fname);
    }

    /* pop entry from directory cache file list */
    std::string PopDirCaheFileEntry()
    {
        std::lock_guard<std::mutex> lk(m_mtxDirCacheFileList);
        if (m_dirCacheFileList.empty()) {
            return "";
        }
        std::string val = m_dirCacheFileList.back();
        m_dirCacheFileList.pop_back();
        return val;
    }

    int GetUpdatedDirCacheIndex()
    {
        std::lock_guard<std::mutex> lk(m_mtxDirCacheFileList);
        m_dirCacheFileIndex++;
        return m_dirCacheFileIndex;
    }
    /* pop entry from directory cache file list */
    SCANNER_STATUS PopDirCaheFileEntry(std::string &fileName1, std::string &fileName2)
    {
        std::lock_guard<std::mutex> lk(m_mtxDirCacheFileList);
        if (m_dirCacheFileList.empty() || m_dirCacheFileList.size() <= 1) {
            return SCANNER_STATUS::FAILED;
        }
        fileName1 = m_dirCacheFileList.back();
        m_dirCacheFileList.pop_back();
        fileName2 = m_dirCacheFileList.back();
        m_dirCacheFileList.pop_back();
        return SCANNER_STATUS::SUCCESS;
    }
};

#endif