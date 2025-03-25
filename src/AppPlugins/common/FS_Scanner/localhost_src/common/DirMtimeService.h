/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: g00554214
 * Create: 2022/08/23
 */

#ifndef FS_SCANNER_DIR_MTIME_PRODUCER_H
#define FS_SCANNER_DIR_MTIME_PRODUCER_H

#include <memory>
#include <string>
#include "ScanConfig.h"
#include "ScanInfo.h"
#include "ScanStructs.h"
#include "StatisticsMgr.h"
#include "ScanConsts.h"

class DirMtimeService {
public:
    explicit DirMtimeService(
        ScanConfig &config,
        std::shared_ptr<StatisticsMgr> statsMgr)
        : m_config(config), m_statsMgr(statsMgr)
    {
    };
    ~DirMtimeService() {};

    virtual SCANNER_STATUS ProcessMtimeFile(const std::string mtimeFile);
    virtual SCANNER_STATUS InitContext();

protected:
    ScanConfig &m_config;
    std::shared_ptr<StatisticsMgr> m_statsMgr;
};
#endif