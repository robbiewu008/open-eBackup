/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Author: a00587389
 * Create: 15/12/2021.
*/

#ifndef DME_NAS_SCANNER_CONTROLFILEUTILS_H
#define DME_NAS_SCANNER_CONTROLFILEUTILS_H

#include <string>
#include <memory>
#include "ScanConfig.h"
#include "ScanInfo.h"
#include "MetaParser.h"
#include "XMetaParser.h"
#include "FileCacheParser.h"
#include "DirCacheParser.h"
#include "CopyCtrlParser.h"
#include "DeleteCtrlParser.h"
#include "HardlinkCtrlParser.h"
#include "MtimeCtrlParser.h"
#include "AdsParser.h"

class ControlFileUtils {
public:
    ControlFileUtils() {};
    virtual ~ControlFileUtils() {};
    std::shared_ptr<Module::MetaParser> CreateMetaFileObj(std::string filename, Module::CTRL_FILE_OPEN_MODE mode,
        ScanConfig &config);
    std::shared_ptr<Module::MetaParser> CreateMetaFileObj(std::string filename);
    void FillMetaFileParams(Module::MetaParser::Params &metaParams, ScanConfig &config);

    std::shared_ptr<Module::XMetaParser> CreateXMetaFileObj(std::string filename, Module::CTRL_FILE_OPEN_MODE mode,
        ScanConfig &config);
    std::shared_ptr<Module::XMetaParser> CreateXMetaFileObj(std::string filename);
    void FillXMetaFileParams(Module::XMetaParser::Params &xmetaParams, ScanConfig &config);

    std::shared_ptr<Module::DirCacheParser> CreateDcacheObj(std::string fname, Module::CTRL_FILE_OPEN_MODE mode,
        ScanConfig &config);
    std::shared_ptr<Module::DirCacheParser> CreateDcacheObj(const std::string& fname);
    void FillDCacheFileParams(Module::DirCacheParser::Params &dcacheParams, ScanConfig &config);
    std::shared_ptr<Module::FileCacheParser> CreateFileCacheObj(std::string filename, Module::CTRL_FILE_OPEN_MODE mode,
        ScanConfig &config);
    std::shared_ptr<Module::FileCacheParser> CreateFileCacheObj(const std::string& filename);
    std::shared_ptr<Module::MtimeCtrlParser> CreateDirectoryMtimeObj(std::string fname,
        ScanConfig &config, Module::CTRL_FILE_OPEN_MODE mode);
    std::shared_ptr<Module::DeleteCtrlParser> CreateDeleteCtrlObj(std::string fname, ScanConfig &config);

    std::shared_ptr<Module::AdsParser> CreateAdsMetaFileObj(const std::string& filename,
        Module::CTRL_FILE_OPEN_MODE mode, const ScanConfig &config);
    std::shared_ptr<Module::AdsParser> CreateAdsMetaFileObj(const std::string& filename);

    bool IsHardlinkFile(Module::FileMeta &fmeta);
    std::string GetMetaFileName(std::string dir, int id) const;
    std::string GetMetaFileCountName(std::string dir) const;
    std::string GetMetaFileName(int id) const;
    std::string GetXMetaFileName(std::string dir, int id) const;
    std::string GetXMetaFileName(int id) const;
    std::string GetFileCacheFileName(std::string dir, int id) const;
    std::string GetDirCacheFileName(std::string dir) const;
    std::string GetControlFileName(std::string directory, int threadId) const;
    std::string GetMtimeFileName(std::string directory, int threadId) const;
    std::string GetHardCtrlFileName(std::string directory, int threadId) const;
    std::string GetFileModifiedFlag(Module::FileMetaWrapper &fmWrapperOne, Module::FileMetaWrapper &fmWrapperTwo);
    bool IsFileMetaModified(Module::FileMetaWrapper &fmWrapperOne, Module::FileMetaWrapper &fmWrapperTwo);
    std::string GetDeleteCtrlFileName(std::string directory, int threadId) const;
    bool GetMetaFileCountFromFile(std::string fileName, uint32_t &metaCount, uint32_t &xMetaCount,
        uint32_t &fcacheCount, uint32_t maxCommonServiceInstance);

    bool m_stopScan = false;
};

#endif  // DME_NAS_SCANNER_CONTROLFILEUTILS_H