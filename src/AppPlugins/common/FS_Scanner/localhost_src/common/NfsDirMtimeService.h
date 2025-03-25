/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: g00554214
 * Create: 2022/08/23
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