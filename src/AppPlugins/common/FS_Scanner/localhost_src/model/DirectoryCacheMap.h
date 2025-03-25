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
#ifndef FS_SCANNER_SCAN_CACHE_MAP_H
#define FS_SCANNER_SCAN_CACHE_MAP_H

#include <unordered_map>
#include "ParserStructs.h"

namespace {
    constexpr uint8_t ACCESS_RETRY_MAX_TIMES = 10;
    constexpr uint8_t STAT_RETRY_MAX_TIMES = 3;
}
class DirectoryCacheMap {
public:
    explicit DirectoryCacheMap(void *ptr, const std::string& path, uint8_t filterFlag)
        : m_ptr(ptr), m_path(path), m_filterFlag(filterFlag)
    {}

    explicit DirectoryCacheMap(const DirectoryCacheMap& cacheMap)
        : m_ptr(cacheMap.m_ptr), m_path(cacheMap.m_path), m_filterFlag(cacheMap.m_filterFlag), m_retry(cacheMap.m_retry)
    {}

    ~DirectoryCacheMap()
    {}

    void *GetPtr()
    {
        return m_ptr;
    }

    void Reset()
    {
        m_scanned = false;
        m_fmetaMap.clear();
        m_faclMap.clear();
        m_curFileCnt = 0;
        m_curStatCnt = 0;
        m_curFailedCnt = 0;
    }

    uint64_t TotalFiles()
    {
        return m_totalFileCnt;
    }

    uint64_t TotalSize()
    {
        return m_totalSize;
    }

    uint64_t TotalFailedFiles()
    {
        return m_totalFailedCnt;
    }

    void SetData(void *data)
    {
        m_data = data;
    }

    void *GetData()
    {
        return m_data;
    }

    void SetDirMeta(const Module::DirMeta& dmeta)
    {
        m_dmeta = dmeta;
    }

    void SetDirAcl(const std::string& dacl)
    {
        m_dacl = dacl;
    }

    void SetFileMeta(const std::string& name, const Module::FileMeta& fmeta)
    {
        m_fmetaMap.emplace(name, fmeta);
    }

    void SetFileCompleted()
    {
        m_curStatCnt++;
    }

    void SetFileFailed()
    {
        m_curFailedCnt++;
        m_totalFailedCnt++;
    }

    void SetFileAcl(const std::string& name, const std::string& facl)
    {
        m_faclMap[name] = facl;
    }

    void SetSize(uint16_t count, uint64_t size)
    {
        m_curFileCnt = count;
        m_totalFileCnt += count;
        m_totalSize += size;
        m_scanned = true;
    }

    bool CacheCompleted()
    {
        return (m_scanned && m_curFileCnt == (m_curStatCnt + m_curFailedCnt));
    }

    void GetDirPath(std::string &path)
    {
        path = m_path;
        return;
    }

    void GetDirMeta(Module::DirMeta &dmeta)
    {
        dmeta = m_dmeta;
        return;
    }

    void GetDirAcl(std::string &dacl)
    {
        dacl = m_dacl;
        return;
    }

    uint8_t GetFilterFlag()
    {
        return m_filterFlag;
    }

    void PopFileItem(std::string &name, std::string &facl, Module::FileMeta &fmeta)
    {
        auto metaIter = m_fmetaMap.begin();
        if (metaIter == m_fmetaMap.end()) {
            return;
        }
        name = metaIter->first;
        fmeta = metaIter->second;
        m_fmetaMap.erase(metaIter);
        auto aclIter = m_faclMap.find(name);
        if (aclIter == m_faclMap.end()) {
            return;
        }
        facl = aclIter->second;
        m_faclMap.erase(aclIter);
        return;
    }

    void Erase(std::string &name)
    {
        auto metaIter = m_fmetaMap.find(name);
        if (metaIter != m_fmetaMap.end()) {
            m_totalSize -= metaIter->second.m_size;
            m_fmetaMap.erase(metaIter);
        }
        auto aclIter = m_faclMap.find(name);
        if (aclIter != m_faclMap.end()) {
            m_faclMap.erase(aclIter);
        }
        m_curFileCnt--;
        m_totalFileCnt--;
        return;
    }

    uint64_t GetFileSize(const std::string &name)
    {
        auto metaIter = m_fmetaMap.find(name);
        if (metaIter == m_fmetaMap.end()) {
            return 0;
        }
        return metaIter->second.m_size;
    }

    bool Empty()
    {
        return m_fmetaMap.size() == 0;
    }

    int Size()
    {
        return m_fmetaMap.size();
    }

    void CallResume()
    {
        m_isResumeCalled = 1;
        return;
    }

    int IsResumeCalled()
    {
        return m_isResumeCalled;
    }

    bool CanRetry()
    {
        return m_retry < ACCESS_RETRY_MAX_TIMES;
    }

    void IncRetry()
    {
        m_retry++;
    }

    void SetPendingFlag()
    {
        m_pendingDelFlag = true;
    }

    bool PendingDelete()
    {
        return m_pendingDelFlag;
    }

private:
    void Clear()
    {
        Reset();
        m_totalFileCnt = 0;
        m_totalSize = 0;
        m_isResumeCalled = 0;
        m_totalFailedCnt = 0;
        m_dmeta = {};
        m_dacl = {};
        m_pendingDelFlag = false;
    }

private:
    void *m_ptr = nullptr;
    void *m_data = nullptr;
    uint64_t m_totalFileCnt {0};
    uint64_t m_totalSize {0};
    uint64_t m_totalFailedCnt {0};
    uint16_t m_curFileCnt {0};
    uint16_t m_curStatCnt {0};
    uint16_t m_curFailedCnt {0};
    uint8_t m_isResumeCalled {0};
    bool m_scanned {false};
    std::string m_path;
    std::string m_dacl;
    Module::DirMeta m_dmeta {};
    std::unordered_map<std::string, Module::FileMeta> m_fmetaMap;
    std::unordered_map<std::string, std::string> m_faclMap;
    uint8_t m_filterFlag {0};
    uint8_t m_retry {0};
    bool m_pendingDelFlag = false;
};

#endif
