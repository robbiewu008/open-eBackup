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
#ifndef FS_SCANNER_H
#define FS_SCANNER_H
#include <memory>
#include "ScanConfig.h"

struct ScanStatistics {
    uint64_t mScanDuration = 0;                 /* Total scan duration (in seconds) */
    uint64_t mTotDirs = 0;                      /* Total num of dir detected in NAS Share */
    uint64_t mTotFiles = 0;                     /* Total num of files detected in NAS Share */
    uint64_t mTotalSize = 0;                    /* Total size of the NAS Share */
    uint64_t mTotDirsToBackup = 0;              /* Total num of dir (new/modified) to backup */
    uint64_t mTotFilesToBackup = 0;             /* Total num of files(new/modified) to backup */
    uint64_t mTotFilesDeleted = 0;              /* Total num of files to be deleted */
    uint64_t mTotDirsDeleted = 0;               /* Total num of dirs to be deleted */
    uint64_t mTotalSizeToBackup = 0;            /* Total size to backup */
    uint64_t mTotalControlFiles = 0;            /* Total Control Files Generated */
    uint64_t mTotFailedDirs = 0;                /* Total num of Failed dir detected in NAS Share */
    uint64_t mTotFailedFiles = 0;               /* Total num of Failed files detected in NAS Share */
    uint64_t mEntriesMayFailedToArchive = 0;    /* Total num of file/dir with long path/name may failed to archive */
};

class Scanner {
public:
    explicit Scanner(const ScanConfig& scanConfig) : m_config(scanConfig)
    {}
    virtual ~Scanner() {}

    virtual SCANNER_STATUS Start() = 0;
    virtual SCANNER_STATUS Abort() = 0;
    virtual SCANNER_STATUS Destroy() = 0;
    virtual SCANNER_STATUS Enqueue(const std::string& path, const std::string& prefix = "", uint8_t filterFlag = 0) = 0;
#ifdef WIN32
    virtual SCANNER_STATUS EnqueueWithoutSnapshot(
        const std::string& path,
        const std::string& prefix = "",
        uint8_t filterFlag = 0) = 0;
#endif
    virtual SCANNER_STATUS EnqueueV2(const std::string& path) = 0;
    virtual SCANNER_STATUS GetStatus() = 0;
    virtual ScanStatistics GetStatistics() = 0;
    virtual ErrRecorder QueryFailure() = 0;

protected:
    ScanConfig m_config;
};

#endif