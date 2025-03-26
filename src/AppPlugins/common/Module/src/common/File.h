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
#ifndef MODULE_FILE_H
#define MODULE_FILE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include "define/Defines.h"

namespace Json {
    class Value;
}

#ifdef WIN32
#define S_ISDIR(m) ((m)&_S_IFDIR)
#endif

#define GETOSERRORCODE(a)                                                                                         \
    do {                                                                                                          \
        if (a) {                                                                                                  \
            COMMLOG(OS_LOG_ERROR, "Call fclose failed, errno[%d]:%s.", errno, strerror(errno)); \
            return FAILED;                                                                                     \
        }                                                                                                         \
    } while (0)

namespace Module {

class AGENT_API CFile {
public:
    static int OpenFile();
    static bool FileExist(const std::string& pszFilePath);
    static bool DirExist(const char* pszDirPath);
    static int CreateDir(const char* pszDirPath);
    static int CreateDir2(const char* pszDirPath);
    static int DelDir(const char* pszDirPath);
    static int GetPathFromFilePath(const std::string& strFileFullPath, std::string& strFilePath);
    static int GetNameFromFilePath(const std::string &strFileFullPath, std::string &strFileName);
    static int FileSize(const char* pszFilePath, uint32_t& uiSize);
    static int GetlLastModifyTime(const char* pszFilePath, time_t& tLastModifyTime);
    static int ReadFile(const std::string& filePath, std::string& fileContent);
    static int ReadFileWithArray(std::string& strFilePath, std::vector<std::string>& vecOutput);
    static int GetFolderFile(std::string& strFolder, std::vector<std::string>& vecFileList);
    static bool WaitForFile(const std::string& pszFilePath, int iSleepInterval, int iSleepCount);
    static int DelFile(const std::string& pszFilePath);
    static int CreateFile(const std::string& pszFilePath);
    static int CheckFileAccess(const std::string& pszFilePath);
    static int CopyFileCoverDest(const std::string& pszSourceFilePath, const std::string& pszDestFilePath);
    static int DelDirAndSubFile(const char* pszDirPath);
    static bool DirEmpty(const std::string& dirPath);
    static bool GetProfileSection(const std::string& file, const std::string& key, std::string& value);
    static int CheckFileName(const std::string &filename);
    static int GetFolderDir(const std::string& strFolder, std::vector<std::string>& vecNameList);
private:
    static int ReadFileAfter(FILE* pFile, int iFileLen, std::vector<std::string>& vecOutput);
    static void Getfilepath(const char *path, const char *filename,  char *filepath, int strfileLen);
    static bool DeleteFile(const char* path, int strfileLen);
};

// ICP文件，用于进程间通信
static const std::string RESULT_ERRDETAIL_FILE = "result_errordetail";
static const std::string RESULT_WARNINFO_FILE = "result_warninginfo";
static const std::string RESULT_TMP_FILE = "result_tmp";
static const std::string INPUT_TMP_FILE  = "input_tmp";
static const std::string TOP_TMP_FILE    = "top_tmp";
static const std::string EN_TMP_FILE     = "en_tmp";
static const std::string PROGRESS_TMP_FILE     = "progress_tmp";

class AGENT_API CIPCFile {
public:
    static int ReadFile(std::string& strFilePath, std::vector<std::string>& vecOutput);
    static int WriteFile(std::string& strFilePath, std::vector<std::string>& vecInput);
    static int WriteInput(std::string& strUniqueID, const std::string& strInput);
    static int ReadInput(std::string& strUniqueID, std::string& strInput);
    static int WriteResult(std::string& strUniqueID, std::vector<std::string>& vecRlt);
    static int ReadResult(const std::string& strFileName, std::vector<std::string>& vecRlt);
    static int ReadOldResult(std::string& strUniqueID, std::vector<std::string>& vecRlt);
    static int ChownResult(std::string& strUniqueID);
    static int RemoveFile(const std::string& strUniqueID);
    static int IsValidFilename(const std::string& strFilename);
    static int WriteJsonStrToKVFile(const std::string& strFileName, Json::Value jsonValue,
                                         const std::string& KVSplitStr);

private:
#ifndef WIN32
    static int WriteFileInner(std::string &strFilePath, std::vector<std::string> &vecInput, 
        int fileExist, struct stat &fileStat);
#else
    static int WriteFileInner(std::string &strFilePath, std::vector<std::string> &vecInput);
#endif
};

} // namespace Module

#endif  // AGENT_FILE_H