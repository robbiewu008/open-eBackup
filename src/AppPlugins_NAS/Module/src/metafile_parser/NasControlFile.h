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
#ifndef MODULE_CONTROL_FILE_H
#define MODULE_CONTROL_FILE_H

#include <map>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>
#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>
#include "log/Log.h"

#define WRITE_TO_AGGR_FILE(mWriteBuffer, mWriteFd, mIsBinaryFile, mResult) do { \
    (mResult) = NAS_CTRL_FILE_RET_SUCCESS; \
    (mWriteFd) << (mWriteBuffer).str(); \
    (mWriteFd).seekp(0, ios::end); \
    if (!(mWriteFd).good()) { \
        (mResult) = NAS_CTRL_FILE_RET_FAILED; \
    } \
} while (0)

#define READ_FROM_AGGR_FILE(mReadBuffer, mReadFd, mResult) do { \
    (mResult) = NAS_CTRL_FILE_RET_SUCCESS; \
    (mReadBuffer) << (mReadFd).rdbuf(); \
    if (!(mReadBuffer).good()) { \
        (mResult) = NAS_CTRL_FILE_RET_FAILED; \
    } \
} while (0)

#define WRITE_TO_FILE(mWriteBuffer, mWriteFd, mIsBinaryFile, mResult) do { \
    (mResult) = NAS_CTRL_FILE_RET_SUCCESS; \
    (mWriteFd) << (mWriteBuffer).str(); \
    (mWriteFd).seekp(0, ios::end); \
    if (!(mWriteFd).good()) { \
        (mWriteFd).close(); \
        std::ios::openmode mFileOpenMode = std::ios::out | std::ios::app; \
        if (mIsBinaryFile) { \
            mFileOpenMode |= std::ios::binary; \
        } \
        (mResult) = FileOpen<std::ofstream>((mWriteFd), mFileOpenMode); \
        if ((mResult) == NAS_CTRL_FILE_RET_SUCCESS) { \
            (mWriteFd) << (mWriteBuffer).str(); \
            (mWriteFd).seekp(0, ios::end); \
            if (!(mWriteFd).good()) { \
                (mWriteFd).close(); \
                (mResult) = NAS_CTRL_FILE_RET_FAILED; \
            } \
        } \
    } \
} while (0)

#define READ_FROM_FILE(mReadBuffer, mReadFd, mResult) do { \
    (mResult) = NAS_CTRL_FILE_RET_SUCCESS; \
    (mReadBuffer) << (mReadFd).rdbuf(); \
    if (!(mReadBuffer).good()) { \
        (mReadFd).close(); \
        (mReadBuffer).clear ((mReadBuffer).goodbit); \
        (mReadBuffer).str(""); \
        (mResult) = FileOpen<std::ifstream>(mReadFd, std::ios::in); \
        if ((mResult) == NAS_CTRL_FILE_RET_SUCCESS) { \
            (mReadBuffer) << (mReadFd).rdbuf(); \
            if (!(mReadBuffer).good()) { \
                (mReadFd).close(); \
                (mReadBuffer).str(""); \
                (mResult) = NAS_CTRL_FILE_RET_FAILED; \
            } \
        } \
    } \
} while (0)

#define READ_FROM_BINARY_FILE(mReadBuffer, mOffset, mReadLen, mReadFd, mResult) do { \
    (mReadFd).read((mReadBuffer), mReadLen); \
    (mReadFd).sync(); \
    if ((!(mReadFd).eof()) && (!(mReadFd).good())) { \
        (mReadFd).close(); \
        (mResult) = FileOpen<std::ifstream>(mReadFd, std::ios::in | std::ios::binary); \
        if ((mResult) == NAS_CTRL_FILE_RET_SUCCESS) { \
            if ((mOffset) != 0) { \
                (mReadFd).seekg((mOffset)); \
                if ((mReadFd).fail()) { \
                    (mResult) = NAS_CTRL_FILE_RET_FAILED; \
                } \
            } \
            if ((mResult) == NAS_CTRL_FILE_RET_SUCCESS) { \
                (mReadFd).read((mReadBuffer), mReadLen); \
                (mReadFd).sync(); \
                if ((!(mReadFd).eof()) && (!(mReadFd).good())) { \
                    (mReadFd).close(); \
                    (mResult) = NAS_CTRL_FILE_RET_FAILED; \
                } \
            } \
        } \
    } \
} while (0)

#define MODULE_SCAN_HASH_COMPARE

constexpr auto CTL_MOD_NAME = "NasControlFile";
constexpr auto INDEX_MOD = "NasIndexFile";
constexpr auto MAP_MOD = "NasFileMap";

constexpr uint8_t NAS_INDEX_DIR_ENTRY_SIZE = 5;
constexpr uint8_t NAS_INDEX_ARCHIVE_ENTRY_SIZE = 1;
constexpr uint8_t NAS_INDEX_FILE_ENTRY_SIZE = 4;

constexpr uint8_t NAS_FILE_MAP_ENTRY_SIZE = 2;

constexpr uint32_t NAS_CTRL_FILE_MAX_SIZE = (4 * 1024 * 1024);      /* 4MB */
constexpr uint16_t NAS_CTRL_MILLI_SEC = 1000;
constexpr uint16_t NAS_CTRL_MAX_COUNT = 1000;
constexpr uint16_t NAS_CTRL_READ_BUFFER_SIZE = 1000;
constexpr uint8_t NAS_CTRL_FILE_SERVER_RETRY_CNT = 5;
constexpr uint16_t NAS_CTRL_FILE_SERVER_RETRY_INTERVAL = 30000;     /* in milli seconds */

constexpr uint8_t NAS_CTRL_FILE_NUMBER_ZERO = 0;
constexpr uint8_t NAS_CTRL_FILE_NUMBER_ONE = 1;
constexpr uint8_t NAS_CTRL_FILE_NUMBER_TWO = 2;
constexpr uint8_t NAS_CTRL_FILE_NUMBER_THREE = 3;
constexpr uint8_t NAS_CTRL_FILE_NUMBER_FOUR = 4;
constexpr uint8_t NAS_CTRL_FILE_NUMBER_FIVE = 5;
constexpr uint8_t NAS_CTRL_FILE_NUMBER_SEVEN = 7;
constexpr uint8_t NAS_CTRL_FILE_NUMBER_EIGHT = 8;
constexpr uint8_t NAS_CTRL_FILE_NUMBER_NINE = 9;
constexpr uint8_t NAS_CTRL_FILE_NUMBER_TEN = 10;
constexpr uint8_t NAS_CTRL_FILE_NUMBER_ELEVEN = 11;
constexpr uint8_t NAS_CTRL_FILE_NUMBER_THIRTEEN = 13;

constexpr uint8_t NAS_CTRL_FILE_OFFSET_0 = 0;
constexpr uint8_t NAS_CTRL_FILE_OFFSET_1 = 1;
constexpr uint8_t NAS_CTRL_FILE_OFFSET_2 = 2;
constexpr uint8_t NAS_CTRL_FILE_OFFSET_3 = 3;
constexpr uint8_t NAS_CTRL_FILE_OFFSET_4 = 4;
constexpr uint8_t NAS_CTRL_FILE_OFFSET_5 = 5;
constexpr uint8_t NAS_CTRL_FILE_OFFSET_6 = 6;
constexpr uint8_t NAS_CTRL_FILE_OFFSET_7 = 7;
constexpr uint8_t NAS_CTRL_FILE_OFFSET_8 = 8;
constexpr uint8_t NAS_CTRL_FILE_OFFSET_9 = 9;
constexpr uint8_t NAS_CTRL_FILE_OFFSET_10 = 10;
constexpr uint8_t NAS_CTRL_FILE_OFFSET_11 = 11;

constexpr bool NAS_CTRL_BINARY_FILE = true;

constexpr uint16_t NAS_SERVER_CHECK_DELAY_INTERVAL = 30000;         /* in milli seconds */
constexpr uint16_t NUMBER_10 = 10;
constexpr uint16_t NUMBER_3 = 3;
constexpr uint16_t ERROR_MSG_SIZE = 256;

enum NAS_INDEX_FILE_OPEN_MODE {
    NAS_INDEX_FILE_OPEN_MODE_READ = 0,
    NAS_INDEX_FILE_OPEN_MODE_WRITE = 1,
    NAS_INDEX_FILE_OPEN_MODE_UPDATE = 2
};

enum NAS_INDEX_FILE_RETCODE {
    NAS_INDEX_FILE_RET_FAILED = -1,
    NAS_INDEX_FILE_RET_SUCCESS = 0,
    NAS_INDEX_FILE_RET_MAX_LIMIT_REACHED = 1,
    NAS_INDEX_FILE_RET_READ_EOF = 2,
    NAS_INDEX_FILE_RET_READ_NEXT_SECTION = 3,
};

enum NAS_FILEMAP_OPEN_MODE {
    NAS_FILEMAP_OPEN_MODE_READ = 0,
    NAS_FILEMAP_OPEN_MODE_WRITE = 1,
    NAS_FILEMAP_OPEN_MODE_UPDATE = 2
};

enum NAS_FILEMAP_RETCODE {
    NAS_FILEMAP_RET_FAILED = -1,
    NAS_FILEMAP_RET_SUCCESS = 0,
    NAS_FILEMAP_RET_MAX_LIMIT_REACHED = 1,
    NAS_FILEMAP_RET_READ_EOF = 2
};

enum NAS_CTRL_FILE_OPEN_MODE {
    NAS_CTRL_FILE_OPEN_MODE_READ = 0,
    NAS_CTRL_FILE_OPEN_MODE_WRITE = 1
};

enum NAS_CTRL_FILE_RETCODE {
    NAS_CTRL_FILE_RET_FAILED = -1,
    NAS_CTRL_FILE_RET_SUCCESS = 0,
    NAS_CTRL_FILE_RET_MAX_LIMIT_REACHED = 1,
    NAS_CTRL_FILE_RET_READ_EOF = 2,
};

time_t GetCurrentTimeInSeconds();
std::string GetParentDirOfFile(std::string filePath);
NAS_CTRL_FILE_RETCODE CheckParentDirIsReachable(std::string dirPath);
uint32_t GetCommaCountOfString(const std::string &strName);
std::string ConstructStringName(uint32_t &offset, uint32_t &totCommaCnt, std::vector<std::string> &lineContents);

#endif //DME_NAS_CONTROL_FILE_H