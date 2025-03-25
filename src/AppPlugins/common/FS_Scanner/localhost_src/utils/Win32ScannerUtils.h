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