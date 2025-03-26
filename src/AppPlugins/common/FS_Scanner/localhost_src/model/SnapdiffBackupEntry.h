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
#ifndef SNAPDIFF_BACKUP_ENTRY_H
#define SNAPDIFF_BACKUP_ENTRY_H


#include "ControlDevice.h"

/* interface for Snapdiff Rest API operation */

typedef std::map<std::string, std::list<Module::SnapdiffMetadataInfo>> SnapdiffResultMap;

// enum value of SnapdiffBackupEntry.m_changeType
enum SNAPDIFF_BACKUPENTRY_CHANGETYPE {
    META = 0, // only meta changed
    NEW = 1, // file or dir created
    DELETE = 2, // file or dir deleted
};

// enum value of SnapdiffBackupEntry.m_mode
enum SNAPDIFF_BACKUPENTRY_MODE {
    DIR_DELETE = 16895, // 16895 corresponds to octal value 40777
    FILE_DELETE = 33279, // 33279 corresponds to octal value 100777
};

// enum value of SnapdiffBackupEntry.m_flags
enum SNAPDIFF_BACKUPENTRY_FLAG {
    FILE_DELETED = 4
};

// enum value of SnapdiffMetaDataInfo.fType
enum SNAPDIFF_METADATA_INFO_F_TYPE {
    FILE_TYPE = 1,
    DIR_TYPE = 2
};

struct SnapdiffBackupEntry {
    std::string m_path {};                          /* absolute path of the directory or file */
    std::string m_ctrlMode {};
    SNAPDIFF_BACKUPENTRY_FLAG m_flags {};           /* refer NAS_BACKUP_ENTRY_F_XXXX */
    SNAPDIFF_BACKUPENTRY_MODE m_mode {};          /* same as posix stat mode_t mode */
    uint32_t m_attr = 0;
    uint32_t m_nlink = 0;                           /* same as posix stat nlink */
    uint32_t m_uid = 0;                             /* same as posix stat uid */
    uint32_t m_gid = 0;                             /* same as posix stat gid */
    uint64_t m_size = 0;                            /* same as posix stat size */
    uint64_t m_mtime = 0;                           /* same as posix stat mtime (sec) */
    uint64_t m_ctime = 0;
    uint64_t m_btime = 0;
    uint64_t m_atime = 0;
    uint64_t m_inode = 0;
    SNAPDIFF_BACKUPENTRY_CHANGETYPE m_changeType {};
    SNAPDIFF_METADATA_INFO_F_TYPE m_fType {};
    std::string m_aclText {};                       /* extended acls (refer NAS_BACKUP_ENTRY_F_EXT_ACL) */
};

#endif  // SNAPDIFF_BACKUP_ENTRY_H
