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
#ifndef WIN32BACKUPENGINEUTILS_H
#define WIN32BACKUPENGINEUTILS_H

#include <string>
#include <map>
#include "log/BackupFailureRecorder.h"
#include "BackupStructs.h"
#include "FileSystemUtil.h"

namespace FS_Backup {

namespace Win32BackupEngineUtils {

    std::string ReverseSlash(const std::string& path);
    
    std::string PathConcat(
        const std::string& forwardPath,
        const std::string& trailPath,
        const std::string& trimPrefixPath = "");

    /* extended unicode path to break MAX_PATH 260 limit */
    std::wstring ExtenedPathW(const std::string& path);
    
    std::string ExtenedPath(std::string path);
    
    std::string ParseErrorMessage(uint32_t winErrorID);

    bool CreateDirectoryRecursively(const std::string& dirPath, const std::string& rootPath, DWORD& errorCode);

    bool IsFileHandleSymbolicLink(const FileHandle& fileHandle);

    bool IsFileHandleJunctionPoint(const FileHandle& fileHandle);

    std::string GetSymbolicLinkTargetPath(const FileHandle& fileHandle);

    std::string GetJunctionPointTargetPath(const FileHandle& fileHandle);

    std::string RemoveExtraSlash(const std::string& path);

    bool RemovePath(const std::string& path);
    
    void ConvertUint64ToDword(
        _In_    uint64_t    source,
        _Out_   DWORD&      dwordHigh,
        _Out_   DWORD&      dwordLow);

    inline bool FileHandleHasAttribute(const FileHandle& fileHandle, DWORD attribute)
    {
        return ((fileHandle.m_file->m_fileAttr & attribute) != 0);
    }

    /*
     * if need to force to process the FileHandle as a sparse file
     * (may need to dismiss the small file optimization branch)
     */
    inline bool ProcessFileHandleAsSparseFile(bool writeSparseFile, const FileHandle& fileHandle)
    {
        return writeSparseFile &&
            FileHandleHasAttribute(fileHandle, FILE_ATTRIBUTE_SPARSE_FILE) &&
            fileHandle.m_block.m_size != 0;
    }
 
    inline bool IsEmptySparseFile(const FileHandle& fileHandle)
    {
        return fileHandle.m_block.m_size == 0 && FileHandleHasAttribute(fileHandle, FILE_ATTRIBUTE_SPARSE_FILE);
    }

    std::optional<std::wstring> GetStreamNameW(const std::wstring& wStreamName);

    std::wstring GetUnusedNewPathW(const std::wstring& wpath);

    bool  IsFileExistsW(const std::wstring& wpath);
};

}

#endif