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
#ifndef SCANNER_META_FILE_H
#define SCANNER_META_FILE_H

#include <queue>
#include "ParserStructs.h"
#include "FileParser.h"
#include "define/Defines.h"

namespace Module {

const std::string META_HEADER_TITLE_NAS = "NAS Scanner Backup Meta File";
const std::string META_HEADER_TITLE_FS = "FS Scanner Backup Meta File";
const std::string META_HEADER_VERSION_V10 = "1.0";
const std::string META_HEADER_VERSION_V20 = "2.0";

enum META_HEADER_INFO {
    META_TITLE = 0,
    META_HEADER_VERSION = 1,
    META_TIMESTAMP = 2,
    META_TASKID = 3,
    META_TASKTYPE = 4,
    META_NASSERVER = 5,
    META_NASSHARE = 6,
    META_PROTOCOL = 7,
    META_PROTOCOL_VERSION = 8,
    META_METADATA_SCOPE = 9,
    META_RESERVED_1 = 10,
    META_RESERVED_2 = 11,
    META_RESERVED_3 = 12,
    META_RESERVED_4 = 13,
    META_RESERVED_5 = 14,
};

enum class TRANSLATE_TYPE {
    META_TO_METARW = 0,
    METARW_TO_META = 1,
    STAT_TO_STATRW = 2,
    STATRW_TO_STAT = 3,
};

class DirectoryMetaReadWrite {
public:
    uint64_t m_inode = 0;
    uint64_t m_size = 0;
    uint64_t m_mtime = 0;
    uint64_t m_atime = 0;
    uint64_t m_ctime = 0;
    uint64_t m_btime = 0;
    uint32_t m_attr = 0;
    uint32_t m_mode = 0;
    uint32_t m_uid = 0;
    uint32_t m_gid = 0;
    uint16_t m_aclSize = 0;
    uint16_t m_pathLen = 0;
};

class FileMetaReadWrite {
public:
    NfsFileHandle m_fh {};
    uint64_t m_inode = 0;
    uint64_t m_mtime = 0;
    uint64_t m_atime = 0;
    uint64_t m_ctime = 0;
    uint64_t m_btime = 0;
    uint32_t m_attr = 0;
    uint64_t m_size = 0;
    uint64_t m_rdev = 0;
    uint32_t m_mode = 0;
    uint32_t m_nlink = 0;
    uint32_t m_uid = 0;
    uint32_t m_gid = 0;
    uint16_t m_aclSize = 0;
    uint16_t m_nameLen = 0;
};

// class will be similar to control file class.
class AGENT_API MetaParser : public FileParser {
public:
    struct Params {
        std::string m_fileName {};       /* Control file title */
        uint32_t maxEntriesPerFile = 0;     /* Max number of entries that can be written in this file */
        uint64_t maxDataSize = 0;           /* Max data size that can be written in this file */
    };

    struct Header {
        std::string title;                  /* Control file title */
        std::string version;                /* Version */
        std::string timestamp;              /* Timestamp - when the file is created */
    };
    /**
    * Contructor to be used by (producers) users for writing to the meta file
    */
    explicit MetaParser(MetaParser::Params params);

    /**
    * Contructor to be used by (consumers) users for reading from the meta file
    */
    explicit MetaParser(std::string metaFileName);

    /**
    * Contructor to be used by (consumers) users for reading from the meta file
    */
    explicit MetaParser(std::string metaFileName, bool isCacheEnable);

    /**
    *  Destructor
    */
    ~MetaParser();

    /**
    * Write directory meta which will write to buffer and return current offset.
    */
    uint16_t WriteDirectoryMeta(const DirMeta &dirMeta);

    /**
    * Write file meta which will write to buffer and return current offset.
    */
    uint16_t WriteFileMeta(const FileMeta &fMeta);

    /**
    * Read directory meta for a given offset.
    */
    CTRL_FILE_RETCODE ReadDirectoryMeta(DirMeta &dirMeta, uint64_t offset);

    /**
    * Read V10 directory meta for a given offset and read length.
    */
    CTRL_FILE_RETCODE ReadDirectoryMetaV10(DirMetaV10 &dirMeta, uint16_t readLen, uint64_t offset);

    /**
    * Read file meta for a given offset.
    */
    CTRL_FILE_RETCODE ReadFileMeta(FileMeta &fMeta, uint64_t offset);

    CTRL_FILE_RETCODE OpenForWrite();
    CTRL_FILE_RETCODE UpdateFileMeta(const FileMeta &fMeta, uint64_t offset);
    void CloseForWrite();

    /**
    * Read V10 file meta for a given offset and read length.
    */
    CTRL_FILE_RETCODE ReadFileMetaV10(FileMetaV10 &fMeta, uint16_t readLen, uint64_t offset);

    /**
    * Read file meta entries
    */
    CTRL_FILE_RETCODE ReadFileMetaEntries(std::queue<FileMeta> &fileMetaQueue, int count);

    /**
    * This will return current offset.
    */
    uint64_t GetCurrentOffset();

    /**
    * Write buffer data into file.
    */
    CTRL_FILE_RETCODE FlushToFile() override;

    /**
    * Get Cache hits
    */
    uint64_t GetCacheHitCount();

    /**
    * Get Misses
    */
    uint64_t GetCacheMissCount();

    /**
    * Get Metafile name
    */
    std::string GetFileName();

    std::string GetFileVersion();

private:
    MetaParser::Header m_header {};          /* File header info */
    uint64_t m_offset = 0;                   /* MetaParser write offset */
    uint64_t m_maxFileSize = 0;              /* Max File size */

    char* m_readCache = nullptr;             /* Read cache (read from file in disk and cache it here) */
    uint64_t m_readCacheOffsetStart = 0;     /* Read cache - starting offset in file */
    uint64_t m_readCacheOffsetEnd = 0;       /* Read cache - end offset in file */
    uint64_t m_cacheHitCnt = 0;                     /* Cache read hits */
    uint64_t m_cacheMissCnt = 0;                     /* ache read miss */
    bool m_isCacheEnable = false;              /* Enable/disable cache read */


    CTRL_FILE_RETCODE OpenWrite() override;
    CTRL_FILE_RETCODE CloseWrite() override;
    CTRL_FILE_RETCODE ReadHeader() override;
    CTRL_FILE_RETCODE WriteHeader() override;
    CTRL_FILE_RETCODE ValidateHeader() override;

    CTRL_FILE_RETCODE AllocReadCache();
    CTRL_FILE_RETCODE FreeReadCache();
    CTRL_FILE_RETCODE RefreshReadCache(uint64_t offset, uint64_t len);
    CTRL_FILE_RETCODE FillReadCache(uint64_t offset, uint32_t readLen);

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
    CTRL_FILE_RETCODE GetHeader(MetaParser::Header &header);

    CTRL_FILE_RETCODE ReadAcl(uint64_t offset, DirMetaV10 &dirMeta, uint16_t readLen, int32_t aclOffset);
    CTRL_FILE_RETCODE ReadAcl(uint64_t offset, FileMetaV10 &fMeta, uint16_t readLen, int32_t aclOffset);
    CTRL_FILE_RETCODE CheckDirMetaV10Validity(DirMetaV10 &dirMeta);
    CTRL_FILE_RETCODE CheckFileMetaV10Validity(FileMetaV10 &fMeta);
    CTRL_FILE_RETCODE ReadDirMetaV10FromBuffer(DirMetaV10 &dirMeta, uint64_t offset, uint16_t readLen);
    CTRL_FILE_RETCODE ReadFileMetaV10FromBuffer(FileMetaV10 &fMeta, uint64_t offset, uint16_t readLen);
    /**
        * Read directory meta from buffer and fill DirMeta object
        */
    CTRL_FILE_RETCODE ReadDirMetaFromReadCache(DirMeta &dirMeta, uint64_t offset);
    /**
    * Read file meta from buffer and fill FileMeta object
    */
    CTRL_FILE_RETCODE ReadFileMetaFromReadCache(FileMeta &fMeta, uint64_t offset);

    /**
        * Read metaobj from file fd.
        */
    CTRL_FILE_RETCODE ReadLenFromFile(uint32_t len);

    /**
        * Read filemeta from buffer
        */
    CTRL_FILE_RETCODE ReadFileMetaFromBuffer(FileMeta &fMeta);

    /**
        * Readfile meta from current offset
        */
    CTRL_FILE_RETCODE ReadFileMeta(FileMeta &fMeta);

    /**
    * Validate file offsets
    */
    CTRL_FILE_RETCODE ValidateMetaFile(uint64_t offset);

    /**
    * Validate file offsets
    */
    CTRL_FILE_RETCODE ValidateMetaFile(uint64_t offset, uint16_t readLen);

    void TranslateToLittleEndian(const DirMeta &dirMeta, DirMeta &dirMetaInLe);
    void TranslateToHostEndian(const DirMeta &dirMeta, DirMeta &dirMetaInHost);
    void TranslateToLittleEndian(const FileMeta &fMeta, FileMeta &fMetaInLe);
    void TranslateToHostEndian(const FileMeta &fMeta, FileMeta &fMetaInHost);
    void TranslateDirMetaV10(DirMetaV10 &dirMeta, DirectoryMetaReadWrite &dirMetaRw);
    CTRL_FILE_RETCODE TranslateFileMetaV10(FileMetaV10 &fMeta, FileMetaReadWrite &fMetaRw);
    void PrintFileMeta(const FileMeta& fileMeta);
    void PrintDirectoryMeta(const DirMeta& dirMeta);
    CTRL_FILE_RETCODE ReadFileMetaWithoutCache(FileMeta &fMeta, uint64_t offset);
    CTRL_FILE_RETCODE ReadFileMetaFromBuffer(FileMeta &fMeta, uint64_t offset);
    CTRL_FILE_RETCODE ReadDirectoryMetaWithoutCache(DirMeta &dirMeta, uint64_t offset);
    CTRL_FILE_RETCODE ReadDirMetaFromBuffer(DirMeta &dirMeta, uint64_t offset);
};

}
#endif // SCANNER_META_FILE_H