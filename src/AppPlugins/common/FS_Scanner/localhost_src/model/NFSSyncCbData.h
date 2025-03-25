/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: g00554214
 * Create: 8/8/2022
 */

#ifndef FS_SCANNER_SYNC_CB_DATA_H
#define FS_SCANNER_SYNC_CB_DATA_H
#include <string.h>
#include "ScanStructs.h"
#include "StatisticsMgr.h"
#include "ParserStructs.h"
#include "NfsContextWrapper.h"
#include "ScanConfig.h"
#include "OpendirResData.h"

#define DME_NAS_SCAN_CACHE_FH
#define DME_NAS_SCAN_ENABLE_META_WRITE
class NFSSyncCbData {
public:
    int m_status = 0;
    int m_isResumeCalled = 0;
    uint8_t m_filterFlag {0};
    bool m_isFilesExistsInPrevReadDir = false;
    std::string m_basePath {};
    Module::DirMeta m_dmeta {};
    nfs_fh_scan m_fh {};
    void *m_ptr = nullptr;
    std::shared_ptr<StatisticsMgr> m_statsMgr = nullptr;
    NFSSyncCbData() {};
    ~NFSSyncCbData() {};
};

using NfsdirPath = struct {
    struct nfsdir *dir = nullptr;
    std::string basePath = "";
};
#endif