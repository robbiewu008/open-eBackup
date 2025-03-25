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
#ifndef SCANNER_OBJECTINODE_H
#define SCANNER_OBJECTINODE_H
#include <mutex>
#include <map>
#include <vector>
#include <unordered_set>

class ObjectInode {
public:
    explicit ObjectInode() {};
    ~ObjectInode() {};

    /**
    * GetInodeValue--获取对象的Inode值
    *
    * @param char *str 对象的key值或者目录的prefix值
    * @param size_t len str的长度
    * @return 对象的Inode值 inode = hash(key) (64bit) << 16 + conflict_id(16bit)
    */

    std::uint64_t GetInodeValue(const char *str, std::size_t len);
    /**
    * SaveConflictRecord--保存冲突的Inode值
    *
    * @param path 文件路径和文件名objectconflictrecord
    * @return true：保存成功, false：保存失败
    */
    bool SaveConflictRecord(const std::string path);
    bool ReadConflictRecord(const std::string path);
    bool IsExistConflict();
private:
    mutable std::mutex m_mutex;
    std::map<std::uint32_t, std::unordered_set<std::uint64_t>> m_hashValueTable;
    std::map<std::uint64_t, std::map<std::string, std::uint64_t>> m_conflictMap;
    void InsertHashValueTable(const std::uint64_t& val);
    bool ContainsHashValue(const std::uint64_t& val);
    std::uint64_t RecordConflict(std::uint64_t hashValueHi48, const char *str);
    std::uint64_t FindConflictRecord(std::uint64_t hashValueHi48, const char *str);
};
#endif // DME_NAS_SCANNER_OBJECTINODE_H