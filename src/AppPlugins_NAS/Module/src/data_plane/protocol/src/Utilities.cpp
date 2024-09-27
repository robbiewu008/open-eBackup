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
#include <protocol/Utilities.h>

Attributes::Attributes(const struct stat &attributes)
    : device(attributes.st_dev), deviceNumber(attributes.st_rdev), inode(attributes.st_ino), mode(attributes.st_mode),
      linkCount(attributes.st_nlink), uid(attributes.st_uid), gid(attributes.st_gid), size(attributes.st_size),
      blockSize(attributes.st_blksize), blocks(attributes.st_blocks),
      accessTime({attributes.st_atim.tv_sec, attributes.st_atim.tv_nsec}),
      modificationTime({attributes.st_mtim.tv_sec, attributes.st_mtim.tv_nsec}),
      statusChangeTime({attributes.st_ctim.tv_sec, attributes.st_ctim.tv_nsec})
{}

struct stat Attributes::ToStat() const
{
    struct stat attributes {};

    attributes.st_dev = device;
    attributes.st_rdev = deviceNumber;
    attributes.st_ino = inode;
    attributes.st_mode = mode;
    attributes.st_nlink = linkCount;
    attributes.st_uid = uid;
    attributes.st_gid = gid;
    attributes.st_size = size;
    attributes.st_blksize = blockSize;
    attributes.st_blocks = blocks;
    attributes.st_atim.tv_sec = accessTime.seconds;
    attributes.st_atim.tv_nsec = accessTime.nanoseconds;
    attributes.st_mtim.tv_sec = modificationTime.seconds;
    attributes.st_mtim.tv_nsec = modificationTime.nanoseconds;
    attributes.st_ctim.tv_sec = statusChangeTime.seconds;
    attributes.st_ctim.tv_nsec = statusChangeTime.nanoseconds;

    return attributes;
}

FuseEntryParams::FuseEntryParams(const fuse_entry_param &fuseEntryParams)
    : generation(fuseEntryParams.generation), attributesTimeout(fuseEntryParams.attr_timeout),
      entryTimeout(fuseEntryParams.entry_timeout), attributes(Attributes(fuseEntryParams.attr))
{}

fuse_entry_param FuseEntryParams::ToParams() const
{
    fuse_entry_param params{};

    params.ino = attributes.inode;
    params.generation = generation;
    params.attr_timeout = attributesTimeout;
    params.entry_timeout = entryTimeout;
    params.attr = attributes.ToStat();

    return params;
}
