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
#ifndef MODULE_HARDLINKCTRL_H
#define MODULE_HARDLINKCTRL_H

#include <queue>
#include "ParserStructs.h"
#include "FileParser.h"
#include "define/Defines.h"
namespace Module {

const std::string HARDLINKCTRL_HEADER_TITLE = "NAS Scanner HardLink Control File";
const std::string HARDLINKCTRL_HEADER_VERSION = "1.0";

class  AGENT_API HardlinkCtrlParser : public FileParser {
public:
    struct Stats {
        uint64_t noOfFiles = 0; /* Num of hardlink file entries in the control file */
        uint64_t dataSize = 0;  /* Total data size represented by all hardlink entries in this control file */
    };

    struct Header {
        std::string title {};                          /* Control file title */
        std::string version {};                        /* Version */
        std::string timestamp {};                      /* Timestamp - when the file is created */
        std::string taskId {};                         /* Task id */
        std::string backupType {};                     /* Full or incremental */
        std::string nasServer {};                      /* Scanned nas server ip address */
        std::string nasSharePath {};                   /* Scanned nas share path */
        std::string proto {};                          /* Protocol used - nfs or cifs */
        std::string protoVersion {};                  /* Protocol version */
        HardlinkCtrlParser::Stats stats {};             /* Statistics of entries in this control file */
    };

    enum HARDLINK_HEADER_INFO {
        HLINK_TITLE = 0,
        HLINK_HEADER_VERSION = 1,
        HLINK_TIMESTAMP = 2,
        HLINK_TASKID = 3,
        HLINK_TASKTYPE = 4,
        HLINK_NASSERVER = 5,
        HLINK_NASSHARE = 6,
        HLINK_PROTOCOL = 7,
        HLINK_PROTOCOL_VERSION = 8,
        HLINK_FILE_COUNT = 9,
        HLINK_DATA_SIZE = 10,
    };

    struct Params {
        uint32_t maxEntriesPerFile = 0;       /* Max number of entries that can be written in this file */
        uint32_t minEntriesPerFile = 0;     /* Min number of entries that can be written in this file */
        uint32_t ctrlFileTimeElapsed = 0;   /* Time elapsed to generate control file */
        uint32_t maxCtrlFileSize = 0;       /* Max size of copy control file */
        uint64_t maxDataSize = 0;             /* Max data size that can be written in this file */
        uint64_t minDataSize = 0;           /* Max data size that can be written in this file */
        std::string ctlFileName {};         /* Control file title */
        std::string taskId {};                /* Task id */
        std::string backupType {};            /* Full or incremental */
        std::string nasServer {};             /* Scanned nas server ip address */
        std::string nasSharePath {};          /* Scanned nas share path */
        std::string proto {};                 /* Protocol used - nfs or cifs */
        std::string protoVersion {};         /* Protocol version */
    };

    /**
        * Contructor to be used by (producers) users for writing to the control file
        */
    explicit HardlinkCtrlParser(HardlinkCtrlParser::Params params);
    /**
        * Contructor to be used by (consumers) users for reading from the control file
        */
    explicit HardlinkCtrlParser(std::string ctlFileName);
    /**
        *  Destructor
        */
    ~HardlinkCtrlParser();

    /**
        * Get the file header info read from file
        */
    CTRL_FILE_RETCODE GetHeader(Header &header);
    /**
        * Read an entry (could be link entry or inode entry) from the file
        * To be used only by consumers
        * File must be Opened with CTRL_FILE_OPEN_MODE_READ mode
        */
    CTRL_FILE_RETCODE ReadEntry(HardlinkCtrlEntry &linkEntry, HardlinkCtrlInodeEntry &inodeEntry);
    /**
        * Write a link entry to the file
        * To be used only by producers
        * File must be Opened with CTRL_FILE_OPEN_MODE_WRITE mode
        */
    CTRL_FILE_RETCODE WriteEntry(HardlinkCtrlEntry &linkEntry);
    /**
        * Write an inode entry to the file
        * To be used only by producers
        * File must be Opened with CTRL_FILE_OPEN_MODE_WRITE mode
        */
    CTRL_FILE_RETCODE WriteInodeEntry(HardlinkCtrlInodeEntry &inodeEntry);
    /**
        * Check the number of entries in the control file
        */
    uint32_t GetEntries();
    /**
        * Check whether the control file time validity has lapsed
        */
    bool CheckCtrlFileTimeElapse();

    CTRL_FILE_RETCODE ReadFileForCheckPoint(std::queue<HardlinkFileCache> &hardLinkFCacheQue,
        std::vector<std::pair<std::string, uint32_t>> &hardlinkFilesCntList);

private:
    uint32_t m_maxEntryPerFile = 0;             /* Maximum entries per control file */
    uint32_t m_minEntriesPerFile = 0;           /* Minimum entries per control file */
    Header m_header {};      /* File header info */

    uint32_t m_maxFileSize = 0;                 /* Maximum size of the control file */
    uint64_t m_dataSize = 0;                    /* Total data size represented by all entries in this file */
    uint64_t m_maxDataSize = 0;                 /* Maximum data size represented by all entries in this file */
    uint64_t m_minDataSize = 0;                 /* Minimum data size represented by all entries in this file */
    uint32_t m_entries = 0;                     /* Number of hard link entries in the file */

    time_t m_ctrlFileCreationTime = 0;          /* ctrl file creation time */
    uint32_t m_ctrlFileTimeElapsed = 0;         /* Time elapsed to generate control file */

    CTRL_FILE_RETCODE OpenWrite() override;
    CTRL_FILE_RETCODE CloseWrite() override;
    CTRL_FILE_RETCODE FlushToFile() override;
    /**
        * Template to Open a File in Read/Write Mode
        */
    template<class FileStream>
    CTRL_FILE_RETCODE FileOpen(FileStream &strmFd, std::ios::openmode fileMode);

    /**
        * Validate a link-entry or inode-entry read from the file
        */
    CTRL_FILE_RETCODE ValidateEntry(std::vector<std::string> &entry, std::string &line);

    /**
        * Validate header information read from the file
        */
    CTRL_FILE_RETCODE ValidateHeader();

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
        * Translate Link entry read from file to HardlinkCtrlEntry sructure
        */
    void TranslateLinkEntry(std::vector<std::string> &fileContents, HardlinkCtrlEntry &linkEntry);

    void PrintHardlinkEntry(HardlinkCtrlEntry& linkEntry);

    void PrintHardlinkInodeEntry(HardlinkCtrlInodeEntry& linkInodeEntry);
};

}
#endif // FS_SCANNER_HARDLINKCTRL_H