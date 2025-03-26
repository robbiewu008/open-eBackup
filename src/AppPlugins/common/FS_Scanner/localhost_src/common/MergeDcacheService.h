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
#ifndef FS_SCANNER_MERGE_DCACHE_SERVICE_H
#define FS_SCANNER_MERGE_DCACHE_SERVICE_H

#include <string>
#include "ParserStructs.h"
#include "ParserUtils.h"
#include "ControlFileUtils.h"
#include "MergeDcacheThread.h"

class MergeDcacheService : public ControlFileUtils {
public:
    explicit MergeDcacheService(ScanConfig &config, ScanInfo &info)
        : m_config(config),
          m_info(info)
    {
        m_directory = m_config.metaPath + "/latest/";
    }
    ~MergeDcacheService() override
    {
    }

    SCANNER_STATUS MergeAllDCacheFiles();
    std::string CreateFinalDirCacheFile();

private:
    std::string m_directory {};
    ScanConfig &m_config;
    ScanInfo &m_info;
    std::vector<std::string> m_dirCacheFileList {};
    int m_dirCacheFileIndex = 0;
    std::vector<std::shared_ptr<MergeDcacheThread>> m_mergeThreads {};

    bool MergeCompleted() const;
    SCANNER_STATUS SubmitMergeFileToThread();
    SCANNER_STATUS CheckMergeThreadStatus();
    SCANNER_STATUS StopAllThreads();
};


#endif  // FS_SCANNER_MERGE_DCACHE_SERVICE_H
