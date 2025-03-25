/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Utils.cpp
 * @brief  The implemention about utils
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "common/Utils.h"
#ifndef WIN32
#include <csignal>
#include <libgen.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#endif
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include "common/Log.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "common/AlarmInfoXmlParser.h"
#include "common/MpString.h"
#include "common/JsonUtils.h"
#include "common/MpString.h"
#include "common/ErrorCode.h"
#include "securec.h"
#include "array/storage_defines.h"
#include "common/Ip.h"
using namespace std;
namespace {
const mp_int32 UTILS_NUM_1000 = 1000;
const mp_uchar UTILS_NUM_255 = 255;
const mp_uchar UTILS_NUM_254 = 254;
const mp_uchar UTILS_NUM_233 = 233;
const mp_uchar UTILS_NUM_4 = 4;
const mp_uchar UTILS_NUM_8 = 8;

const mp_uint32 UTILS_NUM_0 = 0;
const mp_uint32 UTILS_NUM_STORPROTOCOL_ISCSI = 1;
const mp_uint32 UTILS_NUM_STORPROTOCOL_NAS = 2;
const mp_uint32 UTILS_NUM_VMNAME_MAX_LEN = 128;
const mp_uint32 UTILS_NUM_ERRDETAIL_MAX_LEN = 1024;
const mp_uint32 UTILS_NUM_PORTS_MIN = 1;
const mp_uint32 UTILS_NUM_PORTS_MAX = 65535;
const mp_uint32 UTILS_NUM_TASKID_MAX_LEN = 60;
const mp_uint32 UTILS_NUM_FOTMAT_STRING_MAX_LEN = 512;
const mp_uint32 UTILS_NUM_UUID_MAX_LEN = 36;
}  // namespace

/* ------------------------------------------------------------
Description  :睡眠函数
Input        : ms -- 时间
------------------------------------------------------------- */
mp_void DoSleep(mp_uint32 ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    struct timeval stTimeOut;

    stTimeOut.tv_sec = ms / UTILS_NUM_1000;
    stTimeOut.tv_usec = (ms % UTILS_NUM_1000) * UTILS_NUM_1000;
    (mp_void) select(0, NULL, NULL, NULL, &stTimeOut);
#endif
}
/* ------------------------------------------------------------
Description  :注册信号
Input        : signo -- 信号 func -- 跟信号绑定的接口
------------------------------------------------------------- */
mp_int32 SignalRegister(mp_int32 signo, signal_proc func)
{
#ifndef WIN32
    struct sigaction act;

    (mp_void) memset_s(&act, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    if (sigemptyset(&act.sa_mask) != 0) {
        return MP_FAILED;
    }
    act.sa_flags = 0;
    act.sa_handler = func;

    struct sigaction oact;
    if (sigaction(signo, &act, &oact) != 0) {
        return MP_FAILED;
    }
#endif

    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  :命令行输入检查
Input        : str -- 命令行输入
------------------------------------------------------------- */
mp_int32 CheckCmdDelimiter(const mp_string& str)
{
    if (mp_string::npos != str.find(STR_SEMICOLON, 0) || mp_string::npos != str.find(STR_VERTICAL_LINE, 0) ||
        mp_string::npos != str.find(STR_ADDRESS, 0) || mp_string::npos != str.find(SIGN_IN, 0) ||
        mp_string::npos != str.find(SIGN_OUT, 0) || mp_string::npos != str.find(SIGN_BACKQUOTE, 0) ||
        mp_string::npos != str.find(SIGN_EXCLAMATION, 0) || mp_string::npos != str.find(STR_CODE_WARP, 0) ||
        mp_string::npos != str.find(SIGN_DOLLAR, 0)) {
        return ERROR_COMMON_INVALID_PARAM;
    }
#ifdef WIN32
    if (mp_string::npos != str.find(CHAR_SLASH, 0)) {
        return ERROR_COMMON_INVALID_PARAM;
    }
#else
    if (mp_string::npos != str.find(STR_BACKSLASH, 0)) {
        return ERROR_COMMON_INVALID_PARAM;
    }
#endif
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :获取系统错误码
------------------------------------------------------------- */
mp_int32 GetOSError()
{
#ifdef WIN32
    return GetLastError();
#else
    return errno;
#endif
}
/* ------------------------------------------------------------
Description  :获取系统错误描述
------------------------------------------------------------- */
mp_char* GetOSStrErr(mp_int32 err, mp_char buf[], std::size_t buf_len)
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

    mp_int32 iRet = strncpy_s(
        buf, buf_len, reinterpret_cast<mp_char*>(lpMsgBuf), strlen(reinterpret_cast<mp_char*>(lpMsgBuf)));
    if (iRet != EOK) {
        LocalFree(lpMsgBuf);
        return NULL;
    }
    buf[buf_len - 1] = 0;
    LocalFree(lpMsgBuf);
#else
    mp_int32 iRet = strncpy_s(buf, buf_len, strerror(err), strlen(strerror(err)));
    if (iRet != EOK) {
        return nullptr;
    }
    buf[buf_len - 1] = 0;
#endif
    return buf;
}

/* ------------------------------------------------------------
Description  :打开lib
Input        : pszLibName -- lib名
------------------------------------------------------------- */
mp_handle_t DlibOpen(const mp_string& pszLibName)
{
    LOGGUARD("");
#ifdef WIN32
    mp_handle_t h_dlib = LoadLibrary(pszLibName.c_str());
    return h_dlib;
#else
    return DlibOpenEx(pszLibName, MP_TRUE);
#endif
}

/* ------------------------------------------------------------
Description  :打开lib
Input        : pszLibName -- lib名
------------------------------------------------------------- */
mp_handle_t DlibOpenEx(const mp_string& pszLibName, mp_bool bLocal)
{
    LOGGUARD("");
#ifdef WIN32
    return DlibOpen(pszLibName);
#else
    mp_int32 flag = bLocal ? DFLG_LOCAL : DFLG_GLOBAL;
    // Coverity&Fortify误报:FORTIFY.Process_Control
    // pszLibName引用出传递都是绝对路径
    return dlopen(pszLibName.c_str(), flag);
#endif
}

/* ------------------------------------------------------------
Description  :关闭lib
Input        : hDlib -- 打开lib时的句柄
------------------------------------------------------------- */
mp_void DlibClose(mp_handle_t hDlib)
{
    LOGGUARD("");
#ifdef WIN32
    if (hDlib == 0) {
        return;
    }

    FreeLibrary(hDlib);
#else
    if (hDlib == 0) {
        return;
    }

    dlclose(hDlib);
#endif
}

/* ------------------------------------------------------------
Description  :取得符号pszFname的地址
Input        : hDlib -- 句柄  pszFname -- 符号名
------------------------------------------------------------- */
mp_void* DlibDlsym(mp_handle_t hDlib, const mp_string& pszFname)
{
    LOGGUARD("");
#ifdef WIN32
    return GetProcAddress(hDlib, pszFname.c_str());
#else
    return dlsym(hDlib, pszFname.c_str());
#endif
}

/* ------------------------------------------------------------
Description  :取得lib里方法执行出错信息
------------------------------------------------------------- */
const mp_char* DlibError(mp_char szMsg[], mp_uint32 isz)
{
#ifdef WIN32
    mp_int32 iErr = GetOSError();
    return GetOSStrErr(iErr, szMsg, isz);
#else

    CThreadAutoLock cLock(&DlibErrorMutex::GetInstance().GetValue());
    const mp_string pszErr = dlerror();
    if (pszErr.empty()) {
        szMsg[0] = 0;
        return nullptr;
    }

    mp_int32 iRet = strncpy_s(szMsg, isz, pszErr.c_str(), isz - 1);
    if (iRet != EOK) {
        return nullptr;
    }
    szMsg[isz - 1] = 0;
    return szMsg;
#endif
}

// Description  :初始化公共模块
mp_int32 InitCommonModules(const mp_string& pszFullBinPath)
{
    mp_int32 iRet;

    // 初始化Agent路径
    iRet = CPath::GetInstance().Init(pszFullBinPath);
    if (iRet != MP_SUCCESS) {
        printf("Init agent path failed.\n");
        return iRet;
    }

    // 初始化配置文件模块
    iRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF));
    if (iRet != MP_SUCCESS) {
        printf("Init conf file %s failed.\n", AGENT_XML_CONF.c_str());
        return iRet;
    }
    iRet = AlarmInfoXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(ALARMINFO_XML_CONF));
    if (iRet != MP_SUCCESS) {
        printf("Init conf file %s failed.\n", ALARMINFO_XML_CONF.c_str());
        return iRet;
    }

    // 初始化日志模块
    CLogger::GetInstance().Init(AGENT_LOG_NAME.c_str(), CPath::GetInstance().GetLogPath());
    return MP_SUCCESS;
}

mp_int32 GetHostName(mp_string& strHostName)
{
    mp_int32 iRet;
    mp_char szHostName[MAX_HOSTNAME_LENGTH] = {0};
#ifdef WIN32
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(1, 1);
    if (WSAStartup(wVersionRequested, &wsaData) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "WSAStartup failed");
        return MP_FALSE;
    }
    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
        COMMLOG(OS_LOG_ERROR, "WSAStartup failed");
        WSACleanup();
        return MP_FALSE;
    }
    iRet = gethostname(szHostName, sizeof(szHostName));
    if (iRet != MP_SUCCESS) {
        iRet = WSAGetLastError();
    }
    WSACleanup();
#else
    iRet = gethostname(szHostName, sizeof(szHostName));
    if (iRet != MP_SUCCESS) {
        iRet = GetOSError();
    }
#endif
    strHostName = szHostName;
    return iRet;
}

/*------------------------------------------------------------
Description  : 对value向上取2的对数值，比如：value=63，返回值为6，value=64，返回值7
Return       : 返回2的对数值
-------------------------------------------------------------*/
mp_uint32 MakeLogUpperBound(mp_uint64 value)
{
    mp_uint32 ret = 0;
    while (value != 0) {
        ret++;
        value = value >> 1;
    }

    return ret;
}

#ifdef WIN32
AGENT_API mp_int32 GetCurrentUserNameWin(mp_string& strUserName, mp_ulong& iErrCode)
{
    mp_ulong size = WINDOWS_USERNAME_LEN;
    char pUsername[WINDOWS_USERNAME_LEN] = {0};
    if (!GetUserName(pUsername, &size)) {
        iErrCode = GetOSError();
        // 日志模块使用，无法记录日志
        return MP_FAILED;
    }
    strUserName = pUsername;
    iErrCode = 0;
    memset_s(pUsername, WINDOWS_USERNAME_LEN, 0, WINDOWS_USERNAME_LEN);
    return MP_SUCCESS;
}
#elif defined SOLARIS
AGENT_API mp_int32 GetCurrentUserNameSol(mp_string& strUserName, mp_ulong& iErrCode)
{
    // 初始化buf的大小,通过系统函数进行获取
    mp_size size = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (size == MP_FALSE) {
        size = LINUX_USERNAME_LEN;
    }

    mp_char* pbuf = reinterpret_cast<char*>(malloc(size));
    if (pbuf == NULL) {
        iErrCode = GetOSError();
        return MP_FAILED;
    }

    struct passwd pwd;
    struct passwd* result = nullptr;
    mp_int32 error = getpwuid_r(getuid(), &pwd, pbuf, size, &result);
    if (error == MP_FAILED || result == nullptr) {
        iErrCode = static_cast<mp_ulong>(GetOSError());
        free(pbuf);
        pbuf = nullptr;
        return MP_FAILED;
    }
    strUserName = pwd.pw_name;

    free(pbuf);
    pbuf = nullptr;
    return MP_SUCCESS;
}
#else
AGENT_API mp_int32 GetCurrentUserNameOther(mp_string& strUserName, mp_ulong& iErrCode)
{
    // 初始化buf的大小,通过系统函数进行获取
    std::size_t size = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (size == 0) {
        size = LINUX_USERNAME_LEN;
    }

    mp_char* pbuf = reinterpret_cast<char*>(malloc(size));
    if (pbuf == NULL) {
        iErrCode = static_cast<mp_ulong>(GetOSError());
        return MP_FAILED;
    }

    mp_int32 iRet = memset_s(pbuf, size, 0, size);
    if (iRet != EOK) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, iRet %d.", iRet);
        iErrCode = static_cast<mp_ulong>(GetOSError());
        free(pbuf);
        return MP_FAILED;
    }

    struct passwd pwd;
    struct passwd* result = NULL;
    mp_int32 error = getpwuid_r(getuid(), &pwd, pbuf, size, &result);
    if (error == MP_FAILED || result == NULL) {
        iErrCode = static_cast<mp_ulong>(GetOSError());
        free(pbuf);
        return MP_FAILED;
    }
    strUserName = pwd.pw_name;

    free(pbuf);
    return MP_SUCCESS;
}
#endif

/* ------------------------------------------------------------
Description  :获取用户名
Output       : strUserName -- 用户名 iErrCode    --   错误码
------------------------------------------------------------- */
mp_int32 GetCurrentUserName(mp_string& strUserName, mp_ulong& iErrCode)
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
mp_int32 GetCurrentUserNameW(mp_wstring& strUserName, mp_ulong& iErrCode)
{
    mp_ulong size = WINDOWS_USERNAME_LEN;
    mp_wchar pUsername[WINDOWS_USERNAME_LEN] = {0};
    if (!GetUserNameW(pUsername, &size)) {
        iErrCode = GetOSError();
        // 日志模块使用，无法记录日志
        return MP_FAILED;
    }
    strUserName = pUsername;
    iErrCode = 0;
    memset_s(pUsername, WINDOWS_USERNAME_LEN, 0, WINDOWS_USERNAME_LEN);
    return MP_SUCCESS;
}

mp_string GetSystemDiskChangedPathInWin(const mp_string& oriPath)
{
    if (SYSTEM_DISK_NAME == "") {
        mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_WIN_SYSTEM_DISK_VALUE,
        SYSTEM_DISK_NAME);
        if (iRet != MP_SUCCESS) {
            SYSTEM_DISK_NAME = "C";
            WARNLOG("Get system disk name from config failed, use default system disk name: C.");
        }
    }
    if (SYSTEM_DISK_NAME == "C" || SYSTEM_DISK_NAME == "c") {
        DBGLOG("The default system disk name is C, no need to change.");
        return oriPath;
    }
    mp_string changePath = SYSTEM_DISK_NAME + oriPath.substr(1, oriPath.length() - 1);
    DBGLOG("System disk name is %s, the oripath %s has been changed to %s.", SYSTEM_DISK_NAME.c_str(),
        oriPath.c_str(), changePath.c_str());
    return changePath;
}

const mp_wstring BaseFileNameW(const mp_wstring& pszFileName)
{
    std::size_t len = pszFileName.length();
    while (len > 0) {
        if (pszFileName[len] == L'\\') {
            return pszFileName.substr(len + 1);
        }

        len--;
    }

    return pszFileName;
}

#endif

const mp_string BaseFileName(const mp_string& pszFileName)
{
    std::size_t found = pszFileName.find_last_of("/\\");
    return (found == string::npos) ? "" : pszFileName.substr(found + 1);
}

/* ---------------------------------------------------------------------------
Function Name: RemoveFullPathForLog
Description  : 去掉命令中的全路径方便日志打印，
    如/usr/bin/ls filename,变成ls filename
------------------------------------------------------------ */
mp_void RemoveFullPathForLog(mp_string strCmd, mp_string& strLogCmd)
{
    vector<mp_string> vecCmdParams;
    CMpString::StrSplit(vecCmdParams, strCmd, ' ');

    if (vecCmdParams.size() == 0) {
        strLogCmd = strCmd;
    } else {
        strLogCmd = BaseFileName(vecCmdParams[0]);
        vector<mp_string>::iterator iter = vecCmdParams.begin();
        ++iter;
        for (; iter != vecCmdParams.end(); ++iter) {
            strLogCmd = strLogCmd + " " + *iter;
        }
    }
}

#ifndef WIN32
/* ---------------------------------------------------------------------------
Function Name: GetUidByUserName
Description  : 根据用户名称获取UID和GID
------------------------------------------------------------- */
mp_int32 GetUidByUserName(const mp_string& strUserName, mp_int32& uid, mp_int32& gid)
{
    struct passwd* user = NULL;
    uid = -1;
    gid = -1;
    CThreadAutoLock cLock(&UuidByUNameMutex::GetInstance().GetValue());
    user = getpwnam(strUserName.c_str());
    if (user != NULL) {
        uid = static_cast<mp_int32>(user->pw_uid);
        gid = static_cast<mp_int32>(user->pw_gid);
    }
    if (uid == -1) {
        COMMLOG(OS_LOG_ERROR, "Get uid of user(%s) failed.", strUserName.c_str());
        return MP_FAILED;
    } else {
        COMMLOG(OS_LOG_DEBUG, "User(%s) info: uid=%d, gid=%d.", strUserName.c_str(), uid, gid);
        return MP_SUCCESS;
    }
}

/* ---------------------------------------------------------------------------
Function Name: ChownFile
Description  : 设置文件的用户和组权限
------------------------------------------------------------- */
mp_int32 ChownFile(const mp_string& strFileName, const mp_int32& uid, mp_int32& gid)
{
    struct stat buf;
    mp_int32 iRet = lstat(strFileName.c_str(), &buf);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "lstat file(%s) failed, errno[%d]: %s.",
            BaseFileName(strFileName).c_str(),
            errno,
            strerror(errno));
        return MP_FAILED;
    }
    if (S_ISLNK(buf.st_mode)) {
        COMMLOG(OS_LOG_ERROR, "chown file(%s) failed, file is symbolic link.", BaseFileName(strFileName).c_str());
        return MP_FAILED;
    }
    iRet = chown(strFileName.c_str(), static_cast<mp_uint32>(uid), static_cast<mp_uint32>(gid));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "chown file(%s) failed, errno[%d]: %s.",
            BaseFileName(strFileName).c_str(),
            errno,
            strerror(errno));
        return MP_FAILED;
    }

    return iRet;
}

/* ---------------------------------------------------------------------------
Function Name: ChmodFile
Description  : 更改文件权限
------------------------------------------------------------- */
mp_int32 ChmodFile(const mp_string& strFileName, mp_uint32 mode)
{
    struct stat buf;
    mp_int32 iRet = lstat(strFileName.c_str(), &buf);
    if (iRet != MP_SUCCESS) {
        return MP_FAILED;
    }
    if (S_ISLNK(buf.st_mode)) {
        return MP_FAILED;
    }
    iRet = chmod(strFileName.c_str(), mode_t(mode));
    if (iRet != MP_SUCCESS) {
        return MP_FAILED;
    }

    return iRet;
}

/* ---------------------------------------------------------------------------
Function Name: GetFileOwnerName
Description  : 获取文件属主
------------------------------------------------------------- */
mp_int32 GetFileOwnerName(const mp_string& strFileName, mp_string& owner)
{
    struct stat buf;
    struct passwd *fileInfo;
    int iRet = stat(strFileName.c_str(), &buf);
    if (iRet != MP_SUCCESS) {
        return MP_FAILED;
    }
    if ((fileInfo = getpwuid(buf.st_uid)) != NULL) {
        owner = mp_string(fileInfo->pw_name);
        return MP_SUCCESS;
    }
    return MP_FAILED;
}
#endif

vector<mp_string> GrepE(const vector<mp_string>& vecInput, const mp_string& strCondition)
{
    vector<mp_string> vecOut;
    for (vector<mp_string>::const_iterator iter = vecInput.begin(); iter != vecInput.end(); ++iter) {
        if ((*iter).find(strCondition) != mp_string::npos) {
            vecOut.push_back(*iter);
            break;
        }
    }
    return vecOut;
}

vector<mp_string> GrepE(const vector<mp_string>& vecInput, const vector<mp_string>& vecCondition)
{
    vector<mp_string> vecOut;
    for (vector<mp_string>::const_iterator iter = vecInput.begin(); iter != vecInput.end(); ++iter) {
        for (vector<mp_string>::const_iterator it = vecCondition.begin(); it != vecCondition.end(); ++it) {
            if ((*iter).find(*it) != mp_string::npos) {
                vecOut.push_back(*iter);
                break;
            }
        }
    }
    return vecOut;
}

vector<mp_string> GrepV(const vector<mp_string>& vecInput, const mp_string& strCondition)
{
    vector<mp_string> vecOut = vecInput;
    size_t nSize = vecOut.size();
    for (int i = nSize - 1; i >= 0; --i) {
        if (vecOut.at(i).find(strCondition) != std::string::npos) {
            vecOut.erase(vecOut.begin() + i);
        }
    }
    return vecOut;
}

vector<mp_string> GrepV(const vector<mp_string>& vecInput, const vector<mp_string>& vecCondition)
{
    vector<mp_string> vecOut = vecInput;
    size_t nSize = vecOut.size();
    for (int i = nSize - 1; i >= 0; --i) {
        for (vector<mp_string>::const_iterator it = vecCondition.begin(); it != vecCondition.end(); ++it) {
            if (vecOut.at(i).find(*it) != mp_string::npos) {
                vecOut.erase(vecOut.begin() + i);
                break;
            }
        }
    }
    return vecOut;
}

vector<mp_string> GrepW(const vector<mp_string>& vecInput, const mp_string& strCondition)
{
    mp_string strDic = mp_string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    vector<mp_string> vecOut;
    for (vector<mp_string>::const_iterator iter = vecInput.begin(); iter != vecInput.end(); ++iter) {
        mp_size nPosBegin = (*iter).find(strCondition);
        if (nPosBegin == mp_string::npos) {
            continue;
        }
        mp_size nPosEnd = nPosBegin + strCondition.size() - 1;
        if (nPosBegin > 0 && strDic.find((*iter).at(nPosBegin - 1)) != string::npos) {
            continue;
        }
        if (nPosEnd + 1 < (*iter).size() && strDic.find((*iter).at(nPosEnd + 1)) != string::npos) {
            continue;
        }
        vecOut.push_back(*iter);
    }
    return vecOut;
}

vector<mp_string> Awk(const vector<mp_string>& vecInput, int nPrint, char cSep)
{
    vector<mp_string> vecOut;
    for (vector<mp_string>::const_iterator iter = vecInput.begin(); iter != vecInput.end(); ++iter) {
        vector<mp_string> vecTokens;
        CMpString::StrSplit(vecTokens, *iter, cSep);
        vector<mp_string> vecTmp;
        for (vector<mp_string>::const_iterator it = vecTokens.begin(); it != vecTokens.end(); ++it) {
            if (!(*it).empty()) {
                vecTmp.push_back(*it);
            }
        }
        if (vecTmp.empty()) {
            continue;
        }
        if (nPrint < 0 && (nPrint + vecTmp.size()) >= 0) {
            vecOut.push_back(vecTmp.at(vecTmp.size() + nPrint));
        } else if (nPrint == 0) {
            vecOut.push_back(*iter);
        } else if (nPrint > vecTmp.size() || (nPrint + vecTmp.size()) < 0) {
            vecOut.push_back("");
        } else {
            vecOut.push_back(vecTmp.at(nPrint - 1));
        }
    }
    return vecOut;
}

vector<mp_string> Awk(const vector<mp_string>& vecInput, int nPos, const mp_string& strCondition, int nPrint, char cSep)
{
    vector<mp_string> vecOut;
    for (vector<mp_string>::const_iterator iter = vecInput.begin(); iter != vecInput.end(); ++iter) {
        vector<mp_string> vecTokens;
        CMpString::StrSplit(vecTokens, *iter, cSep);
        vector<mp_string> vecTmp;
        for (vector<mp_string>::const_iterator it = vecTokens.begin(); it != vecTokens.end(); ++it) {
            if (!(*it).empty()) {
                vecTmp.push_back(*it);
            }
        }
        if (!(nPos > 0 && nPos <= vecTmp.size() && vecTmp.at(nPos - 1) == strCondition)) {
            continue;
        }
        if (nPrint < 0 && (nPrint + vecTmp.size()) >= 0) {
            vecOut.push_back(vecTmp.at(vecTmp.size() + nPrint));
        } else if (nPrint == 0) {
            vecOut.push_back(*iter);
        } else if (nPrint > vecTmp.size() || (nPrint + vecTmp.size()) < 0) {
            vecOut.push_back("");
        } else {
            vecOut.push_back(vecTmp.at(nPrint - 1));
        }
    }
    return vecOut;
}

#ifndef WIN32
mp_int32 SendIOControl(const mp_string& strPath, mp_int32 iCmd, mp_void* pData, mp_uint32 uiLen)
{
    (mp_void) uiLen;
#ifndef AGENT_TEST_WITHOUT_DRIVER
    mp_int32 iErr;
    mp_char szErr[ERR_INFO_SIZE] = {0};

    COMMLOG(OS_LOG_INFO, "Begin send ioctl, path %s, cmd %d.", strPath.c_str(), iCmd);
    mp_int32 fd = open(strPath.c_str(), O_RDWR);
    if (fd < 0) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Open dev %s failed, errno[%d]:%s.",
            strPath.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Invoke ioctl, cmd %d.", iCmd);
    mp_int32 iRet = ioctl(fd, iCmd, pData);
    if (iRet < MP_SUCCESS) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "ioctl failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        close(fd);
        return MP_FAILED;
    }
    close(fd);
    COMMLOG(OS_LOG_INFO, "Send ioctl succ.");
#endif
    return MP_SUCCESS;
}
#else
mp_int32 SendIOControl(const mp_string& strPath, DWORD dwCmd, mp_void* pData, mp_uint32 uiLen)
{
#ifndef AGENT_TEST_WITHOUT_DRIVER
    mp_int32 iErr;
    BOOL bRet;
    DWORD dwOutput = 0;
    mp_char szErr[ERR_INFO_SIZE] = {0};

    COMMLOG(OS_LOG_DEBUG, "Begin send ioctl, path %s, cmd %lu.", strPath.c_str(), dwCmd);
    HANDLE hDevice = CreateFile(
        strPath.c_str(), GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Open dev %s failed, errno[%d]: %s.",
            strPath.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, "Invoke create file succ.");

    bRet = DeviceIoControl(hDevice, dwCmd, pData, uiLen, NULL, 0, &dwOutput, NULL);
    if (bRet == FALSE) {
        iErr = GetOSError();
        CloseHandle(hDevice);
        COMMLOG(OS_LOG_ERROR,
            "DeviceIoControl failed, dev %s, errno[%d]: %s.",
            strPath.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    CloseHandle(hDevice);
    COMMLOG(OS_LOG_DEBUG, "Send ioctl succ.");

#endif
    return MP_SUCCESS;
}
#endif

// 以下临时使用固定函数实现，后续开源软件选型后采用开源软件优化
/* ---------------------------------------------------------------------------
Function Name: CheckParamString
Description  : 检查string类型的参数
Input        : paramValue -- 输入字符串
                 lenBeg        -- 字符串最小长度
                 lenEnd        -- 字符串最大长度
                 strInclude   -- 必须包含字符串
                 strExclude   --不能包含字符串
------------------------------------------------------------- */
mp_int32 CheckParamString(const mp_string& paramValue,
    mp_int32 lenBeg, mp_int32 lenEnd, const mp_string& strInclude, const mp_string& strExclude)
{
    // CodeDex误报，Dead Code
    if (lenBeg == -1 && lenEnd == -1) {
        COMMLOG(OS_LOG_INFO, "This string(%s) has no length restrictions.", paramValue.c_str());
    } else if (paramValue.length() < lenBeg || paramValue.length() > lenEnd) {  // check length
        COMMLOG(OS_LOG_ERROR, "The len of string(%s) is not between %d and %d.", paramValue.c_str(), lenBeg, lenEnd);
        return MP_FAILED;
    }

    // check include and exclude
    for (auto iter = paramValue.begin(); iter != paramValue.end(); ++iter) {
        // check exclude
        if (strExclude.find(*iter) != string::npos) {
            COMMLOG(OS_LOG_ERROR, "The string(%s) have exclude char %c.", paramValue.c_str(), *iter);
            return MP_FAILED;
        }

        // check include
        if (!strInclude.empty()) {
            if (strInclude.find(*iter) == string::npos) {
                COMMLOG(OS_LOG_ERROR, "Char %c is not in include string(%s).", *iter, paramValue.c_str());
                return MP_FAILED;
            }
        }
    }

    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CheckParamString
Description  : 检查string类型的参数
Input        : paramValue -- 输入字符串
                 lenBeg        -- 字符串最小长度
                 lenEnd        -- 字符串最大长度
                 strPre         -- 字符串前缀
------------------------------------------------------------- */
mp_int32 CheckParamString(const mp_string& paramValue, mp_int32 lenBeg, mp_int32 lenEnd, const mp_string& strPre)
{
    if (paramValue.length() < lenBeg || paramValue.length() > lenEnd) {
        COMMLOG(OS_LOG_ERROR, "The len of string(%s) is not between %d and %d.", paramValue.c_str(), lenBeg, lenEnd);
        return MP_FAILED;
    }

    std::size_t idxPre = paramValue.find_first_of(strPre);
    if (idxPre != 0) {
        COMMLOG(OS_LOG_ERROR, "String(%s) is not begin with %s.", paramValue.c_str(), strPre.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CheckParamString
Description  : 检查string类型的参数
Input        : paramValue -- 输入字符串
                 lenBeg        -- 字符串最小长度
                 lenEnd        -- 字符串最大长度
                 strPre         -- 字符串后缀
------------------------------------------------------------- */
mp_int32 CheckParamStringEnd(mp_string& paramValue, mp_int32 lenBeg, mp_int32 lenEnd, const mp_string& strEnd)
{
    if (paramValue.length() < lenBeg || paramValue.length() > lenEnd) {
        COMMLOG(OS_LOG_ERROR, "The len of string(%s) is not between %d and %d.", paramValue.c_str(), lenBeg, lenEnd);
        return MP_FAILED;
    }

    if (paramValue.length() < strEnd.length()) {
        COMMLOG(OS_LOG_ERROR, "strEnd length is longger than  paramValue length.");
        return MP_FAILED;
    }

    std::size_t idxEnd = paramValue.rfind(strEnd);
    if (idxEnd == std::string::npos) {
        COMMLOG(OS_LOG_ERROR, "String(%s) is not end with %s.", paramValue.c_str(), strEnd.c_str());
        return MP_FAILED;
    }
    if (idxEnd != (paramValue.length() - strEnd.length())) {
        COMMLOG(OS_LOG_ERROR, "String(%s) is not end with %s.", paramValue.c_str(), strEnd.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CheckParamInteger32
Description  : 检查int32类型的参数
Input        : paramValue       -- 输入数字
                 begValue            -- 数字最小值
                 endValue            -- 数字最大值
                 vecExclude         -- 数字不能包含值
------------------------------------------------------------- */
mp_int32 CheckParamInteger32(
    mp_int32 paramValue, mp_int32 begValue, mp_int32 endValue, const vector<mp_int32>& vecExclude)
{
    if (begValue != -1 && paramValue < begValue) {
        COMMLOG(OS_LOG_ERROR, "Check failed, %d < %d.", paramValue, begValue);
        return MP_FAILED;
    }

    if (endValue != -1 && paramValue > endValue) {
        COMMLOG(OS_LOG_ERROR, "Check failed, %d > %d.", paramValue, endValue);
        return MP_FAILED;
    }

    vector<mp_int32>::const_iterator iter = vecExclude.begin();
    for (; iter != vecExclude.end(); ++iter) {
        if (paramValue == *iter) {
            COMMLOG(OS_LOG_ERROR, "Check failed, %d = %d.", paramValue, *iter);
            return MP_FAILED;
        }
    }

    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CheckParamInteger64
Description  : 检查int64类型的参数
Input        : paramValue       -- 输入数字
                 begValue            -- 数字最小值
                 endValue            -- 数字最大值
                 vecExclude         -- 数字不能包含值
------------------------------------------------------------- */
mp_int32 CheckParamInteger64(mp_int64 paramValue, mp_int64 begValue, mp_int64 endValue, vector<mp_int64>& vecExclude)
{
    if (begValue != -1 && paramValue < begValue) {
        COMMLOG(OS_LOG_ERROR, "Check failed, %d < %d.", paramValue, begValue);
        return MP_FAILED;
    }

    if (endValue != -1 && paramValue > endValue) {
        COMMLOG(OS_LOG_ERROR, "Check failed, %d > %d.", paramValue, endValue);
        return MP_FAILED;
    }

    vector<mp_int64>::iterator iter = vecExclude.begin();
    for (; iter != vecExclude.end(); ++iter) {
        if (paramValue == *iter) {
            COMMLOG(OS_LOG_ERROR, "Check failed, %d = %d.", paramValue, *iter);
            return MP_FAILED;
        }
    }

    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CheckParamInteger
Description  : 检查参数是否是IP形式
Input        : paramValue       -- 输入IP字符串
------------------------------------------------------------- */
mp_int32 CheckParamStringIsIP(const mp_string& paramValue)
{
    if (paramValue.find(":", 0) != std::string::npos) {
        return CheckParamStringIsIPv6(paramValue);
    }

    mp_int32 iIpnum1;
    mp_int32 iIpnum2;
    mp_int32 iIpnum3;
    mp_int32 iIpnum4;
    mp_int32 iIpnum = sscanf_s(paramValue.c_str(), "%d.%d.%d.%d", &iIpnum1, &iIpnum2, &iIpnum3, &iIpnum4);

    mp_bool bFlag = (iIpnum == UTILS_NUM_4) && (iIpnum1 >= 1 && iIpnum1 <= UTILS_NUM_233) &&
                    (iIpnum2 >= 0 && iIpnum2 <= UTILS_NUM_255) && (iIpnum3 >= 0 && iIpnum3 <= UTILS_NUM_255) &&
                    (iIpnum4 >= 0 && iIpnum4 <= UTILS_NUM_255);
    if (bFlag) {
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR, "The string(%s) is not ipv4.", paramValue.c_str());
        return MP_FAILED;
    }
}

mp_int32 CheckParamStringIsIPv6(const mp_string& paramValue)
{
    mp_string strIpv6 = paramValue;
    mp_int32 num = count(strIpv6.begin(), strIpv6.end(), ':');
    if (num != UTILS_NUM_8 - 1) {
        mp_string strPadding = ":";
        strPadding.reserve(sizeof("0000:") * UTILS_NUM_8);
        for (mp_int32 i = num; i < UTILS_NUM_8; i++) {
            strPadding += "0000:";
        }
        mp_string::size_type index = strIpv6.find("::", 0);
        if (index != mp_string::npos) {
            strIpv6.replace(index, AWK_COL_FIRST_2, strPadding);
        }
    }
    return CheckIsIpv6(strIpv6, paramValue);
}

mp_int32 CheckIsIpv6(const mp_string& strIpv6, const mp_string& paramValue)
{
    mp_int32 len = strIpv6.length();
    mp_int32 count_colon = 0;  // 计数冒号
    mp_int32 count_bit = 0;    // 计数每组位数，初始化为 0
    for (mp_int32 i = 0; i < len; i++) {
        if (count_bit == 0 && strIpv6[i] == ':') {
            COMMLOG(OS_LOG_ERROR, "The string(%s) is not ipv6.", paramValue.c_str());
            return MP_FAILED;
        }
        char s = strIpv6[i];
        if (!((s >= '0' && s <= '9') || (s >= 'a' && s <= 'f') || (s >= 'A' && s <= 'F') || (s == ':'))) {
            COMMLOG(OS_LOG_ERROR, "The string(%s) is not ipv6.", paramValue.c_str());
            return MP_FAILED;
        }
        count_bit++;  // 如果字符合法，则位数加1
        if (strIpv6[i] == ':') {
            count_bit = 0;  // 位数清0，重新计数
            count_colon++;  // 冒号数加1
        }
        if (count_bit > UTILS_NUM_4) {
            COMMLOG(OS_LOG_ERROR, "The string(%s) is not ipv6.", paramValue.c_str());
            return MP_FAILED;
        }
    }
    if (count_colon != UTILS_NUM_8 - 1) {
        COMMLOG(OS_LOG_ERROR, "The string(%s) is not ipv6.", paramValue.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CheckPathString
Description  : 检查路径字符串是否大于最大长度
Input        : pathValue       -- 路径字符串
------------------------------------------------------------- */
mp_int32 CheckPathString(mp_string& pathValue)
{
#ifdef WIN32
    mp_int32 imaxpath = MAX_PATH;
#else
    mp_int32 imaxpath = PATH_MAX;
#endif
    if (pathValue.length() >= imaxpath) {
        COMMLOG(OS_LOG_ERROR, "The path string(%s) len is not less than %d.", pathValue.c_str(), imaxpath);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CheckPathString
Description  : 检查路径字符串是否大于最大长度
------------------------------------------------------------- */
mp_int32 CheckPathString(mp_string& pathValue, const mp_string& strPre)
{
#ifdef WIN32
    std::size_t imaxpath = MAX_PATH;
#else
    std::size_t imaxpath = PATH_MAX;
#endif
    if (pathValue.length() >= imaxpath) {
        COMMLOG(OS_LOG_ERROR, "The path string(%s) len is not less than %d.", pathValue.c_str(), imaxpath);
        return MP_FAILED;
    }

    std::size_t idxPre = pathValue.find(strPre);
    if (idxPre != 0) {
        COMMLOG(OS_LOG_ERROR, "String(%s) is not begin with %s.", pathValue.c_str(), strPre.c_str());
        return MP_FAILED;
    }

    mp_string strInclude("");
    mp_string strExclude("");
    std::size_t idxSep = pathValue.find_last_of("/");
    if (mp_string::npos == idxSep) {
        COMMLOG(OS_LOG_ERROR, "The string %s does not contain / character.", pathValue.c_str());
        return MP_FAILED;
    }
    mp_string strFileName = pathValue.substr(idxSep + 1);
    CHECK_FAIL_EX(CheckParamString(strFileName, 1, UTILS_NUM_254, strInclude, strExclude));
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CheckPathTraversal
Description  : 检查路径字符串是否存在路径穿越
------------------------------------------------------------- */
mp_int32 CheckPathTraversal(mp_string& pathValue)
{
#ifdef WIN32
    std::size_t imaxpath = MAX_PATH;
#else
    std::size_t imaxpath = PATH_MAX;
#endif
    if (pathValue.length() >= imaxpath) {
        COMMLOG(OS_LOG_ERROR, "The path string(%s) len is not less than %d.", pathValue.c_str(), imaxpath);
        return MP_FAILED;
    }

    if (pathValue.find("..") != mp_string::npos) {
        COMMLOG(OS_LOG_ERROR, "The path string(%s) contained illegal string(..).", pathValue.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CheckFileSysMountParam
Description  : 检查文件系统挂载参数
Input        : pathValue       -- 路径字符串
------------------------------------------------------------- */
mp_int32 CheckFileSysMountParam(const mp_string& strDeviceName, mp_int32 volumeType, const mp_string& strMountPoint)
{
#ifdef WIN32
    return CheckFileSysMountParamWin32(strDeviceName, volumeType, strMountPoint);
#else
    const mp_int32 IDX_INIT_VALUE_4 = 4;
    // 参数校验
    mp_string strFileName;
    mp_string strPre("/\\");
    mp_int32 lenBeg, lenEnd;
    std::size_t idxSep;
    vector<mp_int32> vecExclude;

    lenBeg = 1;
    lenEnd = PATH_MAX - 1;
    CHECK_FAIL_EX(CheckParamString(strDeviceName, lenBeg, lenEnd, strPre));
    idxSep = strDeviceName.find_last_of("/");
    if (mp_string::npos == idxSep) {
        COMMLOG(OS_LOG_ERROR, "The string(%s) does not contain / character.", strDeviceName.c_str());
        return MP_FAILED;
    }
    strFileName = strDeviceName.substr(idxSep + 1);
    mp_string strInclude("");
    mp_string strExclude("");
    CHECK_FAIL_EX(CheckParamString(strFileName, 1, UTILS_NUM_254, strInclude, strExclude));
    CHECK_FAIL_EX(CheckParamInteger32(volumeType, 0, IDX_INIT_VALUE_4, vecExclude));

    lenBeg = 1;
    lenEnd = PATH_MAX - 1;
    strPre = mp_string("/");
    CHECK_FAIL_EX(CheckParamString(strMountPoint, lenBeg, lenEnd, strPre));

    idxSep = strMountPoint.find_last_of("/");
    if (mp_string::npos == idxSep) {
        COMMLOG(OS_LOG_ERROR, "The string(%s) does not contain / character.", strMountPoint.c_str());
        return MP_FAILED;
    }
    strFileName = strMountPoint.substr(idxSep + 1);
    strInclude = mp_string("");
    strExclude = mp_string("");
    if (!strFileName.empty()) {
        CHECK_FAIL_EX(CheckParamString(strFileName, 1, UTILS_NUM_254, strInclude, strExclude));
    }
    return MP_SUCCESS;
#endif
}

#ifdef WIN32
/* ---------------------------------------------------------------------------
Function Name: CheckFileSysMountParamWin32
Description  : Windows下检查文件系统挂载参数
Input        : pathValue       -- 路径字符串
------------------------------------------------------------- */
mp_int32 CheckFileSysMountParamWin32(mp_string strDeviceName, mp_int32 volumeType, mp_string strMountPoint)
{
    const mp_int32 IDX_INIT_VALUE_49 = 49;
    const mp_int32 IDX_INIT_VALUE_4 = 4;
    mp_string strFileName;
    mp_string strPre("/\\");
    mp_int32 lenBeg;
    mp_int32 lenEnd;
    vector<mp_int32> vecExclude;
    lenBeg = lenEnd = IDX_INIT_VALUE_49;
    CHECK_FAIL_EX(CheckParamString(strDeviceName, lenBeg, lenEnd, strPre));
    CHECK_FAIL_EX(CheckParamInteger32(volumeType, 0, IDX_INIT_VALUE_4, vecExclude));
    lenBeg = 1;
    lenEnd = 1;
    mp_string strInclude("BCDEFGHIJKLMNOPQRSTUVWXYZ");
    mp_string strExclude;
    CHECK_FAIL_EX(CheckParamString(strMountPoint, lenBeg, lenEnd, strInclude, strExclude));
    return MP_SUCCESS;
}
#endif
/*---------------------------------------------------------------------------
Function Name: CheckFileSysFreezeParam
Description  : 检查文件系统冻结参数
Input        : pathValue       -- 路径字符串
-------------------------------------------------------------*/
mp_int32 CheckFileSysFreezeParam(const mp_string& strDiskNames)
{
    mp_int32 lenBeg;
    mp_int32 lenEnd;
#ifdef WIN32
    lenBeg = 1;
    lenEnd = 1;
    mp_string strInclude("BCDEFGHIJKLMNOPQRSTUVWXYZ");
    mp_string strExclude;
    CHECK_FAIL_EX(CheckParamString(strDiskNames, lenBeg, lenEnd, strInclude, strExclude));
#else
    lenBeg = 1;
    lenEnd = PATH_MAX - 1;
    mp_string strPre("/");
    CHECK_FAIL_EX(CheckParamString(strDiskNames, lenBeg, lenEnd, strPre));
#endif

#ifndef WIN32
    std::size_t idxSep = strDiskNames.find_last_of("/");
    if (mp_string::npos == idxSep) {
        COMMLOG(OS_LOG_ERROR, "The string(%s) does not contain / character.", strDiskNames.c_str());
        return MP_FAILED;
    }
    mp_string strFileName = strDiskNames.substr(idxSep + 1);
    mp_string strInclude("");
    mp_string strExclude("");
    CHECK_FAIL_EX(CheckParamString(strFileName, 1, UTILS_NUM_254, strInclude, strExclude));
#endif
    return MP_SUCCESS;
}

mp_bool IsHuweiStorage(mp_string arrayVendor)
{
    arrayVendor = CMpString::Trim(arrayVendor);
    if (strcmp(arrayVendor.c_str(), ARRAY_VENDER_HUAWEI.c_str()) != 0 &&
        strcmp(arrayVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI.c_str()) != 0 &&
        strcmp(arrayVendor.c_str(), ARRAY_VENDER_HUASY.c_str()) != 0) {
        return MP_FALSE;
    }
    return MP_TRUE;
}

#ifndef WIN32
mp_int32 ChangeGmonDir()
{
    mp_string strTmpPath = CPath::GetInstance().GetTmpPath();
    if (MP_SUCCESS != chdir(strTmpPath.c_str())) {
        COMMLOG(OS_LOG_ERROR, "change gmon output dir to tmp faild.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 GetProgressInfo(std::vector<mp_string> vecResult)
{
    std::ostringstream oss;
    oss << "ps -aef ";
    mp_string strCmd = oss.str();
    FILE* pStream = popen(strCmd.c_str(), "r");
    if (pStream == nullptr) {
        ERRLOG("Exec popen exec ps -aef failed.");
        return MP_FAILED;
    }

    mp_string tmpStr;
    while (!feof(pStream)) {
        char tmpBuf[1600] = { 0 };
        fgets(tmpBuf, sizeof(tmpBuf), pStream);
        if (strlen(tmpBuf) > 0) {
            tmpBuf[strlen(tmpBuf) - 1] = 0;  // 去掉获取出来的字符串末尾的'\n'
        }
        vecResult.push_back(tmpBuf);
    }
    pclose(pStream);
    return MP_SUCCESS;
}
#endif

mp_int32 WipeSensitiveForJsonData(const mp_string& rawBuffer, mp_string& strValue)
{
    if (rawBuffer == "") {
        strValue = "";
        return MP_FAILED;
    }
    Json::Value jsValue;
    if (CJsonUtils::ConvertStringtoJson(rawBuffer, jsValue) != MP_SUCCESS) {
        strValue = "";
        return MP_FAILED;
    }
    std::set<mp_string> excludedKey;
    excludedKey.insert("DirtyRange");

    WipeJsonStringValue(jsValue, excludedKey);
    Json::FastWriter fastWrite;
    strValue = fastWrite.write(jsValue);
    return MP_SUCCESS;
}

// 搜索json得到指定key的值,暂只支持stringvalue
mp_void WipeJsonStringValue(Json::Value& v, const std::set<mp_string>& excludedKey)
{
    if (v.type() == Json::objectValue) {
        Json::Value::Members mem = v.getMemberNames();
        Json::Value::Members::iterator it;
        for (it = mem.begin(); it != mem.end(); ++it) {
            std::string key = *it;
            if (excludedKey.find(key) != excludedKey.end()) {
                continue;
            }
            if (v[key].type() == Json::stringValue) {
                if (Sensitive::ISensitive(key, v[key].asString()) == MP_TRUE) {
                    v[key] = "******";
                }
                continue;
            }
            WipeJsonStringValue(v[key], excludedKey);
        }
        return;
    }

    if (v.type() == Json::arrayValue) {
        mp_int32 size = v.size();
        for (mp_int32 i = 0; i < size; ++i) {
            WipeJsonStringValue(v[i], excludedKey);
        }
        return;
    }
    return;
}

/*------------------------------------------------------------
Description  : 将字符串写0覆盖3次，用于清理内存密码
Create By    : hwx886037
-------------------------------------------------------------*/
void ClearString(mp_string& strValue)
{
    if (strValue.empty()) {
        return;
    }
    memset_s(&strValue[0], strValue.length(), 0, strValue.length());
}

bool GetNginxListenIP(mp_string& strIP, mp_int32& nPort)
{
    std::string strNginxConfig = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    std::ifstream stream;
    stream.open(strNginxConfig.c_str(), std::ifstream::in);
    mp_string line;
    std::vector<mp_string> substr;
    if (stream.is_open() == false) {
        COMMLOG(OS_LOG_ERROR, "Open Nginx config file failed, filename is:%s", strNginxConfig.c_str());
        return false;
    }
    while (getline(stream, line)) {
        if (line.find("listen") == std::string::npos) {
            continue;
        }
        line.erase(line.find_last_not_of(" ") + 1);
        CMpString::StrSplit(substr, line, ' ');
        DBGLOG("Get Nginx Listen ip is %s", line.c_str());
        break;
    }
    stream.close();
    static int listenIPminiSize = 3;
    if (substr.size() < listenIPminiSize) {
        ERRLOG("Get Nginx Listen ip Fail  size is %d.", substr.size());
        return false;
    }
    int size = substr.size();
    static int listenIPvectSize = 2;
    mp_string temp = substr[size - listenIPvectSize];
    if (temp.empty()) {
        ERRLOG("Get Nginx Listen ip is empty.");
        return false;
    }
    substr.clear();
    DBGLOG("Get Nginx Listen ip[%s]", temp.c_str());
    size_t pos = temp.find_last_of(":");
    if (pos == std::string::npos) {
        ERRLOG("Get Nginx Listen ip is empty.");
        return false;
    }
    strIP = temp.substr(0, pos);
    nPort = atoi(temp.substr(pos + 1).c_str());
    DBGLOG("Get Nginx Listen ip[%s], port[%d].", strIP.c_str(), nPort);
    return true;
}

mp_bool CheckParamValid(mp_string& param)
{
    std::size_t idx;

    idx = param.find(" ", 0);
    if (mp_string::npos != idx) {
        return ERROR_COMMON_INVALID_PARAM;
    }
    idx = param.find("'", 0);
    if (mp_string::npos != idx) {
        return ERROR_COMMON_INVALID_PARAM;
    }
    idx = param.find("\"", 0);
    if (mp_string::npos != idx) {
        return ERROR_COMMON_INVALID_PARAM;
    }
    idx = param.find(STR_CODE_WARP, 0);
    if (mp_string::npos != idx) {
        return ERROR_COMMON_INVALID_PARAM;
    }
    return MP_SUCCESS;
}

mp_string CheckParamInvalidReplace(mp_string& param)
{
    static const int CharNum = 2;
    std::size_t idx;
    mp_string replacestr = param;
    idx = replacestr.find("\n", 0);
    while (idx != mp_string::npos) {
        replacestr.replace(idx, 1, " ");
        idx = idx + CharNum;
        idx = replacestr.find("\n", idx);
    }
    idx = replacestr.find("\t", 0);
    while (idx != mp_string::npos) {
        replacestr.replace(idx, 1, " ");
        idx = idx + CharNum;
        idx = replacestr.find("\t", idx);
    }
    return replacestr;
}

mp_int32 CheckEscapeCht(const mp_string& param)
{
    if (param.empty()) {
        return ERROR_COMMON_INVALID_PARAM;
    } else if (param.find_first_of("\a\b\f\r\n\t\v") != mp_string::npos) {
        return ERROR_COMMON_INVALID_PARAM;
    }

    return MP_SUCCESS;
}

mp_int32 CheckIpAddressValid(mp_string& ipAddress)
{
    if (!CIP::IsIPV4(ipAddress) && !CIP::IsIPv6(ipAddress)) {
        ERRLOG("IP address (%s) is invalid.", ipAddress.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

// 简单校验通用字符串， 长度范围为0~512
mp_int32 CalibrationFormatString(mp_string& params)
{
    if (params.size() > UTILS_NUM_FOTMAT_STRING_MAX_LEN) {
        COMMLOG(OS_LOG_ERROR, "Common string: \"%s\", Input Format Error", params.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

// 简单校验taskId, 长度不超过60
mp_int32 CalibrationFormatTaskId(mp_string& params)
{
    if (params.size() > UTILS_NUM_TASKID_MAX_LEN) {
        COMMLOG(OS_LOG_ERROR, "TaskId: \"%s\", Input Format Error", params.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

// 简单校验StorProtocol, 枚举值{1,2}
mp_int32 CalibrationFormatStorProtocol(mp_uint64 params)
{
    if (params != UTILS_NUM_STORPROTOCOL_ISCSI && params != UTILS_NUM_STORPROTOCOL_NAS) {
        COMMLOG(OS_LOG_ERROR, "Protocol: \"%d\", Input Format Error", params);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 ModifyLineData(const mp_string& fileName, const mp_string& key, const mp_string& value)
{
    LOGGUARD("");
    mp_string tmpKey = key + "=";
    std::ifstream in;
    in.open(fileName);
    if (!in.is_open()) {
        ERRLOG("Failed to open read the file: %s.", fileName.c_str());
        return MP_FAILED;
    }
    mp_string tmpLine;
    mp_string tmpData;
    mp_bool findRet = MP_FALSE;
    while (getline(in, tmpLine)) {
        if (tmpLine.find(tmpKey) == 0) {
            tmpLine = tmpKey + value;
            findRet = MP_TRUE;
        }
        tmpData += tmpLine;
        tmpData += STR_CODE_WARP;
    }
    in.close();
    if (findRet) {
        std::ofstream out;
        out.open(fileName);
        if (!out.is_open()) {
            ERRLOG("Failed to open write the file: %s.", fileName.c_str());
            return MP_FAILED;
        }
        out << tmpData;
        out.flush();
        out.close();
    }
    INFOLOG("The file: %s is modified key: %s successfully.", fileName.c_str(), key.c_str());
    return MP_SUCCESS;
}