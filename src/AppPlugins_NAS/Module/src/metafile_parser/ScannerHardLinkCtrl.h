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
#ifndef MODULE_SCANNER_HARDLINKCTRL_H
#define MODULE_SCANNER_HARDLINKCTRL_H

#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include "NasControlFile.h"
#include "define/Defines.h"

namespace ScannerHardLinkCtrl {
const std::string NAS_SCANNERHARDLINKCTRL_ENTRY_MODE_DATA_MODIFIED = "dm";
const std::string NAS_SCANNERHARDLINKCTRL_ENTRY_MODE_META_MODIFIED = "mm";
const std::string NAS_SCANNERHARDLINKCTRL_ENTRY_MODE_BOTH_MODIFIED = "bm";
const std::string NAS_SCANNERHARDLINKCTRL_ENTRY_MODE_DATA_DELETED = "dd";
const std::string NAS_SCANNERHARDLINKCTRL_ENTRY_MODE_ONLY_FILE_MODIFIED = "fm";
const std::string NAS_SCANNERHARDLINKCTRL_ENTRY_TYPE_FILE  = "f";
const std::string NAS_SCANNERHARDLINKCTRL_ENTRY_TYPE_INODE = "i";
constexpr uint16_t NAS_SCANNERHARDLINKCTRL_TEN_MIN_IN_SEC  = 600;

const std::string NAS_SCANNERHARDLINKCTRL_HEADER_TITLE = "NAS Scanner HardLink Control File";
const std::string NAS_SCANNERHARDLINKCTRL_HEADER_VERSION = "1.0";

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
    Stats stats {};             /* Statistics of entries in this control file */
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

struct LinkEntry {
    std::string metaFileName {};  /* Metafile where the hardlink metadata is saved */
    std::string dirName {};       /* Absolute path of parent directory */
    std::string fileName {};      /* Relative filename */
    uint32_t aclFlag = 0;         /* Boolean flag specifying whether there is acl info present in metadata */
    uint64_t metaFileOffset;      /* Offset inside the metafile to get the metadata for this hardlink */
    uint64_t metaFileReadLen;     /* Length of the metadata */
    uint64_t fileSize = 0;        /* ** Not present in control file ** to be used only to track m_dataSize */
};

struct InodeEntry {
    uint32_t linkCount = 0; /* No of links with same inode */
    uint64_t inode = 0;     /* Inode value */
};

class AGENT_API CtrlFile {
    private:
        std::mutex m_lock {};                            /* Lock */
        std::string m_ctlFileName {};               /* This controlFile name */
        std::string m_ctrlFileParentDir {};         /* Parent dir of the controlFile */
        uint32_t m_maxEntryPerFile = 0;             /* Maximum entries per control file */
        uint32_t m_minEntriesPerFile = 0;           /* Minimum entries per control file */
        Header m_header {};      /* File header info */

        std::ifstream m_readFd {};    /* Read FD */
        std::stringstream m_readBuffer {};               /* Read Buffer */
        std::ofstream m_writeFd {};   /* Write FD */
        std::stringstream m_writeBuffer {};              /* Write Buffer */

        uint32_t m_maxFileSize = 0;                 /* Maximum size of the control file */
        uint64_t m_dataSize = 0;                    /* Total data size represented by all entries in this file */
        uint64_t m_maxDataSize = 0;                 /* Maximum data size represented by all entries in this file */
        uint64_t m_minDataSize = 0;                 /* Minimum data size represented by all entries in this file */
        uint32_t m_entries = 0;                     /* Number of hard link entries in the file */

        time_t m_ctrlFileCreationTime = 0;          /* ctrl file creation time */
        uint32_t m_ctrlFileTimeElapsed = 0;         /* Time elapsed to generate control file */

        /**
         * Template to Open a File in Read/Write Mode
         */
        template<class FileStream>
        NAS_CTRL_FILE_RETCODE FileOpen(FileStream &strmFd, std::ios::openmode fileMode);

        /**
         * Validate a link-entry or inode-entry read from the file
         */
        NAS_CTRL_FILE_RETCODE ValidateEntry(std::vector<std::string> &entry, std::string &line);

        /**
         * Validate header information read from the file
         */
        NAS_CTRL_FILE_RETCODE ValidateHeader();

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
         * Translate Link entry read from file to LinkEntry sructure
         */
        void TranslateLinkEntry(std::vector<std::string> &fileContents, LinkEntry &linkEntry);
    public:
        /**
         * Contructor to be used by (producers) users for writing to the control file
         */
        explicit CtrlFile(Params params);
        /**
         * Contructor to be used by (consumers) users for reading from the control file
         */
        explicit CtrlFile(std::string ctlFileName);
        /**
         *  Destructor
         */
        ~CtrlFile();

        /**
         * Open the file (mode - NAS_CTRL_FILE_OPEN_MODE_READ/NAS_CTRL_FILE_OPEN_MODE_WRITE)
         * Producers must use mode NAS_CTRL_FILE_OPEN_MODE_WRITE
         * Consumers must use mode NAS_CTRL_FILE_OPEN_MODE_READ
         */
        NAS_CTRL_FILE_RETCODE Open(NAS_CTRL_FILE_OPEN_MODE mode);

        /**
         * Close the file
         */
        NAS_CTRL_FILE_RETCODE Close(NAS_CTRL_FILE_OPEN_MODE mode);
        /**
         * Get the file header info read from file
         */
        NAS_CTRL_FILE_RETCODE GetHeader(Header &header);
        /**
         * Read an entry (could be link entry or inode entry) from the file
         * To be used only by consumers
         * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_READ mode
         */
        NAS_CTRL_FILE_RETCODE ReadEntry(LinkEntry &linkEntry, InodeEntry &inodeEntry);
        /**
         * Write a link entry to the file
         * To be used only by producers
         * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_WRITE mode
         */
        NAS_CTRL_FILE_RETCODE WriteEntry(LinkEntry &linkEntry);
        /**
         * Write an inode entry to the file
         * To be used only by producers
         * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_WRITE mode
         */
        NAS_CTRL_FILE_RETCODE WriteInodeEntry(InodeEntry &inodeEntry);
        /**
         * Check the number of entries in the control file
         */
        uint32_t GetEntries();
        /**
         * Check whether the control file time validity has lapsed
         */
        bool CheckCtrlFileTimeElapse();
};
}



#endif // DME_NAS_SCANNER_HARDLINKCTRL_H