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
#include "Utils.h"
#include "securec.h"
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif
#include <fcntl.h>
#include "log/Log.h"

using namespace std;

namespace {
const int WIN_HOSTNAME_BUFFER_SIZE = 32767;
const int WINDOWS_USERNAME_LEN = 512;
const int LINUX_USERNAME_LEN = 1024;
const int UTILS_NUM_1000 = 1000;
const int HOST_NAME_LEN = 512;
}

namespace Module {

int SafeStoi(const std::string& str, int defaultValue)
{
    try {
        int res = std::stoi(str);
        return res;
    } catch (const std::exception& e) {
        ERRLOG("Invalid Argument for stoi: %s, ERR:%s", str.c_str(), e.what());
        return defaultValue;
    }
}

long SafeStol(const std::string& str, long defaultValue)
{
    try {
        long res = std::stol(str);
        return res;
    } catch (const std::exception& e) {
        ERRLOG("Invalid Argument for stoi: %s, ERR:%s", str.c_str(), e.what());
        return defaultValue;
    }
}

long long SafeStoll(const std::string& str, long long defaultValue)
{
    try {
        long long res = std::stoll(str);
        return res;
    } catch (const std::exception& e) {
        ERRLOG("Invalid Argument for stoi: %s, ERR:%s", str.c_str(), e.what());
        return defaultValue;
    }
}

float SafeStof(const std::string& str, float defaultValue)
{
    try {
        return std::stof(str);
    } catch (const std::invalid_argument&) {
        return defaultValue;
    } catch (const std::out_of_range&) {
        return defaultValue;
    }
}

int CheckCmdDelimiter(string& str)
{
    if (string::npos != str.find(STR_SEMICOLON, 0) ||
        string::npos != str.find(STR_VERTICAL_LINE, 0) ||
        string::npos != str.find(STR_ADDRESS, 0) ||
        string::npos != str.find(SIGN_IN, 0) ||
        string::npos != str.find(SIGN_OUT, 0) ||
        string::npos != str.find(SIGN_BACKQUOTE, 0) ||
        string::npos != str.find(SIGN_EXCLAMATION, 0) ||
        string::npos != str.find(STR_CODE_WARP, 0) ||
        string::npos != str.find(SIGN_DOLLAR, 0)) {
        return FAILED;
    }

    return SUCCESS;
}

int GetOSError()
{
#ifdef WIN32
    return GetLastError();
#else
    return errno;
#endif
}

#ifdef WIN32
AGENT_API int GetCurrentUserNameWin(string& strUserName, unsigned long& iErrCode)
{
    unsigned long size = WINDOWS_USERNAME_LEN;
    char pUsername[WINDOWS_USERNAME_LEN] = {0};
    if (!GetUserName(pUsername, &size)) {
        iErrCode = GetOSError();
        // 日志模块使用，无法记录日志
        return FAILED;
    }
    strUserName = pUsername;
    iErrCode = 0;
    memset_s(pUsername, WINDOWS_USERNAME_LEN, 0, WINDOWS_USERNAME_LEN);
    delete[] pUsername;
    return SUCCESS;
}
#elif defined SOLARIS
AGENT_API int GetCurrentUserNameSol(string& strUserName, unsigned long& iErrCode)
{
    // 初始化buf的大小,通过系统函数进行获取
    size_t size = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (size == 0) {
        size = LINUX_USERNAME_LEN;
    }

    char* pbuf = reinterpret_cast<char*>(malloc(size));
    if (pbuf == NULL) {
        iErrCode = GetOSError();
        return FAILED;
    }

    struct passwd pwd;
    struct passwd* result = nullptr;
    int error = getpwuid_r(getuid(), &pwd, pbuf, size, &result);
    if (error == FAILED || result == nullptr) {
        iErrCode = static_cast<unsigned long>(GetOSError());
        free(pbuf);
        return FAILED;
    }
    strUserName = pwd.pw_name;

    free(pbuf);
    pbuf = NULL;

    return SUCCESS;
}
#else
AGENT_API int GetCurrentUserNameOther(string& strUserName, unsigned long& iErrCode)
{
    struct passwd pwd;
    struct passwd* result = NULL;
    int error;

    // 初始化buf的大小,通过系统函数进行获取
    /*lint -e732*/
    size_t size = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (size == 0) {
        size = LINUX_USERNAME_LEN;
    }

    char* pbuf = reinterpret_cast<char*>(malloc(size));
    if (pbuf == NULL) {
        /*lint -e571*/
        iErrCode = static_cast<unsigned long>(GetOSError());
        return FAILED;
    }

    int iRet = memset_s(pbuf, size, 0, size);
    if (iRet != EOK) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, iRet %d.", iRet);
        /*lint -e571*/
        iErrCode = static_cast<unsigned long>(GetOSError());
        free(pbuf);
        return FAILED;
    }

    error = getpwuid_r(getuid(), &pwd, pbuf, size, &result);
    if (error == FAILED || result == NULL) {
        /*lint -e571*/
        iErrCode = static_cast<unsigned long>(GetOSError());
        free(pbuf);
        return FAILED;
    }
    strUserName = pwd.pw_name;

    free(pbuf);
    return SUCCESS;
}
#endif

int GetCurrentUserName(string& strUserName, unsigned long& iErrCode)
{
    // CodeDex误报，UNUSED_VALUE
    // CodeDex误报，NEGATIVE_RETURNS，不用检查iErrCode入参的正负
#ifdef WIN32
    return GetCurrentUserNameWin(strUserName, iErrCode);
#elif defined SOLARIS
    return GetCurrentUserNameSol(strUserName, iErrCode);
#else
    return GetCurrentUserNameOther(strUserName, iErrCode);
#endif
}

#ifdef WIN32
int GetCurrentUserNameW(wstring& strUserName, unsigned long& iErrCode)
{
    unsigned long size = WINDOWS_USERNAME_LEN;
    wchar_t pUsername[WINDOWS_USERNAME_LEN] = {0};
    if (!GetUserNameW(pUsername, &size)) {
        iErrCode = GetOSError();
        // 日志模块使用，无法记录日志
        return FAILED;
    }
    strUserName = pUsername;
    iErrCode = 0;
    memset_s(pUsername, WINDOWS_USERNAME_LEN, 0, WINDOWS_USERNAME_LEN);
    delete[] pUsername;
    return SUCCESS;
}
#endif

#ifndef WIN32
int GetUidByUserName(string strUserName, int& uid, int& gid)
{
    struct passwd* user = NULL;
    uid = -1;
    gid = -1;

    CThreadAutoLock cLock(&UuidByUNameMutex::GetInstance().GetValue());
    setpwent();
    while ((user = getpwent()) != 0) {
        string strUser = string(user->pw_name);
        if (strUser.compare(strUserName) == 0) {
            uid = static_cast<int>(user->pw_uid);
            gid = static_cast<int>(user->pw_gid);
            break;
        }
    }
    endpwent();

    if (uid == -1) {
        COMMLOG(OS_LOG_ERROR, "Get uid of user(%s) failed.", strUserName.c_str());
        return FAILED;
    } else {
        COMMLOG(OS_LOG_DEBUG, "User(%s) info: uid=%d, gid=%d.", strUserName.c_str(), uid, gid);
        return SUCCESS;
    }
}

int ChownFile(string strFileName, int uid, int gid)
{
    struct stat buf;
    int iRet = lstat(strFileName.c_str(), &buf);
    if (iRet != SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "lstat file(%s) failed, errno[%d]: %s.", BaseFileName(strFileName).c_str(),
            errno, strerror(errno));
        return FAILED;
    }
    if (S_ISLNK(buf.st_mode)) {
        COMMLOG(OS_LOG_ERROR, "chown file(%s) failed, file is symbolic link.", BaseFileName(strFileName).c_str());
        return FAILED;
    }

    iRet = chown(strFileName.c_str(), static_cast<uint32_t>(uid), static_cast<uint32_t>(gid));
    if (iRet != SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "chown file(%s) failed, errno[%d]: %s.",
                BaseFileName(strFileName).c_str(), errno, strerror(errno));
        return FAILED;
    }

    return iRet;
}

int ChmodFile(string strFileName, uint32_t mode)
{
    struct stat buf;
    int iRet = lstat(strFileName.c_str(), &buf);
    if (iRet != SUCCESS) {
        return FAILED;
    }
    if (S_ISLNK(buf.st_mode)) {
        return FAILED;
    }
    iRet = chmod(strFileName.c_str(), mode_t(mode));
    if (iRet != SUCCESS) {
        return FAILED;
    }

    return iRet;
}
#endif


size_t GetMachineId()
{
    size_t machineId = 0;
    std::hash<std::string> hashFunc;
#ifdef WIN32
    TCHAR  infoBuf[WIN_HOSTNAME_BUFFER_SIZE];
    DWORD  bufCharCount = WIN_HOSTNAME_BUFFER_SIZE;
    if (!::GetComputerName(infoBuf, &bufCharCount)) {
        ERRLOG("gethostname for windows failed, err: %s", ::GetLastError());
        return machineId;
    }
    machineId = hashFunc(infoBuf);
    return machineId;
#else
    char hostName[HOST_NAME_LEN + 1] = { 0 };
    if (gethostname(hostName, HOST_NAME_LEN) == SUCCESS) {
        machineId = hashFunc(hostName);
        COMMLOG(OS_LOG_DEBUG, "hostName: %s, machineId: %u", hostName, machineId);
    } else {
        ERRLOG("gethostname() failed.");
    }
    return machineId;
#endif
}


const string BaseFileName(const string& pszFileName)
{
    size_t found = pszFileName.find_last_of("/\\");
    return (found == string::npos) ? "" : pszFileName.substr(found + 1);
}

char* GetOSStrErr(int err, char buf[], size_t buf_len)
{
#ifdef WIN32
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD)err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Default language
        (LPTSTR)&lpMsgBuf,
        0,
        NULL);

    int iRet = strncpy_s(buf, buf_len, reinterpret_cast<char*>(lpMsgBuf),
        strlen(reinterpret_cast<char*>(lpMsgBuf)));
    if (iRet != EOK) {
        LocalFree(lpMsgBuf);
        return NULL;
    }
    buf[buf_len - 1] = 0;
    LocalFree(lpMsgBuf);
#else
    int iRet = strncpy_s(buf, buf_len, strerror(err),  strlen(strerror(err)));
    if (iRet != EOK) {
        return nullptr;
    }
    buf[buf_len - 1] = 0;
#endif
    return buf;
}

void DoSleep(uint32_t ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    struct timeval stTimeOut = {};

    stTimeOut.tv_sec = ms / UTILS_NUM_1000;
    stTimeOut.tv_usec = (ms % UTILS_NUM_1000) * UTILS_NUM_1000;
    (void) select(0, NULL, NULL, NULL, &stTimeOut);
#endif
}

} // namespace Module