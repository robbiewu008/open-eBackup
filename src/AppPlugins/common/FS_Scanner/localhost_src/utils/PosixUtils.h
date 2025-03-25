/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: z30016470
 * Create: 7/26/2022.
 */
#ifndef FS_SCANNER_POSIX_UTILS_H
#define FS_SCANNER_POSIX_UTILS_H

#include <dirent.h>
#include "ScanStructs.h"
#include "ScanConfig.h"

class PosixUtils {
public:
    PosixUtils() {};
    virtual ~PosixUtils() {};

    void WrapDirectory(Module::DirMetaWrapper &dirWrapper, DirStat& dirStat, const ScanConfig& config);
    void CopyStatToDirMeta(Module::DirMeta &dmeta, DirStat &dirStat);
    Module::XMetaField GetFileSparse(std::string path);
#if defined _AIX
    Module::XMetaField GetAcl4AIX(const std::string path);
#elif defined SOLARIS
    Module::XMetaField GetAcl4SOLARIS(const std::string path);
#else
    Module::XMetaField GetAccessAcl(std::string path);
    Module::XMetaField GetDefaultAcl(std::string path);
#endif

    bool ReadXattr(std::vector<Module::XMetaField>& xattrList, std::string path,
        char *key, ssize_t& keylen, ssize_t& buflen);
    std::vector<Module::XMetaField> GetXattr(std::string path);
    void RemovePathPrefixInDirectoryWrapper(Module::DirMetaWrapper &dirWrapper, std::string prefix);
    void RecoverDirectoryWrapperOriginPath(Module::DirMetaWrapper& dirWrapper, std::shared_ptr<PathMapper> pathMapper);
    std::string RemovePathPrefixFromString(std::string path, std::string prefix);
    void WrapFile(Module::FileMetaWrapper &fileWrapper, struct stat statbuf,
        std::string path, const ScanConfig& config);
    void CopyStatToDirStat(DirStat &dstat, const struct stat &statbuf, std::string path);
    void CopyStatToFileMeta(Module::FileMeta &fd, const struct stat &statbuf);
};

#endif