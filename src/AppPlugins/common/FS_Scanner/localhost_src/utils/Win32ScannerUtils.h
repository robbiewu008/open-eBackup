/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Author: w30029850
 * Create: 2023/01/16
 */
 
#ifdef WIN32

#ifndef FS_SCANNER_WIN32_SCANNER_UTILS_H
#define FS_SCANNER_WIN32_SCANNER_UTILS_H

#include <optional>
#include "ScanStructs.h"
#include "ScanConfig.h"
#include "common/FileSystemUtil.h"

namespace Win32ScannerUtils {
    
    DirStat BuildDirStatFromStatResult(
        const Module::FileSystemUtil::StatResult&       statResult,
        const std::string&                              dirPath,
        const std::string&                              prefix,
        char                                            originDriver);

    void RecoverOriginWin32DirPathXMeta(
        Module::DirMetaWrapper&                         dirWrapper,
        const std::string&                              prefix,
        char                                            originDriver);

    Module::FileMetaWrapper BuildFileMetaWrapper(
        const Module::FileSystemUtil::StatResult&       statResult,
        const std::string&                              filepath,
        bool                                            detectSparseRange,
        bool                                            obtainSecurityDescriptor);

    std::optional<Module::FileMetaWrapper> GetReparsePointFileMeta(
        const Module::FileSystemUtil::StatResult&       statResult,
        const std::string&                              fullpath,
        const ScanConfig&                               scanConfig);

    Module::FileMeta BuildFileMetaFromStatResult(const Module::FileSystemUtil::StatResult& statResult);

    void ChangeXMetaPathToPosixStyle(std::vector<Module::XMetaField>& xMetaList, const std::string& win32Path);

    void CopyStatToDirMeta(Module::DirMeta &dmeta, const DirStat &dirStat);

    Module::DirMetaWrapper BuildDirMetaWrapper(const DirStat& dirStat, const ScanConfig& scanConfig);

    Module::XMetaField ReadSecurityDescriptorMeta(const std::string& fullpath);

    Module::XMetaField ReadSparseOffsetMeta(const std::string& fullpath);

    bool EnablePrivilege(const wchar_t* privilegeName);

    bool SafeReadFile(HANDLE hFile, void* buffer, DWORD bufferSize, DWORD &bytesRead, DWORD maxRetries = THREE_3);
}

#endif
#endif