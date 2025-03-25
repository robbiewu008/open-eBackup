/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: z30016470
 * Create: 2022/06/30
 */
#ifndef FS_SCANNER_SCAN_STRUCTS_H
#define FS_SCANNER_SCAN_STRUCTS_H

#include "ParserStructs.h"
#include <queue>
#include <memory>
#ifdef _NAS
#include "NfsContextWrapper.h"
#endif

enum class DirStatFlag {
    BUCKET = 1, /* */
    PREFIX = 2  /* 桶的前缀 */
};

struct ObjectMetaData {
    uint64_t size;
    uint64_t lastModified;
    std::string etag;
    std::string metaData;
};

struct DirStat {
#ifdef _NAS
    nfs_fh_scan m_fh {};
#endif
    uint64_t m_size {0};
    uint64_t m_inode {0};
    uint64_t m_atime {0};
    uint64_t m_mtime {0};
    uint64_t m_ctime {0};
    uint64_t m_btime {0};
    uint32_t m_attr {0}; // used for win32 or smb
    uint32_t m_mode {0}; // used for linux or nfs
    uint32_t m_gid {0};
    uint32_t m_uid {0};
    uint8_t m_filterFlag {0};
    bool m_isEmpty = false;
    char m_originDriver = '\0'; // only used in windows to recover origin path
    uint8_t flag;
    std::string m_path = "";
    std::string m_prefix = ""; // should be deprecated in V2 methods
    std::string m_originPrefix = "";
    std::string m_obsMetaData;
    std::vector<std::string> filter;
};

struct FileStat {
    uint64_t m_size;
    uint64_t m_atime;
    uint64_t m_mtime;
    uint64_t m_ctime;
    uint64_t m_gid;
    uint64_t m_uid;
    uint64_t m_mode;
    uint64_t m_inode;
    uint64_t m_rdev;
    uint64_t m_nlink;

    std::string m_key; // 保存对象的key，需要通过key来获取对象元数据
    std::string m_etag;
    std::string m_metaData; // 对象储存的 sysDefMetaData & userDefMetaData
    std::string m_path = "";
    std::string m_prefix = "";
    uint8_t m_filterFlag {0};
};

struct DirStatReadWrite {
#ifdef _NAS
    nfs_fh_scan m_fh {};
#endif
    uint64_t m_size {0};
    uint64_t m_inode {0};
    uint64_t m_atime {0};
    uint64_t m_mtime {0};
    uint64_t m_ctime {0};
    uint64_t m_btime {0};
    uint32_t m_attr {0};
    uint32_t m_mode {0};
    uint32_t m_uid {0};
    uint32_t m_gid {0};
    uint8_t m_filterFlag {0};
    bool m_isEmpty = false;
    char m_originDriver = '\0'; // only used in windows to recover origin path

    uint16_t m_pathLen {0};
    uint16_t m_prefixLen {0};
};

struct DirectoryScan {
    DirectoryScan () { m_fmWrapperList = std::make_shared<std::queue<Module::FileMetaWrapper>>(); }
    ~DirectoryScan () {}
    Module::DirMetaWrapper m_dmWrapper {};  /* Directory meta object */
    std::shared_ptr<std::queue<Module::FileMetaWrapper>> m_fmWrapperList { nullptr }; /* File meta list */
    int m_isResumeCalled = 0;  /* if resume api callled then we should combine previous filelist result */
    bool m_isDirScanCompleted = false;
    uint32_t m_filterFlag = 0;
};

#endif