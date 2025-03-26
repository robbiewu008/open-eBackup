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
#ifndef MODULE_ADSPARSER_FILE_H
#define MODULE_ADSPARSER_FILE_H
#include <vector>
#include "FileParser.h"
#include "define/Defines.h"
#include "ParserStructs.h"

namespace Module {

const std::string ADS_HEADER_FILE = "Host Scanner AdsParser File";
const std::string ADS_HEADER_VERSION_V10 = "1.0";

enum ADS_HEADER_INFO {
    ADS_TITLE = 0,
    ADS_HEADER_VERSION = 1,
    ADS_TIMESTAMP = 2,
    ADS_TASKID = 3,
    ADS_BACKUP_TYPE = 4,
    ADS_FILE_COUNT = 5,
    ADS_STREAM_COUNT = 6,
    ADS_DATA_SIZE = 7
};

class AGENT_API AdsParser : public FileParser {

public:
    struct Params {
        uint32_t maxEntriesPerFile = 0; // max number of entries that can be written in this file.
        uint64_t maxDataSize = 0; // max size that can be written in this file.
        std::string m_fileName {};  // control file title.
        std::string taskId {};
        std::string backupType {};  // Full or incremental.
    };

    struct Header {
        std::string title;
        std::string version;
        std::string timestamp;
        std::string taskId;
        std::string backupType;
        uint64_t    fileCount;
        uint64_t    streamCount;
        uint64_t    dataSize;
    };

    explicit AdsParser(const AdsParser::Params& params);

    explicit AdsParser(const std::string& fileName);

    ~AdsParser();

    uint16_t WriteAdsEntry(AdsStruct& adsEntry);

    uint64_t GetCurrentOffset();

    size_t ReadAdsEntry(uint64_t offset, uint8_t* buffer, uint32_t& nextIndex, uint64_t& nextOffset, bool& isLast);

private:
    AdsParser::Header m_header {};            /* File header info */
    uint64_t m_offset = 0;                      /* AdsParser write offset */
    uint64_t m_maxFileSize = 0;                 /* Max File size */
    uint64_t m_fileCount = 0;                   /* File count */
    uint64_t m_maxEntriesPerFile = 0;           /* Max entries per file */
    uint64_t m_maxDataSize = 0;                 /* Max data size */

    char* m_readCache = nullptr;                /* Read cache (read from file in disk and cache it here) */
    uint64_t m_readCacheOffsetStart = 0;        /* Read cache - starting offset in file */
    uint64_t m_readCacheOffsetEnd = 0;          /* Read cache - end offset in file */
    bool m_isCacheEnable = false;              /* Enable/disable cache read */


    CTRL_FILE_RETCODE OpenWrite() override;
    CTRL_FILE_RETCODE CloseWrite() override;
    CTRL_FILE_RETCODE ReadHeader() override;
    CTRL_FILE_RETCODE WriteHeader() override;
    CTRL_FILE_RETCODE ValidateHeader() override;
    CTRL_FILE_RETCODE FillHeader(uint32_t headerLine, std::vector<std::string> &cltHeaderLineSplit, std::string &cltHeaderLine);
    CTRL_FILE_RETCODE FreeReadCache();
    CTRL_FILE_RETCODE ValidateMetaFile(uint64_t offset);
    std::string GetFileName();

    // Get the line to write in header info of file
    std::string GetFileHeaderLine(uint32_t headerLine);

    // Get the file header info read from file
    CTRL_FILE_RETCODE GetHeader(AdsParser::Header &header);

    // Write buffer data into file.
    CTRL_FILE_RETCODE FlushToFile();

    // Read the file header and retry if its failed
    CTRL_FILE_RETCODE ReadMetaFileHeader();

    void TranslateToLittleEndian(AdsStruct& adsEntry);
    void TranslateToHostEndian(AdsStruct& adsEntry);

    // Read ads entry from buffer and fill AdsStruct object
    CTRL_FILE_RETCODE ReadadsEntryFromBuffer(AdsStruct &adsEntry);

    bool ReadCurrentStreamLengthFromBinaryFile(uint64_t offset, uint32_t readLen);
    bool ValidateAdsEntryOfferset(uint64_t offset);
    bool ReadadsEntryOffsetFromBinaryFile(uint64_t offset);
    bool ValidateCurrentStreamOffset(AdsStruct adsEntry);
};

}

#endif