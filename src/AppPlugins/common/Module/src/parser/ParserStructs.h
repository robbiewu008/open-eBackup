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
#ifndef MODULE_PARSER_STRUCTS_H
#define MODULE_PARSER_STRUCTS_H

#include <vector>
#include <string>
#include <cstring>
#include <openssl/sha.h>
#include "securec.h"

namespace Module {

constexpr bool CTRL_BINARY_FILE = true;
constexpr uint16_t ERROR_MSG_SIZE = 256;
constexpr uint64_t DCACHE_MAX_READBUFF_SIZE = 100000;

constexpr uint32_t CTRL_FILE_MAX_SIZE = (4 * 1024 * 1024);      /* 4MB */
constexpr uint16_t CTRL_MILLI_SEC = 1000;
constexpr uint16_t CTRL_MAX_COUNT = 1000;
constexpr uint16_t CTRL_READ_BUFFER_SIZE = 1000;
constexpr uint8_t CTRL_FILE_SERVER_RETRY_CNT = 5;
constexpr uint16_t CTRL_FILE_SERVER_RETRY_INTERVAL = 30000;     /* in milli seconds */

constexpr uint8_t CTRL_FILE_NUMBER_ZERO = 0;
constexpr uint8_t CTRL_FILE_NUMBER_ONE = 1;
constexpr uint8_t CTRL_FILE_NUMBER_TWO = 2;
constexpr uint8_t CTRL_FILE_NUMBER_THREE = 3;
constexpr uint8_t CTRL_FILE_NUMBER_FOUR = 4;
constexpr uint8_t CTRL_FILE_NUMBER_FIVE = 5;
constexpr uint8_t CTRL_FILE_NUMBER_SEVEN = 7;
constexpr uint8_t CTRL_FILE_NUMBER_EIGHT = 8;
constexpr uint8_t CTRL_FILE_NUMBER_NINE = 9;
constexpr uint8_t CTRL_FILE_NUMBER_TEN = 10;
constexpr uint8_t CTRL_FILE_NUMBER_ELEVEN = 11;
constexpr uint8_t CTRL_FILE_NUMBER_THIRTEEN = 13;

constexpr uint8_t CTRL_FILE_OFFSET_0 = 0;
constexpr uint8_t CTRL_FILE_OFFSET_1 = 1;
constexpr uint8_t CTRL_FILE_OFFSET_2 = 2;
constexpr uint8_t CTRL_FILE_OFFSET_3 = 3;
constexpr uint8_t CTRL_FILE_OFFSET_4 = 4;
constexpr uint8_t CTRL_FILE_OFFSET_5 = 5;
constexpr uint8_t CTRL_FILE_OFFSET_6 = 6;
constexpr uint8_t CTRL_FILE_OFFSET_7 = 7;
constexpr uint8_t CTRL_FILE_OFFSET_8 = 8;
constexpr uint8_t CTRL_FILE_OFFSET_9 = 9;
constexpr uint8_t CTRL_FILE_OFFSET_10 = 10;
constexpr uint8_t CTRL_FILE_OFFSET_11 = 11;

constexpr uint16_t FCACHE_FILE_NOT_MODIFIED = 0;
constexpr uint16_t FCACHE_FILE_META_MODIFIED = 1;
constexpr uint16_t FCACHE_FILE_DATA_MODIFIED = 2;
constexpr uint16_t FCACHE_FILE_BOTH_MODIFIED = 3;

constexpr uint16_t HDR_RETRY_SLEEP_DUR_MIN_MSEC = 1000;     /* in milli seconds */
constexpr uint16_t HDR_RETRY_SLEEP_DUR_MAX_MSEC = 3000;     /* in milli seconds */

const std::string CTRL_ENTRY_MODE_DATA_MODIFIED = "dm";
const std::string CTRL_ENTRY_MODE_META_MODIFIED = "mm";
const std::string CTRL_ENTRY_MODE_BOTH_MODIFIED = "bm";
const std::string CTRL_ENTRY_MODE_DATA_DELETED = "dd";
const std::string CTRL_ENTRY_MODE_ONLY_FILE_MODIFIED = "fm";
const std::string CTRL_ENTRY_MODE_NEW_FILE = "nn";
const std::string CTRL_ENTRY_TYPE_DIR  = "d";
const std::string CTRL_ENTRY_TYPE_FILE  = "f";
constexpr uint16_t CTRL_TEN_MIN_IN_SEC  = 600;

const std::string HARDLINKCTRL_ENTRY_MODE_DATA_MODIFIED = "dm";
const std::string HARDLINKCTRL_ENTRY_MODE_META_MODIFIED = "mm";
const std::string HARDLINKCTRL_ENTRY_MODE_BOTH_MODIFIED = "bm";
const std::string HARDLINKCTRL_ENTRY_MODE_DATA_DELETED = "dd";
const std::string HARDLINKCTRL_ENTRY_MODE_ONLY_FILE_MODIFIED = "fm";
const std::string HARDLINKCTRL_ENTRY_TYPE_FILE  = "f";
const std::string HARDLINKCTRL_ENTRY_TYPE_INODE = "i";
constexpr uint16_t HARDLINKCTRL_TEN_MIN_IN_SEC  = 600;

/*
 * SHA_DIGEST_LENGTH is defined in openssl/sha.h,
 * define SHA_DIGEST_LENGTH here to avoid header file be corrupt
 */
# ifdef SHA_DIGEST_LENGTH
# undef SHA_DIGEST_LENGTH
# endif
const int SHA_DIGEST_LENGTH = 20;

/**
 * bit flags for m_mode field in FileMeta struct (only used for Windows)
 */
const int FILEMETA_FLAG_WIN32_SYMBOLIC_LINK = 0x02; /* mark a windows symbolic file */
const int FILEMETA_FLAG_WIN32_JUNCTION_LINK = 0x04; /* mark a windows junction file */

union Hash {
    uint32_t crc = 0;
    unsigned char sha1[SHA_DIGEST_LENGTH + 1];
};

enum class CTRL_FILE_OPEN_MODE {
    READ = 0,
    WRITE = 1
};

enum class CTRL_FILE_RETCODE {
    FAILED = -1,
    SUCCESS = 0,
    LIMIT_REACHED = 1,
    READ_EOF = 2,
    INVALID_CONTENT = 3
};

enum class MetaType {
    NTS = 1,
    CIFS,
    UNIX,
    WINDOWS,
    OBJECT
};

struct NfsFileHandle {
    int len;
    char value[64];
};

struct DeleteCtrlEntry {
    std::string m_absPath {};           /* Absolute path of directory */
    bool m_isDel = 0;                   /* Is this directory to be deleted */
    bool m_isDir =  0;                  /*  Is this a directory or file entry */
};

struct MtimeCtrlEntry {
    std::string m_absPath {};           /* Absolute path of directory */
    uint64_t m_ctime = 0;
    uint64_t m_atime = 0;               /* atime of directory */
    uint64_t m_mtime = 0;               /* mtime of directory */
    uint64_t m_btime = 0;
    uint32_t m_uid = 0;                 /* user ID of owner */
    uint32_t m_gid = 0;                 /* group ID of owner */
    uint32_t m_attr = 0;
    uint32_t m_subDirsCnt = 0;
    uint32_t m_mode = 0;                /* protection type (rwx) for Linux, used as a reserved field for Windows */
};

struct CopyCtrlDirEntry {
    std::string m_mode {};              /* Refer SCANNERBACKUPCTRL_ENTRY_MODE_XXX */
    std::string m_dirName {};           /* Absolute path of directory */
    std::string m_metaFileName {};      /* Metafile where the dir metadata is saved */
    uint16_t metaFileReadLen;           /* Length of the metadata */
    uint32_t m_aclFlag = 0;             /* Boolean flag specifying whether there is acl info present in metadata */
    uint64_t metaFileOffset;            /* Offset inside the metafile to get the metadata for this dir */
    uint64_t m_fileCount = 0;           /* Num of files under this direcotry */
    uint16_t m_metaFileIndex = 0;       /* Metafile index */
};

struct CopyCtrlFileEntry {
    std::string m_mode {};              /* Refer SCANNERBACKUPCTRL_ENTRY_MODE_XXX */
    std::string m_fileName {};          /* Relative filename */
    std::string m_metaFileName {};      /* Metafile where the file metadata is saved */
    uint16_t metaFileReadLen;           /* Length of the metadata */
    uint32_t m_aclFlag = 0;             /* Boolean flag specifying whether there is acl info present in metadata */
    uint64_t metaFileOffset;            /* Offset inside the metafile to get the metadata for this dir */
    uint64_t m_fileSize = 0;            /* Not present in control file. to be used only to track m_dataSize */
    uint16_t m_metaFileIndex = 0;       /* Metafile index */
};

class DirMeta {
public:
    uint16_t type = 0; // 1.nfs 2.cifs 3.UNIX 4.Windows
    uint32_t m_hardLinkFilesCnt = 0;
    uint32_t m_subDirsCnt = 0;
    uint32_t m_attr = 0;
    uint32_t m_mode = 0; /* (rwx) for Linux, used as a reserved fields for Win32 since 1.3.0 */
    uint32_t m_uid = 0;
    uint32_t m_gid = 0;
    uint64_t m_inode = 0;
    uint64_t m_size = 0;
    uint64_t m_mtime = 0;
    uint64_t m_atime = 0;
    uint64_t m_ctime = 0;
    uint64_t m_btime = 0;
    uint64_t m_xMetaFileIndex = 0;
    uint64_t m_xMetaFileOffset = 0;

    DirMeta() {};
    ~DirMeta() {};
};

class FileMeta {
public:
    uint16_t type = 0; // 1.nfs 2.cifs 3.UNIX 4.Windows
    uint32_t m_attr = 0;
    uint32_t m_mode = 0; /* (rwx) for Linux, used as a reserved fields for Win32 since 1.3.0 */
    uint32_t m_nlink = 0;
    uint32_t m_uid = 0;
    uint32_t m_gid = 0;
    uint64_t m_inode = 0;
    uint64_t m_size = 0;
    uint64_t m_err = 0;
    uint64_t m_rdev = 0;
    uint64_t m_mtime = 0;
    uint64_t m_ctime = 0;
    uint64_t m_atime = 0;
    uint64_t m_btime = 0;
    uint64_t m_blksize = 0;
    uint64_t m_blocks = 0;
    uint64_t m_xMetaFileIndex = 0;
    uint64_t m_xMetaFileOffset = 0;

    FileMeta() {};
    ~FileMeta() {};
};

enum class XMETA_TYPE {
    XMETA_TYPE_DEFAULT = 0,
    /* Store filename of a directory path */
    XMETA_TYPE_NAME = 1,
    /*
     * XMETA_TYPE_ACL is only used for POSIX/CIFS/NFS scanning before 1.3.0.
     * To store Windows ACL(securityDescriptor ACE), see XMETA_TYPE_SECURITYDESCRIPTOR
     * CIFS security descriptor will migrate to XMETA_TYPE_SECURITYDESCRIPTOR in the future as well
     */
    XMETA_TYPE_ACL = 2,
    /* POSIX extend attributes */
    XMETA_TYPE_EXTEND_ATTRIBUTES = 3,
    /* NFS file handle */
    XMETA_TYPE_NFSFH = 4,
    /* sparse file allocate range, <off1>,<len1>;<off2><len2>... */
    XMETA_TYPE_SPARSE_INFO = 5,

    /* these three XMeta type below was introduced since 1.3.0 for windows scanning  */
    /* windows security descriptor ACE string */
    XMETA_TYPE_SECURITYDESCRIPTOR = 6,
    /* final path of a symbolic link */
    XMETA_TYPE_SYMBOLIC_TARGET = 7,
    /* final path of a junction point */
    XMETA_TYPE_JUNCTION_TARGET = 8,

    /* 对象存储中的对象key */
    XMETA_TYPE_KEY = 9,

    /* upper bound of XMETA_TYPE enum */
    XMETA_TYPE_MAX_LENGTH = 10,
};

class XMetaField {
public:
    XMETA_TYPE m_xMetaType;
    std::string m_value;
};

class FileMetaWrapper {
public:
    FileMeta m_meta;
    std::vector<XMetaField> m_xMeta;

    FileMetaWrapper() {};
    ~FileMetaWrapper() {};
};

struct CompareFileMetaWrapper {
    bool operator() (const FileMetaWrapper& fm1, const FileMetaWrapper& fm2) const
    {
        std::string fmFileName1;
        std::string fmFileName2;
        for (uint32_t i = 0; i < fm1.m_xMeta.size(); i++) {
            if (fm1.m_xMeta[i].m_xMetaType == XMETA_TYPE::XMETA_TYPE_NAME) {
                fmFileName1 = fm1.m_xMeta[i].m_value;
                break;
            }
        }
        for (uint32_t i = 0; i < fm2.m_xMeta.size(); i++) {
            if (fm2.m_xMeta[i].m_xMetaType == XMETA_TYPE::XMETA_TYPE_NAME) {
                fmFileName2 = fm2.m_xMeta[i].m_value;
                break;
            }
        }
        unsigned char sha1[SHA_DIGEST_LENGTH + 1];
        unsigned char sha2[SHA_DIGEST_LENGTH + 1];
        // Get FilePath SHA-1 Hash Value
        memset_s(sha1, SHA_DIGEST_LENGTH + 1, 0x0, SHA_DIGEST_LENGTH + 1);
        SHA1((unsigned char *)(fmFileName1.c_str()), fmFileName1.length(), sha1);

        // Get FilePath SHA-1 Hash Value
        memset_s(sha2, SHA_DIGEST_LENGTH + 1, 0x0, SHA_DIGEST_LENGTH + 1);
        SHA1((unsigned char *)(fmFileName2.c_str()), fmFileName2.length(), sha2);
        
        return (memcmp(sha1, sha2, SHA_DIGEST_LENGTH) > 0);
    }
};

class DirMetaWrapper {
public:
    DirMeta m_meta;
    std::vector<XMetaField> m_xMeta;

    DirMetaWrapper() {};
    ~DirMetaWrapper() {};
};

class DirMetaV10 {
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

    DirMetaV10() {};
    ~DirMetaV10() {};
};

class FileMetaV10 {
public:
    NfsFileHandle m_fh {};
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

    FileMetaV10() {};
    ~FileMetaV10() {};
};

class DirCacheV10 {
public:
    uint64_t m_inode = 0;        /* inode of dir */
    uint64_t m_mdataOffset = 0;  /* offset in meta data file */
    uint64_t m_fcacheOffset = 0; /* offset in fcache file */
    uint32_t m_hashTag = 0;      /* Directory hash CRC */
    uint32_t m_crc = 0;          /* CRC */
    uint32_t m_totalFiles = 0;   /* number of files under given dir */
    uint16_t m_fileId = 0;      /* metadata file id */
    uint16_t m_metaLength = 0;  /* Total meta length stored in meta file */

    DirCacheV10() {};
    explicit DirCacheV10(const DirCacheV10 *dc)
    {
        m_hashTag = dc->m_hashTag;
        m_inode = dc->m_inode;
        m_crc = dc->m_crc;
        m_fileId = dc->m_fileId;
        m_fcacheOffset = dc->m_fcacheOffset;
        m_mdataOffset = dc->m_mdataOffset;
        m_totalFiles = dc->m_totalFiles;
        m_metaLength = dc->m_metaLength;
    }
    ~DirCacheV10() {};
};

class DirCacheV20 {
public:
    uint64_t m_inode = 0;        /* inode of dir */
    uint64_t m_mdataOffset = 0;  /* offset in meta data file */
    uint64_t m_fcacheOffset = 0; /* offset in fcache file */
    uint32_t m_hashTag = 0;      /* Directory hash CRC */
    uint32_t m_crc = 0;          /* CRC */
    uint32_t m_totalFiles = 0;   /* number of files under given dir */
    uint16_t m_fileId = 0;      /* metadata file id */
    uint16_t m_fcacheFileId = 0; /* fcache file id */
    uint16_t m_metaLength = 0;  /* Total meta length stored in meta file */

    DirCacheV20() {};
    explicit DirCacheV20(const DirCacheV20 *dc)
    {
        m_hashTag = dc->m_hashTag;
        m_inode = dc->m_inode;
        m_crc = dc->m_crc;
        m_fileId = dc->m_fileId;
        m_fcacheOffset = dc->m_fcacheOffset;
        m_mdataOffset = dc->m_mdataOffset;
        m_totalFiles = dc->m_totalFiles;
        m_metaLength = dc->m_metaLength;
        m_fcacheFileId = dc->m_fcacheFileId;
    }
    ~DirCacheV20() {};
};

class DirCache {
public:
    uint64_t m_inode = 0;        /* inode of dir */
    uint64_t m_mdataOffset = 0;  /* offset in meta data file */
    uint64_t m_fcacheOffset = 0; /* offset in fcache file */
    uint32_t m_totalFiles = 0;   /* number of files under given dir */
    uint16_t m_fileId = 0;      /* metadata file id */
    uint16_t m_fcacheFileId = 0; /* fcache file id */
    uint16_t m_metaLength = 0;  /* Total meta length stored in meta file */
    Hash m_dirPathHash {0};     /* Hash of full-path of the dir */
    Hash m_dirMetaHash {0};     /* Hash of meta-data of the dir */

    DirCache() {};
    explicit DirCache(const DirCache *dc)
    {
        m_inode = dc->m_inode;
        memcpy_s(m_dirPathHash.sha1, SHA_DIGEST_LENGTH, dc->m_dirPathHash.sha1, SHA_DIGEST_LENGTH);
        memcpy_s(m_dirMetaHash.sha1, SHA_DIGEST_LENGTH, dc->m_dirMetaHash.sha1, SHA_DIGEST_LENGTH);
        m_fileId = dc->m_fileId;
        m_fcacheOffset = dc->m_fcacheOffset;
        m_mdataOffset = dc->m_mdataOffset;
        m_totalFiles = dc->m_totalFiles;
        m_metaLength = dc->m_metaLength;
        m_fcacheFileId = dc->m_fcacheFileId;
    };

    explicit DirCache(const DirCacheV10 *dc)
    {
        m_inode = dc->m_inode;
        m_dirPathHash.crc = dc->m_hashTag;
        m_dirMetaHash.crc = dc->m_crc;
        m_fileId = dc->m_fileId;
        m_fcacheOffset = dc->m_fcacheOffset;
        m_mdataOffset = dc->m_mdataOffset;
        m_totalFiles = dc->m_totalFiles;
        m_metaLength = dc->m_metaLength;
        m_fcacheFileId = dc->m_fileId;
    };

    explicit DirCache(const DirCacheV20 *dc)
    {
        m_inode = dc->m_inode;
        m_dirPathHash.crc = dc->m_hashTag;
        m_dirMetaHash.crc = dc->m_crc;
        m_fileId = dc->m_fileId;
        m_fcacheOffset = dc->m_fcacheOffset;
        m_mdataOffset = dc->m_mdataOffset;
        m_totalFiles = dc->m_totalFiles;
        m_metaLength = dc->m_metaLength;
        m_fcacheFileId = dc->m_fcacheFileId;
    };
    ~DirCache() {};
};

class FileCacheV10 {
public:
    uint64_t m_inode = 0;       /* inode of file */
    uint64_t m_mdataOffset = 0; /* offset in meta data file */
    uint32_t m_hashTag = 0;     /* File hash CRC */
    uint32_t m_crc = 0;         /* CRC */
    uint16_t m_metaLength = 0;  /* Total meta length stored in meta file */
    uint16_t m_compareFlag = FCACHE_FILE_NOT_MODIFIED; /* Compare flag which is used at incremental scan */

    FileCacheV10() {};
    explicit FileCacheV10(const FileCacheV10 *fcache)
    {
        m_hashTag = fcache->m_hashTag;
        m_crc = fcache->m_crc;
        m_inode = fcache->m_inode;
        m_mdataOffset = fcache->m_mdataOffset;
        m_compareFlag = fcache->m_compareFlag;
        m_metaLength = fcache->m_metaLength;
    }
    ~FileCacheV10() {};
};

class FileCacheV20 {
public:
    uint64_t m_inode = 0;       /* inode of file */
    uint64_t m_mdataOffset = 0; /* offset in meta data file */
    uint32_t m_hashTag = 0;     /* File hash CRC */
    uint32_t m_crc = 0;         /* CRC */
    uint16_t m_fileId = 0;     /* metadata file id */
    uint16_t m_metaLength = 0;  /* Total meta length stored in meta file */
    uint16_t m_compareFlag = FCACHE_FILE_NOT_MODIFIED; /* Compare flag which is used at incremental scan */

    FileCacheV20() {};
    explicit FileCacheV20(const FileCacheV20 *fcache)
    {
        m_hashTag = fcache->m_hashTag;
        m_crc = fcache->m_crc;
        m_inode = fcache->m_inode;
        m_mdataOffset = fcache->m_mdataOffset;
        m_compareFlag = fcache->m_compareFlag;
        m_metaLength = fcache->m_metaLength;
        m_fileId = fcache->m_fileId;
    }
    ~FileCacheV20() {};
};

class FileCache {
public:
    uint64_t m_inode = 0;       /* inode of file */
    uint64_t m_mdataOffset = 0; /* offset in meta data file */
    uint16_t m_fileId = 0;     /* metadata file id */
    uint16_t m_metaLength = 0;  /* Total meta length stored in meta file */
    uint16_t m_compareFlag = FCACHE_FILE_NOT_MODIFIED; /* Compare flag which is used at incremental scan */
    Hash m_filePathHash {0};    /* Hash of full-path of the file */
    Hash m_fileMetaHash {0};    /* Hash of meta-data of the file */

    FileCache() {};
    explicit FileCache(const FileCache *fcache)
    {
        memcpy_s(m_filePathHash.sha1, SHA_DIGEST_LENGTH, fcache->m_filePathHash.sha1, SHA_DIGEST_LENGTH);
        memcpy_s(m_fileMetaHash.sha1, SHA_DIGEST_LENGTH, fcache->m_fileMetaHash.sha1, SHA_DIGEST_LENGTH);
        m_inode = fcache->m_inode;
        m_mdataOffset = fcache->m_mdataOffset;
        m_compareFlag = fcache->m_compareFlag;
        m_metaLength = fcache->m_metaLength;
        m_fileId = fcache->m_fileId;
    };

    explicit FileCache(const FileCacheV10 *fcache)
    {
        m_filePathHash.crc = fcache->m_hashTag;
        m_fileMetaHash.crc = fcache->m_crc;
        m_inode = fcache->m_inode;
        m_mdataOffset = fcache->m_mdataOffset;
        m_compareFlag = fcache->m_compareFlag;
        m_metaLength = fcache->m_metaLength;
    }

    explicit FileCache(const FileCacheV20 *fcache)
    {
        m_filePathHash.crc = fcache->m_hashTag;
        m_fileMetaHash.crc = fcache->m_crc;
        m_inode = fcache->m_inode;
        m_mdataOffset = fcache->m_mdataOffset;
        m_compareFlag = fcache->m_compareFlag;
        m_metaLength = fcache->m_metaLength;
        m_fileId = fcache->m_fileId;
    }
    ~FileCache() {};
};

struct HardlinkCtrlEntry {
    std::string metaFileName {};  /* Metafile where the hardlink metadata is saved */
    std::string dirName {};       /* Absolute path of parent directory */
    std::string fileName {};      /* Relative filename */
    uint32_t aclFlag = 0;         /* Boolean flag specifying whether there is acl info present in metadata */
    uint32_t m_hardLinkFilesCnt = 0;    /* No. of HardLink Files Present in a Dir */
    uint64_t metaFileOffset;      /* Offset inside the metafile to get the metadata for this hardlink */
    uint64_t metaFileReadLen;     /* Length of the metadata */
    uint64_t fileSize = 0;        /* ** Not present in control file ** to be used only to track m_dataSize */
};

struct HardlinkCtrlInodeEntry {
    uint32_t linkCount = 0; /* No of links with same inode */
    uint64_t inode = 0;     /* Inode value */
};

struct HardlinkFileCache {
    uint64_t m_inode = 0;       /* inode of file */
    std::string m_fileName {};
    std::string m_dirName {};
    uint64_t m_mdataOffset = 0; /* offset in meta data file */
    uint16_t m_metaLength = 0;  /* Total meta length stored in meta file */
    uint32_t m_aclFlag = 0;
    std::string m_metafileName {};
};

}
#endif // FS_SCANNER_PARSER_STRUCTS_H