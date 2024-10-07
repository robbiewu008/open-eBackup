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
#ifndef BACKUP_CONSTANTS_H
#define BACKUP_CONSTANTS_H

const uint64_t NUM1024 = 1024;
const uint64_t KB_SIZE = NUM1024;
const uint64_t MB_SIZE = NUM1024 * KB_SIZE;
const uint64_t GB_SIZE = NUM1024 * MB_SIZE;
const uint64_t TB_SIZE = NUM1024 * GB_SIZE;
const uint64_t PB_SIZE = NUM1024 * TB_SIZE;

static const int DEFAULT_QUEUE_LENGTH = 3200;
static const int DEFAULT_QUEUE_SIZE = (50 * MB_SIZE);
static const int DEFAULT_ERROR_FILE_CNT = (100);
static const int DEFAULT_ERROR_SINGLE_FILE_CNT = (3);
static const uint32_t DEFAULT_BLOCK_SIZE = (4 * MB_SIZE);
static const uint32_t DEFAULT_SYNC_IO_THREAD_NUM = 32;

static const int DEFAULT_MAX_READ_RETRY = 30;
static const int DEFAULT_MAX_REQUEST_RETRY = 30;
static const int DEFAULT_MAX_RETRY_TIMEOUT = 3000;

constexpr uint32_t DEFAULT_ABORT_SLEEP  = 3;
constexpr uint32_t DEFAULT_MAX_NOSPACE = 100;
constexpr uint32_t DEFAULT_MAX_NOACCESS = 100;

constexpr uint32_t DEFAULT_MAX_REQ_COUNT = 128;
constexpr uint32_t DEFAULT_MIN_REQ_COUNT = 96;
constexpr uint32_t DEFAULT_MAX_READ_COUNT = 32;
constexpr uint32_t DEFAULT_MIN_READ_COUNT = 24;
constexpr uint32_t DEFAULT_MAX_WRITE_COUNT = 32;
constexpr uint32_t DEFAULT_MIN_WRITE_COUNT = 24;

constexpr uint32_t DEFAULT_MAX_FAILURE_RECORDS_NUM = 100000; /* max consume 20MB per subtask (if all failed) */
constexpr uint32_t DEFAULT_MAX_FAILURE_RECORDS_BUFFER_SIZE = 100;

constexpr uint32_t DEFAULT_SERVER_CHECK_COUNT = 100;
constexpr uint32_t DEFAULT_SERVER_CHECK_SLEEP = 30000;
constexpr uint32_t DEFAULT_SERVER_CHECK_RETRY = 3;

constexpr uint32_t DEFAULT_FAILURE_SLEEP = 1000;

static const int  DEFAULT_MAX_AGGREGATE_FILE_SIZE = (32 * MB_SIZE);
static const int  DEFAULT_MAX_FILE_SIZE_TO_AGGREGATE = (1 * MB_SIZE);

static const uint32_t MEMORY_THRESHOLD_HIGH = (100 * MB_SIZE);
static const uint32_t MEMORY_THRESHOLD_LOW = (60 * MB_SIZE);

const std::string META_VERSION_V10 = "1.0";
const std::string META_VERSION_V20 = "2.0";

constexpr uint32_t COMPLETION_CHECK_INTERVAL = 120;

const int SYMLINK_BLOCK_SIZE = 8192;

/* Backup error code */
static const uint32_t E_BACKUP_READ_LESS_THAN_EXPECTED = 1577209861;
static const uint32_t E_BACKUP_FILE_OR_DIR_NOT_EXIST = 1577209862;

constexpr uint32_t FILE_IS_ADS_FILE = 0210000; // regular file S_IFREG values ((mode_t) 0100000)
constexpr uint32_t FILE_HAVE_ADS    = 0220000;

constexpr uint64_t DEFAULT_BACKUP_QUEUE_SIZE = 10000;
constexpr uint64_t DEFAULT_BACKUP_QUEUE_MEMORY_SIZE = 400 * 1024 * 1024;

#endif // BACKUP_CONSTANTS_H