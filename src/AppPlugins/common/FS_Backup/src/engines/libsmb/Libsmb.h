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
#ifndef LIBSMB_H
#define LIBSMB_H

#include <cstdint>
#include <cerrno>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include "Backup.h"
#include "libsmb_ctx/SmbContextWrapper.h"

constexpr auto POLL_MAX_TIMEOUT = 500;
constexpr int MAX_SMALL_FILE_SIZE = 4 * 1024 * 1024;
constexpr uint32_t SERVER_CHECK_INTERVAL = 30;

constexpr int MAX_OPENED_FILEHANDLE_COUNT = 100;
constexpr int MAX_PENDING_REQUEST_COUNT = 200;

enum class LibsmbEvent {
    OPEN_SRC_AND_DST = 0,
    OPEN_SRC,
    OPEN_DST,
    READ,
    STAT_SRC,
    STAT_DST,
    ADS,
    WRITE,
    MKDIR,
    SET_SD,
    SET_BASIC_INFO,
    SET_BASIC_INFO_DIR,
    CLOSE_SRC,
    CLOSE_DST,
    LINK,
    UNLINK,
    DELETE,
    RESET_ATTR,
    REPLACE_DIR,
    INVALID
};

struct LibsmbParams {
    std::string srcRootPath;
    std::string dstRootPath;
    std::string linkTarget;
    BackupType           backupType { BackupType::UNKNOWN_TYPE };
    BackupDataFormat     backupDataFormat { BackupDataFormat::UNKNOWN_FORMAT };
    RestoreReplacePolicy restoreReplacePolicy { RestoreReplacePolicy::NONE };
    bool writeMeta { true };
    uint32_t blockSize { DEFAULT_BLOCK_SIZE };

    Module::SmbContextArgs srcSmbContextArgs;
    Module::SmbContextArgs dstSmbContextArgs;
};

#endif // LIBSMB_H
