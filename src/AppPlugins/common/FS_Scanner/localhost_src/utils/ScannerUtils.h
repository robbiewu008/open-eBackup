/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Author: z30016470
 * Create: 6/24/2022.
 */
#ifndef FS_SCANNER_SCANNER_UTILS_H
#define FS_SCANNER_SCANNER_UTILS_H

#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include "ParserStructs.h"
#include "ScanConsts.h"
#include "common/Snowflake.h"
#include "common/Utils.h"
#include "StatisticsMgr.h"

namespace FS_SCANNER {

    std::string GetParentDirOfFile(const std::string& filePath);

    std::string GetFileNameOfPath(std::string filePath);

    bool CheckParentDirIsReachable(const std::string& path);

    uint32_t GetCommaCountOfString(const std::string &strName);

    std::string ConstructStringName(uint32_t &offset, uint32_t &totCommaCnt, std::vector<std::string> &lineContents);

    uint32_t GetRandomNumber(uint32_t minNum, uint32_t maxNum);

    uint16_t Atou16(const char *s);

    bool RemoveFile(const std::string &path);

    bool Rename(const std::string &oldName, const std::string &newName);

    bool ReadFile(const std::string &filePath, std::vector<std::string>& fileContent);
    bool CopyFile(const std::string& srcName, const std::string& dstDir);

    std::string GetUniqueId();

    std::string GetPathFromXMeta(const std::vector<Module::XMetaField> &xMeta);

    std::string GetFileOrDirNameFromXMeta(const std::vector<Module::XMetaField> &xMeta);

    std::string GetAclFromXMeta(const std::vector<Module::XMetaField> &xMeta);

    std::string GetSecurityDescriptorFromXMeta(const std::vector<Module::XMetaField> &xMeta);

    std::string GetObjectEtagAttrFromXMeta(const std::vector<Module::XMetaField> &xMeta);

    std::string GetScanTypeStr(ScanJobType scanType);

    bool CheckAndCreateDirectory(const std::string& dir);

    bool RemoveDir(const std::string &dirName);

    bool CreateDir(const std::string &dirName);
    bool CreateDirRecurve(const std::string& path);

    bool PathExist(const std::string &path);

    bool GetFileListInDirectory(const std::string &path, std::vector<std::string> &fileList);

    uint16_t MaxFileNumber(std::string fileName);

    std::string GetScanStatusFileName(const std::string &dir);

    bool UpdateScannerStatusInFile(SCANNER_STATUS status, const std::string &metaPath);

    bool ReadScannerStatusFromFile(SCANNER_STATUS &status, const std::string &metaPath);

    int64_t ConvertMicroSecToSec(int64_t microSec);

    bool IsPluginInstallationRoot(const std::string& path);

    void RecordFailedScanEntry(
        const std::string& path, const std::string& errorMessage, std::shared_ptr<StatisticsMgr> statsMgr);

    void RecordFailedScanEntry(
        const std::string& path, int errorCode, std::shared_ptr<StatisticsMgr> statsMgr);

    inline std::string StringFrom(const char* ptr)
    {
        return ptr == nullptr ? "" : std::string(ptr);
    }

    void CheckArchiveUnsupportPath(
        const std::string& path,
        const std::string& filename,
        std::shared_ptr<StatisticsMgr> statsMgr);

    // used for windows fs skip dir check (case insensitive)
    bool InSkipDirNameCaseInsensitive(const std::string& dirName, const std::vector<std::string>& skipLists);
    bool IsEndsWith(const std::string& str, const std::string& suffix);
    void MemoryTrim();
    std::string ParseErrorCode(int errorCode);

}
#endif // FS_SCANNER_SCANNER_UTILS_H
