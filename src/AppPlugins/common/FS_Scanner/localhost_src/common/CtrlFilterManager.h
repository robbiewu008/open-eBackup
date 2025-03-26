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
#ifndef CTRL_FILTER_MANAGER_H
#define CTRL_FILTER_MANAGER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <regex>
#include <queue>
#include "ParserStructs.h"

class PathNode {
public:
    explicit PathNode(std::shared_ptr<PathNode> parentNode = nullptr) : m_parent(parentNode) {}
    std::unordered_map<std::string, std::shared_ptr<PathNode>> m_children;
    std::weak_ptr<PathNode> m_parent;   // 父节点，用weak_ptr避免循环引用
    bool m_needAccept = false;
    bool m_isLeafNode = false;
    bool m_hasSetStatInfo = false;
    Module::Hash m_hash;
    uint32_t m_type = 0; // 0 : dir, 1: file
    uint32_t m_totalFiles = UINT32_MAX;
    uint32_t m_subDirs = UINT32_MAX;
    uint32_t m_compeleteFiles = 0;
    uint32_t m_compeleteDirs = 0;

    bool IsComplete()
    {
        return (m_totalFiles == m_compeleteFiles) && (m_subDirs == m_compeleteDirs);
    }
};

class PathNodeComparator {
public:
    bool operator()(const std::shared_ptr<PathNode>& node1, const std::shared_ptr<PathNode>& node2)
    {
        if (memcmp(node1->m_hash.sha1, node2->m_hash.sha1, Module::SHA_DIGEST_LENGTH * sizeof(unsigned char)) > 0) {
            return true;
        }
        return false;
    }
};

class PathTree {
public:
    PathTree()
    {
        m_root = std::make_shared<PathNode>();
    }
    void Insert(const std::string& path, bool isDir);
    bool AcceptDir(const std::string& dirName, uint32_t subDirCnt, uint32_t totalFiles);
    bool AcceptRangeForDir(Module::Hash hash1, Module::Hash hash2, bool isCur);
    void MarkEntryComplete(const std::string& path, bool isDir);
    bool IsComplete();

private:
    std::vector<std::string> SplitPath(const std::string& path);
    // 格式化路径，删除连续的多个 '/'，同时确保路径以单个 '/' 开头
    std::string FormatPath(const std::string& path);
    uint32_t GetNodeDepth(std::shared_ptr<PathNode> node);
    std::string GetFullPath(std::shared_ptr<PathNode> node);
    void MarkFileComplete(std::shared_ptr<PathNode> node,
        const std::vector<std::string>& parts);
    void MarkDirComplete(std::shared_ptr<PathNode> node,
        const std::vector<std::string>& parts);

private:
    std::shared_ptr<PathNode> m_root;
    std::vector<std::shared_ptr<PathNode>> m_leafNodes;
    std::priority_queue<std::shared_ptr<PathNode>, std::vector<std::shared_ptr<PathNode>>,
        PathNodeComparator> m_curQueue;
    std::priority_queue<std::shared_ptr<PathNode>, std::vector<std::shared_ptr<PathNode>>,
        PathNodeComparator> m_prevQueue;
};

class CtrlFilterManager {
public:
    CtrlFilterManager(const std::vector<std::string>& fCtrlFltr, const std::vector<std::string>& dCtrlFltr);
    ~CtrlFilterManager() {}
    void MarkFilterEntryComplete(const std::string& file, bool isDir);
    bool AcceptDir(const std::string& dirName, uint32_t subDirCnt = UINT32_MAX, uint32_t totalFiles = UINT32_MAX);
    bool IsFilterComplete();
    bool IsDirEnabled() { return m_dirFilterEnable; }
    bool AcceptRangeForDir(Module::Hash hash1, Module::Hash hash2, bool isCur);

private:
    PathTree m_pathTree;
    bool m_filterEnable = false;
    bool m_dirFilterEnable = false;
};

#endif