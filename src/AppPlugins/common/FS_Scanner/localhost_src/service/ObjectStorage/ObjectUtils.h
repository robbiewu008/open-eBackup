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
#ifndef FS_SCANNER_OBJECT_UTILS_H
#define FS_SCANNER_OBJECT_UTILS_H

#include <dirent.h>
#include "ScanStructs.h"
#include "ScanConfig.h"
#include "ObjectInode.h"

class ObjectUtils {
public:
    explicit ObjectUtils() {};
    virtual ~ObjectUtils() {};

    int SetObjectInode();
    void SaveInodeConflict(std::string dirPath);
    void ReadInodeConflict(std::string dirPath);
    uint64_t HashForObject(const std::string& key);
    static bool IsBucketName(const std::vector<ObjectStorageBucket>& buckets, const std::string& path);
    std::string GetFullPath(const std::string& prefix, const std::string& path);
    std::string GetObjectDirName(const std::vector<Module::XMetaField>& xMeta);
    std::string MakeObjectDirName(const DirStat &dirStat);
    std::string GetObjectFileName(const Module::FileMeta &fmeta);
    uint64_t GetObjectDirInode(const DirStat &dirStat);
    uint64_t GetObjectFileInode(const FileStat& fileStat);
    std::string GetObjectPath(const std::vector<Module::XMetaField>& xMeta);

    void CopyStatToDirMeta(Module::DirMeta &dmeta, const DirStat &dirStat);
    void CopyStatToFileMeta(Module::FileMeta &fmeta, const FileStat& fileStat);

private:
    std::shared_ptr<ObjectInode> m_genInode {nullptr};
};

#endif
