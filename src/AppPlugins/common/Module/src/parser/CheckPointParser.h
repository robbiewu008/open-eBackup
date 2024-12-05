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
#ifndef MODULE_CHECKPOINT_H
#define MODULE_CHECKPOINT_H

#include "FileParser.h"
#include "ParserStructs.h"
#include "define/Defines.h"


namespace Module {
    const std::string CHECKPOINT_HEADER_TITLE = "NAS Scanner Checkpoint File";
    const std::string CHECKPOINT_HEADER_VERSION = "1.0";

    class AGENT_API CheckPointParser : public FileParser {
    public:
        struct Header {
            std::string title {};                  /* Control file title */
            std::string version {};                /* Version */
            std::string timestamp {};              /* Timestamp - when the file is created */
            std::string taskId {};                 /* Task id */
            std::string backupType {};             /* Full or incremental */
            std::string nasServer {};              /* Scanned nas server ip address */
            std::string nasSharePath {};           /* Scanned nas share path */
            std::string proto {};                  /* Protocol used - nfs or cifs */
            std::string protoVersion {};           /* Protocol version */
            std::string metaDataScope {};          /* Metadata scope - folder-only or folder-and-file */
        };

        struct Params {
            uint32_t maxEntriesPerFile = 0;          /* Maximum entries per control file */
            uint64_t maxFileSize = 0;                /* Maximum size of the control file */
            std::string chkPntFileName {};           /* Control file title */
            std::string taskId {};                   /* Task id */
            std::string backupType {};               /* Full or incremental */
            std::string nasServer {};                /* Scanned nas server ip address */
            std::string nasSharePath {};             /* Scanned nas share path */
            std::string proto {};                    /* Protocol used - nfs or cifs */
            std::string protoVersion {};             /* Protocol version */
            std::string metaDataScope {};            /* Metadata scope - folder-only or folder-and-file */
        };

        enum CHECKPOINT_HEADER_INFO {
            TITLE = 0,
            HEADER_VERSION = 1,
            TIMESTAMP = 2,
            TASKID = 3,
            TASKTYPE = 4,
            NASSERVER = 5,
            NASSHARE = 6,
            PROTOCOL = 7,
            PROTOCOL_VERSION = 8,
            METADATA_SCOPE = 9,
            RESERVED_1 = 10,
            RESERVED_2 = 11,
            RESERVED_3 = 12,
            RESERVED_4 = 13,
            RESERVED_5 = 14,
        };

        /**
        * Contructor to be used by (producers) users for writing to the control file
        */
        explicit CheckPointParser(CheckPointParser::Params params);

        /**
        * Contructor to be used by (consumers) users for reading from the control file
        */
        explicit CheckPointParser(const std::string& chkPntFileName);

        /**
        *  Destructor
        */
        ~CheckPointParser();

        /**
        * Get the file header info read from file
        */
        CTRL_FILE_RETCODE GetHeader(CheckPointParser::Header &header);

        /**
        * Write a directory path to the file
        * To be used only by producers
        * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_WRITE mode
        */
        CTRL_FILE_RETCODE WriteChkPntEntry(const std::string& chkPntEntry);

        /**
        * Read a specified number of directory path entries from the file
        * To be used only by consumers
        * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_READ mode
        */
        std::vector<std::string> ReadMultipleChkPntEntries(int maxReadCnt);

        /**
        * Read all directory path entries from the file
        * To be used only by consumers
        * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_READ mode
        */
        std::vector<std::string> ReadAllChkPntEntries();

    private:
        CheckPointParser::Header m_header {};               /* File header info */

        uint32_t m_entries = 0;                            /* Number of dir path entries in the file */
        uint32_t m_maxEntriesPerFile = 0;                  /* Maximum entries per control file */
        uint32_t m_maxFileSize = 0;                        /* Maximum size of the control file */

        CTRL_FILE_RETCODE OpenWrite() override;
        CTRL_FILE_RETCODE CloseWrite() override;
        CTRL_FILE_RETCODE FlushToFile() override;

        /**
        * Template to Open a File in Read/Write Mode
        */
        template<class FileStream>
        CTRL_FILE_RETCODE FileOpen(FileStream &strmFd, std::ios::openmode fileMode);

        /**
        * Write the file header info to file from m_header
        */
        CTRL_FILE_RETCODE WriteHeader();

        /**
        * Get the line to write in header info of file
        */
        std::string GetFileHeaderLine(uint32_t headerLine);

        /**
        * Read the file header info from file and load to m_header
        */
        CTRL_FILE_RETCODE ReadHeader();
        CTRL_FILE_RETCODE FillHeader(uint32_t &headerLine, std::vector<std::string> &ctlHeaderLineSplit,
            std::string &ctlHeaderLine);

        /**
        * Validate header information read from the file
        */
        CTRL_FILE_RETCODE ValidateHeader();
    };
}

#endif // MODULE_CHECKPOINT_H