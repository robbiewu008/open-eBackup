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
#ifndef MODULE_FCACHE_FILE_H
#define MODULE_FCACHE_FILE_H
#include <queue>
#include "ParserStructs.h"
#include "FileParser.h"
#include "define/Defines.h"

namespace Module {

const std::string FCACHE_HEADER_TITLE = "NAS Scanner Fcache File";
const std::string FCACHE_HEADER_VERSION_V10 = "1.0";
const std::string FCACHE_HEADER_VERSION_V20 = "2.0";
const std::string FCACHE_HEADER_VERSION_V30 = "3.0";

enum FCACHE_HEADER_INFO {
    FCACHE_TITLE = 0,
    FCACHE_HEADER_VERSION = 1,
    FCACHE_TIMESTAMP = 2,
    FCACHE_TASKID = 3,
    FCACHE_TASKTYPE = 4,
    FCACHE_NASSERVER = 5,
    FCACHE_NASSHARE = 6,
    FCACHE_PROTOCOL = 7,
    FCACHE_PROTOCOL_VERSION = 8,
    FCACHE_METADATA_SCOPE = 9,
    FCACHE_RESERVED_1 = 10,
    FCACHE_RESERVED_2 = 11,
    FCACHE_RESERVED_3 = 12,
    FCACHE_RESERVED_4 = 13,
    FCACHE_RESERVED_5 = 14,
};

// class will be similar to control file class.
class AGENT_API FileCacheParser : public FileParser {
public:
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

    struct Params {
        std::string fileName {};          /* Control file title */
        uint64_t readBufferSize = 0;           /* Max data size that can be written in this file */
        std::string taskId {};              /* Task id */
        std::string backupType {};          /* Full or incremental */
        std::string nasServer {};           /* Scanned nas server ip address */
        std::string nasSharePath {};        /* Scanned nas share path */
        std::string proto {};               /* Protocol used - nfs or cifs */
        std::string protoVersion {};       /* Protocol version */
        std::string metaDataScope {};       /* Metadata scope - folder-only or folder-and-file */
        std::string version {};           /* Version */
    };

    class Comparator {
    public:
        int operator() (const FileCache p1, const FileCache p2)
        {
            if (memcmp(p1.m_filePathHash.sha1, p2.m_filePathHash.sha1, SHA_DIGEST_LENGTH * sizeof(unsigned char)) > 0) {
                return true;
            }
            return false;
        }
    };
    class FileCacheComparator {
    public:
        int operator()(const FileCacheRecord& p1, const FileCacheRecord& p2)
        {
            if (memcmp(p1.fcache.m_filePathHash.sha1,
                    p2.fcache.m_filePathHash.sha1,
                    SHA_DIGEST_LENGTH * sizeof(unsigned char)) > 0) {
                return true;
            }
            return false;
        }
    };
    class ComparatorV20 {
    public:
        int operator() (const FileCache p1, const FileCache p2)
        {
            return p1.m_filePathHash.crc > p2.m_filePathHash.crc;
        }
    };
    /**
        * Contructor to be used by (producers) users for writing to the filecache file
        */
    explicit FileCacheParser(FileCacheParser::Params params);

    /**
        * Contructor to be used by (consumers) users for reading from the filecache file
        */
    explicit FileCacheParser(std::string fcacheFileName);

    /**
        *  Destructor
        */
    ~FileCacheParser();

    /*
        * This will add filecache entry to sorted queue. if its full it will write to file.
        */
    CTRL_FILE_RETCODE WriteFileCache(FileCache &fcache);

    /*
    * Write filecache entries from queue to buffer
    */
    CTRL_FILE_RETCODE WriteFileCacheEntries(std::queue<FileCache> &fileCacheQueue);
    CTRL_FILE_RETCODE WriteFileCacheEntries(
        std::priority_queue<FileCache, std::vector<FileCache>, Comparator> &fileCacheQueue);
    CTRL_FILE_RETCODE WriteFileCacheEntries(
        std::priority_queue<FileCache, std::vector<FileCache>, ComparatorV20> &fileCacheQueue);

    /*
        * Get the current offset of a file.
        */
    uint64_t GetCurrentOffset();

    /*
        * Write data from buffer to file.
        */
    CTRL_FILE_RETCODE FlushToFile() override;

    /**
        * Get batch fcache entries
        */
    CTRL_FILE_RETCODE ReadFileCacheEntries(std::queue<FileCache> &fcQueue,
        uint64_t offset, uint32_t totalCount, uint16_t metaFileIndex);

    CTRL_FILE_RETCODE ReadFileCacheEntries(std::queue<FileCache> &fcQueue,
        uint64_t offset, uint32_t totalCount, uint16_t metaFileIndex, uint64_t& nextOffset);
    
    /**
        * Get all fcache entries
        */
    CTRL_FILE_RETCODE ReadFileCacheEntries(std::queue<FileCache> &fcQueue, uint32_t maxEntries,
        uint16_t metaFileIndex);

    /**
        * Get filecache filename
        */
    std::string GetFileName();

private:
    FileCacheParser::Header m_header {};        /* File header info */
    uint64_t m_currWriteOffset = 0;             /* Filecache write offset */
    
    CTRL_FILE_RETCODE OpenWrite() override;

    CTRL_FILE_RETCODE CloseWrite() override;

    /**
        * Validate header information read from the file
        */
    CTRL_FILE_RETCODE ValidateHeader();

    /**
        * Read the file header and retry if its failed
        */
    CTRL_FILE_RETCODE ReadFCacheFileHeader();

    /**
        * Read the file header info from file and load to m_header
        */
    CTRL_FILE_RETCODE ReadHeader();
    CTRL_FILE_RETCODE FillHeader(uint32_t &headerLine, std::vector<std::string> &cltHeaderLineSplit,
        std::string &cltHeaderLine);

    /**
        * Write the file header info to file from m_header
        */
    CTRL_FILE_RETCODE WriteHeader();

    /**
    * Get the line to write in header info of file
    */
    std::string GetFileHeaderLine(uint32_t headerLine);

    /**
        * Get the file header info read from file
        */
    CTRL_FILE_RETCODE GetHeader(FileCacheParser::Header &header);

    /**
        * Read entries from file and set to queue
        */
    CTRL_FILE_RETCODE ReadEntries(std::queue<FileCache> &fcQueue, uint32_t maxEntries, uint16_t metaFileIndex);

    /**
    * Read version 1.0 file
    */
    CTRL_FILE_RETCODE ReadEntriesV10(std::queue<FileCache> &fcQueue, uint32_t maxEntries, uint16_t metaFileIndex);

    /**
    * Read version 2.0 file
    */
    CTRL_FILE_RETCODE ReadEntriesV20(std::queue<FileCache> &fcQueue, uint32_t maxEntries);

    void PrintFileCache(const FileCache& cache);

    void ConvertFcacheToV20(const FileCache &fcache, FileCacheV20 &fcacheV20);
};

}
#endif // FS_SCANNER_META_FILE_H
