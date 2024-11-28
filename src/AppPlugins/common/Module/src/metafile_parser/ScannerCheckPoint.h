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
#ifndef MODULE_SCANNER_CHECKPOINT_FILE_H
#define MODULE_SCANNER_CHECKPOINT_FILE_H

#include <boost/intrusive/list.hpp>
#include <mutex>
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include "NasControlFile.h"

constexpr uint32_t NAS_SCANNER_CHECKPOINT_MAX_ENTRIES_PER_FILE = 100000;
constexpr uint64_t NAS_SCANNER_CHECKPOINT_MAX_FILE_SIZE = (10 * 1024 * 1024);  /* 10 MB */

const std::string NAS_SCANNER_CHECKPOINT_HEADER_TITLE = "NAS Scanner Checkpoint File";
const std::string NAS_SCANNER_CHECKPOINT_HEADER_VERSION = "1.0";

struct ScannerCheckPointHeader {
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

enum CHECKPOINT_HEADER_INFO {
    CHECKPOINT_TITLE = 0,
    CHECKPOINT_HEADER_VERSION = 1,
    CHECKPOINT_TIMESTAMP = 2,
    CHECKPOINT_TASKID = 3,
    CHECKPOINT_TASKTYPE = 4,
    CHECKPOINT_NASSERVER = 5,
    CHECKPOINT_NASSHARE = 6,
    CHECKPOINT_PROTOCOL = 7,
    CHECKPOINT_PROTOCOL_VERSION = 8,
    CHECKPOINT_METADATA_SCOPE = 9,
    CHECKPOINT_RESERVED_1 = 10,
    CHECKPOINT_RESERVED_2 = 11,
    CHECKPOINT_RESERVED_3 = 12,
    CHECKPOINT_RESERVED_4 = 13,
    CHECKPOINT_RESERVED_5 = 14,
};

struct ScannerCheckPointParams {
    std::string chkPntFileName {};           /* Control file title */
    std::string taskId {};                   /* Task id */
    std::string backupType {};               /* Full or incremental */
    std::string nasServer {};                /* Scanned nas server ip address */
    std::string nasSharePath {};             /* Scanned nas share path */
    std::string proto {};                    /* Protocol used - nfs or cifs */
    std::string protoVersion {};             /* Protocol version */
    std::string metaDataScope {};            /* Metadata scope - folder-only or folder-and-file */
};

using auto_unlink_hook = boost::intrusive::list_base_hook<boost::intrusive::link_mode<boost::intrusive::auto_unlink>>;
// This hook removes the node in the destructor
class CheckPointObj : public auto_unlink_hook {
public:
    std::string m_path {};
    uint32_t m_filterFlag = 0;

    CheckPointObj(std::string path, uint32_t filterFlag) : m_path(path), m_filterFlag(filterFlag) {}

    void Unlink()
    {
        auto_unlink_hook::unlink();
    }

    bool IsLinked()
    {
        return auto_unlink_hook::is_linked();
    }
};

// Define a list that will store values using the base hook
// The list can't have constant-time size!
using IntrusiveList = boost::intrusive::list<CheckPointObj, boost::intrusive::constant_time_size<false>>;

struct DeleteDisposer {
    void operator()(const CheckPointObj *chkPntObj)
    {
        delete chkPntObj;
    }
};

class ScannerCheckPoint {
public:
    /**
    * Contructor to be used by (producers) users for writing to the control file
    */
    explicit ScannerCheckPoint(ScannerCheckPointParams params);

    /**
    * Contructor to be used by (consumers) users for reading from the control file
    */
    explicit ScannerCheckPoint(std::string chkPntFileName);

    /**
    *  Destructor
    */
    ~ScannerCheckPoint();

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
    NAS_CTRL_FILE_RETCODE GetHeader(ScannerCheckPointHeader &header);

    /**
    * Write a directory path to the file
    * To be used only by producers
    * File must be Opened with NAS_CTRL_FILE_OPEN_MODE_WRITE mode
    */
    NAS_CTRL_FILE_RETCODE WriteChkPntEntry(std::string chkPntEntry);

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
    std::mutex m_lock {};                                   /* Lock */
    std::string m_chkPntFileName {};                   /* This controlFile name */
    std::string m_chkPntFileParentDir {};              /* Parent dir of the controlFile */
    ScannerCheckPointHeader m_header {};               /* File header info */

    std::ifstream m_readFd {};           /* Read FD */
    std::stringstream m_readBuffer {};                      /* Read Buffer */
    std::ofstream m_writeFd {};          /* Write FD */
    std::stringstream m_writeBuffer {};                     /* Write Buffer */

    uint32_t m_entries = 0;                            /* Number of dir path entries in the file */
    uint32_t m_maxEntriesPerFile = 0;                  /* Maximum entries per control file */
    uint32_t m_maxFileSize = 0;                        /* Maximum size of the control file */

    /**
    * Template to Open a File in Read/Write Mode
    */
    template<class FileStream>
    NAS_CTRL_FILE_RETCODE FileOpen(FileStream &strmFd, std::ios::openmode fileMode);

    /**
    * Write the file header info to file from m_header
    */
    NAS_CTRL_FILE_RETCODE WriteHeader();

    /**
    * Get the line to write in header info of file
    */
    std::string GetFileHeaderLine(uint32_t headerLine);

    /**
    * Read the file header info from file and load to m_header
    */
    NAS_CTRL_FILE_RETCODE ReadHeader();
    NAS_CTRL_FILE_RETCODE FillHeader(uint32_t &headerLine, std::vector<std::string> &ctlHeaderLineSplit,
        std::string &ctlHeaderLine);

    /**
    * Validate header information read from the file
    */
    NAS_CTRL_FILE_RETCODE ValidateHeader();
};

#endif // DME_NAS_SCANNER_CHECKPOINT_FILE_H