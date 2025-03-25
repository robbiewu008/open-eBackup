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
#ifndef DME_NAS_SCANNER_HARDLINK_MANAGER_H
#define DME_NAS_SCANNER_HARDLINK_MANAGER_H
#include "ParserStructs.h"

class HardlinkManager {
public:
    std::map<uint64_t, std::string> m_hardlinkdelMap {};
    std::map<std::string, uint32_t> m_hardlinkFilesCntOfDirPathMap {};
    std::map<uint64_t, std::vector<Module::HardlinkFileCache>> m_hardlinkMap {};
    std::mutex m_mtx {};

    void CleanData()
    {
        m_hardlinkdelMap.clear();
        m_hardlinkMap.clear();
        m_hardlinkFilesCntOfDirPathMap.clear();
    }

    HardlinkManager()
    {}

    void InsertHardLinkFileToMap(Module::HardlinkFileCache fc)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        if (m_hardlinkMap.find(fc.m_inode) == m_hardlinkMap.end()) {
            std::vector<Module::HardlinkFileCache> vfc {};
            vfc.push_back(fc);
            m_hardlinkMap.emplace(fc.m_inode, vfc);
        }  else {
            m_hardlinkMap[fc.m_inode].push_back(fc);
        }
    }

    bool InsertToHardlinkDelMap(uint64_t inode, std::string fileName)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        if (m_hardlinkdelMap.find(inode) == m_hardlinkdelMap.end()) {
            m_hardlinkdelMap.emplace(inode, fileName);
            return true;
        }
        return false;
    }

    bool InsertDirectoryHardlinkFileCount(std::string dirPath, uint32_t hardLinkFilesCnt)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_hardlinkFilesCntOfDirPathMap[dirPath] = hardLinkFilesCnt;
        return true;
    }

    uint32_t GetHardlinkDirCount(std::string dirPath)
    {
        uint32_t count = m_hardlinkFilesCntOfDirPathMap[dirPath];
        return count;
    }

    ~HardlinkManager()
    {}
};

#endif // DME_NAS_SCANNER_HARDLINK_MANAGER_H
