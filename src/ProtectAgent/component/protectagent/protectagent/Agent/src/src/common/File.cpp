/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file File.cpp
 * @brief  Contains function declarations FILE Operations
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifdef WIN32
#include <tchar.h>
#include <atlstr.h>
#include <direct.h>
#include <io.h>
#else
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <iostream>
#include <fstream>
#include "common/Defines.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "securec.h"
#include "common/Path.h"
#include "common/JsonUtils.h"
#include "common/ErrorCode.h"
#include "common/File.h"

using namespace std;
namespace {
const mp_int32 VM_UUDI_FILELEN = 500;
const string FILENAME_INVAILED_CHARS = " /'\"*:\n";
}  // namespace

/* ------------------------------------------------------------
Function Name: FileExist
Description  : 判断指定路径文件是否存在

Others       :-------------------------------------------------------- */
mp_bool CMpFile::FileExist(const mp_string& pszFilePath)
{
#ifdef WIN32
    struct _stat fileStat;
    if (_stat(pszFilePath.c_str(), &fileStat) != 0) {
#else
    struct stat fileStat;
    if (stat(pszFilePath.c_str(), &fileStat) != 0) {
#endif
        return MP_FALSE;
    }

    // 目录返回false
    if (S_ISDIR(fileStat.st_mode)) {
        return MP_FALSE;
    }

    return MP_TRUE;
}

/* ------------------------------------------------------------
Function Name: DirExist
Description  : 判断指定路径文件夹是否存在

Others       :-------------------------------------------------------- */
mp_bool CMpFile::DirExist(const mp_char* pszDirPath)
{
#ifdef WIN32
    struct _stat fileStat;
    if (0 != _stat(pszDirPath, &fileStat)) {
#else
    struct stat fileStat;
    if (0 != stat(pszDirPath, &fileStat)) {
#endif
        return MP_FALSE;
    }

    // 目录返回false
    if (!S_ISDIR(fileStat.st_mode)) {
        return MP_FALSE;
    }

    return MP_TRUE;
}

/* ------------------------------------------------------------
Function Name: CreateDir
Description  : 创建文件夹

Others       :-------------------------------------------------------- */
mp_int32 CMpFile::CreateDir(const mp_char* pszDirPath)
{
    if (NULL == pszDirPath) {
        COMMLOG(OS_LOG_ERROR, "Parameter is null.");
        return MP_FAILED;
    }
    mp_char szErr[256] = { 0 };
#ifdef WIN32
    if (0 != _access(pszDirPath, 0)) {
        if (0 != _mkdir(pszDirPath)) {
            mp_int32 iErr = GetOSError();
            ERRLOG("Failed to create folder:%s, errno[%d]:%s.",
                pszDirPath, iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            return MP_FAILED;
        }
    }
#else
    if (0 != mkdir(pszDirPath, S_IRWXU)) {
        mp_int32 iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Failed to create folder:%s, errno[%d]:%s.",
            pszDirPath,
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
#endif
    return MP_SUCCESS;
}

void CMpFile::Getfilepath(const mp_char* path, const mp_char* filename, mp_char* filepath, mp_int32 strfileLen)
{
    if (strcpy_s(filepath, VM_UUDI_FILELEN, path) != 0) {
        ERRLOG("strcpy_s failed!");
    }
    if (filepath[strlen(path) - 1] != '/') {
        if (strcat_s(filepath, VM_UUDI_FILELEN, "/") != 0) {
            ERRLOG("strcat_s failed!");
        }
    }
    if (strcat_s(filepath, VM_UUDI_FILELEN, filename) != 0) {
        ERRLOG("strcat_s failed!");
    }
}

bool CMpFile::DeleteAllFileInPath(const mp_string& path)
{
    mp_int32 lenthOfPath = path.size();
    bool iRet;
    iRet = DeleteFile(path.c_str(), lenthOfPath);
    return iRet;
}

#ifdef WIN32
bool CMpFile::DeleteFile(const mp_char* path, mp_int32 strfileLen)
{
    _finddata_t findData;
    intptr_t handle = _findfirst(path, &findData);
    if (handle == -1) {
        return true;
    }
    if (!(findData.attrib & _A_SUBDIR)) {
        remove(path);
        _findclose(handle);
        return true;
    }
    intptr_t dirHandle = _findfirst(mp_string(path).append("\\").append("*").c_str(), &findData);
    if (dirHandle == -1) {
        _findclose(handle);
        return true;
    }
    do {
        mp_string strAbsolutePath = mp_string(path).append("\\").append(findData.name);
        if (findData.attrib & _A_SUBDIR) {
            if (!(strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)) {
                DeleteFile(strAbsolutePath.c_str(), strAbsolutePath.length());
            }
        } else {
            remove(strAbsolutePath.c_str());
        }
    } while (_findnext(dirHandle, &findData) == 0);
    _findclose(dirHandle);
    _rmdir(path);
    return true;
}
#else
bool CMpFile::DeleteFile(const mp_char* path, mp_int32 strfileLen)
{
    struct stat statbuf;
    lstat(path, &statbuf);

    if (S_ISREG(statbuf.st_mode)) {  // 判断是否是常规文件
        remove(path);
    } else if (S_ISDIR(statbuf.st_mode)) {  // 判断是否是目录
        DIR *dir = NULL;
        if ((dir = opendir(path)) == NULL) {
            return true;
        }
        struct dirent *dirinfo = NULL;
        char filepath[VM_UUDI_FILELEN] = { 0 };
        while ((dirinfo = readdir(dir)) != NULL) {
            Getfilepath(path, dirinfo->d_name, filepath, strlen(filepath));
            if (strcmp(dirinfo->d_name, ".") == 0 || strcmp(dirinfo->d_name, "..") == 0) {  // 判断是否是特殊目录
                continue;
            }
            DeleteFile(filepath, strlen(filepath));
            rmdir(filepath);
        }
        closedir(dir);
    }
    return true;
}
#endif

/* ------------------------------------------------------------
Function Name: DelDir
Description  : 刪除文件夾和其下的文件
Others       :-------------------------------------------------------- */

mp_int32 CMpFile::DelDirAndSubFile(const mp_char* pszDirPath)
{
    if (NULL == pszDirPath) {
        COMMLOG(OS_LOG_ERROR, "Parameter is null.");
        return MP_FAILED;
    }
    DeleteFile(pszDirPath, strlen(pszDirPath));
    remove(pszDirPath);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: DelDir
Description  : 删除文件夹
Others       :-------------------------------------------------------- */
mp_int32 CMpFile::DelDir(const mp_char* pszDirPath)
{
    if (NULL == pszDirPath) {
        COMMLOG(OS_LOG_ERROR, "Parameter is null.");
        return MP_FAILED;
    }
    if (!CMpFile::DirExist(pszDirPath)) {
        return MP_SUCCESS;
    }
#ifdef WIN32
    if (0 != _rmdir(pszDirPath)) {
#else
    if (0 != rmdir(pszDirPath)) {
#endif
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, "Failed to create folder:%s, errno[%d]:%s.",
            pszDirPath, iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: DeleteDir
Description  : 从文件全路径获取文件路径
Return       :
Call         :
Called by    :
Others       :--------------------------------------------------------*/
mp_int32 CMpFile::GetPathFromFilePath(const mp_string& strFileFullPath, mp_string& strFilePath)
{
    mp_size szInx = strFileFullPath.find_last_of(PATH_SEPARATOR);
    if (mp_string::npos == szInx) {
        COMMLOG(OS_LOG_ERROR, "Failed to find separator in string %s.", strFileFullPath.c_str());
        return MP_FAILED;
    }

    strFilePath = strFileFullPath.substr(0, szInx);
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: DeleteDir
Description  : 从文件全路径获取文件名称
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CMpFile::GetNameFromFilePath(const mp_string& strFileFullPath, mp_string& strFilePath)
{
    mp_size szInx = strFileFullPath.find_last_of(PATH_SEPARATOR);
    if (string::npos == szInx) {
        COMMLOG(OS_LOG_ERROR, "Failed to find separator in string %s.", strFileFullPath.c_str());
        return MP_FAILED;
    }

    strFilePath = strFileFullPath.substr(szInx + 1, szInx);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: FileSize
Description  : 获取文件大小，单位是字节

Others       :-------------------------------------------------------- */
mp_int32 CMpFile::FileSize(const mp_char* pszFilePath, mp_uint32& uiSize)
{
    if (NULL == pszFilePath) {
        return MP_FAILED;
    }

#ifdef WIN32
    struct _stat fileSizeStat;
    if (0 != _stat(pszFilePath, &fileSizeStat)) {
#else
    struct stat fileSizeStat;
    if (0 != stat(pszFilePath, &fileSizeStat)) {
#endif
        return MP_FAILED;
    }

    uiSize = fileSizeStat.st_size;

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetlLastModifyTime
Description  : 获取文件上一次更新时间

Others       :-------------------------------------------------------- */
mp_int32 CMpFile::GetlLastModifyTime(const mp_char* pszFilePath, mp_time& tLastModifyTime)
{
    if (NULL == pszFilePath) {
        return MP_FAILED;
    }

#ifdef WIN32
    struct _stat fileStat;
    if (0 != _stat(pszFilePath, &fileStat)) {
#else
    struct stat fileStat;
    if (0 != stat(pszFilePath, &fileStat)) {
#endif
        return MP_FAILED;
    }

    tLastModifyTime = fileStat.st_mtime;

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: ReadFile
Description  : 从文件中读出每行内容到string的vector中，读完后不删除该文件。
               该函数不做加解密操作
Others       :-------------------------------------------------------- */
mp_int32 CMpFile::ReadFile(const mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    // CodeDex误报，ZERO_LENGTH_ALLOCATIONS
    // CodeDex误报，FORTIFY.Path_Manipulation
    // codedex误报, CANONICAL_FILEPATH，FilePath路径不存在这个问题
    mp_int32 iTmp = strFilePath.find_last_of(PATH_SEPARATOR);
    mp_string strFileName = strFilePath.substr(iTmp + 1);

    if (!CMpFile::FileExist(strFilePath)) {
        COMMLOG(OS_LOG_WARN, "Can't find file %s.", strFileName.c_str());
        return MP_FAILED;
    }

    FILE* pFile = fopen(strFilePath.c_str(), "r");
    if (NULL == pFile) {
        COMMLOG(OS_LOG_ERROR, "Open file %s failed, errno[%d]:%s.", strFileName.c_str(), errno, strerror(errno));
        return MP_FAILED;
    }

    if (fseek(pFile, 0L, SEEK_END) != 0) {
        COMMLOG(OS_LOG_ERROR, "Call fseek failed, errno[%d]:%s.", errno, strerror(errno));
        GETOSERRORCODE(0 != fclose(pFile));
        return MP_FAILED;
    }

    mp_int32 iFileLen = (mp_int32)ftell(pFile);
    // 如果文件为空，直接返回
    if (iFileLen == 0) {
        COMMLOG(OS_LOG_INFO, "File %s to be read is empty.", strFileName.c_str());
        vecOutput.clear();
        GETOSERRORCODE(0 != fclose(pFile));
        return MP_SUCCESS;
    }

    if (fseek(pFile, 0L, SEEK_SET) != 0) {
        COMMLOG(OS_LOG_ERROR, "Call fseek failed, errno[%d]:%s.", errno, strerror(errno));
        GETOSERRORCODE(0 != fclose(pFile));
        return MP_FAILED;
    }

    return ReadFileAfter(pFile, iFileLen, vecOutput);
}

mp_int32 CMpFile::ReadFileAfter(FILE* pFile, mp_int32 iFileLen, vector<mp_string>& vecOutput)
{
    mp_char* pBuff = NULL;
    // CodeDex误报，Memory Leak
    NEW_ARRAY_CATCH(pBuff, mp_char, iFileLen + 1);
    if (!pBuff) {
        COMMLOG(OS_LOG_WARN, "New failed.");
        GETOSERRORCODE(0 != fclose(pFile));
        return MP_FAILED;
    }

    mp_int32 iRet = memset_s(pBuff, iFileLen + 1, 0x00, iFileLen + 1);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
        delete[] pBuff;
        GETOSERRORCODE(0 != fclose(pFile));
        return MP_FAILED;
    }
    while (NULL != fgets(pBuff, iFileLen + 1, pFile)) {
        // 写文件时在每行末尾加上了换行符，读取时去掉换行符'\n'
        // 最后一行可能没有换行符，需要特殊处理
        if (pBuff[strlen(pBuff) - 1] == '\n') {
            pBuff[strlen(pBuff) - 1] = 0;
        }
        vecOutput.push_back(pBuff);
        iRet = memset_s(pBuff, iFileLen + 1, 0x00, iFileLen + 1);
        if (MP_SUCCESS != iRet) {
            COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
            delete[] pBuff;
            fclose(pFile);
            return MP_FAILED;
        }
    }
    delete[] pBuff;
    GETOSERRORCODE(0 != fclose(pFile));
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetFolderFile
Description  : 读出某个文件夹中所有文件名称
Others       :-------------------------------------------------------- */
#ifdef WIN32
mp_int32 CMpFile::GetFolderFile(mp_string& strFolder, vector<mp_string>& vecFileList)
{
    WIN32_FIND_DATA fd = {0};
    HANDLE hSearch;

    mp_string strFilePathName = strFolder + "\\*";
    // CodeDex误报，Missing Check against Null
    hSearch = FindFirstFile(strFilePathName.c_str(), &fd);
    if (INVALID_HANDLE_VALUE == hSearch) {
        COMMLOG(OS_LOG_ERROR, "hSearch is INVALID_HANDLE_VALUE.");
        return MP_FAILED;
    }

    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        mp_bool bFlag = strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..");
        if (bFlag) {
            vecFileList.push_back(fd.cFileName);
        }
    }

    for (;;) {
        mp_int32 iRet = memset_s(&fd, sizeof(fd), 0, sizeof(fd));
        if (EOK != iRet) {
            COMMLOG(OS_LOG_ERROR, "memset_s failed, iRet = %d.", iRet);
            FindClose(hSearch);
            return MP_FAILED;
        }

        if (!FindNextFile(hSearch, &fd)) {
            if (ERROR_NO_MORE_FILES == GetLastError()) {
                break;
            }
            COMMLOG(OS_LOG_ERROR, "FindNextFile failed.");
            FindClose(hSearch);
            return MP_FAILED;
        }

        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            mp_bool bFlag = strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..");
            if (bFlag) {
                vecFileList.push_back(fd.cFileName);
            }
        }
    }

    FindClose(hSearch);
    return MP_SUCCESS;
}
#elif defined(SOLARIS)
mp_int32 CMpFile::GetFolderFile(mp_string& strFolder, vector<mp_string>& vecFileList)
{
    // 1. opens a directory stream
    DIR* dir = opendir(strFolder.c_str());
    if (nullptr == dir) {
        COMMLOG(OS_LOG_ERROR, "opendir failed, dir: %s", strFolder.c_str());
        return MP_FAILED;
    }
    Defer _(nullptr, [&](...) {
            if (dir != nullptr) {
                closedir(dir);
            }
    });

    // 2. malloc memory for traversalFile
    struct dirent* ptr = (struct dirent*)malloc(sizeof(struct dirent) + MAX_PATH_LEN);
    if (ptr == nullptr) {
        ERRLOG("Failed to malloc for memory.");
        return MP_FAILED;
    }
    MEMORY_GUARD(ptr);

    struct dirent* dp = (struct dirent*)malloc(sizeof(struct dirent) + MAX_PATH_LEN);
    if (ptr == nullptr) {
        ERRLOG("Failed to malloc for memory.");
        return MP_FAILED;
    }
    MEMORY_GUARD(dp);

    // 3. traverse all files in the directory
    struct stat strFileInfo = {0};
    while ((MP_SUCCESS == readdir_r(dir, dp, &ptr)) && (NULL != ptr)) {
        mp_string fileName = strFolder + PATH_SEPARATOR + ptr->d_name;
        if (lstat(fileName.c_str(), &strFileInfo) < 0) {
            COMMLOG(OS_LOG_ERROR, "lstat failed.");
            return MP_FAILED;
        }

        if (!S_ISDIR(strFileInfo.st_mode)) {
            if (strcmp(ptr->d_name, ".") && strcmp(ptr->d_name, "..")) {
                vecFileList.push_back(ptr->d_name);
            }
        }
    }
    return MP_SUCCESS;
}
#else
mp_int32 CMpFile::GetFolderFile(mp_string& strFolder, vector<mp_string>& vecFileList)
{
    struct dirent* ptr = NULL;
    struct stat strFileInfo = {0};
    DIR* dir = opendir(strFolder.c_str());
    if (NULL == dir) {
        COMMLOG(OS_LOG_ERROR, "opendir failed, dir: %s, errno[%d]:%s.", strFolder.c_str(), errno, strerror(errno));
        return MP_FAILED;
    }

    struct dirent dp;
    while ((MP_SUCCESS == readdir_r(dir, &dp, &ptr)) && (NULL != ptr)) {
        mp_string fileName = strFolder + PATH_SEPARATOR + ptr->d_name;
        if (lstat(fileName.c_str(), &strFileInfo) < 0) {
            closedir(dir);
            COMMLOG(OS_LOG_ERROR, "lstat failed.");
            return MP_FAILED;
        }

        if (!S_ISDIR(strFileInfo.st_mode)) {
            if (strcmp(ptr->d_name, ".") && strcmp(ptr->d_name, "..")) {
                vecFileList.push_back(ptr->d_name);
            }
        }
    }

    closedir(dir);
    return MP_SUCCESS;
}
#endif

/* ------------------------------------------------------------
Function Name: GetFolderDir
Description  : 读出某个文件夹中所有目录名称
Others       :-------------------------------------------------------- */
#ifdef WIN32
mp_int32 CMpFile::GetFolderDir(const mp_string& strFolder, std::vector<mp_string>& vecNameList)
{
    mp_string strFilePathName = strFolder + "\\*";
    mp_char szErr[MAX_ERROR_MSG_LEN] = { 0 };

    WIN32_FIND_DATAA fd = { 0 };
    HANDLE hSearch = FindFirstFile(strFilePathName.c_str(), &fd);
    if (INVALID_HANDLE_VALUE == hSearch) {
        mp_int32 err = GetOSError();
        ERRLOG("hSearch is INVALID_HANDLE_VALUE, strFolder=%s, errno[%d]:%s.",
            strFolder.c_str(), err, GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!(strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)) {
                vecNameList.push_back(fd.cFileName);
            }
        }
        if (!FindNextFile(hSearch, &fd)) {
            if (ERROR_NO_MORE_FILES == GetLastError()) {
                break;
            } else {
                mp_int32 err = GetOSError();
                ERRLOG("FindNextFile failed, errno[%d]:%s.", err, GetOSStrErr(err, szErr, sizeof(szErr)));
                FindClose(hSearch);
                return MP_FAILED;
            }
        }
    } while (true);
    FindClose(hSearch);
    return MP_SUCCESS;
}
#elif defined(SOLARIS)
mp_int32 CMpFile::GetFolderDir(const mp_string& strFolder, std::vector<mp_string>& vecNameList)
{
    // 1. opens a directory stream
    DIR* dir = opendir(strFolder.c_str());
    if (dir == nullptr) {
        mp_int32 err = GetOSError();
        mp_char szErr[MAX_ERROR_MSG_LEN] = { 0 };
        ERRLOG("Open %s failed, errno[%d]:%s..", strFolder.c_str(), err, GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    Defer _(nullptr, [&](...) {
            if (dir != nullptr) {
                closedir(dir);
            }
    });

    // 2. malloc memory for traversal
    struct dirent* ptr = (struct dirent*)malloc(sizeof(struct dirent) + MAX_PATH_LEN);
    if (ptr == nullptr) {
        ERRLOG("Failed to malloc for memory.");
        return MP_FAILED;
    }
    MEMORY_GUARD(ptr);

    struct dirent* dp = (struct dirent*)malloc(sizeof(struct dirent) + MAX_PATH_LEN);
    if (ptr == nullptr) {
        ERRLOG("Failed to malloc for memory.");
        return MP_FAILED;
    }
    MEMORY_GUARD(dp);

    // 3. traverse all directory in the directory
    struct stat stFileInfo = {0};
    while ((readdir_r(dir, dp, &ptr) == MP_SUCCESS) && (ptr != nullptr)) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }

        mp_string fileName = strFolder + PATH_SEPARATOR + ptr->d_name;
        if (lstat(fileName.c_str(), &stFileInfo) < 0) {
            ERRLOG("Exec lstat failed, fileName: %s.", fileName.c_str());
            return MP_FAILED;
        }

        if (S_ISDIR(stFileInfo.st_mode)) {
            vecNameList.push_back(ptr->d_name);
        }
    }
    return MP_SUCCESS;
}
#else
mp_int32 CMpFile::GetFolderDir(const mp_string& strFolder, std::vector<mp_string>& vecNameList)
{
    struct dirent* ptr = nullptr;
    DIR* dir = opendir(strFolder.c_str());
    if (dir == nullptr) {
        mp_int32 err = GetOSError();
        mp_char szErr[MAX_ERROR_MSG_LEN] = { 0 };
        ERRLOG("Open %s failed, errno[%d]:%s..", strFolder.c_str(), err, GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    struct stat stFileInfo = {0};
#ifdef LINUX
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        } else if (ptr->d_type == DT_DIR) {
            vecNameList.push_back(ptr->d_name);
        } else if (ptr->d_type == DT_UNKNOWN) {
            std::string fileName = strFolder + PATH_SEPARATOR + ptr->d_name;
            if (stat(fileName.c_str(), &stFileInfo) != 0) {
                WARNLOG("Get file [%s] access failed.", fileName.c_str());
                continue;
            }
            if (S_ISDIR(stFileInfo.st_mode)) {
                vecNameList.push_back(ptr->d_name);
            }
        }
    }
#else
    struct dirent dp;
    std::string fileName;
    while ((readdir_r(dir, &dp, &ptr) == MP_SUCCESS) && (ptr != NULL)) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        fileName = strFolder + PATH_SEPARATOR + ptr->d_name;
        if (lstat(fileName.c_str(), &stFileInfo) < 0) {
            closedir(dir);
            ERRLOG("Exec lstat failed.");
            return MP_FAILED;
        }
        if (S_ISDIR(stFileInfo.st_mode)) {
            vecNameList.push_back(ptr->d_name);
        }
    }
#endif
    closedir(dir);
    return MP_SUCCESS;
}

mp_int32 CMpFile::GetFolderDirUmountPoints(const mp_string& strFolder, std::vector<mp_string>& vecNameList)
{
    struct dirent* ptr = nullptr;
    DIR* dir = opendir(strFolder.c_str());
    if (dir == nullptr) {
        mp_int32 err = GetOSError();
        mp_char szErr[MAX_ERROR_MSG_LEN] = { 0 };
        ERRLOG("Open %s failed, errno[%d]:%s..", strFolder.c_str(), err, GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    struct stat stFileInfo = {0};
#ifdef LINUX
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        } else if (ptr->d_type == DT_DIR) {
            vecNameList.push_back(ptr->d_name);
        } else if (ptr->d_type == DT_UNKNOWN) {
            std::string fileName = strFolder + PATH_SEPARATOR + ptr->d_name;
            if (stat(fileName.c_str(), &stFileInfo) != 0) {
                WARNLOG("Get file [%s] access failed.", fileName.c_str());
                vecNameList.push_back(ptr->d_name);
                continue;
            }
            if (S_ISDIR(stFileInfo.st_mode)) {
                vecNameList.push_back(ptr->d_name);
            }
        }
    }
#else
    struct dirent dp;
    std::string fileName;
    while ((readdir_r(dir, &dp, &ptr) == MP_SUCCESS) && (ptr != NULL)) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        fileName = strFolder + PATH_SEPARATOR + ptr->d_name;
        if (lstat(fileName.c_str(), &stFileInfo) < 0) {
            closedir(dir);
            ERRLOG("Exec lstat failed.");
            return MP_FAILED;
        }
        if (S_ISDIR(stFileInfo.st_mode)) {
            vecNameList.push_back(ptr->d_name);
        }
    }
#endif
    closedir(dir);
    return MP_SUCCESS;
}
#endif

#ifdef WIN32
mp_bool CMpFile::DirEmpty(const mp_string& dirPath)
{
    _finddata_t findData;
    string strPath = dirPath + "\\*";
    intptr_t handle = _findfirst(strPath.c_str(), &findData);
    if (handle == -1) {
        return MP_FALSE;
    }
    do {
        if (!(strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)) {
            _findclose(handle);
            return MP_FALSE;
        }
    } while (_findnext(handle, &findData) == 0);
    _findclose(handle);
    return MP_TRUE;
}
#else
mp_bool CMpFile::DirEmpty(const mp_string& dirPath)
{
    struct dirent* ptr = NULL;
    struct stat strFileInfo = {0};
    const int maxPathLen = 2048;
    mp_char acFullName[maxPathLen] = {0};

    DIR* dir = opendir(dirPath.c_str());
    if (NULL == dir) {
        COMMLOG(OS_LOG_ERROR, "opendir failed,dirPath(%s).", dirPath.c_str());
        return MP_FALSE;
    }

    struct dirent dp;
    while ((MP_SUCCESS == readdir_r(dir, &dp, &ptr)) && (NULL != ptr)) {
        if (MP_FAILED == snprintf_s(acFullName, maxPathLen, maxPathLen - 1, "%s/%s", dirPath.c_str(), ptr->d_name)) {
            closedir(dir);
            COMMLOG(OS_LOG_ERROR, "SNPRINTF_S failed,dirPath(%s),d_name(%s).", dirPath.c_str(), ptr->d_name);
            return MP_FALSE;
        }

        if (lstat(acFullName, &strFileInfo) < 0) {
            closedir(dir);
            COMMLOG(OS_LOG_ERROR, "lstat failed,fileName(%s).", acFullName);
            return MP_FALSE;
        }

        // 只有'.'和'..'两个子目录的是空目录
        if (!S_ISDIR(strFileInfo.st_mode)) {
            closedir(dir);
            return MP_FALSE;
        } else {
            if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
                closedir(dir);
                return MP_FALSE;
            }
        }
    }
    closedir(dir);
    return MP_TRUE;
}
#endif

mp_bool CMpFile::GetProfileSection(const mp_string& file, const mp_string& key, mp_string& value)
{
    std::string findKey = key + "=";
    std::ifstream stream;
    stream.open(file.c_str(), std::ifstream::in);
    mp_string line;

    bool bRet = MP_FALSE;
    if (!stream.is_open()) {
        return bRet;
    }

    while (getline(stream, line)) {
        if (line.find(findKey) != std::string::npos) {
            value = line.substr(findKey.length(), line.length() - findKey.length());
            bRet = MP_TRUE;
            break;
        }
    }
    stream.close();
    return bRet;
}

mp_bool CMpFile::WaitForFile(const mp_string& pszFilePath, mp_int32 iSleepInterval, mp_int32 iSleepCount)
{
    mp_bool bIsExist = MP_FALSE;
    mp_int32 iCount = iSleepCount;

    while (iCount > 0) {
        if (FileExist(pszFilePath)) {
            bIsExist = MP_TRUE;
            break;
        }

        COMMLOG(OS_LOG_DEBUG, "File \"%s\" dose not exist, wait for it.", pszFilePath.c_str());

        DoSleep((mp_uint32)iSleepInterval);
        iCount--;
    }

    return bIsExist;
}

// 注意：此接口会被日志接口调用，这里面不能再调用LOG接口写日志，否则可能导致死锁
mp_int32 CMpFile::DelFile(const mp_string& pszFilePath)
{
    if ("" == pszFilePath) {
        return MP_SUCCESS;
    }
    if (!FileExist(pszFilePath)) {
        return MP_SUCCESS;
    }
#ifdef WIN32
    DeleteFile(pszFilePath.c_str(), pszFilePath.length());
    return MP_SUCCESS;
#else
    // Coverity&Fortify误报:FORTIFY.Race_Condition--File_System_Access
    // DelFile方法都没有在setuid提升到root时调用
    mp_int32 ret = remove(pszFilePath.c_str());
    if (ret != EOK) {
        return MP_FAILED;
    }
#endif

    return MP_SUCCESS;
}

mp_int32 CMpFile::CreateFile(const mp_string& pszFilePath)
{
    if ("" == pszFilePath || FileExist(pszFilePath)) {
        return MP_SUCCESS;
    }
    mp_char szErr[MAX_ERROR_MSG_LEN] = { 0 };
#ifdef WIN32
    FILE* pFile = nullptr;
    if (fopen_s(&pFile, pszFilePath.c_str(), "a") != 0 || pFile == nullptr) {
        mp_int32 err = GetOSError();
        ERRLOG("Open file %s failed, errno[%d]:%s.", pszFilePath.c_str(), err, GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
#else
    FILE* pFile = fopen(pszFilePath.c_str(), "a");
    if (NULL == pFile) {
        mp_int32 err = GetOSError();
        ERRLOG("Open file %s failed, errno[%d]:%s.", pszFilePath.c_str(), err, GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
#endif
    fclose(pFile);
    return MP_SUCCESS;
}

mp_int32 CMpFile::CheckFileAccess(const mp_string& pszFilePath)
{
    mp_int32 iRet;
    if ("" == pszFilePath) {
        COMMLOG(OS_LOG_ERROR, "File name is empry.");
        return MP_FAILED;
    }
    if (!FileExist(pszFilePath)) {
        COMMLOG(OS_LOG_ERROR, "File [%s] does not exist.", pszFilePath.c_str());
        return MP_FAILED;
    }
#ifdef WIN32

#else
    struct stat buf;
    iRet = stat(pszFilePath.c_str(), &buf);
    if (0 != iRet) {
        COMMLOG(OS_LOG_ERROR, "Get file [%s] access failed.", pszFilePath.c_str());
        return MP_FAILED;
    }

    if (0 != buf.st_uid) {  // 文件拥有者只能是root, root的uid=0
        COMMLOG(OS_LOG_ERROR, "File [%s] uid is wrong.", pszFilePath.c_str());
        return MP_FAILED;
    }

    if ((0 != (buf.st_mode & S_IWGRP)) || (0 != (buf.st_mode & S_IWOTH))) {  // 文件属组和其他用户不能写
        COMMLOG(OS_LOG_ERROR, "File [%s] access is wrong, group and other cannot write.", pszFilePath.c_str());
        return MP_FAILED;
    }
#endif
    COMMLOG(OS_LOG_DEBUG, "File [%s] access.", pszFilePath.c_str());
    return MP_SUCCESS;
}

mp_int32 CMpFile::CopyFileCoverDest(const mp_string& pszSourceFilePath, const mp_string& pszDestFilePath)
{
    char buff[TASK_STEP_INTERVAL_HUNDERED] = {0};
    if (pszSourceFilePath == "" || pszDestFilePath == "") {
        return MP_SUCCESS;
    }

    std::ifstream streamin;
    streamin.open(pszSourceFilePath.c_str(), std::ifstream::in);
    if (!streamin.is_open()) {
        COMMLOG(OS_LOG_ERROR, "Open file %s failed, errno[%d]:%s.", pszSourceFilePath.c_str(), errno, strerror(errno));
        return MP_FAILED;
    }

    std::ofstream streamout;
    streamout.open(pszDestFilePath.c_str(), std::ifstream::out);
    if (!streamout.is_open()) {
        COMMLOG(OS_LOG_ERROR, "Open file %s failed, errno[%d]:%s.", pszDestFilePath.c_str(), errno, strerror(errno));
        streamin.close();
        return MP_FAILED;
    }

    while (!streamin.eof()) {
        streamin.read(buff, sizeof(buff));
        streamout.write(buff, streamin.gcount());
    }

    streamin.close();
    streamout.flush();
    streamout.close();

    return MP_SUCCESS;
}

int CMpFile::CheckFileName(const std::string& filename)
{
    std::size_t idx;
    for (auto pchar : FILENAME_INVAILED_CHARS) {
        idx = filename.find(pchar, 0);
        if (mp_string::npos != idx) {
            return ERROR_COMMON_INVALID_PARAM;
        }
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: ReadFile
Description  : 从文件中明文读出每行内容到string的vector中，读完后删除该文件。
               该函数不做加解密操作

Others       :-------------------------------------------------------- */
mp_int32 CIPCFile::ReadFile(mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    // CodeDex误报，ZERO_LENGTH_ALLOCATIONS
    // codedex误报CANONICAL_FILEPATH，FilePath路径不存在这个问题

    mp_int32 iRet = CMpFile::ReadFile(strFilePath, vecOutput);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Read file failed.");
        return iRet;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: AppendFile
Description  : 将string的vector中的元素逐条取出，明文添加到目标文件末尾
               该函数不做加密处理

Others       :
-------------------------------------------------------- */
mp_int32 CIPCFile::AppendFile(const mp_string& strFilePath, const vector<mp_string>& vecInput)
{
    if (!CMpFile::FileExist(strFilePath)) {
        COMMLOG(OS_LOG_WARN, "File path: '%s' not exists, need to write file.", strFilePath.c_str());
        // linux系统默认全选为640
        if (WriteFile(strFilePath, vecInput) != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Write file failed!");
            return MP_FAILED;
        } else {
            return MP_SUCCESS;
        }
    }
    ofstream file(strFilePath, ios::app);
    if (file.is_open()) {
        for (auto i : vecInput) {
            file << i << '\n';
        }
        file.close();
    } else {
        file.close();
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_INFO, "WriteFile: file path: '%s' sucessfully.", strFilePath.c_str());
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: WriteFile
Description  : 将string的vector中的元素逐条取出，明文写到文件中
               该函数不做加解密操作

Others       :-------------------------------------------------------- */
mp_int32 CIPCFile::WriteFile(const mp_string& strFilePath, const vector<mp_string>& vecInput)
{
    // CodeDex误报，TOCTOU
#ifndef WIN32
    struct stat fileStat;
    mp_int32 iRet1;
    iRet1 = memset_s(&fileStat, sizeof(fileStat), 0, sizeof(fileStat));
    if (iRet1 != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret = %d.", iRet1);
        return MP_FAILED;
    }
    mp_int32 fileExist = 0;
#endif
    const mp_char* strBaseFileName;
    // CodeDex误报，Unchecked Return Value函数BaseFileName的返回值不会为空
    strBaseFileName = BaseFileName(strFilePath).c_str();

    if (CMpFile::FileExist(strFilePath)) {
        COMMLOG(OS_LOG_INFO, "WriteFile: file path: '%s'.", strFilePath.c_str());
        // 读取文件权限
#ifndef WIN32
        if (0 != stat(strFilePath.c_str(), &fileStat)) {
            COMMLOG(OS_LOG_ERROR, "get file stat of %s failed, errno[%d]:%s.", strBaseFileName, errno, strerror(errno));
            return MP_FAILED;
        }
        fileExist = 1;
#endif
        // 删除重名的文件
        COMMLOG(OS_LOG_INFO, "WriteFile: remove file has the same name.");
        mp_int32 iRet = remove(strFilePath.c_str());
        if (0 != iRet) {
            COMMLOG(OS_LOG_ERROR, "Remove file %s failed, errno[%d]:%s.", strBaseFileName, errno, strerror(errno));
            return MP_FAILED;
        }
    }

#ifndef WIN32
    return WriteFileInner(strFilePath, vecInput, fileExist, fileStat);
#else
    return WriteFileInner(strFilePath, vecInput);
#endif
}

/* ------------------------------------------------------------
Function Name: WriteFileInner
Description  : 将string的vector中的元素逐条取出，明文写到文件中
               该函数不做加解密操作

Others       :
-------------------------------------------------------- */
#ifndef WIN32
mp_int32 CIPCFile::WriteFileInner(
    const mp_string& strFilePath, const vector<mp_string>& vecInput, mp_int32 fileExist, struct stat& fileStat)
#else
mp_int32 CIPCFile::WriteFileInner(const mp_string& strFilePath, const vector<mp_string>& vecInput)
#endif
{
    // codedex误报， Race Condition:File System Access
    FILE* pFile = fopen(strFilePath.c_str(), "a+");
    if (NULL == pFile) {
        COMMLOG(OS_LOG_ERROR, "Open file %s failed, errno[%d]:%s.", strFilePath.c_str(), errno, strerror(errno));
        return MP_FAILED;
    }

    for (auto iter = vecInput.begin(); iter != vecInput.end(); ++iter) {
        fprintf(pFile, "%s\n", iter->c_str());
    }

    mp_int32 iRet = fflush(pFile);
    if (0 != iRet) {
        COMMLOG(OS_LOG_ERROR, "Call fflush failed, errno[%d]:%s.", errno, strerror(errno));
        iRet = fclose(pFile);
        GETOSERRORCODE(0 != iRet);
        return MP_FAILED;
    }
    iRet = fclose(pFile);
    GETOSERRORCODE(0 != iRet);
#ifndef WIN32
    // 将文件修改为删除之前权限
    if (1 == fileExist) {
        if (ChmodFile(strFilePath, fileStat.st_mode) != MP_SUCCESS) {
            CMpFile::DelFile(strFilePath);
            return MP_FAILED;
        }
    }
#endif
    COMMLOG(OS_LOG_DEBUG, "Write file succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: WriteInput
Description  : 将strInput写入文件

Others       :-------------------------------------------------------- */
mp_int32 CIPCFile::WriteInput(const mp_string& strUniqueID, const mp_string& strInput)
{
    if (strInput.empty()) {
        return MP_SUCCESS;
    }

    mp_string strFileName = INPUT_TMP_FILE + strUniqueID;
    // 增加临时文件夹路径
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);

    const mp_char* strBaseFileName;
    strBaseFileName = BaseFileName(strFileName).c_str();

    vector<mp_string> vecInput;
    vecInput.push_back(strInput);

    mp_int32 iRet = WriteFile(strFilePath, vecInput);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Write input info file failed, file %s.", strBaseFileName);
    }

    return iRet;
}

/* ------------------------------------------------------------
Function Name: ReadInput
Description  : 从文件中读取数据内容

Others       :-------------------------------------------------------- */
mp_int32 CIPCFile::ReadInput(const mp_string& strUniqueID, mp_string& strInput)
{
    mp_string strFileName = INPUT_TMP_FILE + strUniqueID;
    // 增加临时文件夹路径
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);
    vector<mp_string> vecOutput;
    const mp_string strBaseFileName = BaseFileName(strFileName);
    mp_int32 iRet = ReadFile(strFilePath, vecOutput);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Write input info file failed, file %s.", strBaseFileName.c_str());
        return iRet;
    }

    if (vecOutput.size() != 1) {
        COMMLOG(
            OS_LOG_ERROR, "Invalid file content, file %s, line count %d.", strBaseFileName.c_str(), vecOutput.size());
        return MP_FAILED;
    }

    strInput = vecOutput[0];
    COMMLOG(OS_LOG_DEBUG, "Read input info file succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: WriteResult
Description  : 从执行结果从vecRlt中逐条读取出来，写入到临时结果文件中

Others       :-------------------------------------------------------- */
mp_int32 CIPCFile::WriteResult(const mp_string& strUniqueID, vector<mp_string>& vecRlt)
{
    if (vecRlt.empty()) {
        COMMLOG(OS_LOG_INFO, "vecRlt is empty, no need to write result file.");
        return MP_SUCCESS;
    }
    mp_string strFileName = RESULT_TMP_FILE + strUniqueID;
    mp_string strFilePath = CPath::GetInstance().GetStmpFilePath(strFileName);

    const mp_char* strBaseFileName;
    strBaseFileName = BaseFileName(strFileName).c_str();

    mp_int32 iRet = WriteFile(strFilePath, vecRlt);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Write result file failed, file %s.", strBaseFileName);
    } else {
        mp_int32 iChownRet = ChownResult(strUniqueID);
        if (MP_SUCCESS != iChownRet) {
            COMMLOG(OS_LOG_ERROR, "Chown result file failed, file %s.", strBaseFileName);
        }
    }

    return iRet;
}

/* ------------------------------------------------------------
Function Name: ReadResult
Description  : 从临时结果文件中读取内容，写入到vecRlt中

-------------------------------------------------------- */
mp_int32 CIPCFile::ReadResult(const mp_string& strFileName, vector<mp_string>& vecRlt)
{
    mp_string strFilePathFile = CPath::GetInstance().GetStmpFilePath(strFileName);

    // 如果结果文件不存在，有可能是没有满足条件的数据，这种情况返回成功
    if (!CMpFile::FileExist(strFilePathFile)) {
        COMMLOG(OS_LOG_WARN, "ReadResult: Can't find file %s.", strFileName.c_str());
        return MP_SUCCESS;
    }

    vecRlt.clear();
    mp_int32 iRet = ReadFile(strFilePathFile, vecRlt);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Read result file failed, file %s.", strFileName.c_str());
    }

#ifdef WIN32
    CMpFile::DelFile(strFilePathFile);
#endif
    return iRet;
}

/* ------------------------------------------------------------
Function Name: ReadOldResult
Description  : 从临时结果文件中读取内容，写入到vecRlt中，临时文件格式采用V1R3风格

-------------------------------------------------------- */
mp_int32 CIPCFile::ReadOldResult(const mp_string& strUniqueID, vector<mp_string>& vecRlt)
{
    mp_string strFileName = "RST" + strUniqueID + ".txt";
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);

    const mp_char* strBaseFileName;
    strBaseFileName = BaseFileName(strFileName).c_str();

    // 如果结果结果文件不存在，有可能是没有满足条件的数据，这种情况返回成功
    if (!CMpFile::FileExist(strFilePath)) {
        COMMLOG(OS_LOG_WARN, "Can't find file %s.", strBaseFileName);
        return MP_SUCCESS;
    }

    mp_int32 iRet = ReadFile(strFilePath, vecRlt);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Read result file failed, file %s.", strBaseFileName);
    }

    return iRet;
}

/* ------------------------------------------------------------
Function Name: ChownResult
Description  : 修改临时结果文件的权限，改成rdadmin用户
Others       :-------------------------------------------------------- */
mp_int32 CIPCFile::ChownResult(const mp_string& strUniqueID)
{
#ifndef WIN32
    mp_int32 iRet;
    mp_string strFileName = RESULT_TMP_FILE + strUniqueID;
    mp_string strFilePath = CPath::GetInstance().GetStmpFilePath(strFileName);

    // 如果结果结果文件不存在，有可能是没有满足条件的数据，这种情况返回成功
    // rdagent在读取结果文件时会判断操作的结果
    if (!CMpFile::FileExist(strFilePath)) {
        COMMLOG(OS_LOG_WARN, "Can't find file %s.", strFileName.c_str());
        return MP_SUCCESS;
    }

    mp_string strDir;
    CMpFile::GetPathFromFilePath(strFilePath, strDir);
    if (strDir != CPath::GetInstance().GetStmpPath()) {
        ERRLOG("Invalid path %s.", strFilePath.c_str());
        return MP_FAILED;
    }

    // 获取rdadmin用户的uid
    mp_int32 uid(-1), gid(-1);
    iRet = GetUidByUserName(AGENT_RUNNING_USER, uid, gid);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Get user(%s) uid and gid failed.", AGENT_RUNNING_USER.c_str());
        return MP_FAILED;
    }

    // 设置rdadmin的uid和gid
    iRet = ChownFile(strFilePath, uid, gid);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Chown file failed, file %s.", strFileName.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
#else
    return MP_SUCCESS;
#endif
}

mp_int32 CIPCFile::RemoveFile(const mp_string& strUniqueID)
{
    mp_string strFileName = INPUT_TMP_FILE + strUniqueID;
    // 增加临时文件夹路径
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);
    return CMpFile::DelFile(strFilePath);
}

/* ------------------------------------------------------------
Function Name: IsValidFilename
Description  : 检查文件名称是否合法，不允许出现特殊字符
               只允许出现：大/小写字母、数字、下划线、英文句号

Others       :-------------------------------------------------------- */
mp_int32 CIPCFile::IsValidFilename(const mp_string& strFilename)
{
    mp_int32 iLength = 0, iIndex = 0;
    mp_char cSpecialChar[] = {
        '/', ':', ';', '<', '=', '>', '?', '@', '[', '\\', ']', '^', '`', '{', '|', '}', '~', ' ', '\0'};
    mp_char cCtrlCharBegin = 0x00, cCtrlCharEnd = 0x2D;
    const mp_char* filename = strFilename.c_str();

    if (filename == NULL) {
        return MP_FAILED;
    } else {
        iLength = strlen(filename);
        if (iLength >= MAX_FILE_NAME_LEN)
            return MP_FAILED;
    }

    for (iIndex = 0; iIndex < iLength; iIndex++) {
        if ((filename[iIndex] >= cCtrlCharBegin) && (filename[iIndex] <= cCtrlCharEnd)) {
            return MP_FAILED;
        } else if (strchr(cSpecialChar, filename[iIndex]) != NULL) {
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 CIPCFile::WriteJsonStrToKVFile(
    const mp_string& strFileName, const Json::Value& jsonValue, const mp_string& KVSplitStr)
{
#ifndef WIN32
    static const mp_int32 MAX_ITEME_SIZE = 512;

    std::ostringstream oss;
    Json::Value::Members members;
    members = jsonValue.getMemberNames();  // 获取所有key的值
    // 遍历每个key
    for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); ++iterMember) {
        mp_string strKey = *iterMember;
        if (jsonValue[strKey.c_str()].isString()) {
            mp_string tempValue = jsonValue[strKey.c_str()].asString();
            if (tempValue.size() > MAX_ITEME_SIZE) {
                COMMLOG(OS_LOG_ERROR, "The value of Json key \"%s\" over max size", strKey.c_str());
                return MP_FAILED;
            }
            oss << strKey << STR_EQUAL << tempValue << std::endl;
        } else {
            COMMLOG(OS_LOG_ERROR, "The value of Json key \"%s\" is not string.", strKey.c_str());
            return MP_FAILED;
        }
    }

    mp_string strFilepath = CPath::GetInstance().GetTmpFilePath(strFileName);
    mp_string strJsonparm = oss.str();
    std::vector<mp_string> strvec;
    strvec.emplace_back(strJsonparm);

    mp_int32 iRet = CIPCFile::WriteFile(strFilepath, strvec);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Write result file failed, file %s.", strFilepath);
        return MP_FAILED;
    } else {
        // 获取rdadmin用户的uid
        mp_int32 uid(-1), gid(-1);
        iRet = GetUidByUserName(AGENT_RUNNING_USER, uid, gid);
        if (MP_SUCCESS != iRet) {
            COMMLOG(OS_LOG_ERROR, "Get user(%s) uid and gid failed.", AGENT_RUNNING_USER.c_str());
            return MP_FAILED;
        }

        // 设置rdadmin的uid和gid
        iRet = ChownFile(strFilepath, uid, gid);
        if (MP_SUCCESS != iRet) {
            COMMLOG(OS_LOG_ERROR, "Chown file failed, file %s.", strFileName.c_str());
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
#else
    return MP_FAILED;
#endif
}
