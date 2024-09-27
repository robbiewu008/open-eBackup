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
#ifndef MODULE_SCANNER_META_FILE_H
#define MODULE_SCANNER_META_FILE_H

#include <mutex>
#include <vector>
#include <string>
#ifdef _NAS
#include "nfsc/libnfs.h"
#include "nfsc/libnfs-raw.h"
#include "nfsc/libnfs-raw-nfs.h"
#endif
#include <fstream>
#include "define/Defines.h"
#include "securec.h"
#include "NasControlFile.h"
// #include "NfsContextContainer.h"
// #include "ScannerUtils.h"

constexpr uint32_t SCANNER_PATH_LEN_MAX = 4096;
constexpr uint32_t SCANNER_FILE_NAME_LEN_MAX = 256;
constexpr uint32_t ACL_MAX_LEN = 65535;

constexpr uint32_t NAS_BACKUP_ENTRY_F_META_CHANGED = 1;
constexpr uint32_t NAS_BACKUP_ENTRY_F_DATA_CHANGED = 2;
constexpr uint32_t NAS_BACKUP_ENTRY_F_FILE_DELETED = 4;
constexpr uint32_t NAS_BACKUP_ENTRY_F_EXT_ACL = 8;
constexpr uint16_t NAS_HDR_RETRY_SLEEP_DUR_MIN_MSEC = 1000;     /* in milli seconds */
constexpr uint16_t NAS_HDR_RETRY_SLEEP_DUR_MAX_MSEC = 3000;     /* in milli seconds */

namespace NasScanner {

const std::string NAS_SCANNERBACKUPMETA_HEADER_TITLE = "NAS Scanner Backup Meta File";
const std::string NAS_SCANNERBACKUPMETA_HEADER_VERSION = "1.0";

#define NAS_BACKUP_ENTRY_IS_META_CHANGED(flag) \
    if ((NAS_BACKUP_ENTRY_F_META_CHANGED & (flag)) == NAS_BACKUP_ENTRY_F_META_CHANGED)

#define NAS_BACKUP_ENTRY_IS_DATA_CHANGED(flag) \
    if ((NAS_BACKUP_ENTRY_F_DATA_CHANGED & (flag)) == NAS_BACKUP_ENTRY_F_DATA_CHANGED)

#define NAS_BACKUP_ENTRY_IS_FILE_DELETED(flag) \
    if ((NAS_BACKUP_ENTRY_F_FILE_DELETED & (flag)) == NAS_BACKUP_ENTRY_F_FILE_DELETED)

#define NAS_BACKUP_ENTRY_IS_EXT_ACL(flag) \
    if ((NAS_BACKUP_ENTRY_F_EXT_ACL & (flag)) == NAS_BACKUP_ENTRY_F_EXT_ACL)

struct ScannerMetaCtrlHeader {
    std::string title;                  /* Control file title */
    std::string version;                /* Version */
    std::string timestamp;              /* Timestamp - when the file is created */
    std::string taskId;                 /* Task id */
    std::string backupType;             /* Full or incremental */
    std::string nasServer;              /* Scanned nas server ip address */
    std::string nasSharePath;           /* Scanned nas share path */
    std::string proto;                  /* Protocol used - nfs or cifs */
    std::string protoVersion;          /* Protocol version */
    std::string metaDataScope;          /* Metadata scope - folder-only or folder-and-file */
};

enum META_HEADER_INFO {
    META_TITLE = 0,
    META_HEADER_VERSION = 1,
    META_TIMESTAMP = 2,
    META_TASKID = 3,
    META_TASKTYPE = 4,
    META_NASSERVER = 5,
    META_NASSHARE = 6,
    META_PROTOCOL = 7,
    META_PROTOCOL_VERSION = 8,
    META_METADATA_SCOPE = 9,
    META_RESERVED_1 = 10,
    META_RESERVED_2 = 11,
    META_RESERVED_3 = 12,
    META_RESERVED_4 = 13,
    META_RESERVED_5 = 14,
};

struct ScannerMetaCtrlParams {
    std::string m_fileName {};       /* Control file title */
    uint32_t maxEntriesPerFile = 0;     /* Max number of entries that can be written in this file */
    uint64_t maxDataSize = 0;           /* Max data size that can be written in this file */
    std::string taskId {};              /* Task id */
    std::string backupType {};          /* Full or incremental */
    std::string nasServer {};           /* Scanned nas server ip address */
    std::string nasSharePath {};        /* Scanned nas share path */
    std::string proto {};               /* Protocol used - nfs or cifs */
    std::string protoVersion {};       /* Protocol version */
    std::string metaDataScope {};       /* Metadata scope - folder-only or folder-and-file */
};

enum TRANSLATE_TYPE {
    META_TO_METARW = 0,
    METARW_TO_META = 1,
    STAT_TO_STATRW = 2,
    STATRW_TO_STAT = 3,
};

class DirectoryMetaReadWrite {
public:
    uint64_t m_inode = 0;
    uint64_t m_size = 0;
    uint64_t m_mtime = 0;
    uint64_t m_atime = 0;
    uint64_t m_ctime = 0;
    uint64_t m_btime = 0;
    uint32_t m_attr = 0;
    uint32_t m_mode = 0;
    uint32_t m_uid = 0;
    uint32_t m_gid = 0;
    uint16_t m_aclSize = 0;
    uint16_t m_pathLen = 0;
};

class FileMetaReadWrite {
public:
#ifdef _NAS
    nfs_fh_scan m_fh {};
#endif
    uint64_t m_inode = 0;
    uint64_t m_mtime = 0;
    uint64_t m_atime = 0;
    uint64_t m_ctime = 0;
    uint64_t m_btime = 0;
    uint32_t m_attr = 0;
    uint64_t m_size = 0;
    uint64_t m_rdev = 0;
    uint32_t m_mode = 0;
    uint32_t m_nlink = 0;
    uint32_t m_uid = 0;
    uint32_t m_gid = 0;
    uint16_t m_aclSize = 0;
    uint16_t m_nameLen = 0;
};

class DirectoryMeta {
public:
    uint64_t m_inode = 0;
    uint64_t m_size = 0;
    uint64_t m_mtime = 0;
    uint64_t m_atime = 0;
    uint64_t m_ctime = 0;
    uint64_t m_btime = 0;
    uint32_t m_attr = 0;
    uint32_t m_mode = 0;
    uint32_t m_uid = 0;
    uint32_t m_gid = 0;
    uint16_t m_aclSize = 0;
    std::string m_path = "";
    std::string m_aclText = "";

    DirectoryMeta() {};
    ~DirectoryMeta() {};
};

class FileMeta {
public:
#ifdef _NAS
    nfs_fh_scan m_fh {};
#endif
    uint64_t m_inode = 0;
    uint64_t m_size = 0;
    uint64_t m_rdev = 0;
    uint64_t m_mtime = 0;
    uint64_t m_ctime = 0;
    uint64_t m_atime = 0;
    uint64_t m_btime = 0;
    uint32_t m_attr = 0;
    uint32_t m_mode = 0;
    uint32_t m_nlink = 0;
    uint32_t m_uid = 0;
    uint32_t m_gid = 0;
    uint16_t m_aclSize = 0;
    std::string m_name = "";
    std::string m_aclText = "";

    FileMeta() {};
    ~FileMeta() {};
};

// class will be similar to control file class.
class AGENT_API ScannerBackupMeta {
    public:
        /**
         * Contructor to be used by (producers) users for writing to the meta file
         */
        explicit ScannerBackupMeta(ScannerMetaCtrlParams params);

        /**
         * Contructor to be used by (consumers) users for reading from the meta file
         */
        explicit ScannerBackupMeta(std::string metaFileName);

        /**
         *  Destructor
         */
        ~ScannerBackupMeta();

        /**
         * Open file in read/write mode
         */
        NAS_CTRL_FILE_RETCODE Open(NAS_CTRL_FILE_OPEN_MODE mode);

        /**
         * Write directory meta which will write to buffer and return current offset.
         */
        uint16_t WriteDirectoryMeta(DirectoryMeta &dirMeta);

        /**
         * Write file meta which will write to buffer and return current offset.
         */
        uint16_t WriteFileMeta(FileMeta &fMeta);

        /**
         * Read directory meta for a given offset.
         */
        NAS_CTRL_FILE_RETCODE ReadDirectoryMeta(DirectoryMeta &dirMeta, uint16_t readLen, uint64_t offset);

        /**
         * Read directory meta for a given offset.
         */
        NAS_CTRL_FILE_RETCODE ReadFileMeta(FileMeta &fMeta, uint16_t readLen, uint64_t offset);


        /**
         * Read file meta entries
         */
        NAS_CTRL_FILE_RETCODE ReadFileMetaEntries(std::queue<FileMeta> &fileMetaQueue, int count);

        /**
         * This will return current offset.
         */
        uint64_t GetCurrentOffset();

        /**
         * flush all data and Close file.
         */
        NAS_CTRL_FILE_RETCODE Close(NAS_CTRL_FILE_OPEN_MODE mode);

        /**
         * Write buffer data into file.
         */
        NAS_CTRL_FILE_RETCODE FlushToFile();

        /**
         * Get Metafile name
         */
        std::string GetFileName();

    private:
        std::mutex m_lock {};                            /* Lock */
        char *m_readBuffer {};                      /* Read buffer */
        std::stringstream m_writeBuffer {};              /* Write buffer */
        ScannerMetaCtrlHeader m_header {};          /* File header info */
        std::ifstream m_readFd {};    /* Read FD */
        std::ofstream m_writeFd {};   /* Write FD */
        uint64_t m_offset = 0;                      /* Meta write offset */
        uint64_t m_maxFileSize = 0;                 /* Max File size */
        std::string m_metaFileName {};                   /* Meta File name */
        uint32_t m_readBufferSize = 0;              /* Read Buffer Size */
        std::string m_metaFileParentDir {};            /* Parent Dir of Meta File */

        /**
         * Template to Open a File in Read/Write Mode
         */
        template<class FileStream>
        NAS_CTRL_FILE_RETCODE FileOpen(FileStream &strmFd, std::ios::openmode fileMode);

        /**
         * Translate FileMeta structure to FileMetaReadWrite based on options
         */
        NAS_CTRL_FILE_RETCODE TranslateFileMeta(FileMeta &fMeta, FileMetaReadWrite &fMetaRw, int32_t option);

        /**
         * Translate directoryMeta and DirectoryMetaReadWrite structure based on option.
         */
        NAS_CTRL_FILE_RETCODE TranslateDirMeta(DirectoryMeta &dirMeta,
            DirectoryMetaReadWrite &dirMetaRw, int32_t option);

        /**
         * Validate header information read from the file
         */
        NAS_CTRL_FILE_RETCODE ValidateHeader();

        /**
         * Read the file header and retry if its failed
         */
        NAS_CTRL_FILE_RETCODE ReadMetaFileHeader();

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
         * Get the file header info read from file
         */
        NAS_CTRL_FILE_RETCODE GetHeader(ScannerMetaCtrlHeader &header);

        /**
         * Read directory meta from buffer and fill DirectoryMeta object
         */
        NAS_CTRL_FILE_RETCODE ReadDirMetaFromBuffer(DirectoryMeta &dirMeta, uint64_t offset, uint16_t readLen);
        /**
         * Read file meta from buffer and fill FileMeta object
         */
        NAS_CTRL_FILE_RETCODE ReadFileMetaFromBuffer(FileMeta &fMeta, uint64_t offset, uint16_t readLen);

        /**
         * Read metaobj from file fd.
         */
        NAS_CTRL_FILE_RETCODE ReadFromFile(int len);

        /**
         * Read filemeta from buffer
         */
        NAS_CTRL_FILE_RETCODE ReadFileMetaFromBuffer(FileMeta &fMeta);

        /**
         * Readfile meta from current offset
         */
        NAS_CTRL_FILE_RETCODE ReadFileMeta(FileMeta &fMeta);

        /**
         * Read directory aclText
         */
        NAS_CTRL_FILE_RETCODE ReadAcl(uint64_t offset, DirectoryMeta &dirMeta,
            uint16_t readLen, int32_t aclOffset);

        /**
         * Read file aclText
         */
        NAS_CTRL_FILE_RETCODE ReadAcl(uint64_t offset, FileMeta &fMeta,
            uint16_t readLen, int32_t aclOffset);

        /**
         * Check directory meta valid or not
         */
        NAS_CTRL_FILE_RETCODE CheckDirMetaValidity(DirectoryMeta &dirMeta);
        /**
         * Check file meta valid or not
         */
        NAS_CTRL_FILE_RETCODE CheckFileMetaValidity(FileMeta &fMeta);

        /**
         * Validate file offsets
         */
        NAS_CTRL_FILE_RETCODE ValidateMetaFile(uint64_t offset, uint16_t readLen);

        uint32_t GetRandomNumber(uint32_t minNum, uint32_t maxNum);
};

}
#endif // DME_NAS_SCANNER_META_FILE_H