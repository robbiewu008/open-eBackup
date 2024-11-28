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
#ifndef MODULE_SCANNER_BACKUPCTRL_H
#define MODULE_SCANNER_BACKUPCTRL_H

#include <mutex>
#include <string>
#include <vector>
#include <fstream>
#include "NasControlFile.h"
#include "define/Defines.h"


const std::string NAS_SCANNERBACKUPCTRL_ENTRY_MODE_DATA_MODIFIED = "dm";
const std::string NAS_SCANNERBACKUPCTRL_ENTRY_MODE_META_MODIFIED = "mm";
const std::string NAS_SCANNERBACKUPCTRL_ENTRY_MODE_BOTH_MODIFIED = "bm";
const std::string NAS_SCANNERBACKUPCTRL_ENTRY_MODE_DATA_DELETED = "dd";
const std::string NAS_SCANNERBACKUPCTRL_ENTRY_MODE_ONLY_FILE_MODIFIED = "fm";
const std::string NAS_SCANNERBACKUPCTRL_ENTRY_TYPE_DIR  = "d";
const std::string NAS_SCANNERBACKUPCTRL_ENTRY_TYPE_FILE  = "f";
constexpr uint16_t NAS_SCANNERBACKUPCTRL_TEN_MIN_IN_SEC  = 600;

const std::string NAS_SCANNERBACKUPCTRL_HEADER_TITLE = "NAS Scanner Backup Control File";
const std::string NAS_SCANNERBACKUPCTRL_HEADER_VERSION_V10 = "1.0";
const std::string NAS_SCANNERBACKUPCTRL_HEADER_VERSION = "2.0";

struct ScannerBackupCtrlStats {
    uint64_t noOfDirs = 0;              /* Num of dir entries in the control file */
    uint64_t noOfFiles = 0;             /* Num of file entries in the control file */
    uint64_t dataSize = 0;              /* Total data size represented by all entries in this control file */
};

struct ScannerBackupCtrlHeader {
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
    ScannerBackupCtrlStats stats {};       /* Statistics of entries in this control file */
};

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

struct ScannerBackupCtrlParams {
    uint32_t maxEntriesPerFile = 0;     /* Max number of entries that can be written in this file */
    uint32_t minEntriesPerFile = 0;     /* Min number of entries that can be written in this file */
    uint32_t m_ctrlFileTimeElapsed = 0; /* Time elapsed to generate control file */
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

struct ScannerBackupCtrlDirEntry {
    std::string m_mode {};              /* Refer NAS_SCANNERBACKUPCTRL_ENTRY_MODE_XXX */
    std::string m_dirName {};           /* Absolute path of directory */
    std::string m_metaFileName {};      /* Metafile where the dir metadata is saved */
    uint16_t metaFileReadLen;           /* Length of the metadata */
    uint32_t m_aclFlag = 0;             /* Boolean flag specifying whether there is acl info present in metadata */
    uint64_t metaFileOffset;            /* Offset inside the metafile to get the metadata for this dir */
    uint64_t m_fileCount = 0;           /* Num of files under this direcotry */
    uint16_t m_metaFileIndex = 0;       /* Metafile index */
};

struct ScannerBackupCtrlFileEntry {
    std::string m_mode {};              /* Refer NAS_SCANNERBACKUPCTRL_ENTRY_MODE_XXX */
    std::string m_fileName {};          /* Relative filename */
    std::string m_metaFileName {};      /* Metafile where the file metadata is saved */
    uint16_t metaFileReadLen;           /* Length of the metadata */
    uint32_t m_aclFlag = 0;             /* Boolean flag specifying whether there is acl info present in metadata */
    uint64_t metaFileOffset;            /* Offset inside the metafile to get the metadata for this dir */
    uint64_t m_fileSize = 0;            /* Not present in control file. to be used only to track m_dataSize */
    uint16_t m_metaFileIndex = 0;       /* Metafile index */
};

class AGENT_API ScannerBackupCtrl {
    private:
        std::mutex m_lock {};                            /* Lock */
        std::string m_ctlFileName {};               /* This controlFile name */
        std::string m_ctrlFileParentDir {};         /* Parent dir of the controlFile */
        uint32_t m_maxEntryPerFile = 0;             /* Maximum entries per control file */
        uint32_t m_minEntriesPerFile = 0;           /* Minimum entries per control file */
        ScannerBackupCtrlHeader m_header {};        /* File header info */

        std::ifstream m_readFd {};    /* Read FD */
        std::stringstream m_readBuffer {};               /* Read Buffer */
        std::ofstream m_writeFd {};   /* Write FD */
        std::stringstream m_writeBuffer {};              /* Write Buffer */
        std::stringstream m_writeFileBuffer {};     /* Temporary write buffer to hold file entries for a dir entry */

        uint32_t m_maxFileSize = 0;                 /* Maximum size of the control file */
        uint64_t m_maxDataSize = 0;                 /* Maximum data size represented by all entries in this file */
        uint64_t m_minDataSize = 0;                 /* Minimum data size represented by all entries in this file */
        uint64_t m_dataSize = 0;                    /* Total data size represented by all entries in this file */
        uint32_t m_entries = 0;                     /* Number of file/dir entries in the file */
        uint32_t m_fileCount = 0;                   /* To track filecount per dir. Will be reset on each dir update */

        time_t m_ctrlFileCreationTime = 0;         /* Ctrl file creation time */
        uint32_t m_ctrlFileTimeElapsed = 0;         /* Time elapsed since control file created */
        std::string m_metaFileName {};              /* Current Meta File Name */

        /**
         * Template to Open a File in Read/Write Mode
         */
        template<class FileStream>
        NAS_CTRL_FILE_RETCODE FileOpen(FileStream &strmFd, std::ios::openmode fileMode);

        /**
         * Validate a dir-entry or file-entry read from the file
         */
        NAS_CTRL_FILE_RETCODE ValidateEntry(std::vector<std::string> &entry, std::string &line);
        NAS_CTRL_FILE_RETCODE ValidateDirEntry(std::vector<std::string> &lineContents, std::string &line);
        NAS_CTRL_FILE_RETCODE ValidateFileEntry(std::vector<std::string> &lineContents, std::string &line);

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
            NAS_CTRL_FILE_RETCODE FillHeader2(uint32_t &headerLine, std::vector<std::string> &cltHeaderLineSplit,
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
         * Translate dir entry read from file to ScannerBackupCtrlDirEntry sructure
         */
        void TranslateDirEntry(std::vector<std::string> &dirEntryStringFromFile, ScannerBackupCtrlDirEntry &dirEntry);
        void TranslateDirEntryV10(std::vector<std::string> &dirEntryStringFromFile,
            ScannerBackupCtrlDirEntry &dirEntry);

        /**
         * Translate file entry read from file to ScannerBackupCtrlFileEntry sructure
         */
        void TranslateFileEntry(std::vector<std::string> &fileEntryStringFromFile,
            ScannerBackupCtrlFileEntry &fileEntry);
        void TranslateFileEntryV10(std::vector<std::string> &fileEntryStringFromFile,
            ScannerBackupCtrlFileEntry &fileEntry);

        /**
         * Get metafile name from metafile index
         */
        std::string GetMetaFileName(uint16_t metaFileIndex);

    public:

        /**
         * Contructor to be used by (producers) users for writing to the control file
         */
        explicit ScannerBackupCtrl(ScannerBackupCtrlParams params);

        /**
         * Contructor to be used by (consumers) users for reading from the control file
         */
        explicit ScannerBackupCtrl(std::string ctlFileName);

        /**
         *  Destructor
         */
        ~ScannerBackupCtrl();

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
        NAS_CTRL_FILE_RETCODE GetHeader(ScannerBackupCtrlHeader &header);

        /**
         * Read an entry (could be dir entry or file entry) from the file
         * To be used only by consumers
         * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_READ mode
         */
        NAS_CTRL_FILE_RETCODE ReadEntry(ScannerBackupCtrlFileEntry &fileEntry, ScannerBackupCtrlDirEntry &dirEntry);

        /**
         * Write an directory entry to the file
         * To be used only by producers
         * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_WRITE mode
         */
        NAS_CTRL_FILE_RETCODE WriteDirEntry(ScannerBackupCtrlDirEntry &dirEntry);

        /**
         * Write an file entry to the file
         * To be used only by producers
         * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_WRITE mode
         */
        NAS_CTRL_FILE_RETCODE WriteFileEntry(ScannerBackupCtrlFileEntry &fileEntry);

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
};

#endif // DME_NAS_SCANNER_BACKUPCTRL_H
