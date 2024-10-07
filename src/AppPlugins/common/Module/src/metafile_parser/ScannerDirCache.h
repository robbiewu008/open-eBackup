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
#ifndef MODULE_SCANNER_DCACHE_FILE_H
#define MODULE_SCANNER_DCACHE_FILE_H
#include <cstdint>
#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include <fstream>
#include "securec.h"
#include "NasControlFile.h"
// #include "ScannerUtils.h"

namespace ScannerDirCache {
const std::string NAS_SCANNERBACKUPDCACHE_HEADER_TITLE = "NAS Scanner DirCache File";
const std::string NAS_SCANNERBACKUPDCACHE_HEADER_VERSION_V10 = "1.0";
const std::string NAS_SCANNERBACKUPDCACHE_HEADER_VERSION = "2.0";
constexpr uint64_t NAS_SCANNERBACKUPDCACHE_MAX_READBUFF_SIZE = 100000;

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

enum DCACHE_WRITE_INFO {
    DCACHE_WRITE_TO_BUFFER = 0,
    DCACHE_ADD_TO_QUEUE = 1,
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
};

class CacheV10 {
public:
    uint64_t m_inode = 0;        /* inode of dir */
    uint64_t m_mdataOffset = 0;  /* offset in meta data file */
    uint64_t m_fcacheOffset = 0; /* offset in fcache file */
    uint32_t m_hashTag = 0;      /* Directory hash CRC */
    uint32_t m_crc = 0;          /* CRC */
    uint32_t m_totalFiles = 0;   /* number of files under given dir */
    uint16_t m_fileId = 0;      /* metadata file id */
    uint16_t m_metaLength = 0;  /* Total meta length stored in meta file */

    CacheV10() {};
    explicit CacheV10(const CacheV10 *dc)
    {
        m_hashTag = dc->m_hashTag;
        m_inode = dc->m_inode;
        m_crc = dc->m_crc;
        m_fileId = dc->m_fileId;
        m_fcacheOffset = dc->m_fcacheOffset;
        m_mdataOffset = dc->m_mdataOffset;
        m_totalFiles = dc->m_totalFiles;
        m_metaLength = dc->m_metaLength;
    }
    ~CacheV10() {};
};

class Cache {
public:
    uint64_t m_inode = 0;        /* inode of dir */
    uint64_t m_mdataOffset = 0;  /* offset in meta data file */
    uint64_t m_fcacheOffset = 0; /* offset in fcache file */
    uint32_t m_hashTag = 0;      /* Directory hash CRC */
    uint32_t m_crc = 0;          /* CRC */
    uint32_t m_totalFiles = 0;   /* number of files under given dir */
    uint16_t m_fileId = 0;      /* metadata file id */
    uint16_t m_fcacheFileId = 0; /* fcache file id */
    uint16_t m_metaLength = 0;  /* Total meta length stored in meta file */

    Cache() {};
    explicit Cache(const Cache *dc)
    {
        m_hashTag = dc->m_hashTag;
        m_inode = dc->m_inode;
        m_crc = dc->m_crc;
        m_fileId = dc->m_fileId;
        m_fcacheOffset = dc->m_fcacheOffset;
        m_mdataOffset = dc->m_mdataOffset;
        m_totalFiles = dc->m_totalFiles;
        m_metaLength = dc->m_metaLength;
        m_fcacheFileId = dc->m_fcacheFileId;
    };
    explicit Cache(const CacheV10 *dc)
    {
        m_hashTag = dc->m_hashTag;
        m_inode = dc->m_inode;
        m_crc = dc->m_crc;
        m_fileId = dc->m_fileId;
        m_fcacheOffset = dc->m_fcacheOffset;
        m_mdataOffset = dc->m_mdataOffset;
        m_totalFiles = dc->m_totalFiles;
        m_metaLength = dc->m_metaLength;
        m_fcacheFileId = dc->m_fileId;
    };
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
         * Contructor to be used by (producers) users for writing to the dircache file
         */
        explicit CacheFile(Params params);

        /**
         * Contructor to be used by (consumers) users for reading from the dircache file
         */
        explicit CacheFile(std::string dirFileName);

        /**
         *  Destructor
         */
        ~CacheFile();

        /*
         * Open file based on read/write mode.
         */
        NAS_CTRL_FILE_RETCODE Open(NAS_CTRL_FILE_OPEN_MODE mode);

        /*
         * This will add directory entry to sorted queue.
         */
        NAS_CTRL_FILE_RETCODE WriteDirCache(Cache &dcache, DCACHE_WRITE_INFO writeInfo);

        /*
         * Write dircache entries to buffer
         */
        int32_t WriteDirCacheEntries(std::queue<Cache> &dcQueue);
        int32_t WriteDirCacheEntries(
            std::priority_queue<Cache, std::vector<Cache>, Comparator> &dcQueue);

        /**
         * Get total size
         */
        uint64_t GetSize();

        /*
         * Get batch dircache entries from file.
         */
        NAS_CTRL_FILE_RETCODE ReadDirCacheEntries(std::queue<Cache> &dcQueue, uint32_t numOfEntriesToRead);

        /*
         * Flush data from buffer to file and close.
         */
        NAS_CTRL_FILE_RETCODE Close(NAS_CTRL_FILE_OPEN_MODE mode);

        /**
         * Get dircache file name
         */
        std::string GetFileName();

    private:
        std::mutex m_lock {};                            /* Lock */
        char *m_readBuffer = nullptr;               /* Read buffer */
        std::stringstream m_writeBuffer {};              /* Write Buffer */
        Header m_header {};        /* File header info */
        std::ifstream m_readFd {};    /* Read FD */
        std::ofstream m_writeFd {};   /* Write FD */
        std::string m_dcacheFileName {};              /* File name */
        uint64_t m_readBufferSize = 0;              /* Filecache write offset */
        std::string m_dcacheFileParentDir {};            /* Parent Dir of DCache File */
        std::priority_queue<Cache, std::vector<Cache>, Comparator> m_dirCacheQueue {}; /* Sorted queue */

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
        NAS_CTRL_FILE_RETCODE ReadDCacheFileHeader();

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

        /*
         * Flush data from buffer to file.
         */
        NAS_CTRL_FILE_RETCODE FlushToFile();

        /**
         * Read dircache entries V10 version
         */
        NAS_CTRL_FILE_RETCODE ReadDirCacheEntriesV10(std::queue<Cache> &dcQueue, uint32_t numOfEntriesToRead);

        uint32_t GetRandomNumber(uint32_t minNum, uint32_t maxNum);
};
}

#endif // DME_NAS_SCANNER_META_FILE_H