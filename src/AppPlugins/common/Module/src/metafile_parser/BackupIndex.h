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
#ifndef MODULE_BACKUP_INDEX_H
#define MODULE_BACKUP_INDEX_H

#include <string>
#include <map>
#include <set>
#include <vector>
#include <mutex>
#include "NasControlFile.h"
#include "ScannerBackupMeta.h"

/* "/" cannot be used for file name on both NTFS & UNIX platform */
const std::string NAS_BACKUPINDEX_ENTRY_DELIMITER  = "/";

const std::string NAS_BACKUPINDEX_HEADER_TITLE = "NAS Backup Index File";
const std::string NAS_BACKUPINDEX_HEADER_VERSION = "1.0";
const std::string NAS_BACKUPINDEX_SECTION_DELIMITER = "-----------------------------------------------------";

namespace NasScanner {
struct BackupIndexStats {
    uint64_t noOfArchives = 0;          /* Num of dir entries in the control file */
    uint64_t noOfFiles = 0;             /* Num of file entries in the control file */
};

struct BackupIndexHeader {
    std::string title;                  /* Control file title */
    std::string version;                /* Version */
    std::string timestamp;              /* Timestamp - when the file is created */
    std::string taskId;                 /* Task id */
    std::string backupType;             /* Full or incremental */
    std::string nasServer;              /* Scanned nas server ip address */
    std::string nasSharePath;           /* Scanned nas share path */
    std::string proto;                  /* Protocol used - nfs or cifs */
    std::string protoVersion;           /* Protocol version */
    std::string metaDataScope;          /* Metadata scope - folder-only or folder-and-file */
    BackupIndexStats stats;             /* Statistics of entries in this control file */
};

enum INDEX_HEADER_INFO {
    INDEX_TITLE = 0,
    INDEX_HEADER_VERSION = 1,
    INDEX_TIMESTAMP = 2,
    INDEX_TASKID = 3,
    INDEX_TASKTYPE = 4,
    INDEX_NASSERVER = 5,
    INDEX_NASSHARE = 6,
    INDEX_PROTOCOL = 7,
    INDEX_PROTOCOL_VERSION = 8,
    INDEX_METADATA_SCOPE = 9,
    INDEX_ARCHIVE_COUNT = 10,
    INDEX_FILE_COUNT = 11,
    INDEX_RESERVED_1 = 12,
    INDEX_RESERVED_2 = 13,
    INDEX_RESERVED_3 = 14,
    INDEX_RESERVED_4 = 15,
    INDEX_RESERVED_5 = 16
};

enum INDEX_HEADER_ENTRY_OFFSET {
    INDEX_HEADER_ENTRY_OFFSET_KEY = 0,
    INDEX_HEADER_ENTRY_OFFSET_VALUE = 1
};

enum INDEX_ENTRY_OP_TYPE {
    INDEX_ENTRY_OP_FULL = 0,
    INDEX_ENTRY_OP_INC = 1
};

enum INDEX_ENTRY_TYPE {
    INDEX_ENTRY_TYPE_DIR = 0,
    INDEX_ENTRY_TYPE_ARCHIVE = 1,
    INDEX_ENTRY_TYPE_FILE = 2
};

enum INDEX_ENTRY_DIR_OFFSET {
    INDEX_ENTRY_DIR_OFFSET_DIRNAME = 0,
    INDEX_ENTRY_DIR_OFFSET_METAFILE_NAME = 1,
    INDEX_ENTRY_DIR_OFFSET_METAFILE_OFFSET = 2,
    INDEX_ENTRY_DIR_OFFSET_METAFILE_LEN = 3,
    INDEX_ENTRY_DIR_OFFSET_ACL_FLAG = 4
};

enum INDEX_ENTRY_ARCHIVE_OFFSET {
    INDEX_ENTRY_ARCHIVE_OFFSET_ARCHIVENAME = 0
};

enum INDEX_ENTRY_FILE_OFFSET {
    INDEX_ENTRY_FILE_OFFSET_DIRNAME = 0,
    INDEX_ENTRY_FILE_OFFSET_METAFILE_OFFSET = 1,
    INDEX_ENTRY_FILE_OFFSET_METAFILE_LEN = 2,
    INDEX_ENTRY_FILE_OFFSET_ACL_FLAG = 3
};

struct BackupIndexParams {
    uint32_t indexFileTimeElapsed = 0;  /* Time elapsed to generate index file */
    std::string indexFileName;          /* index file full name */
    std::string tmpIndexFileName;       /* tmp index file full name, for update */
    std::string taskId;                 /* Task id */
    std::string backupType;             /* Full or incremental */
    std::string nasServer;              /* Scanned nas server ip address */
    std::string nasSharePath;           /* Scanned nas share path */
    std::string proto;                  /* Protocol used - nfs or cifs */
    std::string proto_version;          /* Protocol version */
    std::string metaDataScope;          /* Metadata scope - folder-only or folder-and-file */
};

struct BackupIndexDirEntry {
    std::string dirName;                /* Absolute path of directory */
    std::string metaFileName;           /* Metafile where the dir metadata is saved */
    uint16_t metaFileReadLen = 0;       /* Length of the metadata */
    uint64_t metaFileOffset = 0;        /* Offset inside the metafile to get the metadata for this dir */
    uint32_t aclFlag = 0;               /* Boolean flag specifying whether there is acl info present in metadata */
};

struct BackupIndexArchiveEntry {
    std::string archiveName;            /* Relative archive name */
};

struct BackupIndexFileEntry {
    std::string fileName;               /* Relative file name */
    std::string archiveName;            /* Relative archive name */
    uint16_t metaDataLen = 0;           /* Length of the metadata */
};

struct BackupIndexFileMetaInfo {
    uint16_t metaDataLen = 0;           /* Length of the metadata */
    bool toBeDelete = false;            /* Used to track file entry delete in delete phase */
};

class BackupIndex {
public:
    /**
    * Contructor to be used by (producers) users for writing to the control file
    */
    explicit BackupIndex(const BackupIndexParams &params);

    /**
    * Contructor to be used by (consumers) users for reading from the control file
    */
    explicit BackupIndex(std::string &indexFileName);

    /**
    *  Destructor
    */
    ~BackupIndex();

    /**
    * Open the index file
    * Producers must use mode NAS_INDEX_FILE_OPEN_MODE_WRITE/NAS_INDEX_FILE_OPEN_MODE_UPDATE
    * Consumers must use mode NAS_INDEX_FILE_OPEN_MODE_READ
    *
    */
    NAS_INDEX_FILE_RETCODE Open(NAS_INDEX_FILE_OPEN_MODE mode);

    /**
    * Close the index file
    */
    NAS_INDEX_FILE_RETCODE Close(NAS_INDEX_FILE_OPEN_MODE mode);

    /**
    * Load index file into in-memory map
    */
    NAS_INDEX_FILE_RETCODE Load();

    /**
    * Dump in-memory map into index file
    */
    NAS_INDEX_FILE_RETCODE Unload();

    /**
    * Write current dir entry
    */
    NAS_INDEX_FILE_RETCODE WriteDirEntry(BackupIndexDirEntry &dirEntry);

    /**
    * Write current dir metadata
    */
    NAS_INDEX_FILE_RETCODE WriteDirectoryMeta(DirectoryMeta &dirMeta);

    /**
    * Write a file entry
    * during full backup copy phase, opType should be INDEX_ENTRY_OP_FULL
    * during inc backup copy phase,  optype should be INDEX_ENTRY_OP_INC
    */
    NAS_INDEX_FILE_RETCODE WriteFileEntry(BackupIndexFileEntry &fileEntry, INDEX_ENTRY_OP_TYPE opType);

    /**
    * Write a file entry list
    * during full backup copy phase, opType should be INDEX_ENTRY_OP_FULL
    * during inc backup copy phase,  optype should be INDEX_ENTRY_OP_INC
    */
    NAS_INDEX_FILE_RETCODE WriteFileEntries(
        std::vector<BackupIndexFileEntry> &fileEntries, INDEX_ENTRY_OP_TYPE opType);

    /**
    * Check whether a file exist in previous copy's file list. Should be call after Load
    * It's better to not use this API
    */
    bool Exist(const std::string &fileName);

    /**
    * Query for delete & partial delete archive list from index file. Should be call after Load
    */
    NAS_INDEX_FILE_RETCODE GetDeleteList(const std::unordered_set<std::string> &deleteFiles,
        std::unordered_set<std::string> &archiveDeleteList,
        std::unordered_map<std::string, std::unordered_set<std::string>> &archivePartialDeleteList);

    /**
    * Get the archive map for this index file
    */
    NAS_INDEX_FILE_RETCODE GetArchiveMap(
        std::unordered_map<std::string, std::unordered_map<std::string, BackupIndexFileMetaInfo>> &archiveMap);
    /**
    * Get the directory entry
    */
    NAS_INDEX_FILE_RETCODE GetDirEntry(BackupIndexDirEntry &dirEntry);

    /**
    * Merge a list of index files with this->m_indexFileName
    *
    */
    NAS_INDEX_FILE_RETCODE Merge(std::vector<std::string> &tmpIndexFileList);

    /**
    * Synthetic index file to the final status. should be call after WriteDirEntry/WriteFileEntry/GetDeleteList
    */
    NAS_INDEX_FILE_RETCODE Synthetic();

    /**
    * Check whether there's no file entry
    */
    bool IsFileBufferEmpty();

    /**
    * Get total number of file entries
    */
    uint32_t GetEntries();

    /**
    * Check index file time elapse
    */
    bool CheckIndexFileTimeElapse();

private:
    std::mutex m_lock {};                       /* Lock */
    std::string m_indexFileName;                /* This index file name */
    BackupIndexHeader m_header;                 /* File header info */
    BackupIndexDirEntry m_dirEntry;             /* Dir entry of this index file */

    std::ifstream m_readFd;       /* Read FD */
    std::stringstream m_readBuffer;                  /* Read Buffer */
    std::ofstream m_writeFd;      /* Write FD */
    std::stringstream m_writeBuffer;                 /* Write Buffer */

    uint32_t m_entries { 0 };                   /* Number of file/dir entries in the file */
    uint32_t m_fileEntryCount { 0 };            /* To track filecount per file. */

    time_t m_indexFileCreationTime { 0 };       /* Index file creation time */
    uint32_t m_indexFileTimeElapsed { 0 };      /* Time elapsed since index file created */

    std::unordered_map<std::string, std::unordered_map<std::string, BackupIndexFileMetaInfo>> m_archiveMap;
    std::unordered_map<std::string, std::unordered_map<std::string, BackupIndexFileMetaInfo>> m_archiveMapInc;
    std::unordered_set<std::string> m_archiveDeleteList;
    std::unordered_map<std::string, std::unordered_set<std::string>> m_archivePartialDeleteList;
    std::unordered_set<std::string> m_incFileSet;

    /**
    * Open index file as read mode
    */
    NAS_INDEX_FILE_RETCODE OpenRead();

    /**
    * Open index file as write mode
    */
    NAS_INDEX_FILE_RETCODE OpenWrite();

    /**
    * Open index file as read & write mode
    */
    NAS_INDEX_FILE_RETCODE OpenReadWrite();

    /**
    * Template to Open a File in Read or Write Mode
    */
    template<class FileStream>
    NAS_CTRL_FILE_RETCODE FileOpen(FileStream &strmFd, std::string fileName, std::ios::openmode fileMode);

    /**
    * Read file entries from index file to specific archive map
    */
    NAS_INDEX_FILE_RETCODE ReadEntries(INDEX_ENTRY_OP_TYPE opType);

    /**
    * Read an entry from index file which would be dir entry, archive entry or file entry
    */
    NAS_INDEX_FILE_RETCODE ReadEntry(
        BackupIndexDirEntry &dirEntry, BackupIndexArchiveEntry &archiveEntry, BackupIndexFileEntry &fileEntry,
        INDEX_ENTRY_TYPE &entryType);

    /**
    * Release internal in-memory map
    */
    NAS_INDEX_FILE_RETCODE Release();

    /**
    * Dump archive map into index file
    */
    NAS_INDEX_FILE_RETCODE UnloadArchiveMap();

    /**
    * Dump archive map inc part into index file
    */
    NAS_INDEX_FILE_RETCODE UnloadArchiveMapInc();

    /**
    * Write the file header info to file from m_header
    */
    NAS_INDEX_FILE_RETCODE WriteHeader();

    /**
    * Merge external archive map to this archive map
    */
    NAS_INDEX_FILE_RETCODE Merge(
        std::unordered_map<std::string, std::unordered_map<std::string, BackupIndexFileMetaInfo>> &archiveMap);

    /**
    * Get header line
    */
    std::string GetFileHeaderLine(INDEX_HEADER_INFO headerLine);

    /**
    * Write dir entry to index file
    */
    NAS_INDEX_FILE_RETCODE WriteDir();

    /**
    * Read the file header info from file and load to m_header
    */
    NAS_INDEX_FILE_RETCODE ReadHeader();
    NAS_INDEX_FILE_RETCODE FillHeader(uint32_t &headerLine, std::vector<std::string> &indexHeaderLineSplit,
        std::string &indexHeaderLine);

    /**
    * Validate header information read from the file
    */
    NAS_INDEX_FILE_RETCODE ValidateHeader();

    /**
    * Translate dir entry to string
    */
    std::string TranslateFromDirEntry(BackupIndexDirEntry &dirEntry);

    /**
    * Translate string content to dir entry
    */
    void TranslateToDirEntry(const std::vector<std::string> &fileContents, BackupIndexDirEntry &dirEntry);

    /**
    * Translate archive entry to string
    */
    std::string TranslateFromArchiveEntry(BackupIndexArchiveEntry &archiveEntry);

    /**
    * Translate string content to archive entry
    */
    void TranslateToArchiveEntry(const std::vector<std::string> &archiveContents,
        BackupIndexArchiveEntry &archiveEntry);

    /**
    * Translate file entry to string
    */
    std::string TranslateFromFileEntry(BackupIndexFileEntry &fileEntry);

    /**
    * Translate string content to file entry
    */
    void TranslateToFileEntry(const std::vector<std::string> &fileContents, BackupIndexFileEntry &fileEntry);
};
}

#endif // DME_NAS_BACKUP_INDEX_H
