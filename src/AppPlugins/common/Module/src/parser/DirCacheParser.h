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
#ifndef MODULE_DCACHE_FILE_H
#define MODULE_DCACHE_FILE_H
#include <cstdint>
#include <queue>
#include "ParserStructs.h"
#include "FileParser.h"
#include "define/Defines.h"

namespace Module {

const std::string DCACHE_HEADER_TITLE = "NAS Scanner DirCache File";
const std::string DCACHE_HEADER_VERSION_V10 = "1.0";
const std::string DCACHE_HEADER_VERSION_V20 = "2.0";
const std::string DCACHE_HEADER_VERSION_V30 = "3.0";

enum DCACHE_HEADER_INFO {
    DCACHE_TITLE = 0,
    DCACHE_HEADER_VERSION = 1,
    DCACHE_TIMESTAMP = 2,
    DCACHE_TASKID = 3,
    DCACHE_TASKTYPE = 4,
    DCACHE_NASSERVER = 5,
    DCACHE_NASSHARE = 6,
    DCACHE_PROTOCOL = 7,
    DCACHE_PROTOCOL_VERSION = 8,
    DCACHE_METADATA_SCOPE = 9,
    DCACHE_RESERVED_1 = 10,
    DCACHE_RESERVED_2 = 11,
    DCACHE_RESERVED_3 = 12,
    DCACHE_RESERVED_4 = 13,
    DCACHE_RESERVED_5 = 14,
};

enum class DCACHE_WRITE_INFO {
    DCACHE_WRITE_TO_BUFFER = 0,
    DCACHE_ADD_TO_QUEUE = 1,
};

// class will be similar to control file class.
class AGENT_API DirCacheParser : public FileParser {
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
        std::string fileName {};       /* Control file title */
        uint32_t maxEntriesPerFile = 0;     /* Max number of entries that can be written in this file */
        uint64_t maxDataSize = 0;           /* Max data size that can be written in this file */
        std::string taskId {};              /* Task id */
        std::string backupType {};          /* Full or incremental */
        std::string nasServer {};           /* Scanned nas server ip address */
        std::string nasSharePath {};        /* Scanned nas share path */
        std::string proto {};               /* Protocol used - nfs or cifs */
        std::string protoVersion {};       /* Protocol version */
        std::string metaDataScope {};       /* Metadata scope - folder-only or folder-and-file */
        std::string version {};
    };

    class Comparator {
    public:
        bool operator() (const DirCache p1, const DirCache p2)
        {
            if (memcmp(p1.m_dirPathHash.sha1, p2.m_dirPathHash.sha1, SHA_DIGEST_LENGTH * sizeof(unsigned char)) > 0) {
                return true;
            }
            return false;
        }
    };

    class ComparatorV20 {
    public:
        bool operator() (const DirCache p1, const DirCache p2)
        {
            return p1.m_dirPathHash.crc > p2.m_dirPathHash.crc;
        }
    };
    /**
        * Contructor to be used by (producers) users for writing to the dircache file
        */
    explicit DirCacheParser(DirCacheParser::Params params);

    /**
        * Contructor to be used by (consumers) users for reading from the dircache file
        */
    explicit DirCacheParser(std::string dirFileName);

    /**
        *  Destructor
        */
    ~DirCacheParser();

    /*
        * This will add directory entry to sorted queue.
        */
    CTRL_FILE_RETCODE WriteDirCache(DirCache &dcache, DCACHE_WRITE_INFO writeInfo);

    /*
        * Write dircache entries to buffer
        */
    int32_t WriteDirCacheEntries(std::queue<DirCache> &dcQueue);
    int32_t WriteDirCacheEntries(
        std::priority_queue<DirCache, std::vector<DirCache>, DirCacheParser::Comparator> &dcQueue);

    /**
        * Get total size
        */
    uint64_t GetSize();

    /*
        * Get batch dircache entries from file.
        */
    CTRL_FILE_RETCODE ReadDirCacheEntries(std::queue<DirCache> &dcQueue, uint32_t numOfEntriesToRead);

    /**
    * Get dircache file name
    */
    std::string GetFileName();

    /**
    * Get dircache file Header Version
    */
    std::string GetVersion();

private:
    DirCacheParser::Header m_header {};        /* File header info */
    std::string m_dcacheFileParentDir {};            /* Parent Dir of DCache File */
    std::priority_queue<DirCache, std::vector<DirCache>,
        DirCacheParser::Comparator> m_dirCacheQueue {}; /* Sorted queue */
    std::priority_queue<DirCache, std::vector<DirCache>,
        DirCacheParser::ComparatorV20> m_dirCacheQueueV20 {}; /* Sorted queue */


    CTRL_FILE_RETCODE OpenWrite() override;

    CTRL_FILE_RETCODE CloseWrite() override;
    /**
        * Validate header information read from the file
        */
    CTRL_FILE_RETCODE ValidateHeader() override;

    /**
        * Read the file header and retry if its failed
        */
    CTRL_FILE_RETCODE ReadDCacheFileHeader();

    /**
        * Read the file header info from file and load to m_header
        */
    CTRL_FILE_RETCODE ReadHeader() override;
    CTRL_FILE_RETCODE FillHeader(uint32_t &headerLine, std::vector<std::string> &cltHeaderLineSplit,
        std::string &cltHeaderLine);

    /**
        * Write the file header info to file from m_header
        */
    CTRL_FILE_RETCODE WriteHeader() override;

    /**
    * Get the line to write in header info of file
    */
    std::string GetFileHeaderLine(uint32_t headerLine);

    /**
        * Get the file header info read from file
        */
    CTRL_FILE_RETCODE GetHeader(DirCacheParser::Header &header);

    /*
        * Flush data from buffer to file.
        */
    CTRL_FILE_RETCODE FlushToFile() override;

    /**
    * Read dircache entries V10 version
    */
    CTRL_FILE_RETCODE ReadDirCacheEntriesV10(std::queue<DirCache> &dcQueue, uint32_t numOfEntriesToRead);

    /**
    * Read dircache entries V20 version
    */
    CTRL_FILE_RETCODE ReadDirCacheEntriesV20(std::queue<DirCache> &dcQueue, uint32_t numOfEntriesToRead);

    void PrintDirCache(DirCache& dcache);

    void ConvertDirCacheToV20(const DirCache &dcache, DirCacheV20 &dcacheV20);
    void WriteToBfrBasedOnVersion(DirCache &dcache);

};

}
#endif // FS_SCANNER_DCACHE_FILE_H