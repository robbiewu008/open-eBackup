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
#ifndef LIBNFS_CONSTANTS_H
#define LIBNFS_CONSTANTS_H

constexpr uint16_t READ_NFS_CONTEXT_CNT = 1;
constexpr uint16_t WRITE_NFS_CONTEXT_CNT = 1;
constexpr uint16_t DELETE_NFS_CONTEXT_CNT = 1;
constexpr uint16_t DIR_MTIME_NFS_CONTEXT_CNT = 10;
constexpr uint16_t MKDIR_NFS_CONTEXT_CNT = 1;
constexpr uint16_t LSTAT_NFS_CONTEXT_CNT = 1;
constexpr uint16_t SERVER_CHECK_NFS_CONTEXT_CNT = 1;

constexpr uint16_t REQ_CNT_PER_NFS_CONTEXT = 32;

constexpr uint32_t RATELIMIT_MAX_PENDING_REQ_CNT = 32;
constexpr uint32_t RATELIMIT_MAX_PENDING_REQ_CNT_75_PERCENT = 24;
constexpr uint32_t RATELIMIT_MIN_PENDING_REQ_CNT = 4;
constexpr uint32_t RATELIMIT_MIN_PENDING_REQ_CNT_75_PERCENT = 3;

constexpr uint16_t NUMBER_ZERO = 0;
constexpr uint16_t NUMBER_ONE = 1;
constexpr uint16_t NUMBER_TWO = 2;
constexpr uint16_t NUMBER_THREE = 3;
constexpr uint16_t NUMBER_EIGHT = 8;
constexpr uint16_t NUMBER256 = 256;

const std::string SEP = "/";
const std::string NFS_URL = "nfs://";

const std::string LIBNFS_READER = "Reader";
const std::string LIBNFS_WRITER = "Writer";

constexpr uint8_t BACKUP_ERR_ENOENT = 2;
constexpr uint8_t BACKUP_ERR_EEXIST = 17;
constexpr uint8_t BACKUP_ERR_NOTDIR = 20;
constexpr uint8_t BACKUP_ERR_EISDIR = 21;
constexpr uint8_t BACKUP_ERR_INVALID = 22;
constexpr uint8_t BACKUP_ERR_STALE = 70;
constexpr uint8_t BACKUP_ERR_NOTEMPTY = 39;

constexpr uint32_t DIRECTORY_SIZE = 4 * 1024; /* dirctory size 4KB */

constexpr uint8_t ATIME_MTIME_ARRAY_LEN = 2;
constexpr uint8_t ATIME_STRUCT = 0;
constexpr uint8_t MTIME_STRUCT = 1;

constexpr uint8_t MAX_BLOCKS_BEFORE_COMMIT = 40;
constexpr uint16_t DEFAULT_REQUEST_RETRY_TIMER = 3000;

constexpr int POLL_EXPIRE_TIMEOUT = 10;

constexpr uint32_t RATELIMIT_TIMER_INTERVAL = 120;
#endif // LIBNFS_CONSTANTS_H