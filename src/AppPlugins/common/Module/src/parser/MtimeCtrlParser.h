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
#ifndef MODULE_MTIMECTRL_H
#define MODULE_MTIMECTRL_H

#include "ParserStructs.h"
#include "FileParser.h"
#include "define/Defines.h"

namespace Module {

const std::string MTIMECTRL_HEADER_TITLE = "NAS Backup Mtime Control File";
const std::string MTIMECTRL_HEADER_VERSION = "1.0";

enum class MTIME_CTRL_HEADER_INFO {
    MTIME_CTRL_TITLE = 0,
    MTIME_CTRL_HEADER_VERSION = 1,
    MTIME_CTRL_TIMESTAMP = 2,
    MTIME_CTRL_TASKID = 3,
    MTIME_CTRL_TASKTYPE = 4,
    MTIME_CTRL_NASSERVER = 5,
    MTIME_CTRL_NASSHARE = 6,
    MTIME_CTRL_PROTOCOL = 7,
    MTIME_CTRL_PROTOCOL_VERSION = 8,
    MTIME_CTRL_DIR_MTIME_COUNT = 9,
    MTIME_CTRL_RESERVED_1 = 10,
    MTIME_CTRL_RESERVED_2 = 11,
    MTIME_CTRL_RESERVED_3 = 12,
    MTIME_CTRL_RESERVED_4 = 13,
    MTIME_CTRL_RESERVED_5 = 14,
};

class AGENT_API MtimeCtrlParser : public FileParser {
public:
    struct Stats {
        uint64_t noOfDirs = 0;           /* Num of directories whose atime and mtime changed in the control file */
    };

    struct Params {
        uint32_t maxEntriesPerFile = 0;     /* Max number of entries that can be written in this file */
        std::string m_ctlFileName {};       /* Control file title */
        std::string taskId {};              /* Task Id */
        std::string backupType {};          /* Full or incremental */
        std::string nasServer {};           /* Scanned nas server ip address */
        std::string nasSharePath {};        /* Scanned nas share path */
        std::string proto {};               /* protocol used - nfs or cifs */
        std::string protoVersion {};       /* protocol version */
    };

    struct Header {
        std::string title;                  /* Control file title */
        std::string version;                /* Version */
        std::string timestamp;              /* Timestamp - when the file is created */
        std::string taskId;                 /* Task id */
        std::string backupType;             /* Backup type */
        std::string nasServer;              /* Scanned nas server ip address */
        std::string nasSharePath;           /* Scanned nas share path */
        std::string proto;                  /* Protocol used - nfs or cifs */
        std::string protoVersion;          /* Protocol version */
        MtimeCtrlParser::Stats stats;         /* Statistics of entries in this control file */
    };
    /**
        * Contructor to be used by (producers) users for writing to the control file
        */
    explicit MtimeCtrlParser(MtimeCtrlParser::Params params);

    /**
        * Contructor to be used by (consumers) users for reading from the control file
        */
    explicit MtimeCtrlParser(std::string ctlFileName);

    /**
        *  Destructor
        */
    ~MtimeCtrlParser();

    /**
        * Get the file header info read from file
        */
    CTRL_FILE_RETCODE GetHeader(MtimeCtrlParser::Header &header);

    /**
        * Read an entry from the file
        * To be used only by consumers
        * File must be Opened with CTRL_FILE_OPEN_MODE_READ mode
        */
    CTRL_FILE_RETCODE ReadEntry(MtimeCtrlEntry &mtimeEntry);

    /**
        * Write a directory entry to the file
        * To be used only by producers
        * File must be Opened with CTRL_FILE_OPEN_MODE_WRITE mode
        */
    CTRL_FILE_RETCODE WriteEntry(MtimeCtrlEntry &mtimeEntry);

    /**
        * Check the number of entries in the control file
        */
    uint32_t GetEntries();

    /**
        * Get the ctrl file name
        */
    std::string GetCtrlFileName();

private:
    uint32_t m_maxEntryPerFile = 0;             /* Maximum entries per control file */
    MtimeCtrlParser::Header m_header {};            /* File header info */

    uint32_t m_maxFileSize = 0;                 /* Maximum size of the control file */
    uint32_t m_entries = 0;                     /* Number of file/dir entries in the file */

    CTRL_FILE_RETCODE OpenWrite() override;
    CTRL_FILE_RETCODE CloseWrite() override;
    CTRL_FILE_RETCODE FlushToFile() override;
    /**
        * Validate a dir-entry read from the file
        */
    CTRL_FILE_RETCODE ValidateEntry(const std::vector<std::string> &lineContents, const std::string &line) const;

    /**
        * Validate header information read from the file
        */
    CTRL_FILE_RETCODE ValidateHeader() override;

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
        * Translate dir entry read from file to MtimeCtrlEntry sructure
        */
    void TranslateEntry(std::vector<std::string> &lineContents, MtimeCtrlEntry &mtimeEntry);

    void PrintEntry(MtimeCtrlEntry& mtimeEntry);

    /**
     * 处理存在换行符的名称，直到获取到正常行为止
     */
    CTRL_FILE_RETCODE HandleBreakLine(std::vector<std::string> &lineContents, std::string& ctlFileLine);

    /**
     * 判断是否是有效的entry
     */
    bool IsNormalEntry(std::vector<std::string> &lineContents, const std::string& ctlFileLine) const;
};

}
#endif // FS_BACKUP_MTIMECTRL_H