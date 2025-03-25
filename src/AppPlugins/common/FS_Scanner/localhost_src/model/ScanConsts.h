/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: g00554214
 * Create: 10/7/2020.
 */
#ifndef FS_SCANNER_SCAN_CONSTS_H
#define FS_SCANNER_SCAN_CONSTS_H

#include <cstdint>
#include <sys/stat.h>
#include <string>
#include <functional>

#define FS_SCAN_CACHE_FH
#define FS_SCAN_ENABLE_META_WRITE
#define FS_SCAN_WRITE_TO_CTRL_BFR
#define FS_SCAN_WRITE_TO_CTRL_FILE
#define FS_SCAN_HASH_COMPARE


#ifdef HAVE_UNISTD_H
#undef HAVE_UNISTD_H
#define HAVE_UNISTD_H 0
#endif

static const double BYTES_IN_GB = 1073741824.0;

constexpr uint8_t FS_SCAN_MOUNT_RETRY_MAX = 3;
constexpr uint32_t FS_SCAN_MX_RNDM = 10000;
#ifndef WIN32
constexpr mode_t FS_SCAN_SET_ALL_PERMSN = 0777;
#endif
constexpr uint8_t FS_SCAN_WORD_BYTE_LOW = 2;
constexpr uint8_t FS_SCAN_WORD_BYTE_HI = 2;
constexpr uint32_t SCANNER_ABS_PATH_LEN_MAX = 4096;
constexpr uint32_t SCANNER_GBL_DIRQ_SZ_MIN = 8000;
constexpr uint32_t SCANNER_GBL_DIRQ_SZ_MAX = 10000;
constexpr uint32_t STRING_SIZE = 100;

#ifdef WIN32
const std::string PREVIOUSCONTROL = "\\previousControl";
const std::string PREVIOUS = "\\previous";
const std::string LATESTCONTROL = "\\latestControl";
const std::string LATEST = "\\latest";
const std::string PATH_SEPERATOR = "\\";
#else
const std::string PREVIOUSCONTROL = "/previousControl";
const std::string PREVIOUS = "/previous";
const std::string LATESTCONTROL = "/latestControl";
const std::string LATEST = "/latest";
const std::string PATH_SEPERATOR = "/";
#endif

const std::string DOT_TXT = ".txt";
const std::string FCACHE_MOD_TYPE_NEW_FILE = "nn";
const std::string FCACHE_MOD_TYPE_DEL_FILE = "dd";
const std::string FCACHE_MOD_TYPE_MOD_FILE_DATA = "dm";
const std::string FCACHE_MOD_TYPE_MOD_FILE_META = "mm";
const std::string FCACHE_MOD_TYPE_MOD_FILE_META_AND_DATA = "bm";
const std::string DCACHE_MOD_TYPE_NEW_DIR = "nn";
const std::string DCACHE_MOD_TYPE_DEL_DIR = "dd";
const std::string DCACHE_MOD_TYPE_MOD_DIR_DATA = "dm";
const std::string DCACHE_MOD_TYPE_MOD_DIR_META = "mm";
const std::string DCACHE_MOD_TYPE_MOD_DIR_FILE_DATA = "fm";
const std::string DCACHE_MOD_TYPE_MOD_DIR_META_AND_DATA = "bm";
const std::string FS_SCAN_NFS_CLIENT_TYPE_LIBNFS = "libnfs";
const std::string FS_SCAN_NFS_CLIENT_TYPE_LIBNFS_ASYNC = "libnfs_async";
const std::string FS_SCAN_SMB_CLIENT_TYPE_LIBSMB_ASYNC = "libsmb_async";
constexpr uint16_t TEMP_LIST_SIZE = 10000;
constexpr uint8_t CTRL_FILE_TIME = 5;
constexpr uint16_t READ_BUFFER_SIZE = 1000;
constexpr uint32_t DCACHE_ENTRY_BATCH_READ_CNT = 10000;
const std::string SCAN_FULL = "FULL";
const std::string SCAN_INC = "INCREMENTAL";
constexpr int SCAN_PROTOCOL_NFS = 1;
constexpr int SCAN_PROTOCOL_CIFS = 0;
constexpr uint16_t TEN_MIN_IN_SECONDS = 600;
constexpr uint16_t MAX_PROTECTED_SERVER_FAIL_CNT = 100;
constexpr uint64_t TEN_GB_IN_BYTES = 10737418240;
constexpr uint64_t FIVE_GB_IN_BYTES = 5368709120;
constexpr uint32_t ONE_GB_IN_BYTES = 1073741824;
constexpr uint32_t POLL_WAIT_10000MS = 10000;
constexpr int CURRENT_WORKING_TASK = 0;

enum class ScanTaskLevel {
    REDO = 1,
    REGULAR = 2,
    LOWLEVEL = 3
};

enum class ScanJobType {
    FULL = 0,
    INC = 1,
    RESTORE = 2,
    INDEX = 3,
    ARCHIVE = 4,
    CONTROL_GEN = 5,
    RFI_GEN = 6,
    SNAPDIFFNAS_GEN = 7
};
enum class IOEngine {
    DEFAULT = 0,
	LIBNFS = 1,
	LIBSMB2 = 2,
	POSIX = 3,
	WIN32_IO = 4,
	SNAPDIFFNAS = 5,
    OBJECTSTORAGE = 6
};
enum class SCANNER_STATUS {
    FREE = -8,
    INCOMPLETE_SCAN_REACH_LIMIT = -7,
    PROTECTED_SERVER_NOT_REACHABLE = -6,
    SECONDARY_SERVER_NOT_REACHABLE = -5,
    ERROR_INC_TO_FULL = -4,
    ABORTED = -2,
    ABORT_IN_PROGRESS = -3,
    FAILED = -1,
    SUCCESS = 0,
    INIT = 1,
    SCAN_IN_PROGRESS = 2,
    SCAN_COMPLETED = 3,
    SCAN_READ_IN_PROGRESS = 4,
    SCAN_READ_COMPLETED = 5,
    META_WRITE_IN_PROGRESS = 6,
    META_WRITE_IN_FLUSH = 7,
    META_WRITE_COMPLETED = 8,
    CACHE_MERGE_IN_PROGRESS = 9,
    CACHE_MERGE_COMPLETED = 10,
    CTRL_DIFF_IN_PROGRESS = 11,
    CTRL_DIFF_COMPLETED = 12,
    SNAPDIFF_IN_PROGRESS = 13,
    SNAPDIFF_COMPLETED = 14,
    CLEAN_IN_PROGRESS = 15,
    CLEAN_COMPLETED = 16,
    COMPLETED = 17,
};

enum class CHECKPOINT_STATUS {
    FAILED = -1,
    SUCCESS = 0,
    INIT = 1,
    IDLE = 2,
    IN_PROGRESS = 3,
    COMPLETED = 4,
};

enum NAS_METABACKUP_TYPE_E {
    METABACKUP_TYPE_ERR              = -1,
    METABACKUP_TYPE_FOLDER_ONLY      = 1,
    METABACKUP_TYPE_FILE_AND_FOLDER  = 2,
};

enum class NAS_PROTOCOL {
    NFS,
    SMB
};

enum class TRANSLATE_TYPE {
    META_TO_METARW = 0,
    METARW_TO_META = 1,
    STAT_TO_STATRW = 2,
    STATRW_TO_STAT = 3,
};

enum SCAN_HASH_TYPE {
    CRC = 0,
    SHA_1 = 1
};

constexpr uint8_t INDEX_DIR_ENTRY_SIZE = 5;
constexpr uint8_t INDEX_ARCHIVE_ENTRY_SIZE = 1;
constexpr uint8_t INDEX_FILE_ENTRY_SIZE = 4;

constexpr uint8_t FILE_MAP_ENTRY_SIZE = 2;

constexpr uint32_t SCANNER_PATH_LEN_MAX = 4096;
constexpr uint32_t SCANNER_FILE_NAME_LEN_MAX = 255;
constexpr uint32_t ACL_MAX_LEN = 1000;
constexpr uint32_t SCANNER_ARCHIVE_PATH_MAX = 4096;

constexpr uint32_t BACKUP_ENTRY_F_META_CHANGED = 1;
constexpr uint32_t BACKUP_ENTRY_F_DATA_CHANGED = 2;
constexpr uint32_t BACKUP_ENTRY_F_FILE_DELETED = 4;
constexpr uint32_t BACKUP_ENTRY_F_EXT_ACL = 8;

constexpr uint8_t ZERO_0 = 0;
constexpr uint8_t THREE_3 = 3;
constexpr uint8_t FOUR_4 = 4;
constexpr uint8_t FIVE_5 = 5;
constexpr uint8_t SIX_6 = 6;
constexpr uint8_t SEVEN_7 = 7;
constexpr uint8_t EIGHT_8 = 8;

constexpr uint32_t RATELIMIT_INTERVAL = 120;
constexpr uint32_t MAX_BUCKET_NUM_ONCE = 1;

#endif // FS_SCANNER_SCAN_CONSTS_H
