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
#ifndef FS_SCANNER_MERGE_DCACHE_OBJECT_H
#define FS_SCANNER_MERGE_DCACHE_OBJECT_H
 
#include <string>
#include "ParserStructs.h"
#include "ParserUtils.h"
#include "ControlFileUtils.h"
 
class MergeDcache : public ControlFileUtils {
public:
    explicit MergeDcache(ScanConfig &config)
        : m_config(config)
    {
#ifdef WIN32
    m_directory = m_config.metaPath + "\\latest\\";
#else
    m_directory = m_config.metaPath + "/latest/";
#endif
    }
    ~MergeDcache() override
    {
    }
 
    std::string MergeDirCacheFiles(std::string fileName1, std::string fileName2, int index);
private:
    std::string m_directory {};
    ScanConfig &m_config;
 
    void RemoveDcacheFiles(std::string fileName1, std::string fileName2) const;
    void CopyToprevHash(Module::DirCache &dcache, Module::DirCache &prevHash);
    SCANNER_STATUS CheckDircachePath(Module::DirCache &dcache1, Module::DirCache &dcache2);
    std::shared_ptr<Module::DirCacheParser> InitDcacheObj(std::string fname, Module::CTRL_FILE_OPEN_MODE mode);
    void WriteToTmpDirCacheFile(std::queue<Module::DirCache> &dcQueue,
        std::shared_ptr<Module::DirCacheParser> &dcacheObj, Module::DirCache &prevHash);
    void CompareAndWriteToTmpDirCache(std::queue<Module::DirCache> &dcQueue1,
        std::queue<Module::DirCache> &dcQueue2, std::shared_ptr<Module::DirCacheParser> &dcacheObj,
        Module::DirCache &prevHash);
    std::string GetDirCacheFileName(int index) const;
    void CompareAndWriteToTmpDirCacheByCrc(std::queue<Module::DirCache> &dcQueue1,
        std::queue<Module::DirCache> &dcQueue2, std::shared_ptr<Module::DirCacheParser> &dcacheObj,
        Module::DirCache &prevHash);
    void CompareAndWriteToTmpDirCacheBySha1(std::queue<Module::DirCache> &dcQueue1,
        std::queue<Module::DirCache> &dcQueue2, std::shared_ptr<Module::DirCacheParser> &dcacheObj,
        Module::DirCache &prevHash);
};
 
 
#endif  // FS_SCANNER_MERGE_DCACHE_OBJECT_H