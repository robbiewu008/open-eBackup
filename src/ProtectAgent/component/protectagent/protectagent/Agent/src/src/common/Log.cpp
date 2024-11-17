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
#include "common/Log.h"
#ifdef WIN32
#include <process.h>
#else
#include <csignal>
#include <sys/wait.h>
#endif
#include <memory>
#include <fstream>
#include "common/Log.h"
#include "common/Path.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "securec.h"
using namespace std;
CLogger CLogger::m_instance;
CLogger& CLogger::GetInstance()
{
    return m_instance;
}
namespace {
const mp_int32 LOG_NUM_1024 = 1024;
const mp_int32 LOG_NUM_3600 = 3600;
const mp_int32 LOG_NUM_1900 = 1900;
const mp_int32 LOG_NUM_1000 = 1000;
const mp_uchar LOG_NUM_2 = 2;
const mp_uchar LOG_NUM_4 = 4;
const mp_uchar LOG_NUM_6 = 6;
const std::string FUNCTION_LINE_SEPARARTOR = ":";
const mp_int32 MB_TO_BYTE = 1048576;
const mp_int32 DAY_TO_SECOND = 86400;
const mp_string IN_AGENT_LOG_LEVAL_FILE = "/opt/common-conf/loglevel";
const mp_string IN_AGENT_TYPE = "1";
const mp_string ENVIRONMENT_TYPE = "ENVIRONMENT_TYPE";
const mp_string BACKUP_SCENE = "BACKUP_SCENE";
const mp_string DORADO_ENVIRONMENT = "0";
static std::map<mp_string, mp_int32> logLevelMap = {
    {"DEBUG", OS_LOG_DEBUG}, {"INFO", OS_LOG_INFO}, {"WARN", OS_LOG_WARN}, {"ERROR", OS_LOG_ERROR}};
}  // namespace

CLogger::CLogger()
{
    CMpThread::InitLock(&m_tLock);
    m_iMaxSize = DEFAULT_LOG_SIZE;
    // 设置日志默认级别和个数，日志保存天数
    m_iLogLevel = OS_LOG_INFO;
    m_iLogCount = DEFAULT_LOG_COUNT;
    m_iLogKeepTime = DEFAULT_LOG_KEEP_TIME;
    m_cacheThreshold = (mp_uint64)LOG_CACHE_MAXSIZE * LOG_NUM_1024 * LOG_NUM_1024;
    m_cacheFlg = LOG_CACHE_OFF;
    // 设置打开日志次数
    m_OpenCacheNum = 0;
    m_cacheSize = 0;
    m_funcWriteLog = NULL;
}

CLogger::~CLogger()
{
    CMpThread::DestroyLock(&m_tLock);
    while (!m_cacheContent.empty()) {
        m_cacheContent.pop();
    }
}

/* ------------------------------------------------------------
Description  :设置log级别
Input        :      iLogLevel---log级别
Return       :    MP_SUCCESS---设置成功
                   MP_FAILED---设置失败
------------------------------------------------------------- */
mp_int32 CLogger::SetLogLevel(mp_int32 iLogLevel)
{
    if (iLogLevel < OS_LOG_DEBUG || iLogLevel > OS_LOG_CRI) {
        return MP_FAILED;
    }

    m_iLogLevel = iLogLevel;

    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  :设置log计数
Input        :      iLogCount---log计数
Return       :    MP_SUCCESS---设置成功
                   MP_FAILED---设置失败
------------------------------------------------------------- */
mp_int32 CLogger::SetLogCount(mp_int32 iLogCount)
{
    if (iLogCount <= 0) {
        return MP_FAILED;
    }

    m_iLogCount = iLogCount;

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :打开日志缓存
Return       :    MP_SUCCESS---设置成功
                   MP_FAILED---设置失败
------------------------------------------------------------- */
mp_void CLogger::OpenLogCache()
{
    CThreadAutoLock tlock(&m_tLock);

    m_cacheFlg = LOG_CACHE_ON;
    ++m_OpenCacheNum;
}

/* ------------------------------------------------------------
Description  :关闭日志缓存
Return       :    MP_SUCCESS---设置成功
                   MP_FAILED---设置失败
------------------------------------------------------------- */
mp_int32 CLogger::CloseLogCache()
{
    CThreadAutoLock tlock(&m_tLock);

    // 适配多次打开日志缓存的情况，
    // 必须需要等到全部关闭，才去关闭缓存
    --m_OpenCacheNum;
    if (m_OpenCacheNum > 0) {
        return MP_SUCCESS;
    }

    m_cacheFlg = LOG_CACHE_OFF;

    // 写入日志缓存到日志文件中
    FILE* pFile = GetLogFile();
    if (pFile == NULL) {
        return MP_FAILED;
    }

    while (!m_cacheContent.empty()) {
        if (m_cacheContent.front().codeType == LOG_CACHE_ASCII) {
            fprintf(pFile, "%s", m_cacheContent.front().logCache.c_str());
        } else {
            // CodeDex误报,KLOCWORK.SV.FMT_STR.PRINT_FORMAT_MISMATCH.UNDESIRED
            fwprintf(pFile, L"%s", m_cacheContent.front().logCacheW.c_str());
        }
        m_cacheContent.pop();
    }
    m_cacheSize = 0;

    fflush(pFile);
    fclose(pFile);

    return MP_SUCCESS;
}

// 创建日志信息的"标题头"
mp_int32 CLogger::MkHead(mp_int32 iLevel, mp_char pszHeadBuf[], mp_int32 iBufLen)
{
    mp_int32 iRet = MP_SUCCESS;

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

    if (iRet == MP_FAILED) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CLogger::ZipLogFile(const mp_string& strCommand, const mp_string& strLogFile)
{
#ifndef WIN32
    (mp_void) strLogFile;
    if (CheckCmdDelimiter(strCommand) != MP_SUCCESS) {
        return MP_FAILED;
    }

    // Coverity&Fortify误报:FORTIFY.Command_Injection
    // 已经按修复建议增加命令分隔符判断CheckCmdDelimiter
    mp_int32 iRet = system(strCommand.c_str());
    if (!WIFEXITED(iRet)) {
        // system异常返回
        return MP_FAILED;
    }
#else  // windows
    mp_uint32 uiRetCode = 0;
    if (CheckCmdDelimiter(strCommand) != MP_SUCCESS) {
        return MP_FAILED;
    }

    mp_int32 iRet = ExecWinCmd(strCommand, uiRetCode);
    if ((iRet != MP_SUCCESS) || (uiRetCode != 0)) {
        return MP_FAILED;
    }

    (void)remove(strLogFile.c_str());
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :选择log文件
Input        :      pszLogPath---log存放路径，pszLogName---log名，iLogCount---log计数行
------------------------------------------------------------- */
mp_int32 CLogger::SwitchLogFile(
    const mp_string& pszLogPath, const mp_string& pszLogName, mp_int32 iLogCount, mp_time LogKeepTime)
{
    mp_char OldLogFileName[MAX_FULL_PATH_LEN] = {0};
    mp_char NewLogFileName[MAX_FULL_PATH_LEN] = {0};
    mp_string strSuffix;
    mp_int32 iRet = MP_FAILED;

#ifndef WIN32
    strSuffix = "gz";
#else
    strSuffix = "zip";
#endif
    // 处理转储的日志文件
    for (mp_int32 LogNum = iLogCount; LogNum > 0; --LogNum) {
        iRet = snprintf_s(OldLogFileName, MAX_FULL_PATH_LEN, MAX_FULL_PATH_LEN - 1, "%s%s%s.%d.%s",
            pszLogPath.c_str(), PATH_SEPARATOR.c_str(), pszLogName.c_str(), LogNum, strSuffix.c_str());
        if (iRet == MP_FAILED) {
            return iRet;
        }

        iRet = snprintf_s(NewLogFileName, MAX_FULL_PATH_LEN, MAX_FULL_PATH_LEN - 1, "%s%s%s.%d.%s",
            pszLogPath.c_str(), PATH_SEPARATOR.c_str(), pszLogName.c_str(), LogNum + 1, strSuffix.c_str());
        if (iRet == MP_FAILED) {
            return iRet;
        }

        mp_string OldLogFileNameStr = OldLogFileName;
        mp_string NewLogFileNameStr = NewLogFileName;
        iRet = HandleEachLogFile(OldLogFileNameStr, NewLogFileNameStr, LogNum, iLogCount, LogKeepTime);
        if (iRet != MP_SUCCESS) {
            return iRet;
        }
    }

    // 转储当前日志文件并重命名
    iRet = HandleCurrentLogFile(pszLogPath, pszLogName);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    // 生成当前日志文件
    mp_string strLogFile = mp_string("") + pszLogPath + PATH_SEPARATOR + pszLogName;
    iRet = CreateLogFile(strLogFile.c_str());
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    return MP_SUCCESS;
}

mp_int32 CLogger::HandleEachLogFile(const mp_string& OldLogNameStr, const mp_string& NewLogNameStr, mp_int32 LogNum,
    mp_int32 MaxLogCount, mp_time MaxKeepTime)
{
    const mp_char* OldLogName = OldLogNameStr.c_str();
    // 日志不存在无需处理
    if (!CMpFile::FileExist(OldLogName)) {
        return MP_SUCCESS;
    }

    // 达到最大日志数要删除
    if (LogNum >= MaxLogCount) {
        CMpFile::DelFile(OldLogName);
        return MP_SUCCESS;
    }

    // 日志过期要删除
    mp_time LastModifyTime;
    mp_time NowTime;
    mp_time KeepTime;

    CMpTime::Now(NowTime);
    mp_int32 iRet = CMpFile::GetlLastModifyTime(OldLogName, LastModifyTime);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    KeepTime = NowTime - LastModifyTime;
    if (KeepTime > MaxKeepTime) {
        CMpFile::DelFile(OldLogName);
        return MP_SUCCESS;
    }

    // 变更日志文件名
    const mp_char* NewLogName = NewLogNameStr.c_str();
    (void)rename(OldLogName, NewLogName);
#ifndef WIN32
    (void)ChmodFile(NewLogNameStr, S_IRUSR);
#endif

    return MP_SUCCESS;
}
mp_int32 CLogger::HandleCurrentLogFile(const mp_string& LogPath, const mp_string& LogName)
{
    mp_string strLogFile = mp_string("") + LogPath + PATH_SEPARATOR + LogName;
    mp_string strSuffix;
    mp_int32 iRet = MP_FAILED;

#ifndef WIN32
    strSuffix = "gz";
#else
    strSuffix = "zip";
#endif
    mp_char OldLogFileName[MAX_FULL_PATH_LEN] = {0};
    iRet = snprintf_s(
        OldLogFileName, MAX_FULL_PATH_LEN, MAX_FULL_PATH_LEN - 1, "%s.%s", strLogFile.c_str(), strSuffix.c_str());
    if (iRet == MP_FAILED) {
        return iRet;
    }
    mp_char NewLogFileName[MAX_FULL_PATH_LEN] = {0};
    iRet = snprintf_s(
        NewLogFileName, MAX_FULL_PATH_LEN, MAX_FULL_PATH_LEN - 1, "%s.%d.%s", strLogFile.c_str(), 1, strSuffix.c_str());
    if (iRet == MP_FAILED) {
        return iRet;
    }

    // 压缩日志文件
    mp_string strCommand;
#ifndef WIN32
    strCommand = mp_string("gzip -f -q -9 \"") + strLogFile + "\"";
#else
    mp_string strZipTool = CPath::GetInstance().GetBinFilePath("7z.exe");
    strCommand = mp_string("\"") + strZipTool + "\" a -y -tzip \"" + OldLogFileName + "\" \"" + strLogFile + "\" -mx=9";
#endif
    ZipLogFile(strCommand, strLogFile);

    // 重命名日志文件
    (void)rename(OldLogFileName, NewLogFileName);
#ifndef WIN32
    mp_string strFile = NewLogFileName;
    (void)ChmodFile(strFile, S_IRUSR);
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :创建log文件
Input        :      pszLogFile---log文件名
Return       :    MP_SUCCESS---创建成功
                   MP_FAILED---创建失败
------------------------------------------------------------- */
mp_int32 CLogger::CreateLogFile(const mp_string& pszLogFile)
{
#ifdef WIN32
    FILE* pFile = NULL;
    mp_string pszLogFileName;
    mp_int32 itmp;
    // CodeDex误报，Race Condition:File System Access
    pFile = fopen(pszLogFile.c_str(), "w");
    if (pFile == NULL) {
        return MP_FAILED;
    }
    fclose(pFile);

    itmp = pszLogFile.find_last_of(PATH_SEPARATOR);
    if (string::npos == itmp) {
        return MP_FAILED;
    }
    pszLogFileName = pszLogFile.substr(itmp + 1);

    string strCommand = "cmd.exe /c echo Y | cacls.exe \"" + string(pszLogFile) + "\" /E /R Users > NUL";
    mp_uint32 uiRetCode = 0;

    mp_int32 iRtn = ExecWinCmd(strCommand, uiRetCode);
    if ((iRtn != MP_SUCCESS) || (uiRetCode != 0)) {
        return MP_FAILED;
    }
#else
    mp_int32 itmp = pszLogFile.find_last_of(PATH_SEPARATOR);
    if (string::npos == itmp) {
        return MP_FAILED;
    }
    mp_string pszLogFileName = pszLogFile.substr(itmp + 1);
    mp_int32 fd = open(pszLogFile.c_str(), O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        return MP_FAILED;
    }
    close(fd);
    // 设置权限
    if (pszLogFileName == "rdagent.log") {
        // 虚拟化插件调用Agent的systemexec代码执行sudo命令，日志会打印到rdagent.log，exadmin用户无rdadmin.log写入权限
        (void)ChmodFile(pszLogFile, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    } else {
        (void)ChmodFile(pszLogFile, S_IRUSR | S_IWUSR);
    }
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :读取log级别和计数
------------------------------------------------------------- */
mp_void CLogger::ReadLevelAndCount()
{
    mp_int32 iLogLevel = 0;
    if (IsInAgent()) {
        mp_string inAgentLogLevel;
        if (GetInAgentLogLevel(inAgentLogLevel) == MP_SUCCESS) {
            auto it = logLevelMap.find(inAgentLogLevel);
            if (it != logLevelMap.end()) {
                iLogLevel = it->second;
            }
        }
    } else {
        mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_LOG_LEVEL, iLogLevel);
        if (iRet != MP_SUCCESS) {
            return;
        }
    }
    m_iLogLevel = iLogLevel;
    mp_int32 iLogCount = 0;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_LOG_COUNT, iLogCount);
    if (iRet != MP_SUCCESS) {
        return;
    }
    m_iLogCount = iLogCount;
}

/* ------------------------------------------------------------
Description  :读取log文件大小限制和log文件保存时间
------------------------------------------------------------- */
mp_void CLogger::ReadMaxSizeAndKeepTime()
{
    mp_int32 LogMaxSize = 0;
    mp_int32 iRet = MP_FAILED;

    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_LOG_MAX_SIZE, LogMaxSize);
    if (iRet != MP_SUCCESS) {
        return;
    }
    LogMaxSize *= MB_TO_BYTE;

    mp_int32 LogKeepTime = 0;
    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_LOG_KEEP_TIME, LogKeepTime);
    if (iRet != MP_SUCCESS) {
        return;
    }
    LogKeepTime *= DAY_TO_SECOND;

    m_iMaxSize = LogMaxSize;
    m_iLogKeepTime = LogKeepTime;
}

/* ------------------------------------------------------------
Description  :读取log缓存的阈值
------------------------------------------------------------- */
mp_void CLogger::ReadLogCacheThreshold()
{
    mp_int32 iLogCacheThreshold = 0;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_SYSTEM_SECTION, CFG_LOG_CACHE_THRESHOLD, iLogCacheThreshold);
    if (iRet != MP_SUCCESS) {
        m_cacheThreshold = (mp_uint64)LOG_CACHE_MAXSIZE * LOG_NUM_1024 * LOG_NUM_1024;
        return;
    }
    m_cacheThreshold = static_cast<mp_uint64>(iLogCacheThreshold) * LOG_NUM_1024 * LOG_NUM_1024;
}

/* ------------------------------------------------------------
Description  :打开log文件
Return       :     strMsg---字符流
                    NULL---打开失败
------------------------------------------------------------- */
mp_void CLogger::WriteLog2Cache(ostringstream& strMsg)
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
mp_void CLogger::WriteLog2Cache(wostringstream& strWMsg)
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
    mp_string strLogFilePath;
    mp_uint32 uiFileSize = 0;
    FILE* pFile = NULL;
    mp_int32 iRet = 0;

    strLogFilePath = m_strFilePath + PATH_SEPARATOR + m_strFileName;
    if (MP_TRUE != CMpFile::FileExist(strLogFilePath)) {
        iRet = CreateLogFile(strLogFilePath.c_str());
        if (iRet == MP_FAILED) {
            return NULL;
        }
    } else {
        if (CMpFile::FileSize(strLogFilePath.c_str(), uiFileSize) != MP_SUCCESS) {
            return NULL;
        }

        if (uiFileSize >= (mp_uint32)m_iMaxSize) {
            iRet = SwitchLogFile(m_strFilePath, m_strFileName, m_iLogCount, m_iLogKeepTime);
            if (iRet == MP_FAILED) {
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
EXTER_ATTACK FILE* CLogger::GetLogFile()
{
    // codedex误报CANONICAL_FILEPATH，日志路径不存在这个问题
    mp_string strLogFilePath;
    FILE* pFile = NULL;
    mp_int32 fd;
    struct stat st;
    mp_int32 iRet = 0;

    strLogFilePath = m_strFilePath + PATH_SEPARATOR + m_strFileName;
    fd = open(strLogFilePath.c_str(), O_RDONLY);
    if (-1 == fd) {
        iRet = CreateLogFile(strLogFilePath.c_str());
        if (iRet == MP_FAILED) {
            return NULL;
        }
    } else {
        (void)fstat(fd, &st);
        close(fd);
        // 日志超过大小切换日志
        if (st.st_size >= m_iMaxSize) {
            iRet = SwitchLogFile(m_strFilePath, m_strFileName, m_iLogCount, m_iLogKeepTime);
            if (iRet == MP_FAILED) {
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
Description  :执行Windows命令
Input        :      pszCmdBuf---命令缓存
Output       :     uiRetCode---返回码
Return       :     MP_SUCCESS---执行成功
                    MP_FAILED---执行失败
------------------------------------------------------------- */
mp_int32 CLogger::ExecWinCmd(mp_string pszCmdBuf, mp_uint32& uiRetCode)
{
    list<mp_string> lstContents;
    mp_int32 iRet = 0;
    mp_string strCmd = pszCmdBuf;
    STARTUPINFO stStartupInfo;
    PROCESS_INFORMATION stProcessInfo;
    DWORD dwCode = 0;

    if (CheckCmdDelimiter(strCmd) != MP_SUCCESS) {
        return MP_FAILED;
    }

    ZeroMemory(&stStartupInfo, sizeof(stStartupInfo));
    stStartupInfo.cb = sizeof(stStartupInfo);
    stStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    stStartupInfo.wShowWindow = SW_HIDE;
    ZeroMemory(&stProcessInfo, sizeof(stProcessInfo));

    if (!CreateProcess(NULL, TEXT((mp_char*)pszCmdBuf.c_str()),
        NULL, NULL, MP_FALSE, 0, NULL, NULL, &stStartupInfo, &stProcessInfo)) {
        uiRetCode = GetLastError();
        return MP_FAILED;
    }

    if (WAIT_TIMEOUT == WaitForSingleObject(stProcessInfo.hProcess, LOG_NUM_1000 * LOG_NUM_3600)) {
        uiRetCode = 1;
    } else {
        GetExitCodeProcess(stProcessInfo.hProcess, &dwCode);
        uiRetCode = dwCode;
    }

    CloseHandle(stProcessInfo.hProcess);
    CloseHandle(stProcessInfo.hThread);

    return MP_SUCCESS;
}
#endif

mp_string CLogger::GetUserString()
{
    mp_string strUserName;
    mp_ulong iErrCode;
    ostringstream strMsg;

    if (GetCurrentUserName(strUserName, iErrCode) == MP_SUCCESS) {
        strMsg << std::dec << getpid() << "][" << CMpThread::GetThreadId() << "][" << strUserName;
    } else {
#ifdef WIN32
        strMsg << std::dec << getpid() << "][" << CMpThread::GetThreadId() << "][e-" << std::dec << iErrCode;
#else
        strMsg << std::dec << getpid() << "][" << CMpThread::GetThreadId() << "][u:" << std::dec << getuid()
            << "e:" << std::dec << iErrCode;
#endif // WIN32
    }
    return strMsg.str();
}

mp_string CLogger::GetModuleName()
{
    mp_string strUserName;
    mp_ulong iErrCode;
    ostringstream strMsg;

#ifdef WIN32
    strMsg << std::dec << CMpThread::GetThreadId();
    if (GetCurrentUserName(strUserName, iErrCode) == MP_SUCCESS) {
        strMsg << strUserName;
    } else {
        strMsg << "e-" << std::dec << iErrCode;
    }
#else
    // CodeDex误报，System Information Leak:Internal
    if (GetCurrentUserName(strUserName, iErrCode) == MP_SUCCESS) {
        strMsg << strUserName;
    } else {
        strMsg << "u:" << std::dec << getuid() << "e:" << std::dec << iErrCode;
    }
#endif
    return strMsg.str();
}

mp_string CLogger::FilterModleNamePath(const mp_string& str)
{
    std::size_t end = str.find(".");
    if (end == std::string::npos) {
        return str;
    }
    std::size_t begin = str.find_last_of("/\\");
    if (begin == std::string::npos) {
        begin = 0;
    } else {
        begin = begin + 1;
    }
    std::size_t count = end - begin;
    return str.substr(begin, count);
}

mp_void CLogger::StrReplaceSpecSymbols(mp_string &logContent)
{
    //  替换特殊字符
    std::vector<std::pair<mp_string, mp_string>> replaceRules = {
        {"\n", " "},
        {"\r", " "},
        {"\b", " "},
        {"\x08", " "},
        {"\x0a", " "},
        {"\x0b", " "},
        {"\x0c", " "},
        {"\x0d", " "},
        {"\x7f", " "}
    };
    for (const auto &rule : replaceRules) {
        logContent = CMpString::StrReplace(logContent, rule.first, rule.second);
    }
    return;
}

mp_int32 CLogger::InitLogContent(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, va_list& pszArgp, std::ostringstream& strMsg)
{
    // 初始化日志缓冲区
    static mp_char acMsg[MAX_MSG_SIZE] = {0};
    // 初始化日志缓冲区
    if (memset_s(acMsg, sizeof(MAX_MSG_SIZE), 0, sizeof(MAX_MSG_SIZE)) != EOK) {
        va_end(pszArgp);
        return MP_FAILED;
    }
    // Coverity&Fortify误报:FORTIFY.Format_String
    // pszFormat都是程序内记录日志硬编码的
    mp_int32 iRet = vsnprintf_s(acMsg, sizeof(acMsg), sizeof(acMsg) - 1, pszFormat.c_str(), pszArgp);
    if (iRet == MP_FAILED) {
        va_end(pszArgp);
        return MP_FAILED;
    }
    mp_string strLog = acMsg;
    StrReplaceSpecSymbols(strLog);

    mp_char acMsgHead[LOG_HEAD_LENGTH] = {0};
    iRet = MkHead(iLevel, acMsgHead, sizeof(acMsgHead));
    if (iRet != MP_SUCCESS) {
        va_end(pszArgp);
        return MP_FAILED;
    }
    mp_tm stCurTime;
    mp_time tLogTime;
    CMpTime::Now(tLogTime);
    mp_tm* pstCurTime = CMpTime::LocalTimeR(tLogTime, stCurTime);
    if (pstCurTime == NULL) {
        va_end(pszArgp);
        return MP_FAILED;
    }

    struct timeval tv;
    CMpTime::GetTimeOfDay(tv);
    strMsg << "[" << std::setfill('0') << std::setw(LOG_NUM_4) << (pstCurTime->tm_year + LOG_NUM_1900) << "-"
           << std::setw(LOG_NUM_2) << (pstCurTime->tm_mon + 1) << "-" << std::setw(LOG_NUM_2) << pstCurTime->tm_mday
           << " " << std::setw(LOG_NUM_2) << pstCurTime->tm_hour << ":" << std::setw(LOG_NUM_2) << pstCurTime->tm_min
           << ":" << std::setw(LOG_NUM_2) << pstCurTime->tm_sec << "." << std::setw(LOG_NUM_6) << tv.tv_usec << "][";
    strMsg << acMsgHead << "][" << strLog << "][";
    strMsg << FilterModleNamePath(pszFileName) << ", " << pszFuncction << ":" << iFileLine << "][" << GetUserString();
    strMsg << std::dec << "]" << std::endl;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :打印日志信息
Input        :      iLevel---日志级别，iFileLine---文件行号，pszFormat---格式
------------------------------------------------------------- */
mp_void CLogger::Log(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...)
{
    va_list pszArgp;
    ostringstream strMsg;

    CThreadAutoLock tlock(&m_tLock);

    // 如果配置日志写函数，则直接调用写日志函数
    if (m_funcWriteLog != NULL) {
        va_start(pszArgp, pszFormat);
        if (InitLogContent(iLevel, iFileLine, pszFileName, pszFuncction, pszFormat, pszArgp, strMsg) != MP_SUCCESS) {
            va_end(pszArgp);
            return;
        }
        m_funcWriteLog(iLevel, pszFileName, iFileLine, pszFuncction, strMsg.str());
        va_end(pszArgp);
        return;
    }

    // 支持动态修改日志级别
    ReadLevelAndCount();
    // 根据定义的日志级别记录日志
    if (iLevel < m_iLogLevel) {
        return;
    }
    // 支持动态修改日志文件大小限制和日志文件保存时间
    ReadMaxSizeAndKeepTime();
    // 支持动态修改日志缓存阈值大小
    ReadLogCacheThreshold();

    va_start(pszArgp, pszFormat);
    if (InitLogContent(iLevel, iFileLine, pszFileName, pszFuncction, pszFormat, pszArgp, strMsg) != MP_SUCCESS) {
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

mp_void CLogger::SetWriteLogFunc(CallbackWriteLogPtr writeLogFunc)
{
    if (writeLogFunc != NULL) {
        m_funcWriteLog = writeLogFunc;
    }
}

#ifdef WIN32
mp_wstring CLogger::GetUserStringW()
{
    mp_wstring strUserName;
    mp_ulong iErrCode;
    wostringstream strMsg;
    strMsg << std::dec << CMpThread::GetThreadId();
    if (GetCurrentUserNameW(strUserName, iErrCode) == MP_SUCCESS) {
        strMsg << L"][" << strUserName;
    } else {
        strMsg << L"e-" << std::dec << iErrCode;
    }
    return strMsg.str();
}

mp_wstring CLogger::FilterModleNamePath(const mp_wstring& str)
{
    std::size_t end = str.find(L".");
    if (end == std::string::npos) {
        return str;
    }
    std::size_t begin = str.find_last_of(L"/\\");
    if (begin == std::string::npos) {
        begin = 0;
    } else {
        begin = begin + 1;
    }
    std::size_t count = end - begin;
    return str.substr(begin, count);
}

mp_int32 CLogger::InitLogContentW(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_wstring& pszFileName,
    const mp_wstring& pszFuncction, const mp_wstring pszFormat, va_list& pszArgp, std::wostringstream& strMsg)
{
    // 初始化日志缓冲区
    static mp_wchar acMsg[MAX_MSG_SIZE] = { 0 };
    mp_int32 iRet = _vsnwprintf_s(acMsg, MAX_MSG_SIZE, MAX_MSG_SIZE - 1, pszFormat.c_str(), pszArgp);
    if (iRet == MP_FAILED) {
        va_end(pszArgp);
        return MP_FAILED;
    }

    mp_wstring msgHead;
    switch (iLevel) {
        case OS_LOG_DEBUG: msgHead = L"DBG";
            break;
        case OS_LOG_INFO: msgHead = L"INFO";
            break;
        case OS_LOG_WARN: msgHead = L"WARN";
            break;
        case OS_LOG_ERROR: msgHead = L"ERR";
            break;
        case OS_LOG_CRI: msgHead = L"CRI";
            break;
        default: msgHead = L"";
            break;
    }

    mp_tm stCurTime;
    mp_time tLogTime;
    CMpTime::Now(tLogTime);
    mp_tm* pstCurTime = CMpTime::LocalTimeR(tLogTime, stCurTime);
    if (pstCurTime == NULL) {
        va_end(pszArgp);
        return MP_FAILED;
    }

    struct timeval tv;
    CMpTime::GetTimeOfDay(tv);
    strMsg << L"[" << std::setfill(L'0') << std::setw(LOG_NUM_4) << (pstCurTime->tm_year + LOG_NUM_1900) << L"-"
        << std::setw(LOG_NUM_2) << (pstCurTime->tm_mon + 1) << L"-" << std::setw(LOG_NUM_2) << pstCurTime->tm_mday
        << L" " << std::setw(LOG_NUM_2) << pstCurTime->tm_hour << L":" << std::setw(LOG_NUM_2) << pstCurTime->tm_min
        << L":" << std::setw(LOG_NUM_2) << pstCurTime->tm_sec << L"." << std::setw(LOG_NUM_6) << tv.tv_usec << L"][";
    strMsg << msgHead << L"][" << acMsg << L"][";
    strMsg << FilterModleNamePath(pszFileName) << L", " << pszFuncction << L":" << iFileLine << L"][" <<
        GetUserStringW() << std::dec << L"]" << std::endl;
    return MP_SUCCESS;
}

mp_void CLogger::LogW(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_wstring& pszFileName,
    const mp_wstring& pszFuncction, const mp_wstring pszFormat, ...)
{
    va_list pszArgp;
    wostringstream strMsg;
    CThreadAutoLock tlock(&m_tLock);

    // 支持动态修改日志级别
    ReadLevelAndCount();
    // 支持动态修改日志文件大小限制和日志文件保存时间
    ReadMaxSizeAndKeepTime();
    // 支持动态修改日志缓存阈值大小
    ReadLogCacheThreshold();
    // 根据定义的日志级别记录日志
    if (iLevel < m_iLogLevel) {
        return;
    }

    va_start(pszArgp, pszFormat);
    if (InitLogContentW(iLevel, iFileLine, pszFileName, pszFuncction, pszFormat, pszArgp, strMsg) != MP_SUCCESS) {
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

        fprintf(pFile, "%ws", strMsg.str().c_str());
        fflush(pFile);
        fclose(pFile);
    }

    va_end(pszArgp);
}
#endif

CLogGuard::CLogGuard(mp_int32 iLine, const mp_string& strFunctionName, const mp_string& strFileName,
    const mp_string& pszFormat, ...)
    : m_iLine(iLine), m_strFileName(strFileName)
{
    mp_char acMsg[LOG_NUM_1024] = {0};
    va_list pszArgp;
    va_start(pszArgp, pszFormat);
    // Coverity&Fortify误报:FORTIFY.Format_String
    // pszFormat都是程序内记录日志硬编码的
    mp_int32 iRet = vsnprintf_s(acMsg, sizeof(acMsg), sizeof(acMsg) - 1, pszFormat.c_str(), pszArgp);
    if (iRet == MP_FAILED) {
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
    CLogger::GetInstance().Log(
        OS_LOG_DEBUG, 0xffff, m_strFileName, m_strFunctionName, "Leave %s...", m_strFunctionName.c_str());
#else
    CLogger::GetInstance().Log(
        OS_LOG_DEBUG, 0xffff, m_strFileName, m_strFunctionName, "Leave %s...", m_strFunctionName.c_str());
#endif
}

mp_bool CLogger::IsInAgent()
{
    mp_string sceneType;
    if (GetTestCfgInfo(sceneType) != MP_SUCCESS) {
        return MP_FALSE;
    }
    if (sceneType == IN_AGENT_TYPE) {
        return MP_TRUE;
    }
    return MP_FALSE;
}

mp_int32 CLogger::GetTestCfgInfo(mp_string& strSceneType)
{
    mp_string strFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    if (CMpFile::FileExist(strFilePath) != MP_TRUE) {
        return MP_FAILED;
    }

    std::ifstream stream;
    stream.open(strFilePath.c_str(), std::ifstream::in);
    mp_string line;
    mp_string strSceneText = "";

    if (!stream.is_open()) {
        return MP_FAILED;
    }
    std::vector<mp_string> vecTestCfgFileInfo;
    while (getline(stream, line)) {
        vecTestCfgFileInfo.push_back(line);
    }
    stream.close();
    for (auto item : vecTestCfgFileInfo) {
        if (item.find(BACKUP_SCENE) != std::string::npos) {
            strSceneText = item;
        }
    }
    std::size_t start = strSceneText.find("=", 0);
    if (start == std::string::npos) {
        return MP_FAILED;
    }
    strSceneType = strSceneText.substr(start + 1);

    return MP_SUCCESS;
}

mp_int32 CLogger::GetInAgentLogLevel(mp_string& logLevelInfo)
{
    std::ifstream stream;
    mp_string line;
    stream.open(IN_AGENT_LOG_LEVAL_FILE.c_str(), std::ifstream::in);

    if (!stream.is_open()) {
        return MP_FAILED;
    }
    std::vector<mp_string> vecLogLevelFileInfo;
    while (getline(stream, line)) {
        vecLogLevelFileInfo.push_back(line);
    }
    if (vecLogLevelFileInfo.empty()) {
        return MP_FAILED;
    }
    logLevelInfo = vecLogLevelFileInfo.front();
    return MP_SUCCESS;
}