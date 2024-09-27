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
#ifndef MODULE_FILE_MAP_H
#define MODULE_FILE_MAP_H

#include <string>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include "NasControlFile.h"

const std::string NAS_FILEMAP_HEADER_VERSION = "1.0";
const std::string NAS_FILEMAP_ENTRY_DELIMITER  = ",";

struct FileMapStats {
    uint64_t noOfDirEntries = 0;        /* Num of dir entries in this file map */
    uint64_t noOfFileEntries = 0;       /* Num of file entries in this file map */
};

struct FileMapHeader {
    std::string title;                  /* File map title */
    std::string version;                /* Version */
    std::string timestamp;              /* Timestamp - when the file is created */
    std::string taskId;                 /* Task id */
    std::string nasServer;              /* Scanned nas server ip address */
    std::string nasSharePath;           /* Scanned nas share path */
    std::string proto;                  /* Protocol used - nfs or cifs */
    std::string protoVersion;           /* Protocol version */
    FileMapStats stats;                 /* Statistics of entries in this control file */
};

enum FILEMAP_HEADER_INFO {
    FILEMAP_TITLE = 0,
    FILEMAP_HEADER_VERSION = 1,
    FILEMAP_TIMESTAMP = 2,
    FILEMAP_TASKID = 3,
    FILEMAP_NASSERVER = 4,
    FILEMAP_NASSHARE = 5,
    FILEMAP_PROTOCOL = 6,
    FILEMAP_PROTOCOL_VERSION = 7,
    FILEMAP_DIR_COUNT = 8,
    FILEMAP_FILE_COUNT = 9,
    FILEMAP_RESERVED_1 = 10,
    FILEMAP_RESERVED_2 = 11,
    FILEMAP_RESERVED_3 = 12,
    FILEMAP_RESERVED_4 = 13,
    FILEMAP_RESERVED_5 = 14,
};

enum FILEMAP_HEADER_ENTRY_OFFSET {
    FILEMAP_HEADER_ENTRY_OFFSET_KEY = 0,
    FILEMAP_HEADER_ENTRY_OFFSET_VALUE = 1
};

enum FILEMAP_ENTRY_OP_TYPE {
    FILEMAP_ENTRY_OP_FULL = 0,
    FILEMAP_ENTRY_OP_INC = 1
};

enum FILEMAP_MERGE_TYPE {
    FILEMAP_MERGE_NEW = 0,
    FILEMAP_MERGE_UPDATE = 1
};

enum FILEMAP_ENTRY_OFFSET {
    FILEMAP_ENTRY_DIRNAME = 0,
    FILEMAP_ENTRY_FILENAME = 1
};

struct FileMapEntry {
    std::string absPath;                /* Absolute path of parent directory */
    std::string mapFileName;                /* map file name */
};

struct FileMapParams {
    uint32_t maxEntriesPerFile = 0;     /* Max number of entries that can be written in this file */
    std::string title;                  /* File map title */
    std::string fileMapName;            /* File map name */
    std::string tmpFileMapName;         /* Temporary map file name */
    std::string taskId;                 /* Task Id */
    std::string nasServer;              /* Scanned nas server ip address */
    std::string nasSharePath;           /* Scanned nas share path */
    std::string proto;                  /* protocol used - nfs or cifs */
    std::string protoVersion;           /* protocol version */
};

class FileMap {
private:
    std::mutex m_lock {};                            /* Lock */
    std::string m_fileMapName;                  /* This file map name */
    uint32_t m_maxEntryPerFile = 0;             /* Maximum entries per control file */
    FileMapHeader m_header {};	                /* File header info */

    std::ifstream m_readFd {};    /* Read FD */
    std::stringstream m_readBuffer {};               /* Read Buffer */
    std::ofstream m_writeFd {};   /* Write FD */
    std::stringstream m_writeBuffer {};              /* Write Buffer */

    uint32_t m_entries = 0;                     /* Number of file/dir entries in the file */

    std::unordered_map<std::string, std::unordered_set<std::string>> m_fileMap;

    /**
    * Open a File in READ Mode
    */
    NAS_FILEMAP_RETCODE OpenRead();

    /**
    * Open a File in WRITE Mode
    */
    NAS_FILEMAP_RETCODE OpenWrite();

    /**
    * Open a File in UPDATE Mode
    */
    NAS_FILEMAP_RETCODE OpenUpdate();

    /**
    * Template to Open a File in Read/Write Mode
    */
    template<class FileStream>
    NAS_CTRL_FILE_RETCODE FileOpen(FileStream &strmFd, std::string fileName, std::ios::openmode fileMode);

    /**
     * Read the file header info from file and load to m_header
     */
    NAS_FILEMAP_RETCODE ReadHeader();
    NAS_FILEMAP_RETCODE FillHeader(uint32_t &headerLine, std::vector<std::string> &headerLineSplit, std::string &mapFileHeaderLine);

    /**
     * Write the file header info to file from m_header
     */
    NAS_FILEMAP_RETCODE WriteHeader();

    /**
     * Get the line to write in header info of file
     */
    std::string GetFileHeaderLine(uint32_t headerLine);

    /**
     * Validate header information read from the file
     */
    NAS_FILEMAP_RETCODE ValidateHeader();

    /**
     * Translate FileMapEntry structure to file map line string
     */
    std::string TranslateFromFileMapEntry(FileMapEntry &fileMapEntry);

    /**
     * Translate file map entry read from file to FileMapEntry structure
     */
    void TranslateToFileMapEntry(std::vector<std::string> &dirEntryStringFromFile, FileMapEntry &fileMapEntry);

    /**
     * Merge external file map with self file map.
     */
    NAS_FILEMAP_RETCODE Merge(std::unordered_map<std::string, std::unordered_set<std::string>> &fileMap);

    /**
     * Merge a list of file maps with this object.
     */
    NAS_FILEMAP_RETCODE Merge(std::vector<std::string> &fileMapList);

public:

    /**
     * Contructor to be used by (producers) users for writing to the file map
     */
    explicit FileMap(const FileMapParams &params);

    /**
     * Contructor to be used by (consumers) users for reading from the file map
     */
    explicit FileMap(const std::string &fileMapName);

    /**
     *  Destructor
     */
    ~FileMap();

    /**
     * Open the file
     * Producers must use mode NAS_FILEMAP_OPEN_MODE_WRITE/NAS_FILEMAP_OPEN_MODE_UPDATE
     * Consumers must use mode NAS_FILEMAP_OPEN_MODE_READ
     */
    NAS_FILEMAP_RETCODE Open(NAS_FILEMAP_OPEN_MODE mode);

    /**
     * Close the file
     */
    NAS_FILEMAP_RETCODE Close(NAS_FILEMAP_OPEN_MODE mode);

    /**
     * Get the file map header info read from file
     */
    NAS_FILEMAP_RETCODE GetHeader(FileMapHeader &header);

    /**
     * Read an entry (could be dir entry or file entry) from the file
     * To be used only by consumers
     * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_READ mode
     */
    NAS_FILEMAP_RETCODE ReadEntry(FileMapEntry &fileMapEntry);

    /**
     * Write a file entry to the file
     * To be used by Unload / producer
     * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_WRITE mode
     */
    NAS_FILEMAP_RETCODE WriteEntry(FileMapEntry &fileMapEntry);

    /**
     * Load file into in-memory map
     */
    NAS_FILEMAP_RETCODE UpdateEntry(FileMapEntry &fileMapEntry);

    /**
     * Check the number of entries in the file map
     */
    uint32_t GetEntries();

    /**
     * Merge with NEW mode or UPDATE mode
     * for UPDATE mode, the caller should give the tmpFileMapName
     */
    NAS_FILEMAP_RETCODE Merge(std::vector<std::string> &fileMapList, FILEMAP_MERGE_TYPE mergeType);

    /**
     * Load file into in-memory map
     */
    NAS_FILEMAP_RETCODE Load();

    /**
     * Load file into in-memory map
     */
    NAS_FILEMAP_RETCODE Unload();

    /**
     * Load file into in-memory map
     */
    NAS_FILEMAP_RETCODE GetFileMap(std::unordered_map<std::string, std::unordered_set<std::string>> &fileMap);
};

#endif // DME_NAS_FILE_MAP_H