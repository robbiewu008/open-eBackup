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
#ifndef MODULE_BACKUP_MTIMECTRL_H
#define MODULE_BACKUP_MTIMECTRL_H
#include "NasControlFile.h"
#include "define/Defines.h"
#include "define/Types.h"
#include <string>
#include <vector>
#include <fstream>
#include <mutex>

const std::string NAS_BACKUPMTIMECTRL_HEADER_TITLE = "NAS Backup Mtime Control File";
const std::string NAS_BACKUPMTIMECTRL_HEADER_VERSION = "1.0";

struct BackupMtimeCtrlStats {
    uint64_t noOfDirs = 0;           /* Num of directories whose atime and mtime changed in the control file */
};

struct BackupMtimeCtrlParams {
    uint32_t maxEntriesPerFile = 0;     /* Max number of entries that can be written in this file */
    std::string m_ctlFileName {};       /* Control file title */
    std::string taskId {};              /* Task Id */
    std::string backupType {};          /* Full or incremental */
    std::string nasServer {};           /* Scanned nas server ip address */
    std::string nasSharePath {};        /* Scanned nas share path */
    std::string proto {};               /* protocol used - nfs or cifs */
    std::string protoVersion {};       /* protocol version */
};

struct BackupMtimeCtrlHeader {
    std::string title;                  /* Control file title */
    std::string version;                /* Version */
    std::string timestamp;              /* Timestamp - when the file is created */
    std::string taskId;                 /* Task id */
    std::string backupType;             /* Backup type */
    std::string nasServer;              /* Scanned nas server ip address */
    std::string nasSharePath;           /* Scanned nas share path */
    std::string proto;                  /* Protocol used - nfs or cifs */
    std::string protoVersion;          /* Protocol version */
    BackupMtimeCtrlStats stats;         /* Statistics of entries in this control file */
};

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

struct BackupMtimeCtrlEntry {
    std::string m_absPath {};           /* Absolute path of directory */
    uint64_t m_ctime = 0;
    uint64_t m_atime = 0;               /* atime of directory */
    uint64_t m_mtime = 0;               /* mtime of directory */
    uint64_t m_btime = 0;
    uint32_t m_uid = 0;                 /* user ID of owner */
    uint32_t m_gid = 0;                 /* group ID of owner */
    uint32_t m_attr = 0;
	#ifndef WIN32
	mode_t   m_mode = 0;                /* protection type (rwx) */
	#endif
};

class AGENT_API BackupMtimeCtrl {
    private:
        std::mutex m_lock {};                            /* Lock */
        std::string m_ctlFileName {};               /* This controlFile name */
        std::string m_ctrlFileParentDir {};         /* Parent dir of the controlFile */
        uint32_t m_maxEntryPerFile = 0;             /* Maximum entries per control file */
        BackupMtimeCtrlHeader m_header {};			/* File header info */

        std::ifstream m_readFd {};    /* Read FD */
        std::stringstream m_readBuffer {};               /* Read Buffer */
        std::ofstream m_writeFd {};   /* Write FD */
        std::stringstream m_writeBuffer {};              /* Write Buffer */

        uint32_t m_maxFileSize = 0;					/* Maximum size of the control file */
        uint32_t m_entries = 0;						/* Number of file/dir entries in the file */

        /**
         * Template to Open a File in Read/Write Mode
         */
        template<class FileStream>
        NAS_CTRL_FILE_RETCODE FileOpen(FileStream &strmFd, std::ios::openmode fileMode);

        /**
         * Validate a dir-entry read from the file
         */
        NAS_CTRL_FILE_RETCODE ValidateEntry(std::vector<std::string> lineContents, std::string line);

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
         * Translate dir entry read from file to BackupMtimeCtrlEntry sructure
         */
        void TranslateEntry(std::vector<std::string> &lineContents, BackupMtimeCtrlEntry &mtimeEntry);

    public:
        /**
         * Contructor to be used by (producers) users for writing to the control file
         */
        explicit BackupMtimeCtrl(BackupMtimeCtrlParams params);

        /**
         * Contructor to be used by (consumers) users for reading from the control file
         */
        explicit BackupMtimeCtrl(std::string ctlFileName);

        /**
         *  Destructor
         */
        ~BackupMtimeCtrl();

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
        NAS_CTRL_FILE_RETCODE GetHeader(BackupMtimeCtrlHeader &header);

        /**
         * Read an entry from the file
         * To be used only by consumers
         * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_READ mode
         */
        NAS_CTRL_FILE_RETCODE ReadEntry(BackupMtimeCtrlEntry &mtimeEntry);

        /**
         * Write a directory entry to the file
         * To be used only by producers
         * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_WRITE mode
         */
        NAS_CTRL_FILE_RETCODE WriteEntry(BackupMtimeCtrlEntry &mtimeEntry);

        /**
         * Check the number of entries in the control file
         */
        uint32_t GetEntries();

        /**
         * Get the ctrl file name
         */
        std::string GetCtrlFileName();
};

#endif // DME_NAS_BACKUP_MTIMECTRL_H