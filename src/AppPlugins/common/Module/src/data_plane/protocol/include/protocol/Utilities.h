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
#ifndef DP_UTIL_H
#define DP_UTIL_H
 
// std
#include <cstdint>
 
// Linux
#include <sys/stat.h>
 
// Fuse
#define FUSE_USE_VERSION 34
#include <fuse_lowlevel.h>
 
struct TimePoint {
    std::int64_t seconds;
    std::int64_t nanoseconds;
};
 
struct Attributes {
    Attributes() = default;
    explicit Attributes(const struct stat& attributes);
 
    struct stat ToStat() const;
 
    /*! Device */
    std::uint64_t device;
    /*! Device number, if device */
    std::uint64_t deviceNumber;
    /*! Inode number */
    std::uint64_t inode;
    /*! File mode */
    std::uint32_t mode;
    /*! Number of links */
    std::uint64_t linkCount;
    /*! User ID of the file's owner */
    std::uint32_t uid;
    /*! Group ID of the file's group */
    std::uint32_t gid;
    /*! Size of file, in bytes */
    std::int64_t size;
    /*! Optimal block size for I/O */
    std::int64_t blockSize;
    /*! Amount of 512-byte blocks allocated */
    std::int64_t blocks;
    /*! Time of last access */
    TimePoint accessTime;
    /*! Time of last modification */
    TimePoint modificationTime;
    /*! Time of last status change */
    TimePoint statusChangeTime;
};
 
struct FuseEntryParams {
    explicit FuseEntryParams(const fuse_entry_param& fuseEntryParams);
 
    fuse_entry_param ToParams() const;
 
    std::uint64_t generation;
    double attributesTimeout;
    double entryTimeout;
    Attributes attributes;
};

struct UserGroup {
    std::uint32_t uid;
    std::uint32_t gid;
    int pid;
    std::uint32_t umask;

    UserGroup(std::uint32_t u, std::uint32_t g, int p, std::uint32_t m)
        : uid(u), gid(g), pid(p), umask(m)
    {
    }
    
    UserGroup(const struct fuse_ctx *ctx)
    {
        uid = ctx->uid;
        gid = ctx->gid;
        pid = ctx->pid;
        umask = ctx->umask;
    }
};

using AuxDataSize = std::uint16_t;

#endif
