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
#ifndef MODULE_DELETECTRL_H
#define MODULE_DELETECTRL_H
#include "define/Defines.h"
#include "define/Types.h"
#include "ParserStructs.h"
#include "FileParser.h"
#include "define/Defines.h"

namespace Module {

const std::string DELETECTRL_HEADER_TITLE = "NAS Backup Delete Control File";
const std::string DELETECTRL_HEADER_VERSION = "1.0";

const std::string DELETECTRL_ENTRY_TYPE_DIR  = "d";
const std::string DELETECTRL_ENTRY_TYPE_FILE  = "f";

class  AGENT_API DeleteCtrlParser : public FileParser {
public:
    struct Stats {
        uint64_t noOfDelFiles = 0;  /* Num of directories to be deleted in the control file */
        uint64_t noOfDelDirs = 0;   /* Num of files to be deleted in the control file */
    };

    struct Header {
        std::string title;                  /* Control file title */
        std::string version;                /* Version */
        std::string timestamp;              /* Timestamp - when the file is created */
        std::string taskId;                 /* Task id */
        std::string nasServer;              /* Scanned nas server ip address */
        std::string nasSharePath;           /* Scanned nas share path */
        std::string proto;                  /* Protocol used - nfs or cifs */
        std::string protoVersion;          /* Protocol version */
        DeleteCtrlParser::Stats stats;        /* Statistics of entries in this control file */
    };

    enum class DELETE_CTRL_HEADER_INFO {
        DELETE_CTRL_TITLE = 0,
        DELETE_CTRL_HEADER_VERSION = 1,
        DELETE_CTRL_TIMESTAMP = 2,
        DELETE_CTRL_TASKID = 3,
        DELETE_CTRL_NASSERVER = 4,
        DELETE_CTRL_NASSHARE = 5,
        DELETE_CTRL_PROTOCOL = 6,
        DELETE_CTRL_PROTOCOL_VERSION = 7,
        DELETE_CTRL_DIRS_TO_DELETE_COUNT = 8,
        DELETE_CTRL_FILE_TO_DELETE_COUNT = 9,
        DELETE_CTRL_RESERVED_1 = 10,
        DELETE_CTRL_RESERVED_2 = 11,
        DELETE_CTRL_RESERVED_3 = 12,
        DELETE_CTRL_RESERVED_4 = 13,
        DELETE_CTRL_RESERVED_5 = 14,
    };

    struct Params {
        uint32_t maxEntriesPerFile = 0;     /* Max number of entries that can be written in this file */
        std::string m_ctlFileName {};       /* Control file title */
        std::string taskId {};              /* Task Id */
        std::string nasServer {};           /* Scanned nas server ip address */
        std::string nasSharePath {};        /* Scanned nas share path */
        std::string proto {};               /* protocol used - nfs or cifs */
        std::string protoVersion {};       /* protocol version */
    };
    /**
     * Contructor to be used by (producers) users for writing to the control file
     */
    explicit DeleteCtrlParser(DeleteCtrlParser::Params params);

    /**
     * Contructor to be used by (consumers) users for reading from the control file
     */
    explicit DeleteCtrlParser(std::string ctlFileName);

    /**
     *  Destructor
     */
    ~DeleteCtrlParser();

    /**
     * Get the file header info read from file
     */
    CTRL_FILE_RETCODE GetHeader(DeleteCtrlParser::Header &header);

    /**
     * Read an entry (could be dir entry or file entry) from the file
     * To be used only by consumers
     * File must be Opened with CTRL_FILE_OPEN_MODE_READ mode
     */
    CTRL_FILE_RETCODE ReadEntry(DeleteCtrlEntry &deleteEntry, std::string &m_fileName);

    /**
     * Write a directory entry to the file
     * To be used only by producers
     * File must be Opened with CTRL_FILE_OPEN_MODE_WRITE mode
     */
    CTRL_FILE_RETCODE WriteDirEntry(DeleteCtrlEntry &deleteEntry);

    /**
     * Write a file entry to the file
     * To be used only by producers
     * File must be Opened with CTRL_FILE_OPEN_MODE_WRITE mode
     */
    CTRL_FILE_RETCODE WriteFileEntry(std::string  &m_fileName);

    /**
     * Check the number of entries in the control file
     */
    uint32_t GetEntries();

    /**
     * Get the control file name
     */
    std::string GetCtrlFileName();

private:
    uint32_t m_maxEntryPerFile = 0;             /* Maximum entries per control file */
    DeleteCtrlParser::Header m_header {};	        /* File header info */

    uint32_t m_maxFileSize = 0;	                /* Maximum size of the control file */
    uint32_t m_entries = 0;                     /* Number of file/dir entries in the file */

    CTRL_FILE_RETCODE OpenWrite() override;
    CTRL_FILE_RETCODE CloseWrite() override;
    CTRL_FILE_RETCODE FlushToFile() override;
    /**
     * Validate a dir-entry or file-entry read from the file
     */
    CTRL_FILE_RETCODE ValidateEntry(std::vector<std::string> entry, std::string line);

    /**
     * Validate header information read from the file
     */
    CTRL_FILE_RETCODE ValidateHeader();

    /**
     * Read the file header info from file and load to m_header
     */
    CTRL_FILE_RETCODE ReadHeader();
    CTRL_FILE_RETCODE FillHeader(uint32_t &headerLine, std::vector<std::string> &cltHeaderLineSplit, std::string &cltHeaderLine);

    /**
     * Write the file header info to file from m_header
     */
    CTRL_FILE_RETCODE WriteHeader();

    /**
     * Get the line to write in header info of file
     */
    std::string GetFileHeaderLine(uint32_t headerLine);

    /**
     * Translate dir entry read from file to DeleteCtrlEntry sructure
     */
    void TranslateDirEntry(std::vector<std::string> &dirEntryStringFromFile, DeleteCtrlEntry &deleteEntry);

    void PrintDirEntry(DeleteCtrlEntry& deleteEntry);

    void PrintFileEntry(std::string fileName);
};

}
#endif // FS_BACKUP_DELETECTRL_H