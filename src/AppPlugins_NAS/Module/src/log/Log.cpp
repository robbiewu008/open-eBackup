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
#include "Log.h"

#include <memory>
#include <fstream>
#include <thread>
#include "securec.h"
#include <chrono>
#ifndef WIN32
#include <csignal>
#include <sys/wait.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#else
#include <tchar.h>
#include <atlstr.h>
#include <direct.h>
#include <io.h>
#endif

using namespace std;
using namespace Module;
using namespace chrono;

thread_local size_t g_threadLocalRequestID;

#ifdef WIN32
#define S_ISDIR(m) ((m)&_S_IFDIR)
#endif

namespace {
const int LOG_NUM_1024 = 1024;
const int VM_UUDI_FILELEN = 500;
const int LINUX_USERNAME_LEN = 1024;

const int LOG_NUM_3600 = 3600;
const int LOG_NUM_1900 = 1900;

const int LOG_NUM_1000 = 1000;
const unsigned char LOG_NUM_2 = 2;
const unsigned char LOG_NUM_4 = 4;
const unsigned char LOG_NUM_6 = 6;
const string FUNCTION_LINE_SEPARARTOR = ":";
const int MB_TO_BYTE = 1048576;
const int DAY_TO_SECOND = 86400;
const int CMD_TIME_OUT = 110;
#ifdef WIN32
const int WINDOWS_USERNAME_LEN = 512;
const int SECOND_AND_MICROSECOND_TIMES = 1000000;
const string RELATIVE_PATH_7Z = R"(\DataBackup\ProtectClient\ProtectClient-E\bin\7z.exe)";
#endif
const string ENV_KEY_DATA_BACKUP_AGENT_HOME = "DATA_BACKUP_AGENT_HOME";
const string IN_AGENT_LOG_LEVAL_FILE = "/opt/common-conf/loglevel";
const string IN_AGENT_TYPE = "1";
const string ENVIRONMENT_TYPE = "ENVIRONMENT_TYPE";
const string BACKUP_SCENE = "BACKUP_SCENE";
const string DORADO_ENVIRONMENT = "0";


// 命令行输入检查
int CheckCmdDelimiter(string& str)
{
    if (string::npos != str.find(STR_SEMICOLON, 0) || string::npos != str.find(STR_VERTICAL_LINE, 0) ||
        string::npos != str.find(STR_ADDRESS, 0) || string::npos != str.find(SIGN_IN, 0) ||
        string::npos != str.find(SIGN_OUT, 0) || string::npos != str.find(SIGN_BACKQUOTE, 0) ||
        string::npos != str.find(SIGN_EXCLAMATION, 0) ||
        string::npos != str.find(STR_CODE_WARP, 0) || string::npos != str.find(SIGN_DOLLAR, 0)) {
        return FAILED;
    }
#ifdef WIN32
    if (string::npos != str.find(CHAR_SLASH, 0)) {
        return FAILED;
    }
#else
    if (string::npos != str.find(STR_BACKSLASH, 0)) {
        return FAILED;
    }
#endif
    return SUCCESS;
}

void Getfilepath(const char* path, const char* filename, char* filepath, int strfileLen)
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

/* ------------------------------------------------------------
Function Name: FileExist
Description  : 判断指定路径文件是否存在

Others       :-------------------------------------------------------- */
bool FileExist(const string& pszFilePath)
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

#ifdef WIN32
bool DeleteFile(const char* path, int strfileLen)
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
        } else {
            remove(strAbsolutePath.c_str());
        }
    } while (_findnext(dirHandle, &findData) == 0);
    _findclose(dirHandle);
    _rmdir(path);
    return true;
}
#else
bool DeleteFile(const char* path, int strfileLen)
{
    DIR *dir = NULL;
    struct dirent *dirinfo = NULL;
    struct stat statbuf;
    char filepath[VM_UUDI_FILELEN] = { 0 };
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

/* ------------------------------------------------------------
Function Name: FileSize
Description  : 获取文件大小，单位是字节

Others       :-------------------------------------------------------- */
int FileSize(const char* pszFilePath, uint32_t& uiSize)
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

/* ------------------------------------------------------------
Function Name: GetlLastModifyTime
Description  : 获取文件上一次更新时间

Others       :-------------------------------------------------------- */
int GetlLastModifyTime(const char* pszFilePath, time_t& tLastModifyTime)
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

// 注意：此接口会被日志接口调用，这里面不能再调用LOG接口写日志，否则可能导致死锁
int DelFile(const string& pszFilePath)
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

/* ------------------------------------------------------------
Description  :获取系统错误码
------------------------------------------------------------- */
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

/* ------------------------------------------------------------
Description  :获取用户名
Output       : strUserName -- 用户名 iErrCode    --   错误码
------------------------------------------------------------- */
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

#ifndef WIN32
/* ---------------------------------------------------------------------------
Function Name: ChmodFile
Description  : 更改文件权限
------------------------------------------------------------- */
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

const string BaseFileName(const string& pszFileName)
{
    size_t found = pszFileName.find_last_of("/\\");
    return (found == string::npos) ? "" : pszFileName.substr(found + 1);
}

string StrReplace(const string& str, const string& to_replaced, const string& newchars)
{
    string strRet = str;
    for (string::size_type pos = 0; pos != string::npos; pos += newchars.length()) {
        pos = strRet.find(to_replaced, pos);
        if (pos != string::npos) {
            strRet.replace(pos, to_replaced.length(), newchars);
        } else {
            break;
        }
    }
    return strRet;
}

/* ------------------------------------------------------------
Description  :将时间秒数转化为本地时间
------------------------------------------------------------- */
tm* LocalTimeR(time_t& pTime, tm& pTm)
{
#ifdef WIN32
    localtime_s(&pTm, &pTime);
#else
    localtime_r(&pTime, &pTm);
#endif

    return &pTm;
}
/* ------------------------------------------------------------
Description  :精确 获取当前时间
------------------------------------------------------------- */
void GetTimeOfDay(timeval& tp)
{
#ifdef WIN32
    LARGE_INTEGER liQPF;  // CPU Frequency
    LARGE_INTEGER lPerformanceCount;

    QueryPerformanceFrequency(&liQPF);  // get cpu Frequency
    // retrieves the current value of the high-resolution performance counter
    QueryPerformanceCounter(&lPerformanceCount);

    // calc time (sec + usec)
    tp.tv_sec = (uint32_t)(lPerformanceCount.QuadPart / liQPF.QuadPart);
    tp.tv_usec = (uint32_t)(
        ((lPerformanceCount.QuadPart - liQPF.QuadPart * tp.tv_sec) * SECOND_AND_MICROSECOND_TIMES) / liQPF.QuadPart);

    return;
#else
    (void) gettimeofday(&tp, NULL);
    return;
#endif
}

}  // namespace

namespace Module {
std::mutex CLogger::m_mutex;
CLogger CLogger::m_instance;
CLogger& CLogger::GetInstance()
{
    return m_instance;
}

CLogger::CLogger()
{
    m_iMaxSize = DEFAULT_LOG_SIZE;
    // 设置日志默认级别和个数，日志保存天数
    m_iLogLevel = OS_LOG_INFO;
    m_iLogCount = DEFAULT_LOG_COUNT;
    m_iLogKeepTime = DEFAULT_LOG_KEEP_TIME;
    m_cacheThreshold = (uint64_t)LOG_CACHE_MAXSIZE * LOG_NUM_1024 * LOG_NUM_1024;
    m_cacheFlg = LOG_CACHE_OFF;
    // 设置打开日志次数
    m_OpenCacheNum = 0;
    m_cacheSize = 0;
    m_funcWriteLog = NULL;
}

CLogger::~CLogger()
{
    /*lint -e1551*/
    while (!m_cacheContent.empty()) {
        /*lint -e1551*/
        m_cacheContent.pop();
    }
}

void CLogger::Init(const char* pcLogFileName, std::string strFilePath)
{
    m_strFilePath = strFilePath;
    m_strFileName = pcLogFileName;
}

void CLogger::Init(const char* pcLogFileName, std::string strFilePath, int iLogLevel, int iLogCount, int iLogMaxSize)
{
    m_strFilePath = strFilePath;
    m_strFileName = pcLogFileName;
    m_iLogLevel = iLogLevel;
    m_iLogCount = iLogCount;
    m_iMaxSize = iLogMaxSize * MB_TO_BYTE;
}

/* ------------------------------------------------------------
Description  :设置log级别
Input        :      iLogLevel---log级别
Return       :    SUCCESS---设置成功
                   FAILED---设置失败
------------------------------------------------------------- */
int CLogger::SetLogLevel(int iLogLevel)
{
    if (iLogLevel < OS_LOG_DEBUG || iLogLevel > OS_LOG_CRI) {
        return FAILED;
    }

    m_iLogLevel = iLogLevel;

    return SUCCESS;
}
/* ------------------------------------------------------------
Description  :设置log计数
Input        :      iLogCount---log计数
Return       :    SUCCESS---设置成功
                   FAILED---设置失败
------------------------------------------------------------- */
int CLogger::SetLogCount(int iLogCount)
{
    if (iLogCount <= 0) {
        return FAILED;
    }

    m_iLogCount = iLogCount;

    return SUCCESS;
}

/* ------------------------------------------------------------
Description  :打开日志缓存
Return       :    SUCCESS---设置成功
                   FAILED---设置失败
------------------------------------------------------------- */
void CLogger::OpenLogCache()
{
    lock_guard<std::mutex> lck(CLogger::m_mutex);

    m_cacheFlg = LOG_CACHE_ON;
    ++m_OpenCacheNum;
}

/* ------------------------------------------------------------
Description  :关闭日志缓存
Return       :    SUCCESS---设置成功
                   FAILED---设置失败
------------------------------------------------------------- */
int CLogger::CloseLogCache()
{
    lock_guard<std::mutex> lck(CLogger::m_mutex);

    // 适配多次打开日志缓存的情况，
    // 必须需要等到全部关闭，才去关闭缓存
    --m_OpenCacheNum;
    if (m_OpenCacheNum > 0) {
        return SUCCESS;
    }

    m_cacheFlg = LOG_CACHE_OFF;

    // 写入日志缓存到日志文件中
    FILE* pFile = GetLogFile();
    if (pFile == NULL) {
        return FAILED;
    }

    while (!m_cacheContent.empty()) {
        if (m_cacheContent.front().codeType == LOG_CACHE_ASCII) {
            fprintf(pFile, "%s", m_cacheContent.front().logCache.c_str());
        } else {
            // CodeDex误报,KLOCWORK.SV.FMT_STR.PRINT_FORMAT_MISMATCH.UNDESIRED
            /*lint -e559*/
            fwprintf(pFile, L"%s", m_cacheContent.front().logCacheW.c_str());
        }
        m_cacheContent.pop();
    }
    m_cacheSize = 0;

    fflush(pFile);
    fclose(pFile);

    return SUCCESS;
}

// 创建日志信息的"标题头"
int CLogger::MkHead(int iLevel, char pszHeadBuf[], int iBufLen)
{
    int iRet = SUCCESS;

    switch (iLevel) {
        case OS_LOG_DEBUG:
            iRet = snprintf_s(pszHeadBuf, iBufLen, iBufLen - 1, "%s", "DBG");
            break;
        case OS_LOG_INFO:
            iRet = snprintf_s(pszHeadBuf, iBufLen, iBufLen - 1, "%s", "INFO");
            break;
        case OS_LOG_WARN:
            iRet = snprintf_s(pszHeadBuf, iBufLen, iBufLen - 1, "%s", "WARN");
            break;
        case OS_LOG_ERROR:
            iRet = snprintf_s(pszHeadBuf, iBufLen, iBufLen - 1, "%s", "ERR");
            break;
        case OS_LOG_CRI:
            iRet = snprintf_s(pszHeadBuf, iBufLen, iBufLen - 1, "%s", "CRI");
            break;
        default:
            iRet = snprintf_s(pszHeadBuf, iBufLen, iBufLen - 1, "%s", "");
            break;
    }

    if (iRet == FAILED) {
        return FAILED;
    }
    return SUCCESS;
}

int CLogger::ZipLogFile(string strCommand, string strLogFile)
{
#ifndef WIN32
    (void) strLogFile;
    if (CheckCmdDelimiter(strCommand) != SUCCESS) {
        return FAILED;
    }

    // Coverity&Fortify误报:FORTIFY.Command_Injection
    // 已经按修复建议增加命令分隔符判断CheckCmdDelimiter
    int iRet = system(strCommand.c_str());
    if (!WIFEXITED(iRet)) {
        // system异常返回
        return FAILED;
    }
#else  // windows
    uint32_t uiRetCode = 0;
    if (CheckCmdDelimiter(strCommand) != SUCCESS) {
        return FAILED;
    }

    int iRet = ExecWinCmd(strCommand, uiRetCode);
    /*
     * 7z状态码
     * 0：成功
     * 1：警告(非致命错误)
     * 2：失败(致命错误)
    */
    if (iRet != SUCCESS || uiRetCode > 1) {
        return FAILED;
    }

    (void)remove(strLogFile.c_str());
#endif

    return SUCCESS;
}

/* ------------------------------------------------------------
Description  :选择log文件
Input        :      pszLogPath---log存放路径，pszLogName---log名，iLogCount---log计数行
------------------------------------------------------------- */
int CLogger::SwitchLogFile(
        const string& pszLogPath, const string& pszLogName, int iLogCount, time_t LogKeepTime)
{
    // 重命名当前日志文件
    string currLogFile = pszLogPath + PATH_SEPARATOR + pszLogName;
    std::string timeStamp = to_string(static_cast<uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count()));
    string tmpLogFile = pszLogPath + PATH_SEPARATOR + TMP + timeStamp + "_" + pszLogName;
    (void)rename(currLogFile.c_str(), tmpLogFile.c_str());
    // 生成当前日志文件
    if (CreateLogFile(currLogFile) != SUCCESS) {
        return FAILED;
    }

    std::thread th { &CLogger::AsyncSwitchLogFile, this, currLogFile, tmpLogFile, iLogCount, LogKeepTime };
    th.detach();
    return SUCCESS;
}

int CLogger::AsyncSwitchLogFile(const string& logFileName, const string& tmpLogFile, int iLogCount, time_t LogKeepTime)
{
    string zipSuffix ;
#ifndef WIN32
    zipSuffix  = ".gz";
#else
    zipSuffix  = ".zip";
#endif
    // 处理转储的日志文件
    for (int LogNum = iLogCount; LogNum > 0; --LogNum) {
        string oldLogZipName = logFileName + DOT + to_string(LogNum) + zipSuffix ;
        string newLogZipName = logFileName + DOT + to_string(LogNum + 1) + zipSuffix ;
        if (HandleEachLogFile(oldLogZipName, newLogZipName, LogNum, iLogCount, LogKeepTime) != SUCCESS) {
            return FAILED;
        }
    }

    // 转储当前日志文件并重命名
    string finalZipLogFile = logFileName + DOT + "1" + zipSuffix;
    return HandleCurrentLogFile(tmpLogFile, finalZipLogFile);
}

int CLogger::HandleEachLogFile(
    const string &OldLogNameStr, const string &NewLogNameStr, int LogNum,
    int MaxLogCount, time_t MaxKeepTime)
{
    const char *OldLogName = OldLogNameStr.c_str();
    const char *NewLogName = NewLogNameStr.c_str();

    // 日志不存在无需处理
    if (!FileExist(OldLogName)) {
        return SUCCESS;
    }

    // 达到最大日志数要删除
    if (LogNum >= MaxLogCount) {
        DelFile(OldLogName);
        return SUCCESS;
    }

    // 日志过期要删除
    time_t LastModifyTime;
    time_t NowTime;
    time_t KeepTime;

    time(&NowTime);
    int iRet = GetlLastModifyTime(OldLogName, LastModifyTime);
    if (iRet != SUCCESS) {
        return iRet;
    }
    KeepTime = NowTime - LastModifyTime;
    if (KeepTime > MaxKeepTime) {
        DelFile(OldLogName);
        return SUCCESS;
    }

    // 变更日志文件名
    (void)rename(OldLogName, NewLogName);
#ifndef WIN32
    (void)ChmodFile(NewLogNameStr, S_IRUSR);
#endif

    return SUCCESS;
}

int CLogger::HandleCurrentLogFile(const string &currLogFile, const string &finalZipLogFile)
{
    // 压缩当前日志文件
    string zipSuffix;
    string currZipLogName;
    string zipCmd;

#ifndef WIN32
    zipSuffix = ".gz";
    currZipLogName = currLogFile + zipSuffix;
    zipCmd = string("gzip -f -q -9 \"") + currLogFile + "\"";
#else
    zipSuffix = ".zip";
    currZipLogName = currLogFile + zipSuffix;
    // 通过环境变量来获取安装路径计算7z的路径
    std::string agentHomeStr { "C:" };
    char *agentHome = std::getenv(ENV_KEY_DATA_BACKUP_AGENT_HOME.c_str());
    if (agentHome != nullptr) {
        agentHomeStr = std::string(agentHome);
    }
    zipCmd = string("\"") + agentHomeStr + RELATIVE_PATH_7Z + "\" a -y -tzip \"" +
        currZipLogName + "\" \"" + currLogFile + "\" -mx=9";
#endif

    if (ZipLogFile(zipCmd, currLogFile) != SUCCESS) {
        return FAILED;
    }

    // 重命名日志压缩文件
    (void)rename(currZipLogName.c_str(), finalZipLogFile.c_str());
#ifndef WIN32
    (void)ChmodFile(finalZipLogFile, S_IRUSR);
#endif
    return SUCCESS;
}

/* ------------------------------------------------------------
Description  :创建log文件
Input        :      pszLogFile---log文件名
Return       :    SUCCESS---创建成功
                   FAILED---创建失败
------------------------------------------------------------- */
int CLogger::CreateLogFile(const string& pszLogFile)
{
#ifdef WIN32
    FILE* pFile = NULL;
    string pszLogFileName;
    int itmp;
    // CodeDex误报，Race Condition:File System Access
    pFile = fopen(pszLogFile.c_str(), "w");
    if (pFile == NULL) {
        return FAILED;
    }
    fclose(pFile);

    itmp = pszLogFile.find_last_of(PATH_SEPARATOR);
    if (string::npos == itmp) {
        return FAILED;
    }
    pszLogFileName = pszLogFile.substr(itmp + 1);

    string strCommand = "cmd.exe /c echo Y | cacls.exe \"" + string(pszLogFile) + "\" /E /R Users > NUL";
    uint32_t uiRetCode = 0;

    int iRtn = ExecWinCmd(strCommand, uiRetCode);
    if ((iRtn != SUCCESS) || (uiRetCode != 0)) {
        return FAILED;
    }
#else
    int fd = open(pszLogFile.c_str(), O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        return FAILED;
    }
    close(fd);
    // 设置权限
    (void)ChmodFile(pszLogFile, S_IRUSR | S_IWUSR);
#endif

    return SUCCESS;
}

/* ------------------------------------------------------------
Description  :打开log文件
Return       :     strMsg---字符流
                    NULL---打开失败
------------------------------------------------------------- */
void CLogger::WriteLog2Cache(ostringstream& strMsg)
{
    // 检查当前缓存是否超过阈值，有日志线程锁，此处为单线程操作
    // 超过阈值则删除最旧的日志直到满足阈值
    m_cacheSize += strMsg.str().length();
    while (m_cacheSize > m_cacheThreshold) {
        m_cacheSize -= m_cacheContent.front().cacheLen;
        if (!m_cacheContent.empty()) {
            m_cacheContent.pop();
        }
    }

    CacheData LogCache;
    LogCache.codeType = LOG_CACHE_ASCII;
    LogCache.logCache = strMsg.str();
    LogCache.cacheLen = strMsg.str().length();
    m_cacheContent.push(LogCache);
}

#ifdef WIN32
/* ------------------------------------------------------------
Description  :打开log文件
Return       :     strWMsg---长字节字符流
                    NULL---打开失败
------------------------------------------------------------- */
void CLogger::WriteLog2Cache(wostringstream& strWMsg)
{
    // 检查当前缓存是否超过阈值，有日志线程锁，此处为单线程操作
    // 超过阈值则删除最旧的日志直到满足阈值
    m_cacheSize += strWMsg.str().length();
    while (m_cacheSize > m_cacheThreshold) {
        m_cacheSize -= m_cacheContent.front().cacheLen;
        if (!m_cacheContent.empty()) {
            m_cacheContent.pop();
        }
    }

    CacheData LogCacheW;
    LogCacheW.codeType = LOG_CACHE_UNICODE;
    LogCacheW.logCacheW = strWMsg.str();
    LogCacheW.cacheLen = strWMsg.str().length();
    m_cacheContent.push(LogCacheW);
}

/* ------------------------------------------------------------
Description  :打开log文件
Return       :     pFile---文件描述符
                    NULL---打开失败
------------------------------------------------------------- */
FILE* CLogger::GetLogFile()
{
    // codedex误报CANONICAL_FILEPATH，日志路径不存在这个问题
    string strLogFilePath;
    uint32_t uiFileSize = 0;
    FILE* pFile = NULL;
    int iRet = 0;

    strLogFilePath = m_strFilePath + PATH_SEPARATOR + m_strFileName;
    if (!FileExist(strLogFilePath)) {
        iRet = CreateLogFile(strLogFilePath.c_str());
        if (iRet == FAILED) {
            return NULL;
        }
    } else {
        if (FileSize(strLogFilePath.c_str(), uiFileSize) != SUCCESS) {
            return NULL;
        }

        if (uiFileSize >= (uint32_t)m_iMaxSize) {
            iRet = SwitchLogFile(m_strFilePath, m_strFileName, m_iLogCount, m_iLogKeepTime);
            if (iRet == FAILED) {
                return NULL;
            }
        }
    }

    pFile = fopen(strLogFilePath.c_str(), "a+");
    if (pFile == NULL) {
        return NULL;
    }

    return pFile;
}
#else
/* ------------------------------------------------------------
Description  :打开log文件
Return       :     pFile---文件描述符
                    NULL---打开失败
------------------------------------------------------------- */
FILE* CLogger::GetLogFile()
{
    // codedex误报CANONICAL_FILEPATH，日志路径不存在这个问题
    string strLogFilePath;
    FILE* pFile = NULL;
    int fd;
    struct stat st;
    int iRet = 0;

    strLogFilePath = m_strFilePath + PATH_SEPARATOR + m_strFileName;
    fd = open(strLogFilePath.c_str(), O_RDONLY);
    if (-1 == fd) {
        iRet = CreateLogFile(strLogFilePath.c_str());
        if (iRet == FAILED) {
            return NULL;
        }
    } else {
        (void)fstat(fd, &st);
        close(fd);
        // 日志超过大小切换日志
        if (st.st_size >= m_iMaxSize) {
            iRet = SwitchLogFile(m_strFilePath, m_strFileName, m_iLogCount, m_iLogKeepTime);
            if (iRet == FAILED) {
                return NULL;
            }
        }
    }

    pFile = fopen(strLogFilePath.c_str(), "a+");
    if (pFile == NULL) {
        return NULL;
    }

    return pFile;
}

#endif

#ifdef WIN32
/* ------------------------------------------------------------
Description  :  执行Windows命令，仅提供日志模块使用，不主动检查命令
Input        :      pszCmdBuf---命令缓存
Output       :     uiRetCode---返回码
Return       :     SUCCESS---执行成功
                    FAILED---执行失败
------------------------------------------------------------- */
int CLogger::ExecWinCmd(string pszCmdBuf, uint32_t& uiRetCode)
{
    list<string> lstContents;
    int iRet = 0;
    string strCmd = pszCmdBuf;
    STARTUPINFO stStartupInfo;
    PROCESS_INFORMATION stProcessInfo;
    DWORD dwCode = 0;

    ZeroMemory(&stStartupInfo, sizeof(stStartupInfo));
    stStartupInfo.cb = sizeof(stStartupInfo);
    stStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    stStartupInfo.wShowWindow = SW_HIDE;
    ZeroMemory(&stProcessInfo, sizeof(stProcessInfo));

    if (!CreateProcess(NULL,
        TEXT((char*)pszCmdBuf.c_str()),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &stStartupInfo,
        &stProcessInfo)) {
        uiRetCode = GetLastError();

        return FAILED;
    }

    if (WAIT_TIMEOUT == WaitForSingleObject(stProcessInfo.hProcess, LOG_NUM_1000 * LOG_NUM_3600)) {
        uiRetCode = CMD_TIME_OUT;  // 超时
    } else {
        GetExitCodeProcess(stProcessInfo.hProcess, &dwCode);
        uiRetCode = dwCode;
    }

    CloseHandle(stProcessInfo.hProcess);
    CloseHandle(stProcessInfo.hThread);

    return SUCCESS;
}
#endif

string CLogger::GetUserString()
{
    string strUserName;
    unsigned long iErrCode;
    ostringstream strMsg;

#ifdef WIN32
    strMsg << dec << this_thread::get_id();
    if (GetCurrentUserName(strUserName, iErrCode) == SUCCESS) {
        strMsg << "][" << strUserName;
    } else {
        strMsg << "][e-" << dec << iErrCode;
    }
#else
    // CodeDex误报，System Information Leak:Internal
    if (GetCurrentUserName(strUserName, iErrCode) == SUCCESS) {
        strMsg << dec << getpid() << "][" << this_thread::get_id() << "][" << strUserName;
    } else {
        strMsg << dec << getpid() << "][" << this_thread::get_id() << "][u:" << dec << getuid()
               << "e:" << dec << iErrCode;
    }
#endif
    return strMsg.str();
}

string CLogger::GetModuleName()
{
    string strUserName;
    unsigned long iErrCode;
    ostringstream strMsg;

#ifdef WIN32
    strMsg << dec << this_thread::get_id();
    if (GetCurrentUserName(strUserName, iErrCode) == SUCCESS) {
        strMsg << strUserName;
    } else {
        strMsg << "e-" << dec << iErrCode;
    }
#else
    // CodeDex误报，System Information Leak:Internal
    if (GetCurrentUserName(strUserName, iErrCode) == SUCCESS) {
        strMsg << strUserName;
    } else {
        strMsg << "u:" << dec << getuid() << "e:" << dec << iErrCode;
    }
#endif
    return strMsg.str();
}

string CLogger::FilterModleNamePath(const string& str)
{
    size_t end = str.find(".");
    if (end == string::npos) {
        return str;
    }
    size_t begin = str.find_last_of("/\\");
    if (begin == string::npos) {
        begin = 0;
    } else {
        begin = begin + 1;
    }
    size_t count = end - begin;
    return str.substr(begin, count);
}
int CLogger::InitLogContent(const int iLevel, const int iFileLine, const string& pszFileName,
                                 const string& pszFuncction, const string pszFormat, va_list& pszArgp, ostringstream& strMsg)
{
    // 初始化日志缓冲区
    char acMsgHead[LOG_HEAD_LENGTH] = {0};
    static char acMsg[MAX_MSG_SIZE] = {0};
    // 初始化日志缓冲区
    if (memset_s(acMsg, sizeof(MAX_MSG_SIZE), 0, sizeof(MAX_MSG_SIZE)) != EOK) {
        va_end(pszArgp);
        return FAILED;
    }
    // Coverity&Fortify误报:FORTIFY.Format_String
    // pszFormat都是程序内记录日志硬编码的
    int iRet = vsnprintf_s(acMsg, sizeof(acMsg), sizeof(acMsg) - 1, pszFormat.c_str(), pszArgp);
    if (iRet == FAILED) {
        va_end(pszArgp);
        return FAILED;
    }
    string strLog = acMsg;
    strLog = StrReplace(strLog, "\n", " ");
    strLog = StrReplace(strLog, "\r", " ");

    iRet = MkHead(iLevel, acMsgHead, sizeof(acMsgHead));
    if (iRet != SUCCESS) {
        va_end(pszArgp);
        return FAILED;
    }
    tm stCurTime;
    time_t tLogTime;
    time(&tLogTime);
    tm* pstCurTime = LocalTimeR(tLogTime, stCurTime);
    if (pstCurTime == NULL) {
        va_end(pszArgp);
        return FAILED;
    }

    struct timeval tv;
    GetTimeOfDay(tv);
    size_t reqID = *(HCPTSP::getInstance().get());
    strMsg << "[" << setfill('0') << setw(LOG_NUM_4) << (pstCurTime->tm_year + LOG_NUM_1900) << "-"
           << setw(LOG_NUM_2) << (pstCurTime->tm_mon + 1) << "-" << setw(LOG_NUM_2) << pstCurTime->tm_mday
           << " " << setw(LOG_NUM_2) << pstCurTime->tm_hour << ":" << setw(LOG_NUM_2) << pstCurTime->tm_min
           << ":" << setw(LOG_NUM_2) << pstCurTime->tm_sec << "." << setw(LOG_NUM_6) << tv.tv_usec << "][";
    strMsg << acMsgHead << "][" << strLog << "][";
    strMsg << FilterModleNamePath(pszFileName) << ", " << pszFuncction << ":" << iFileLine << "][" << dec
           << getpid() << "][" << this_thread::get_id() << "][";
    strMsg << GetModuleName() << dec << "][" << reqID << "]" << endl;
    return SUCCESS;
}

/* ------------------------------------------------------------
Description  :打印日志信息
Input        :      iLevel---日志级别，iFileLine---文件行号，pszFormat---格式
------------------------------------------------------------- */
void CLogger::Log(const int iLevel, const int iFileLine, const string& pszFileName,
                     const string& pszFuncction, const string& pszFormat, ...)
{
    lock_guard<std::mutex> lck(CLogger::m_mutex);

    // 如果配置日志写函数，则直接调用写日志函数
    if (m_funcWriteLog != NULL) {
        va_list pszArgp;
        va_start(pszArgp, pszFormat);
        ostringstream strMsg;
        if (InitLogContent(iLevel, iFileLine, pszFileName, pszFuncction, pszFormat, pszArgp, strMsg) != SUCCESS) {
            va_end(pszArgp);
            return;
        }
        m_funcWriteLog(iLevel, pszFileName, iFileLine, pszFuncction, strMsg.str());
        va_end(pszArgp);
        return;
    }

    // 根据定义的日志级别记录日志
    if (iLevel < m_iLogLevel) {
        return;
    }

    va_list pszArgp;
    va_start(pszArgp, pszFormat);
    ostringstream strMsg;
    if (InitLogContent(iLevel, iFileLine, pszFileName, pszFuncction, pszFormat, pszArgp, strMsg) != SUCCESS) {
        va_end(pszArgp);
        return;
    }

    // 判断日志缓存是否开启
    if (m_cacheFlg == LOG_CACHE_ON) {
        WriteLog2Cache(strMsg);
    } else {
        FILE* pFile = GetLogFile();
        if (pFile == NULL) {
            va_end(pszArgp);
            return;
        }

        fprintf(pFile, "%s", strMsg.str().c_str());
        fflush(pFile);
        fclose(pFile);
    }

    va_end(pszArgp);
}

void CLogger::SetWriteLogFunc(CallbackWriteLogPtr writeLogFunc)
{
    if (writeLogFunc != NULL) {
        m_funcWriteLog = writeLogFunc;
    }
}

CLogGuard::CLogGuard(int iLine, string strFunctionName, string strFileName, const string& pszFormat, ...)
        : m_iLine(iLine), m_strFileName(strFileName)
{
    char acMsg[LOG_NUM_1024] = {0};
    va_list pszArgp;
    /*lint -e437*/
    va_start(pszArgp, pszFormat);
    // Coverity&Fortify误报:FORTIFY.Format_String
    // pszFormat都是程序内记录日志硬编码的
    int iRet = vsnprintf_s(acMsg, sizeof(acMsg), sizeof(acMsg) - 1, pszFormat.c_str(), pszArgp);
    if (iRet == FAILED) {
        va_end(pszArgp);
        COMMLOG(OS_LOG_ERROR, "VSNPRINTF_S failed.");
        return;
    }
    // 不打印函数参数
    m_strFunctionName = GetFunctionName(strFunctionName);
#ifdef WIN32
    CLogger::GetInstance().Log(
        OS_LOG_DEBUG, m_iLine, m_strFileName, m_strFunctionName, "Enter %s...%s", m_strFunctionName.c_str(), acMsg);
#else
    CLogger::GetInstance().Log(
            OS_LOG_DEBUG, m_iLine, m_strFileName, m_strFunctionName, "Enter %s...%s", m_strFunctionName.c_str(), acMsg);
#endif
    va_end(pszArgp);
}

CLogGuard::~CLogGuard()
{
#ifdef WIN32
    /*lint -e1551*/
    CLogger::GetInstance().Log(
        OS_LOG_DEBUG, 0xffff, m_strFileName, m_strFunctionName, "Leave %s...", m_strFunctionName.c_str());
#else
    /*lint -e1551*/
    CLogger::GetInstance().Log(
            OS_LOG_DEBUG, 0xffff, m_strFileName, m_strFunctionName, "Leave %s...", m_strFunctionName.c_str());
#endif
}

int CLogger::SetLogConf(int iLogLevel, int iLogCount, int iLogMaxSize, int iLogKeepTime, int iLogCacheThreshold)
{
    lock_guard<std::mutex> lck(CLogger::m_mutex);
    m_iLogLevel = iLogLevel;
    m_iLogCount = iLogCount;
    m_iMaxSize = iLogMaxSize;
    m_iLogKeepTime = iLogKeepTime * DAY_TO_SECOND;
    m_cacheThreshold = static_cast<uint64_t>(iLogCacheThreshold) * LOG_NUM_1024 * LOG_NUM_1024;
    return 0;
}

int CLogger::SetLogConf(int iLogLevel, int iLogCount, int iLogMaxSize)
{
    lock_guard<std::mutex> lck(CLogger::m_mutex);
    m_iLogLevel = iLogLevel;
    m_iLogCount = iLogCount;
    m_iMaxSize = iLogMaxSize * MB_TO_BYTE;
    return 0;
}

std::string CLogger::GetLogRootPath() const
{
    return m_strFilePath;
}

string WipeSensitiveDataForLog(const string& str)
{
    if (str.empty()) {
        return "";
    }
    return Sensitive::WipeSensitive(str, str);
}

} // namespace Module

void HCPTSP::reset(std::size_t x)
{
    g_threadLocalRequestID = x;
}

std::size_t* HCPTSP::get()
{
    return &(g_threadLocalRequestID);
}

HCPTSP HCPTSP::instance;
HCPTSP& HCPTSP::getInstance(){
    return instance;
}
