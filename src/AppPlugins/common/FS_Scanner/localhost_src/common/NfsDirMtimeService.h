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
#ifndef FS_SCANNER_NFS_DIR_MTIME_PRODUCER_H
#define FS_SCANNER_NFS_DIR_MTIME_PRODUCER_H

#include <dirent.h>
#include <thread>
#include "NfsContextWrapper.h"
#include "ScanConfig.h"
#include "ScanInfo.h"
#include "OpendirResData.h"
#include "NFSSyncCbData.h"
#include "BufferQueue.h"
#include "ScanStructs.h"
#include "StatisticsMgr.h"
#include "ScanConsts.h"
#include "ScanConsts.h"
#include "ParserStructs.h"
#include "DirMtimeService.h"
#include "MtimeCtrlParser.h"
#include "ControlFileUtils.h"
#include "Crc32.h"

class NfsDirMtimeService : public DirMtimeService {
public:
    explicit NfsDirMtimeService(
        ScanConfig &config,
        std::shared_ptr<StatisticsMgr> statsMgr)
        : DirMtimeService(config, statsMgr)
    {
    };
    ~NfsDirMtimeService() {};

    SCANNER_STATUS ProcessMtimeFile(const std::string mtimeFile) override;
    SCANNER_STATUS InitContext() override;

private:
    std::shared_ptr<Module::NfsContextWrapper> m_nfsCtx;

    SCANNER_STATUS ParseMtimeDirAndPushToMap(std::string dirPath, std::vector<uint32_t> &mtimeDirHashList,
        std::shared_ptr<Module::MtimeCtrlParser> backupMtimeCtrl);
    uint32_t CreateHash(const std::string dirPath) const;
    SCANNER_STATUS HandleDirsNotInMtimeFiles(std::string dirPath,
        std::shared_ptr<Module::MtimeCtrlParser> backupMtimeCtrl);
    SCANNER_STATUS FillMtimeCtrlEntry(Module::MtimeCtrlEntry &mtimeEntry, std::string dirPath);
    inline bool IsRetryMount(int32_t errorCode) const;
};
#endif