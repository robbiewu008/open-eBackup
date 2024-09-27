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
#ifndef MODULE_XMETA_FILE_H
#define MODULE_XMETA_FILE_H

#include <vector>
#include "ParserStructs.h"
#include "FileParser.h"
#include "define/Defines.h"

namespace Module {

const std::string XMETA_HEADER_TITLE_FS = "FS Scanner Backup XMeta File";
const std::string XMETA_HEADER_VERSION_V10 = "1.0";

enum XMETA_HEADER_INFO {
    XMETA_TITLE = 0,
    XMETA_HEADER_VERSION = 1,
    XMETA_TIMESTAMP = 2,
    XMETA_TASKID = 3,
    XMETA_TASKTYPE = 4,
    XMETA_NASSERVER = 5,
    XMETA_NASSHARE = 6,
    XMETA_PROTOCOL = 7,
    XMETA_PROTOCOL_VERSION = 8,
    XMETA_METADATA_SCOPE = 9,
    XMETA_RESERVED_1 = 10,
    XMETA_RESERVED_2 = 11,
    XMETA_RESERVED_3 = 12,
    XMETA_RESERVED_4 = 13,
    XMETA_RESERVED_5 = 14,
};

// class will be similar to control file class.
class AGENT_API XMetaParser : public FileParser {
public:
    struct Params {
        std::string m_fileName {};       /* Control file title */
        uint32_t maxEntriesPerFile = 0;     /* Max number of entries that can be written in this file */
        uint64_t maxDataSize = 0;           /* Max data size that can be written in this file */
        std::string taskId {};              /* Task id */
        std::string backupType {};          /* Full or incremental */
        std::string nasServer {};           /* Scanned nas server ip address */
        std::string nasSharePath {};        /* Scanned nas share path */
        std::string proto {};               /* Protocol used - nfs or cifs */
        std::string protoVersion {};       /* Protocol version */
        std::string metaDataScope {};       /* Metadata scope - folder-only or folder-and-file */
    };

    struct Header {
        std::string title;                  /* Control file title */
        std::string version;                /* Version */
        std::string timestamp;              /* Timestamp - when the file is created */
        std::string taskId;                 /* Task id */
        std::string backupType;             /* Full or incremental */
        std::string nasServer;              /* Scanned nas server ip address */
        std::string nasSharePath;           /* Scanned nas share path */
        std::string proto;                  /* Protocol used - nfs or cifs */
        std::string protoVersion;          /* Protocol version */
        std::string metaDataScope;          /* Metadata scope - folder-only or folder-and-file */
    };
    /**
    * Contructor to be used by (producers) users for writing to the meta file
    */
    explicit XMetaParser(const XMetaParser::Params& params);

    /**
    * Contructor to be used by (consumers) users for reading from the meta file
    */
    explicit XMetaParser(std::string metaFileName, bool isCacheEnable = false);

    /**
    *  Destructor
    */
    ~XMetaParser();

    /**
    * Write xmeta list which will write to buffer and return current offset.
    */
    uint64_t WriteXMeta(std::vector<XMetaField> &entry);

    /**
    * Read xmeta entry for a given offset.
    */
    CTRL_FILE_RETCODE ReadXMeta(std::vector<XMetaField> &entry, uint64_t offset);

    /**
    * This will return current offset.
    */
    uint64_t GetCurrentOffset();

    /**
    * Write buffer data into file.
    */
    CTRL_FILE_RETCODE FlushToFile() override;

    /**
    * Get XMetafile name
    */
    std::string GetFileName();

private:
    XMetaParser::Header m_header {};            /* File header info */
    uint64_t m_offset = 0;                      /* XMetaParser write offset */
    uint64_t m_maxFileSize = 0;                 /* Max File size */

    char* m_readCache = nullptr;                /* Read cache (read from file in disk and cache it here) */
    uint64_t m_readCacheOffsetStart = 0;        /* Read cache - starting offset in file */
    uint64_t m_readCacheOffsetEnd = 0;          /* Read cache - end offset in file */
    bool m_isCacheEnable = false;              /* Enable/disable cache read */

    CTRL_FILE_RETCODE OpenWrite() override;
    CTRL_FILE_RETCODE CloseWrite() override;
    CTRL_FILE_RETCODE ReadHeader() override;
    CTRL_FILE_RETCODE WriteHeader() override;
    CTRL_FILE_RETCODE ValidateHeader() override;

    CTRL_FILE_RETCODE AllocReadCache();
    CTRL_FILE_RETCODE FreeReadCache();
    CTRL_FILE_RETCODE RefreshReadCache(uint64_t offset);
    CTRL_FILE_RETCODE ReadData(uint64_t &offset, uint32_t &readLen, uint32_t &numOfTlvs, uint32_t &lenOfAllTlvs);
    CTRL_FILE_RETCODE FillReadCache(uint64_t offset, uint32_t readLen);
    CTRL_FILE_RETCODE ReAllocReadCache(uint32_t len);
    CTRL_FILE_RETCODE ReadDataWithoutCache(uint64_t offset, std::vector<XMetaField> &entry);
    CTRL_FILE_RETCODE ReadBuffer(uint32_t numOfTlvs, uint32_t lenOfAllTlvs, std::vector<XMetaField> &entry);
    CTRL_FILE_RETCODE ReadXMetaFieldFromBuffer(uint64_t offset, XMetaField &field, uint32_t &tlvLen);
    CTRL_FILE_RETCODE FillTlvsDetails(uint32_t &numOfTlvs, uint32_t &lenOfAllTlvs);

    /**
    * Read the file header and retry if its failed
    */
    CTRL_FILE_RETCODE ReadMetaFileHeader();

    CTRL_FILE_RETCODE FillHeader(uint32_t &headerLine, std::vector<std::string> &cltHeaderLineSplit,
        std::string &cltHeaderLine);

    /**
    * Get the line to write in header info of file
    */
    std::string GetFileHeaderLine(uint32_t headerLine);

    /**
    * Get the file header info read from file
    */
    CTRL_FILE_RETCODE GetHeader(XMetaParser::Header &header);

    /**
    * Read xmeta entry from buffer and fill xmeta object
    */
    CTRL_FILE_RETCODE ReadXMetaEntryFromBuffer(std::vector<XMetaField> &entry, uint64_t offset);

    CTRL_FILE_RETCODE ReadXMetaSize(uint64_t offset, uint32_t &numOfTlvs, uint32_t &lenOfAllTlvs);

    CTRL_FILE_RETCODE ReadXMetaFieldFromReadCache(uint64_t offset, XMetaField &tlv, uint32_t &tlvLen);

    /**
    * Read metaobj from file fd.
    */
    CTRL_FILE_RETCODE ReadLenFromFile(uint32_t len);

    /**
    * Read following xmeta entry.
    */
    CTRL_FILE_RETCODE ReadXMeta(std::vector<XMetaField> &entry);

    /**
    * Validate file offsets
    */
    CTRL_FILE_RETCODE ValidateMetaFile(uint64_t offset);

    void PrintEntries(std::vector<XMetaField>& entry);
};

}
#endif // SCANNER_XMETA_FILE_H