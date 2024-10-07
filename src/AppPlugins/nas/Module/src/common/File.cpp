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
#include "File.h"

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

#include <fstream>
#if defined(LINUX)
#include "common/File.h"
#include<dirent.h>
#include <cstring>
#include <fstream>
#elif defined(AIX)
#include <cstring>
#include <fstream>
#include <dirent.h>
#include "common/File.h"
#endif
#include "securec.h"

#include "define/Defines.h"
#include "log/Log.h"
#include "common/Path.h"
#include "common/Utils.h"
#include "common/CTime.h"
#include "common/JsonUtils.h"

using namespace std;

namespace {
const int VM_UUDI_FILELEN = 500;
const string FILENAME_INVAILED_CHARS = " /'\"*:\n";
const int TASK_STEP_INTERVAL_HUNDERED = 100;
const string AGENT_RUNNING_USER = "rdadmin";
}

namespace Module {

int CFile::OpenFile()
{
    // to do
    return SUCCESS;
}

bool CFile::FileExist(const string& pszFilePath)
{
#ifdef WIN32
    struct _stat fileStat;
    if (_stat(pszFilePath.c_str(), &fileStat) != 0) {
#else
    struct stat fileStat;
    if (stat(pszFilePath.c_str(), &fileStat) != 0) {
#endif
        return false;
    }

    // 目录返回false
    if (S_ISDIR(fileStat.st_mode)) {
        return false;
    }

    return true;
}

bool CFile::DirExist(const char* pszDirPath)
{
#ifdef WIN32
    struct _stat fileStat;
    if (0 != _stat(pszDirPath, &fileStat)) {
#else
    struct stat fileStat;
    if (0 != stat(pszDirPath, &fileStat)) {
#endif
        return false;
    }

    // 目录返回false
    if (!S_ISDIR(fileStat.st_mode)) {
        return false;
    }

    return true;
}

int CFile::CreateDir(const char* pszDirPath)
{
    if (NULL == pszDirPath) {
        COMMLOG(OS_LOG_ERROR, "Parameter is null.");
        return FAILED;
    }

#ifdef WIN32
    COMMLOG(OS_LOG_ERROR, "Windows does not support create folder.");
    return FAILED;
#else
    char szErr[256] = {0};
    if (0 != mkdir(pszDirPath, S_IRWXU)) {
        int iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Failed to create folder:%s, errno[%d]:%s.",
            pszDirPath,
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return FAILED;
    }
#endif
    return SUCCESS;
}

void CFile::Getfilepath(const char* path, const char* filename, char* filepath, int strfileLen)
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

#ifdef WIN32
bool CFile::DeleteFile(const char* path, int strfileLen)
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
    intptr_t dirHandle = _findfirst(string(path).append("\\").append("*").c_str(), &findData);
    if (dirHandle == -1) {
        _findclose(handle);
        return true;
    }
    do {
        string strAbsolutePath = string(path).append("\\").append(findData.name);
        if (findData.attrib & _A_SUBDIR) {
            if (!(strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)) {
                DeleteFile(strAbsolutePath.c_str(), strAbsolutePath.length());
            }
        }
        else {
            remove(strAbsolutePath.c_str());
        }
    } while (_findnext(dirHandle, &findData) == 0);
    _findclose(dirHandle);
    _rmdir(path);
    return true;
}
#else
bool CFile::DeleteFile(const char* path, int strfileLen)
{
    DIR *dir = NULL;
    struct dirent *dirinfo = NULL;
    struct stat statbuf;
    char filepath[VM_UUDI_FILELEN] = {0};
    lstat(path, &statbuf);

    if (S_ISREG(statbuf.st_mode)) {  // 判断是否是常规文件
        remove(path);
    } else if (S_ISDIR(statbuf.st_mode)) {  // 判断是否是目录
        if ((dir = opendir(path)) == NULL) {
            return true;
        }
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

int CFile::DelDirAndSubFile(const char* pszDirPath)
{
    if (NULL == pszDirPath) {
        COMMLOG(OS_LOG_ERROR, "Parameter is null.");
        return FAILED;
    }
    DeleteFile(pszDirPath, strlen(pszDirPath));
    remove(pszDirPath);
    return SUCCESS;
}

int CFile::DelDir(const char* pszDirPath)
{
    if (NULL == pszDirPath) {
        COMMLOG(OS_LOG_ERROR, "Parameter is null.");
        return FAILED;
    }

#ifdef WIN32
    COMMLOG(OS_LOG_ERROR, "Windows does not support delete folder.");
    return FAILED;
#else
    char szErr[256] = {0};

    if (!CFile::DirExist(pszDirPath)) {
        return SUCCESS;
    }

    if (0 != rmdir(pszDirPath)) {
        int iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Failed to create folder:%s, errno[%d]:%s.",
            pszDirPath,
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return FAILED;
    }
#endif

    return SUCCESS;
}

int CFile::GetPathFromFilePath(const string& strFileFullPath, string& strFilePath)
{
    size_t szInx = strFileFullPath.find_last_of(PATH_SEPARATOR);
    if (string::npos == szInx) {
        COMMLOG(OS_LOG_ERROR, "Failed to find separator in string %s.", strFileFullPath.c_str());
        return FAILED;
    }

    strFilePath = strFileFullPath.substr(0, szInx);
    return SUCCESS;
}

int CFile::GetNameFromFilePath(const string& strFileFullPath, string& strFilePath)
{
    size_t szInx = strFileFullPath.find_last_of(PATH_SEPARATOR);
    if (string::npos == szInx) {
        COMMLOG(OS_LOG_ERROR, "Failed to find separator in string %s.", strFileFullPath.c_str());
        return FAILED;
    }

    strFilePath = strFileFullPath.substr(szInx + 1, szInx);
    return SUCCESS;
}

int CFile::FileSize(const char* pszFilePath, uint32_t& uiSize)
{
    if (NULL == pszFilePath) {
        return FAILED;
    }

#ifdef WIN32
    struct _stat fileSizeStat;
    if (0 != _stat(pszFilePath, &fileSizeStat)) {
#else
    struct stat fileSizeStat;
    if (0 != stat(pszFilePath, &fileSizeStat)) {
#endif
        return FAILED;
    }

    uiSize = fileSizeStat.st_size;

    return SUCCESS;
}

int CFile::GetlLastModifyTime(const char* pszFilePath, time_t& tLastModifyTime)
{
    if (NULL == pszFilePath) {
        return FAILED;
    }

#ifdef WIN32
    struct _stat fileStat;
    if (0 != _stat(pszFilePath, &fileStat)) {
#else
    struct stat fileStat;
    if (0 != stat(pszFilePath, &fileStat)) {
#endif
        return FAILED;
    }

    tLastModifyTime = fileStat.st_mtime;

    return SUCCESS;
}

int CFile::ReadFile(const string& filePath, string& fileContent)
{
    int iRet = FAILED;
    fileContent = "";
    int iTmp = filePath.find_last_of(PATH_SEPARATOR);
    string strFileName = filePath.substr(iTmp + 1);
    if (!CFile::FileExist(filePath.c_str()))
    {
        COMMLOG(OS_LOG_WARN, "Can't find file %s.", strFileName.c_str());
        return FAILED;
    }
    FILE* pFile = fopen(filePath.c_str(), "r");
    if (NULL == pFile)
    {
        COMMLOG(OS_LOG_ERROR, "Open file %s failed, errno[%d]:%s.", strFileName.c_str(), errno, strerror(errno));
        return FAILED;
    }
    
    if(fseek(pFile, 0L, SEEK_END) != 0)
    {
        COMMLOG(OS_LOG_ERROR, "Call fseek failed, errno[%d]:%s.", errno, strerror(errno));
        iRet = fclose(pFile);
        COMMLOG(OS_LOG_INFO, "Close file(%s) ret is %d.", strFileName.c_str(), iRet);
        return FAILED;
    }

    int iFileLen = (int)ftell(pFile);
    //如果文件为空，直接返回
    if (iFileLen == 0)
    {
        COMMLOG(OS_LOG_INFO, "File %s to be read is empty.", strFileName.c_str());
        iRet = fclose(pFile);
        COMMLOG(OS_LOG_INFO, "Close file(%s) ret is %d.", strFileName.c_str(), iRet);
        return SUCCESS;
    }

    if(fseek(pFile, 0L, SEEK_SET) != 0)
    {
        COMMLOG(OS_LOG_ERROR, "Call fseek failed, errno[%d]:%s.", errno, strerror(errno));
        iRet = fclose(pFile);
        COMMLOG(OS_LOG_INFO, "Close file(%s) ret is %d.", strFileName.c_str(), iRet);
        return FAILED;
    }

    char *pBuff = NULL;
    NEW_ARRAY_CATCH(pBuff, char, iFileLen + 1);
    if (!pBuff)
    {
        COMMLOG(OS_LOG_WARN, "New failed.");
        iRet = fclose(pFile);
        COMMLOG(OS_LOG_INFO, "Close file(%s) ret is %d.", strFileName.c_str(), iRet);
        return FAILED;
    }
    iRet = memset_s(pBuff, iFileLen + 1, 0, iFileLen + 1);
    if (SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
        delete[] pBuff;
        iRet = fclose(pFile);
        COMMLOG(OS_LOG_INFO, "Close file(%s) ret is %d.", strFileName.c_str(), iRet);
        return FAILED;
    }
    while(NULL != fgets(pBuff, iFileLen + 1, pFile))
    {
        fileContent.append(pBuff);
        iRet = memset_s(pBuff, iFileLen + 1, 0, iFileLen + 1);
        if (SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
            delete[] pBuff;
            iRet = fclose(pFile);
            COMMLOG(OS_LOG_INFO, "Close file(%s) ret is %d.", strFileName.c_str(), iRet);
            return FAILED;
        }
    }

    delete[] pBuff;
    iRet = fclose(pFile);
    COMMLOG(OS_LOG_INFO, "Close file(%s) ret is %d.", strFileName.c_str(), iRet);
    return SUCCESS;
}

int CFile::ReadFileWithArray(string& strFilePath, vector<string>& vecOutput)
{
    // CodeDex误报，ZERO_LENGTH_ALLOCATIONS
    // CodeDex误报，FORTIFY.Path_Manipulation
    // codedex误报, CANONICAL_FILEPATH，FilePath路径不存在这个问题
    LOGGUARD("");
    int iTmp = strFilePath.find_last_of(PATH_SEPARATOR);
    /*lint -e732 -e776*/
    string strFileName = strFilePath.substr(iTmp + 1);

    if (!CFile::FileExist(strFilePath)) {
        COMMLOG(OS_LOG_WARN, "Can't find file %s.", strFileName.c_str());
        return FAILED;
    }

    FILE* pFile = fopen(strFilePath.c_str(), "r");
    if (NULL == pFile) {
        COMMLOG(OS_LOG_ERROR, "Open file %s failed, errno[%d]:%s.", strFileName.c_str(), errno, strerror(errno));
        return FAILED;
    }

    if (fseek(pFile, 0L, SEEK_END) != 0) {
        COMMLOG(OS_LOG_ERROR, "Call fseek failed, errno[%d]:%s.", errno, strerror(errno));
        /*lint -e717*/
        GETOSERRORCODE(0 != fclose(pFile));
        return FAILED;
    }

    int iFileLen = (int)ftell(pFile);
    // 如果文件为空，直接返回
    if (iFileLen == 0) {
        COMMLOG(OS_LOG_INFO, "File %s to be read is empty.", strFileName.c_str());
        vecOutput.clear();
        /*lint -e717*/
        GETOSERRORCODE(0 != fclose(pFile));
        return SUCCESS;
    }

    if (fseek(pFile, 0L, SEEK_SET) != 0) {
        COMMLOG(OS_LOG_ERROR, "Call fseek failed, errno[%d]:%s.", errno, strerror(errno));
        /*lint -e717*/
        GETOSERRORCODE(0 != fclose(pFile));
        return FAILED;
    }

    return ReadFileAfter(pFile, iFileLen, vecOutput);
}

int CFile::ReadFileAfter(FILE* pFile, int iFileLen, vector<string>& vecOutput)
{
    char* pBuff = NULL;
    // CodeDex误报，Memory Leak
    /*lint -e737*/
    NEW_ARRAY_CATCH(pBuff, char, iFileLen + 1);
    if (!pBuff) {
        COMMLOG(OS_LOG_WARN, "New failed.");
        /*lint -e717*/
        GETOSERRORCODE(0 != fclose(pFile));
        return FAILED;
    }

    int iRet = memset_s(pBuff, iFileLen + 1, 0, iFileLen + 1);
    if (SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
        delete[] pBuff;
        /*lint -e717*/
        GETOSERRORCODE(0 != fclose(pFile));
        return FAILED;
    }

    while (NULL != fgets(pBuff, iFileLen + 1, pFile)) {
        // 写文件时在每行末尾加上了换行符，读取时去掉换行符'\n'
        // 最后一行可能没有换行符，需要特殊处理
        if (pBuff[strlen(pBuff) - 1] == '\n') {
            pBuff[strlen(pBuff) - 1] = 0;
        }
        vecOutput.push_back(pBuff);
        iRet = memset_s(pBuff, iFileLen + 1, 0, iFileLen + 1);
        if (SUCCESS != iRet) {
            COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
            delete[] pBuff;
            fclose(pFile);
            return FAILED;
        }
    }

    delete[] pBuff;
    /*lint -e717*/
    GETOSERRORCODE(0 != fclose(pFile));
    return SUCCESS;
}

#ifdef WIN32
int CFile::GetFolderFile(string& strFolder, vector<string>& vecFileList)
{
    WIN32_FIND_DATA fd = {0};
    HANDLE hSearch;

    string strFilePathName = strFolder + "\\*";
    // CodeDex误报，Missing Check against Null
    hSearch = FindFirstFile(strFilePathName.c_str(), &fd);
    if (INVALID_HANDLE_VALUE == hSearch) {
        COMMLOG(OS_LOG_ERROR, "hSearch is INVALID_HANDLE_VALUE.");
        return FAILED;
    }

    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        bool bFlag = strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..");
        if (bFlag) {
            vecFileList.push_back(fd.cFileName);
        }
    }

    for (;;) {
        int iRet = memset_s(&fd, sizeof(fd), 0, sizeof(fd));
        if (EOK != iRet) {
            COMMLOG(OS_LOG_ERROR, "memset_s failed, iRet = %d.", iRet);
            FindClose(hSearch);
            return FAILED;
        }

        if (!FindNextFile(hSearch, &fd)) {
            if (ERROR_NO_MORE_FILES == GetLastError()) {
                break;
            }
            COMMLOG(OS_LOG_ERROR, "FindNextFile failed.");
            FindClose(hSearch);
            return FAILED;
        }

        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            bool bFlag = strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..");
            if (bFlag) {
                vecFileList.push_back(fd.cFileName);
            }
        }
    }

    FindClose(hSearch);
    return SUCCESS;
}
#else
int CFile::GetFolderFile(string& strFolder, vector<string>& vecFileList)
{
    struct dirent* ptr = NULL;
    struct stat strFileInfo = {0};
    char acFullName[MAX_PATH_LEN] = {0};

    DIR* dir = opendir(strFolder.c_str());
    if (NULL == dir) {
        COMMLOG(OS_LOG_ERROR, "opendir failed.");
        return FAILED;
    }

    while ((ptr = readdir(dir)) != ptr) {
        if (FAILED ==
            snprintf_s(acFullName, MAX_PATH_LEN, MAX_PATH_LEN - 1, "%s/%s", strFolder.c_str(), ptr->d_name)) {
            closedir(dir);
            COMMLOG(OS_LOG_ERROR, "SNPRINTF_S failed.");
            return FAILED;
        }

        if (lstat(acFullName, &strFileInfo) < 0) {
            closedir(dir);
            COMMLOG(OS_LOG_ERROR, "lstat failed.");
            return FAILED;
        }

        if (!S_ISDIR(strFileInfo.st_mode)) {
            if (strcmp(ptr->d_name, ".") && strcmp(ptr->d_name, "..")) {
                vecFileList.push_back(ptr->d_name);
            }
        }
    }

    closedir(dir);
    return SUCCESS;
}
#endif

#ifdef WIN32
bool CFile::DirEmpty(const string& dirPath)
{
    _finddata_t findData;
    string strPath = dirPath + "\\*";
    intptr_t handle = _findfirst(strPath.c_str(), &findData);
    if (handle == -1) {
        return false;
    }
    do {
        if (!(strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)) {
            _findclose(handle);
            return false;
        }
    } while (_findnext(handle, &findData) == 0);
    _findclose(handle);
    return true;
}
#else
bool CFile::DirEmpty(const string& dirPath)
{
    struct dirent* ptr = NULL;
    struct stat strFileInfo = {0};
    const int maxPathLen = 2048;
    char acFullName[maxPathLen] = {0};

    DIR* dir = opendir(dirPath.c_str());
    if (NULL == dir) {
        COMMLOG(OS_LOG_ERROR, "opendir failed,dirPath(%s).", dirPath.c_str());
        return false;
    }

    while ((ptr = readdir(dir)) != ptr) {
        if (FAILED ==
            snprintf_s(acFullName, maxPathLen, maxPathLen - 1, "%s/%s", dirPath.c_str(), ptr->d_name)) {
            closedir(dir);
            COMMLOG(OS_LOG_ERROR, "SNPRINTF_S failed,dirPath(%s),d_name(%s).", dirPath.c_str(), ptr->d_name);
            return false;
        }

        if (lstat(acFullName, &strFileInfo) < 0) {
            closedir(dir);
            COMMLOG(OS_LOG_ERROR, "lstat failed,fileName(%s).", acFullName);
            return false;
        }

        // 只有'.'和'..'两个子目录的是空目录
        if (!S_ISDIR(strFileInfo.st_mode)) {
            closedir(dir);
            return false;
        } else {
            if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
                closedir(dir);
                return false;
            }
        }
    }

    closedir(dir);
    return true;
}
#endif

bool CFile::GetProfileSection(const string& file, const string& key, string& value)
{
    string findKey = key + "=";
    ifstream stream;
    stream.open(file.c_str(), ifstream::in);
    string line;

    bool bRet = false;
    if (!stream.is_open()) {
        return bRet;
    }

    while (getline(stream, line)) {
        if (line.find(findKey) != string::npos) {
            value = line.substr(findKey.length(), line.length() - findKey.length());
            bRet = true;
            break;
        }
    }
    stream.close();
    return bRet;
}

bool CFile::WaitForFile(const string& pszFilePath, int iSleepInterval, int iSleepCount)
{
    bool bIsExist = false;
    int iCount = iSleepCount;

    while (iCount > 0) {
        if (FileExist(pszFilePath)) {
            bIsExist = true;
            break;
        }

        COMMLOG(OS_LOG_DEBUG, "File \"%s\" dose not exist, wait for it.", pszFilePath.c_str());

        DoSleep((uint32_t)iSleepInterval);
        iCount--;
    }

    return bIsExist;
}

// 注意：此接口会被日志接口调用，这里面不能再调用LOG接口写日志，否则可能导致死锁
int CFile::DelFile(const string& pszFilePath)
{
    if ("" == pszFilePath) {
        return SUCCESS;
    }
    if (!FileExist(pszFilePath)) {
        return SUCCESS;
    }
#ifdef WIN32
    DeleteFile(pszFilePath.c_str(), pszFilePath.length());
    return SUCCESS;
#else
    // Coverity&Fortify误报:FORTIFY.Race_Condition--File_System_Access
    // DelFile方法都没有在setuid提升到root时调用
    int ret = remove(pszFilePath.c_str());
    if (ret != EOK) {
        return FAILED;
    }
#endif

    return SUCCESS;
}

int CFile::CreateFile(const string& pszFilePath)
{
    if ("" == pszFilePath) {
        return SUCCESS;
    }

    if (FileExist(pszFilePath)) {
        return SUCCESS;
    }

#ifdef WIN32
    COMMLOG(OS_LOG_ERROR, "Windows does not support delete folder.");
    return FAILED;
#else
    FILE* pFile = fopen(pszFilePath.c_str(), "a");
    if (NULL == pFile) {
        COMMLOG(OS_LOG_ERROR, "Open file %s failed, errno[%d]:%s.", pszFilePath.c_str(), errno, strerror(errno));
        return FAILED;
    }
    fclose(pFile);
    return SUCCESS;
    
#endif
}

int CFile::CheckFileAccess(const string& pszFilePath)
{
    int iRet;
    struct stat buf;

    if ("" == pszFilePath) {
        COMMLOG(OS_LOG_ERROR, "File name is empry.");
        return FAILED;
    }
    if (!FileExist(pszFilePath)) {
        COMMLOG(OS_LOG_ERROR, "File [%s] does not exist.", pszFilePath.c_str());
        return FAILED;
    }
#ifdef WIN32

#else
    iRet = stat(pszFilePath.c_str(), &buf);
    if (0 != iRet) {
        COMMLOG(OS_LOG_ERROR, "Get file [%s] access failed.", pszFilePath.c_str());
        return FAILED;
    }

    if (0 != buf.st_uid) {  // 文件拥有者只能是root, root的uid=0
        COMMLOG(OS_LOG_ERROR, "File [%s] uid is wrong.", pszFilePath.c_str());
        return FAILED;
    }

    if ((0 != (buf.st_mode & S_IWGRP)) || (0 != (buf.st_mode & S_IWOTH))) {  // 文件属组和其他用户不能写
        COMMLOG(OS_LOG_ERROR, "File [%s] access is wrong, group and other cannot write.", pszFilePath.c_str());
        return FAILED;
    }
#endif
    COMMLOG(OS_LOG_DEBUG, "File [%s] access.", pszFilePath.c_str());
    return SUCCESS;
}

int CFile::CopyFileCoverDest(const string& pszSourceFilePath, const string& pszDestFilePath)
{
    char buff[TASK_STEP_INTERVAL_HUNDERED] = {0};
    if (pszSourceFilePath == "" || pszDestFilePath == "") {
        return SUCCESS;
    }

    ifstream streamin;
    streamin.open(pszSourceFilePath.c_str(), ifstream::in);
    if (!streamin.is_open()) {
        COMMLOG(OS_LOG_ERROR, "Open file %s failed, errno[%d]:%s.", pszSourceFilePath.c_str(), errno, strerror(errno));
        return FAILED;
    }

    ofstream streamout;
    streamout.open(pszDestFilePath.c_str(), ifstream::out);
    if (!streamout.is_open()) {
        COMMLOG(OS_LOG_ERROR, "Open file %s failed, errno[%d]:%s.", pszDestFilePath.c_str(), errno, strerror(errno));
        streamin.close();
        return FAILED;
    }

    while (!streamin.eof()) {
        streamin.read(buff, sizeof(buff));
        streamout.write(buff, streamin.gcount());
    }

    streamin.close();
    streamout.flush();
    streamout.close();

    return SUCCESS;
}

int CFile::CheckFileName(const string &filename)
{
    size_t idx;
    for (auto pchar:FILENAME_INVAILED_CHARS) {
        idx = filename.find(pchar, 0);
        if (string::npos != idx) {
            return FAILED;
        }
    }
    return SUCCESS;
}

#ifdef WIN32
int CFile::GetFolderDir(const string& strFolder, std::vector<string>& vecNameList)
{
    std::string strFilePathName = strFolder + "\\*";
    char szErr[MAX_ERROR_MSG_LEN] = { 0 };

    WIN32_FIND_DATAA fd = { 0 };
    HANDLE hSearch = FindFirstFile(strFilePathName.c_str(), &fd);
    if (INVALID_HANDLE_VALUE == hSearch) {
        int err = GetOSError();
        ERRLOG("hSearch is INVALID_HANDLE_VALUE, strFolder=%s, errno[%d]:%s.",
            strFolder.c_str(), err, GetOSStrErr(err, szErr, sizeof(szErr)));
        return FAILED;
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
                int err = GetOSError();
                ERRLOG("FindNextFile failed, errno[%d]:%s.", err, GetOSStrErr(err, szErr, sizeof(szErr)));
                FindClose(hSearch);
                return FAILED;
            }
        }
    } while (true);
    FindClose(hSearch);
    return SUCCESS;
}
#else
int CFile::GetFolderDir(const string& strFolder, std::vector<string>& vecNameList)
{
    struct dirent* ptr = nullptr;
    DIR* dir = opendir(strFolder.c_str());
    if (dir == nullptr) {
        int err = GetOSError();
        char szErr[MAX_ERROR_MSG_LEN] = { 0 };
        ERRLOG("Open %s failed, errno[%d]:%s..", strFolder.c_str(), err, GetOSStrErr(err, szErr, sizeof(szErr)));
        return FAILED;
    }
    struct dirent dp;
    struct stat stFileInfo = {0};
    std::string fileName;
    while ((readdir_r(dir, &dp, &ptr) == SUCCESS) && (ptr != NULL)) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }

        fileName = strFolder + PATH_SEPARATOR + ptr->d_name;
        if (lstat(fileName.c_str(), &stFileInfo) < 0) {
            closedir(dir);
            ERRLOG("Exec lstat failed.");
            return FAILED;
        }

        if (S_ISDIR(stFileInfo.st_mode)) {
            vecNameList.push_back(ptr->d_name);
        }
    }

    closedir(dir);
    return SUCCESS;
}
#endif

int CIPCFile::ReadFile(string& strFilePath, vector<string>& vecOutput)
{
    // CodeDex误报，ZERO_LENGTH_ALLOCATIONS
    // codedex误报CANONICAL_FILEPATH，FilePath路径不存在这个问题
    int iTmp = strFilePath.find_last_of(PATH_SEPARATOR);
    /*lint -e732 -e776*/
    string strFileName = strFilePath.substr(iTmp + 1);

    int iRet = CFile::ReadFileWithArray(strFilePath, vecOutput);
    if (SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Read file failed.");
        return iRet;
    }

    return SUCCESS;
}

int CIPCFile::WriteFile(string& strFilePath, vector<string>& vecInput)
{
    // CodeDex误报，TOCTOU
#ifndef WIN32
    struct stat fileStat;
    int iRet1;
    iRet1 = memset_s(&fileStat, sizeof(fileStat), 0, sizeof(fileStat));
    if (iRet1 != SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret = %d.", iRet1);
        return FAILED;
    }
    int fileExist = 0;
#endif
    int iRet = FAILED;

    const char* strBaseFileName;
    // CodeDex误报，Unchecked Return Value函数BaseFileName的返回值不会为空
    strBaseFileName = BaseFileName(strFilePath).c_str();

    if (CFile::FileExist(strFilePath)) {
        COMMLOG(OS_LOG_INFO, "WriteFile: file path: '%s'.", strFilePath.c_str());
        // 读取文件权限
#ifndef WIN32
        if (0 != stat(strFilePath.c_str(), &fileStat)) {
            COMMLOG(OS_LOG_ERROR, "get file stat of %s failed, errno[%d]:%s.", strBaseFileName, errno, strerror(errno));
            return FAILED;
        }
        fileExist = 1;
#endif
        // 删除重名的文件
        COMMLOG(OS_LOG_INFO, "WriteFile: remove file has the same name.");
        iRet = remove(strFilePath.c_str());
        if (0 != iRet) {
            COMMLOG(OS_LOG_ERROR, "Remove file %s failed, errno[%d]:%s.", strBaseFileName, errno, strerror(errno));
            return FAILED;
        }
    }

#ifndef WIN32
    return WriteFileInner(strFilePath, vecInput, fileExist, fileStat);
#else
    return WriteFileInner(strFilePath, vecInput);
#endif
}

#ifndef WIN32
int CIPCFile::WriteFileInner(
    string& strFilePath, vector<string>& vecInput, int fileExist, struct stat& fileStat)
#else
int CIPCFile::WriteFileInner(string& strFilePath, vector<string>& vecInput)
#endif
{
    // codedex误报， Race Condition:File System Access
    FILE* pFile = fopen(strFilePath.c_str(), "a+");
    if (NULL == pFile) {
        COMMLOG(OS_LOG_ERROR, "Open file %s failed, errno[%d]:%s.", strFilePath.c_str(), errno, strerror(errno));
        return FAILED;
    }

    for (vector<string>::iterator iter = vecInput.begin(); iter != vecInput.end(); iter++) {
        fprintf(pFile, "%s\n", iter->c_str());
    }

    int iRet = fflush(pFile);
    if (0 != iRet) {
        COMMLOG(OS_LOG_ERROR, "Call fflush failed, errno[%d]:%s.", errno, strerror(errno));
        iRet = fclose(pFile);
        /*lint -e717*/
        GETOSERRORCODE(0 != iRet);
        return FAILED;
    }
    iRet = fclose(pFile);
    /*lint -e717*/
    GETOSERRORCODE(0 != iRet);
#ifndef WIN32
    // 将文件修改为删除之前权限
    if (1 == fileExist) {
        if (ChmodFile(strFilePath, fileStat.st_mode) != SUCCESS) {
            CFile::DelFile(strFilePath);
            return FAILED;
        }
    }
#endif
    COMMLOG(OS_LOG_DEBUG, "Write file succ.");
    return SUCCESS;
}

int CIPCFile::WriteInput(string& strUniqueID, const string& strInput)
{
    if (strInput.empty()) {
        return SUCCESS;
    }

    string strFileName = INPUT_TMP_FILE + strUniqueID;
    // 增加临时文件夹路径
    string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);

    const char* strBaseFileName;
    strBaseFileName = BaseFileName(strFileName).c_str();

    vector<string> vecInput;
    vecInput.push_back(strInput);

    int iRet = WriteFile(strFilePath, vecInput);
    if (SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Write input info file failed, file %s.", strBaseFileName);
    }

    return iRet;
}

int CIPCFile::ReadInput(string& strUniqueID, string& strInput)
{
    string strFileName = INPUT_TMP_FILE + strUniqueID;
    // 增加临时文件夹路径
    string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);
    vector<string> vecOutput;
    const string strBaseFileName = BaseFileName(strFileName);
    int iRet = ReadFile(strFilePath, vecOutput);
    if (SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Write input info file failed, file %s.", strBaseFileName.c_str());
        return iRet;
    }

    if (vecOutput.size() != 1) {
        COMMLOG(
            OS_LOG_ERROR, "Invalid file content, file %s, line count %d.", strBaseFileName.c_str(), vecOutput.size());
        return FAILED;
    }

    strInput = vecOutput[0];
    COMMLOG(OS_LOG_DEBUG, "Read input info file succ.");
    return SUCCESS;
}

int CIPCFile::WriteResult(string& strUniqueID, vector<string>& vecRlt)
{
    if (vecRlt.empty()) {
        COMMLOG(OS_LOG_INFO, "vecRlt is empty, no need to write result file.");
        return SUCCESS;
    }
    string strFileName = RESULT_TMP_FILE + strUniqueID;
    string strFilePath = CPath::GetInstance().GetStmpFilePath(strFileName);

    const char* strBaseFileName;
    strBaseFileName = BaseFileName(strFileName).c_str();

    int iRet = WriteFile(strFilePath, vecRlt);
    if (SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Write result file failed, file %s.", strBaseFileName);
    } else {
        int iChownRet = ChownResult(strUniqueID);
        if (SUCCESS != iChownRet) {
            COMMLOG(OS_LOG_ERROR, "Chown result file failed, file %s.", strBaseFileName);
        }
    }

    return iRet;
}

int CIPCFile::ReadResult(const string& strFileName, vector<string>& vecRlt)
{
    string strFilePathFile = CPath::GetInstance().GetStmpFilePath(strFileName);

    const char* strBaseFileName;
    strBaseFileName = BaseFileName(strFileName).c_str();

    // 如果结果文件不存在，有可能是没有满足条件的数据，这种情况返回成功
    if (!CFile::FileExist(strFilePathFile)) {
        COMMLOG(OS_LOG_WARN, "ReadResult: Can't find file %s.", strFileName.c_str());
        return SUCCESS;
    }

    vecRlt.clear();
    int iRet = ReadFile(strFilePathFile, vecRlt);
    if (SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Read result file failed, file %s.", strFileName.c_str());
    }

    return iRet;
}

int CIPCFile::ReadOldResult(string& strUniqueID, vector<string>& vecRlt)
{
    string strFileName = "RST" + strUniqueID + ".txt";
    string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);

    const char* strBaseFileName;
    strBaseFileName = BaseFileName(strFileName).c_str();

    // 如果结果结果文件不存在，有可能是没有满足条件的数据，这种情况返回成功
    if (!CFile::FileExist(strFilePath)) {
        COMMLOG(OS_LOG_WARN, "Can't find file %s.", strBaseFileName);
        return SUCCESS;
    }

    int iRet = ReadFile(strFilePath, vecRlt);
    if (SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Read result file failed, file %s.", strBaseFileName);
    }

    return iRet;
}

int CIPCFile::ChownResult(string& strUniqueID)
{
#ifndef WIN32
    int iRet;
    // 获取rdadmin用户的uid
    int uid(-1), gid(-1);

    string strFileName = RESULT_TMP_FILE + strUniqueID;
    string strFilePath = CPath::GetInstance().GetStmpFilePath(strFileName);

    // 如果结果结果文件不存在，有可能是没有满足条件的数据，这种情况返回成功
    // rdagent在读取结果文件时会判断操作的结果
    if (!CFile::FileExist(strFilePath)) {
        COMMLOG(OS_LOG_WARN, "Can't find file %s.", strFileName.c_str());
        return SUCCESS;
    }

    iRet = GetUidByUserName(AGENT_RUNNING_USER, uid, gid);
    if (SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Get user(%s) uid and gid failed.", AGENT_RUNNING_USER.c_str());
        return FAILED;
    }

    // 设置rdadmin的uid和gid
    iRet = ChownFile(strFilePath, uid, gid);
    if (SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Chown file failed, file %s.", strFileName.c_str());
        return FAILED;
    }

    return SUCCESS;
#else
    return SUCCESS;
#endif
}

int CIPCFile::RemoveFile(const string& strUniqueID)
{
    string strFileName = INPUT_TMP_FILE + strUniqueID;
    // 增加临时文件夹路径
    string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);
    return CFile::DelFile(strFilePath);
}

int CIPCFile::IsValidFilename(const string& strFilename)
{
    int iLength = 0, iIndex = 0;
    char cSpecialChar[] = {
        '/', ':', ';', '<', '=', '>', '?', '@', '[', '\\', ']', '^', '`', '{', '|', '}', '~', ' ', '\0'};
    char cCtrlCharBegin = 0x00, cCtrlCharEnd = 0x2D;
    const char* filename = strFilename.c_str();

    if (filename == NULL) {
        return FAILED;
    } else {
        iLength = strlen(filename);
        if (iLength >= MAX_FILE_NAME_LEN)
            return FAILED;
    }

    for (iIndex = 0; iIndex < iLength; iIndex++) {
        if ((filename[iIndex] >= cCtrlCharBegin) && (filename[iIndex] <= cCtrlCharEnd)) {
            return FAILED;
        } else if (strchr(cSpecialChar, filename[iIndex]) != NULL) {
            return FAILED;
        }
    }
    return SUCCESS;
}

int CIPCFile::WriteJsonStrToKVFile(
    const string& strFileName, Json::Value jsonValue, const string& KVSplitStr)
{
#ifndef WIN32
    static const int MAX_ITEME_SIZE = 512;

    ostringstream oss;
    Json::Value::Members members;
    members = jsonValue.getMemberNames();  // 获取所有key的值
    // 遍历每个key
    for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); ++iterMember) {
        string strKey = *iterMember;
        if (jsonValue[strKey.c_str()].isString()) {
            string tempValue = jsonValue[strKey.c_str()].asString();
            if (tempValue.size() > MAX_ITEME_SIZE) {
                COMMLOG(OS_LOG_ERROR, "The value of Json key \"%s\" over max size", strKey.c_str());
                return FAILED;
            }
            oss << strKey << STR_EQUAL << tempValue << endl;
        } else {
            COMMLOG(OS_LOG_ERROR, "The value of Json key \"%s\" is not string.", strKey.c_str());
            return FAILED;
        }
    }

    string strFilepath = CPath::GetInstance().GetTmpFilePath(strFileName);
    string strJsonparm = oss.str();
    vector<string> strvec;
    strvec.push_back(strJsonparm);

    int iRet = CIPCFile::WriteFile(strFilepath, strvec);
    if (SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Write result file failed, file %s.", strFilepath);
        return FAILED;
    } else {
        // 获取rdadmin用户的uid
        int uid(-1), gid(-1);
        iRet = GetUidByUserName(AGENT_RUNNING_USER, uid, gid);
        if (SUCCESS != iRet) {
            COMMLOG(OS_LOG_ERROR, "Get user(%s) uid and gid failed.", AGENT_RUNNING_USER.c_str());
            return FAILED;
        }

        // 设置rdadmin的uid和gid
        iRet = ChownFile(strFilepath, uid, gid);
        if (SUCCESS != iRet) {
            COMMLOG(OS_LOG_ERROR, "Chown file failed, file %s.", strFileName.c_str());
            return FAILED;
        }
    }
    return SUCCESS;
#else
    return FAILED;
#endif
}

} // namespace Module
