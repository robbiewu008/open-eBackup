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
#include "afs/FileSystemFactory.h"
#include "afs/Ext4FS.h"
#include "afs/NtfsFS.h"
#include "afs/XfsFS.h"
#include "afs/LogMsg.h"

/**
 * @brief 根据文件系统类型创建对应的handler
 * @param fstype 文件系统类型
 * @return  相应文件系统类型的对象，失败返回NULL
 *
 */
filesystemHandler *filesystemFactory::createObject(int32_t fstype)
{
    AFS_TRACE_OUT_DBG("ENTER filesystemFactory::createObject()");
    switch (fstype) {
        case AFS_FILESYSTEM_EXT4:
            AFS_TRACE_OUT_INFO("Create EXT4 file system object.");
            registerClass("ext4", ext4Handler::CreateObject);
            return static_cast<ext4Handler *>(createObjectByName("ext4"));
        case AFS_FILESYSTEM_NTFS:
            AFS_TRACE_OUT_INFO("Create NTFS file system object.");
            registerClass("ntfs", ntfsHandler::CreateObject);
            return static_cast<ntfsHandler *>(createObjectByName("ntfs"));
        case AFS_FILESYSTEM_XFS:
            AFS_TRACE_OUT_INFO("Create XFS file system object.");
            registerClass("xfs", xfsHandler::CreateObject);
            return static_cast<xfsHandler *>(createObjectByName("xfs"));
        default:
            AFS_TRACE_OUT_ERROR("Cannot recognize file system type. FSType=%d", fstype);
            break;
    }

    return NULL;
}
