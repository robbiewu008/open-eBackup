/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Author: g00554214
 * Create: 05/08/2021.
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
