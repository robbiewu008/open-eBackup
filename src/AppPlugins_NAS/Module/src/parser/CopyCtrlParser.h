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
#ifndef MODULE_BACKUPCTRL_H
#define MODULE_BACKUPCTRL_H

#include "ParserStructs.h"
#include "FileParser.h"
#include "define/Defines.h"
#include "define/Types.h"

namespace Module {

const std::string CTRL_HEADER_TITLE = "NAS Scanner Backup Control File";
const std::string CTRL_HEADER_VERSION_V10 = "1.0";
const std::string CTRL_HEADER_VERSION = "2.0";

enum HEADER_INFO {
    TITLE = 0,
    HEADER_VERSION = 1,
    TIMESTAMP = 2,
    TASKID = 3,
    TASKTYPE = 4,
    NASSERVER = 5,
    NASSHARE = 6,
    CTRL_PROTOCOL = 7,
    CTRL_PROTOCOL_VERSION = 8,
    METADATA_SCOPE = 9,
    DIR_COUNT = 10,
    FILE_COUNT = 11,
    DATA_SIZE = 12,
    RESERVED_1 = 13,
    RESERVED_2 = 14,
    RESERVED_3 = 15,
    RESERVED_4 = 16,
    RESERVED_5 = 17,
};

class AGENT_API CopyCtrlParser : public FileParser {
public:
    struct Stats {
        uint64_t noOfDirs = 0;              /* Num of dir entries in the control file */
        uint64_t noOfFiles = 0;             /* Num of file entries in the control file */
        uint64_t dataSize = 0;              /* Total data size represented by all entries in this control file */
    };

    struct Header {
        std::string title {};                  /* Control file title */
        std::string version {};                /* Version */
        std::string timestamp {};              /* Timestamp - when the file is created */
        std::string taskId {};                 /* Task id */
        std::string backupType {};             /* Full or incremental */
        std::string nasServer {};              /* Scanned nas server ip address */
        std::string nasSharePath {};           /* Scanned nas share path */
        std::string proto {};                  /* Protocol used - nfs or cifs */
        std::string protoVersion {};          /* Protocol version */
        std::string metaDataScope {};          /* Metadata scope - folder-only or folder-and-file */
        CopyCtrlParser::Stats stats {};       /* Statistics of entries in this control file */
    };

    struct Params {
        uint32_t maxEntriesPerFile = 0;     /* Max number of entries that can be written in this file */
        uint32_t minEntriesPerFile = 0;     /* Min number of entries that can be written in this file */
        uint32_t m_ctrlFileTimeElapsed = 0; /* Time elapsed to generate control file */
        uint32_t maxCtrlFileSize = 0;       /* Max size of copy control file */
        uint64_t maxDataSize = 0;           /* Max data size that can be written in this file */
        uint64_t minDataSize = 0;           /* Max data size that can be written in this file */
        std::string m_ctlFileName {};       /* Control file title */
        std::string taskId {};              /* Task id */
        std::string backupType {};          /* Full or incremental */
        std::string nasServer {};           /* Scanned nas server ip address */
        std::string nasSharePath {};        /* Scanned nas share path */
        std::string proto {};               /* Protocol used - nfs or cifs */
        std::string protoVersion {};       /* Protocol version */
        std::string metaDataScope {};       /* Metadata scope - folder-only or folder-and-file */
    };

    /**
        * Contructor to be used by (producers) users for writing to the control file
        */
    explicit CopyCtrlParser(CopyCtrlParser::Params params);

    /**
        * Contructor to be used by (consumers) users for reading from the control file
        */
    explicit CopyCtrlParser(std::string ctlFileName);

    /**
        *  Destructor
        */
    ~CopyCtrlParser();

    /**
        * Get the file header info read from file
        */
    CTRL_FILE_RETCODE GetHeader(CopyCtrlParser::Header &header);

    /**
        * Read an entry (could be dir entry or file entry) from the file
        * To be used only by consumers
        * File must be Opened with CTRL_FILE_OPEN_MODE_READ mode
        */
    CTRL_FILE_RETCODE ReadEntry(CopyCtrlFileEntry &fileEntry, CopyCtrlDirEntry &dirEntry);

    /**
        * Write an directory entry to the file
        * To be used only by producers
        * File must be Opened with CTRL_FILE_OPEN_MODE_WRITE mode
        */
    CTRL_FILE_RETCODE WriteDirEntry(CopyCtrlDirEntry &dirEntry);

    /**
        * Write an file entry to the file
        * To be used only by producers
        * File must be Opened with CTRL_FILE_OPEN_MODE_WRITE mode
        */
    CTRL_FILE_RETCODE WriteFileEntry(CopyCtrlFileEntry &fileEntry);

    /**
        * Check whether the file buffer is empty
        */
    bool IsFileBufferEmpty();

    /**
        * Check the number of entries in the control file
        */
    uint32_t GetEntries();

    /**
        * Check whether the control file time validity has lapsed
        */
    bool CheckCtrlFileTimeElapse();

private:
    uint32_t m_maxEntryPerFile = 0;             /* Maximum entries per control file */
    uint32_t m_minEntriesPerFile = 0;           /* Minimum entries per control file */
    CopyCtrlParser::Header m_header {};        /* File header info */
    std::stringstream m_writeFileBuffer {};          /* Temporary write buffer to hold file entries for a dir entry */

    uint32_t m_maxFileSize = 0;                 /* Maximum size of the control file */
    uint64_t m_maxDataSize = 0;                 /* Maximum data size represented by all entries in this file */
    uint64_t m_minDataSize = 0;                 /* Minimum data size represented by all entries in this file */
    uint64_t m_dataSize = 0;                    /* Total data size represented by all entries in this file */
    uint32_t m_entries = 0;                     /* Number of file/dir entries in the file */
    uint32_t m_fileCount = 0;                   /* To track filecount per dir. Will be reset on each dir update*/

    time_t m_ctrlFileCreationTime = 0;         /* Ctrl file creation time */
    uint32_t m_ctrlFileTimeElapsed = 0;         /* Time elapsed since control file created */
    std::string m_metaFileName {};              /* Current Meta File Name */

    CTRL_FILE_RETCODE OpenWrite() override;
    CTRL_FILE_RETCODE CloseWrite() override;
    CTRL_FILE_RETCODE FlushToFile() override;
    /**
        * Validate a dir-entry or file-entry read from the file
        */
    CTRL_FILE_RETCODE ValidateEntry(std::vector<std::string> &entry, const std::string &line);
    CTRL_FILE_RETCODE ValidateDirEntry(std::vector<std::string> &lineContents, const std::string &line);
    CTRL_FILE_RETCODE ValidateFileEntry(std::vector<std::string> &lineContents, const std::string &line);

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
        CTRL_FILE_RETCODE FillHeader2(uint32_t &headerLine, std::vector<std::string> &cltHeaderLineSplit,
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
        * Translate dir entry read from file to CopyCtrlDirEntry sructure
        */
    void TranslateDirEntry(std::vector<std::string> &dirEntryStringFromFile, CopyCtrlDirEntry &dirEntry);
    void TranslateDirEntryV10(std::vector<std::string> &dirEntryStringFromFile, CopyCtrlDirEntry &dirEntry);

    /**
        * Translate file entry read from file to CopyCtrlFileEntry sructure
        */
    void TranslateFileEntry(std::vector<std::string> &fileEntryStringFromFile, CopyCtrlFileEntry &fileEntry);
    void TranslateFileEntryV10(std::vector<std::string> &fileEntryStringFromFile,
        CopyCtrlFileEntry &fileEntry);
    CTRL_FILE_RETCODE ReadEntryWithLine(CopyCtrlFileEntry &fileEntry, CopyCtrlDirEntry &dirEntry,
        const std::string& ctlFileLine);
    void HandleBreakLine(std::string& ctlFileLine);
    bool IsNormalEntry(const std::string& fileLine);
    

    /**
        * Get metafile name from metafile index
        */
    std::string GetMetaFileName(uint16_t metaFileIndex);

    void PrintCopyCtrlDirEntry(CopyCtrlDirEntry& dirEntry);
    void PrintCopyCtrlFileEntry(CopyCtrlFileEntry& fileEntry);
};

}

#endif // FS_SCANNER_BACKUPCTRL_H
