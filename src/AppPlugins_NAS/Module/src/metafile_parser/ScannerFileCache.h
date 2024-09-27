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
#ifndef MODULE_SCANNER_FCACHE_FILE_H
#define MODULE_SCANNER_FCACHE_FILE_H
#include <queue>
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include "securec.h"
#include "NasControlFile.h"
// #include "ScannerUtils.h"

namespace ScannerFileCache {
const std::string NAS_SCANNERBACKUPFCACHE_HEADER_TITLE = "NAS Scanner Fcache File";
const std::string NAS_SCANNERBACKUPFCACHE_HEADER_VERSION_V10 = "1.0";
const std::string NAS_SCANNERBACKUPFCACHE_HEADER_VERSION = "2.0";

const std::uint16_t FCACHE_FILE_NOT_MODIFIED = 0;
const std::uint16_t FCACHE_FILE_META_MODIFIED = 1;
const std::uint16_t FCACHE_FILE_DATA_MODIFIED = 2;
const std::uint16_t FCACHE_FILE_BOTH_MODIFIED = 3;

constexpr uint16_t NAS_HDR_RETRY_SLEEP_DUR_MIN_MSEC = 1000;     /* in milli seconds */
constexpr uint16_t NAS_HDR_RETRY_SLEEP_DUR_MAX_MSEC = 3000;     /* in milli seconds */

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
};

class CacheV10 {
public:
    uint64_t m_inode = 0;       /* inode of file */
    uint64_t m_mdataOffset = 0; /* offset in meta data file */
    uint32_t m_hashTag = 0;     /* File hash CRC */
    uint32_t m_crc = 0;         /* CRC */
    uint16_t m_metaLength = 0;  /* Total meta length stored in meta file */
    uint16_t m_compareFlag = FCACHE_FILE_NOT_MODIFIED; /* Compare flag which is used at incremental scan */

    CacheV10() {};
    explicit CacheV10(const CacheV10 *fcache)
    {
        m_hashTag = fcache->m_hashTag;
        m_crc = fcache->m_crc;
        m_inode = fcache->m_inode;
        m_mdataOffset = fcache->m_mdataOffset;
        m_compareFlag = fcache->m_compareFlag;
        m_metaLength = fcache->m_metaLength;
    }
    ~CacheV10() {};
};

class Cache {
public:
    uint64_t m_inode = 0;       /* inode of file */
    uint64_t m_mdataOffset = 0; /* offset in meta data file */
    uint32_t m_hashTag = 0;     /* File hash CRC */
    uint32_t m_crc = 0;         /* CRC */
    uint16_t m_fileId = 0;     /* metadata file id */
    uint16_t m_metaLength = 0;  /* Total meta length stored in meta file */
    uint16_t m_compareFlag = FCACHE_FILE_NOT_MODIFIED; /* Compare flag which is used at incremental scan */

    Cache() {};
    explicit Cache(const Cache *fcache)
    {
        m_hashTag = fcache->m_hashTag;
        m_crc = fcache->m_crc;
        m_inode = fcache->m_inode;
        m_mdataOffset = fcache->m_mdataOffset;
        m_compareFlag = fcache->m_compareFlag;
        m_metaLength = fcache->m_metaLength;
        m_fileId = fcache->m_fileId;
    };
    explicit Cache(const CacheV10 *fcache)
    {
        m_hashTag = fcache->m_hashTag;
        m_crc = fcache->m_crc;
        m_inode = fcache->m_inode;
        m_mdataOffset = fcache->m_mdataOffset;
        m_compareFlag = fcache->m_compareFlag;
        m_metaLength = fcache->m_metaLength;
    }
    ~Cache() {};
};

class Comparator {
public:
    int operator() (const Cache p1, const Cache p2)
    {
#ifdef DME_NAS_SCAN_HASH_COMPARE
        return p1.m_hashTag > p2.m_hashTag;
#else
        return p1.m_inode > p2.m_inode;
#endif
    }
};

// class will be similar to control file class.
class CacheFile {
    public:
        /**
         * Contructor to be used by (producers) users for writing to the filecache file
         */
        explicit CacheFile(Params params);

        /**
         * Contructor to be used by (consumers) users for reading from the filecache file
         */
        explicit CacheFile(std::string fcacheFileName);

        /**
         *  Destructor
         */
        ~CacheFile();

        /*
         * Open file based on read/write mode.
         */
        NAS_CTRL_FILE_RETCODE Open(NAS_CTRL_FILE_OPEN_MODE mode);

        /*
         * This will add filecache entry to sorted queue. if its full it will write to file.
         */
        NAS_CTRL_FILE_RETCODE WriteFileCache(Cache &fcache);

        /*
         * Write filecache entries from queue to buffer
         */
        NAS_CTRL_FILE_RETCODE WriteFileCacheEntries(std::queue<Cache> &fileCacheQueue);
        NAS_CTRL_FILE_RETCODE WriteFileCacheEntries(
            std::priority_queue<Cache, std::vector<Cache>, Comparator> &fileCacheQueue);

        /*
         * Get the current offset of a file.
         */
        uint64_t GetCurrentOffset();

        /*
         * Flush all the data from buffer to file and close file.
         */
        NAS_CTRL_FILE_RETCODE Close(NAS_CTRL_FILE_OPEN_MODE mode);

        /*
         * Write data from buffer to file.
         */
        NAS_CTRL_FILE_RETCODE FlushToFile();

        /**
         * Get batch fcache entries
         */
        NAS_CTRL_FILE_RETCODE ReadFileCacheEntries(std::queue<Cache> &fcQueue,
            uint64_t offset, uint32_t totalCount, uint16_t metaFileIndex);
        
        /**
         * Get all fcache entries
         */
        NAS_CTRL_FILE_RETCODE ReadFileCacheEntries(std::queue<Cache> &fcQueue, uint32_t maxEntries,
            uint16_t metaFileIndex);

        /**
         * Get filecache filename
         */
        std::string GetFileName();

    private:

        std::mutex m_lock {};                            /* Lock */
        std::string m_fcacheFileName {};                 /* Fcache filename */
        std::string m_fcacheFileParentDir {};            /* Parent Dir of Fcache File */
        Header m_header {};        /* File header info */

        std::stringstream m_writeBuffer {};                 /* Write Buffer */
        std::ifstream m_readFd {};    /* Read FD */
        std::ofstream m_writeFd {};   /* Write FD */

        uint64_t m_currWriteOffset = 0;             /* Filecache write offset */
        uint64_t m_readBufferSize = 0;              /* Filecache write offset */
        char *m_readBuffer = nullptr;               /* Read buffer */

        /**
         * Template to Open a File in Read/Write Mode
         */
        template<class FileStream>
        NAS_CTRL_FILE_RETCODE FileOpen(FileStream &strmFd, std::ios::openmode fileMode);

        /**
         * Validate header information read from the file
         */
        NAS_CTRL_FILE_RETCODE ValidateHeader();

        /**
         * Read the file header and retry if its failed
         */
        NAS_CTRL_FILE_RETCODE ReadFCacheFileHeader();

        /**
         * Read the file header info from file and load to m_header
         */
        NAS_CTRL_FILE_RETCODE ReadHeader();
        NAS_CTRL_FILE_RETCODE FillHeader(uint32_t &headerLine, std::vector<std::string> &cltHeaderLineSplit,
            std::string &cltHeaderLine);

        /**
         * Write the file header info to file from m_header
         */
        NAS_CTRL_FILE_RETCODE WriteHeader();

        /**
        * Get the line to write in header info of file
        */
        std::string GetFileHeaderLine(uint32_t headerLine);

        /**
         * Get the file header info read from file
         */
        NAS_CTRL_FILE_RETCODE GetHeader(Header &header);

        /**
         * Read entries from file and set to queue
         */
        NAS_CTRL_FILE_RETCODE ReadEntries(std::queue<Cache> &fcQueue, uint32_t maxEntries, uint16_t metaFileIndex);

        /**
         * Read version 1.0 file
         */
        NAS_CTRL_FILE_RETCODE ReadEntriesV10(std::queue<Cache> &fcQueue, uint32_t maxEntries, uint16_t metaFileIndex);

        uint32_t GetRandomNumber(uint32_t minNum, uint32_t maxNum);
};
}

#endif // DME_NAS_SCANNER_META_FILE_H
