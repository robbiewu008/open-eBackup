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