/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file File.h
 * @brief  Contains function declarations File Operations
 * @version 1.0.0
 * @date 2020-08-01
 * @author yangwenjun 00275736
 */
#ifndef __AGENT_FILE_H__
#define __AGENT_FILE_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include "common/Defines.h"

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
            return MP_FAILED;                                                                                     \
        }                                                                                                         \
    } while (0)

class AGENT_API CMpFile {
public:
    static mp_bool FileExist(const mp_string& pszFilePath);
    static mp_bool DirExist(const mp_char* pszDirPath);
    static mp_int32 CreateDir(const mp_char* pszDirPath);
    static mp_int32 DelDir(const mp_char* pszDirPath);
    static mp_int32 GetPathFromFilePath(const mp_string& strFileFullPath, mp_string& strFilePath);
    static mp_int32 GetNameFromFilePath(const mp_string &strFileFullPath, mp_string &strFileName);
    static mp_int32 FileSize(const mp_char* pszFilePath, mp_uint32& uiSize);
    static mp_int32 GetlLastModifyTime(const mp_char* pszFilePath, mp_time& tLastModifyTime);
    static mp_int32 ReadFile(const mp_string& strFilePath, std::vector<mp_string>& vecOutput);
    static mp_int32 GetFolderFile(mp_string& strFolder, std::vector<mp_string>& vecFileList);
    static mp_int32 GetFolderDir(const mp_string& strFolder, std::vector<mp_string>& vecNameList);
    static mp_int32 GetFolderDirUmountPoints(const mp_string& strFolder, std::vector<mp_string>& vecNameList);
    static mp_bool WaitForFile(const mp_string& pszFilePath, mp_int32 iSleepInterval, mp_int32 iSleepCount);
    static mp_int32 DelFile(const mp_string& pszDirPath);
    static mp_int32 CreateFile(const mp_string& pszFilePath);
    static mp_int32 CheckFileAccess(const mp_string& pszFilePath);
    static mp_int32 CopyFileCoverDest(const mp_string& pszSourceFilePath, const mp_string& pszDestFilePath);
    static mp_int32 DelDirAndSubFile(const mp_char* pszDirPath);
    static mp_bool DirEmpty(const mp_string& dirPath);
    static mp_bool GetProfileSection(const mp_string& file, const mp_string& key, mp_string& value);
    static mp_int32 CheckFileName(const std::string &filename);
    static bool DeleteAllFileInPath(const mp_string& path);
private:
    static bool DeleteFile(const mp_char* path, mp_int32 strfileLen);
    static mp_int32 ReadFileAfter(FILE* pFile, mp_int32 iFileLen, std::vector<mp_string>& vecOutput);
    static void Getfilepath(const mp_char *path, const mp_char *filename,  mp_char *filepath, mp_int32 strfileLen);
};

// ICP文件，用于进程间通信
static const mp_string RESULT_ERRDETAIL_FILE = "result_errordetail";
static const mp_string RESULT_WARNINFO_FILE = "result_warninginfo";
static const mp_string RESULT_TMP_FILE = "result_tmp";
static const mp_string INPUT_TMP_FILE  = "input_tmp";
static const mp_string TOP_TMP_FILE    = "top_tmp";
static const mp_string EN_TMP_FILE     = "en_tmp";
static const mp_string PROGRESS_TMP_FILE     = "progress_tmp";

class AGENT_API CIPCFile {
public:
    static mp_int32 ReadFile(mp_string& strFilePath, std::vector<mp_string>& vecOutput);
    static mp_int32 WriteFile(const mp_string& strFilePath, const std::vector<mp_string>& vecInput);
    static mp_int32 WriteInput(const mp_string& strUniqueID, const mp_string& strInput);
    static mp_int32 ReadInput(const mp_string& strUniqueID, mp_string& strInput);
    static mp_int32 WriteResult(const mp_string& strUniqueID, std::vector<mp_string>& vecRlt);
    static mp_int32 ReadResult(const mp_string& strFileName, std::vector<mp_string>& vecRlt);
    static mp_int32 ReadOldResult(const mp_string& strUniqueID, std::vector<mp_string>& vecRlt);
    static mp_int32 ChownResult(const mp_string& strUniqueID);
    static mp_int32 RemoveFile(const mp_string& strUniqueID);
    static mp_int32 IsValidFilename(const mp_string& strFilename);
    static mp_int32 WriteJsonStrToKVFile(const mp_string& strFileName, const Json::Value& jsonValue,
                                         const mp_string& KVSplitStr);
    static mp_int32 AppendFile(const mp_string& strFilePath, const std::vector<mp_string>& vecInput);

private:
#ifndef WIN32
    static mp_int32 WriteFileInner(const mp_string &strFilePath, const std::vector<mp_string> &vecInput,
        mp_int32 fileExist, struct stat &fileStat);
#else
    static mp_int32 WriteFileInner(const mp_string &strFilePath, const std::vector<mp_string> &vecInput);
#endif
};

#endif  // __AGENT_FILE_H__
