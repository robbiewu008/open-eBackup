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
#ifndef FS_SCANNER_OBJECT_META_READ_THREAD_H
#define FS_SCANNER_OBJECT_META_READ_THREAD_H

#include "BufferQueue.h"
#include "ScanStructs.h"
#include "StatisticsMgr.h"
#include "ScanConfig.h"
#include "ObjectUtils.h"
#include "DiffControlService.h"

struct SubPrefixParser {
    std::fstream fd {};
};

class ObjectMetaReadThread : public ObjectUtils {
public:
    ObjectMetaReadThread(std::shared_ptr<BufferQueue<Module::DirCache>> input,
        std::shared_ptr<BufferQueue<DirectoryScan>> output,
        std::shared_ptr<MetadataStat> scanMetaStatPtr,
        ScanConfig config,
        std::shared_ptr<StatisticsMgr> statsMgr,
        std::string subPrefixRootDir,
        std::string delimiter
        ) : m_input(input), m_output(output), m_scanMetaStat(scanMetaStatPtr), m_config(config),
        m_statsMgr(statsMgr), m_subPrefixRootDir(subPrefixRootDir), m_delimiter(delimiter)
    {};
    ~ObjectMetaReadThread() override {};
    bool Start();
    void ThreadFunc();
    bool IsExistNewKey();
    void PushDirToWriteQueue(DirectoryScan &node, const Module::DirMetaWrapper &dirWrapper);
    int FillDirMetaWrapperByMetaFile(uint16_t metaId, uint64_t metaOffset, Module::DirMetaWrapper &dmWrapper);
    int FillFileMetaWrapperByMetaFile(uint16_t metaId, uint64_t metaOffset, Module::FileMetaWrapper &fmWrapper);
    int ReadXMeta(uint64_t xMetaId, uint64_t xMetaOffset, std::vector<Module::XMetaField> &xMeta);
    int SetDirDcanNode(DirectoryScan &dirNode, Module::FileCache& fcache);
    void CleanFileRes();
    int HandleFcache(DirectoryScan& dirNode, Module::DirCache &dcache);
    void Exit();
    bool IsReadCompleted();
    std::string GetObjectLogFile(std::string &key);
    void ReadFromLogFile(std::string &key);

private:
    std::shared_ptr<BufferQueue<Module::DirCache>> m_input {};
    std::shared_ptr<BufferQueue<DirectoryScan>> m_output;
    std::shared_ptr<MetadataStat> m_scanMetaStat;
    bool m_exit;
    std::shared_ptr<std::thread> m_mainThread {};
    bool m_readCompleted = false;
    std::map<std::string, std::string> m_fileOperations {};
    bool m_dirLogExist { false };
    ScanConfig m_config {};
    std::shared_ptr<StatisticsMgr> m_statsMgr;
    std::string m_subPrefixRootDir;
    std::string m_delimiter;
    bool m_flag;
};
#endif