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
#ifndef MODULE_LOG_H
#define MODULE_LOG_H

#include <stdarg.h>
#include <list>
#include <queue>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <regex>
#include <mutex>

#include "define/Defines.h"
#include "define/Types.h"

// HCPLOG 日志级别
enum severity_level {
    DEBUG = 0,
    TRACE = 0,
    INFO = 1,
    WARN = 2,
    ERR = 3,
    CRIT = 4,
};

namespace Module {
// 日志级别
static const int OS_LOG_DEBUG = 0;
static const int OS_LOG_INFO = 1;
static const int OS_LOG_WARN = 2;
static const int OS_LOG_ERROR = 3;
static const int OS_LOG_CRI = 4;

static const unsigned char MAX_HEAD_SIZE = 16;
static const uint32_t MAX_MSG_SIZE = 4096000;  // 1024*1000*4
static const unsigned char LOG_HEAD_LENGTH = 10;
static const unsigned char DEFAULT_LOG_COUNT = 5;
static const uint32_t UNIT_K = 1024;
static const uint32_t UNIT_M = 1048576;
static const uint32_t DEFAULT_LOG_SIZE = 52428800;       // 50M
static const uint32_t MAX_LOG_FILE_SIZE = 52428800;      // 50M
static const uint32_t DEFAULT_LOG_KEEP_TIME = 15552000;  // 180 days

static const unsigned char LOG_CACHE_ON = 1;
static const unsigned char LOG_CACHE_OFF = 0;
static const unsigned char LOG_CACHE_ASCII = 1;
static const unsigned char LOG_CACHE_UNICODE = 0;
static const unsigned char LOG_CACHE_MAXSIZE = 30;

#define ADDLOG(x) ADDLLU(x)
#define ADDLLU(x) x##LLU

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)
#define WFILE__ WIDEN(__FILE)

#ifdef __WINDOWS__
#define FUNCTION_NAME __FUNCTION__
#elif defined SOLARIS
#define FUNCTION_NAME __FUNCTION__
#elif defined(_AIX)
#define FUNCTION_NAME __FUNCTION__
#else
#define FUNCTION_NAME __PRETTY_FUNCTION__
#endif
#define LOG_FUNCTION_NAME Module::GetFunctionName(FUNCTION_NAME)

#ifdef WIN32
#define LOGGUARD(pszFormat, ...) Module::CLogGuard logGuard(__LINE__, __FUNCTION__, __FILE__, pszFormat, __VA_ARGS__)
#define COMMLOG(iLevel, pszFormat, ...)                                                                                \
    Module::CLogger::GetInstance().Log(iLevel, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, __VA_ARGS__)
#define DBGLOG(pszFormat, ...)                                                                                         \
    Module::CLogger::GetInstance().Log(Module::OS_LOG_DEBUG, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, __VA_ARGS__)
#define INFOLOG(pszFormat, ...)                                                                                        \
    Module::CLogger::GetInstance().Log(Module::OS_LOG_INFO, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, __VA_ARGS__)
#define WARNLOG(pszFormat, ...)                                                                                        \
    Module::CLogger::GetInstance().Log(Module::OS_LOG_WARN, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, __VA_ARGS__)
#define ERRLOG(pszFormat, ...)                                                                                         \
    Module::CLogger::GetInstance().Log(Module::OS_LOG_ERROR, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, __VA_ARGS__)
#elif defined SOLARIS
#define LOGGUARD(...) Module::CLogGuard logGuard(__LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
#define COMMLOG(iLevel, ...) Module::CLogger::GetInstance().Log(iLevel, __LINE__, __FILE__, LOG_FUNCTION_NAME, __VA_ARGS__)
#define DBGLOG(...) Module::CLogger::GetInstance().Log(Module::OS_LOG_DEBUG, __LINE__, __FILE__, LOG_FUNCTION_NAME, __VA_ARGS__)
#define INFOLOG(...) Module::CLogger::GetInstance().Log(Module::OS_LOG_INFO, __LINE__, __FILE__, LOG_FUNCTION_NAME, __VA_ARGS__)
#define WARNLOG(...) Module::CLogger::GetInstance().Log(Module::OS_LOG_WARN, __LINE__, __FILE__, LOG_FUNCTION_NAME, __VA_ARGS__)
#define ERRLOG(...) Module::CLogger::GetInstance().Log(Module::OS_LOG_ERROR, __LINE__, __FILE__, LOG_FUNCTION_NAME, __VA_ARGS__)
#else
#define COMMLOG(iLevel, pszFormat, args...)                                                                            \
    Module::CLogger::GetInstance().Log(iLevel, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, ##args)
#define LOGGUARD(pszFormat, args...) Module::CLogGuard logGuard(__LINE__, __FUNCTION__, __FILE__, pszFormat, ##args)
#define DBGLOG(pszFormat, args...)                                                                                     \
    Module::CLogger::GetInstance().Log(Module::OS_LOG_DEBUG, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, ##args)
#define INFOLOG(pszFormat, args...)                                                                                    \
    Module::CLogger::GetInstance().Log(Module::OS_LOG_INFO, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, ##args)
#define WARNLOG(pszFormat, args...)                                                                                    \
    Module::CLogger::GetInstance().Log(Module::OS_LOG_WARN, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, ##args)
#define ERRLOG(pszFormat, args...)                                                                                     \
    Module::CLogger::GetInstance().Log(Module::OS_LOG_ERROR, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, ##args)
#endif

#define DBGSTR(x) #x "=" << Sensitive::WipeSensitive(#x, x) << " "
#define DBG(x) #x "=" << (x) << " "

#define HEX(x) "0x" << std::hex << x << std::dec
#define HEXDBG(x) " " #x "=0x" << std::hex << x << std::dec
#define DPLOG(level) Module::LogProxy(level, __FILE__, LOG_FUNCTION_NAME, __LINE__) << ""
#define DBGLOG_EX() DPLOG(Module::OS_LOG_DEBUG)
#define INFOLOG_EX() DPLOG(Module::OS_LOG_INFO)
#define WARNLOG_EX() DPLOG(Module::OS_LOG_WARN)
#define ERRLOG_EX() DPLOG(Module::OS_LOG_ERROR)

#define HCP_Logger(level, module, requestID) Module::LogProxy(level, module, requestID, LOG_FUNCTION_NAME, __LINE__) << ""
#define HCP_Logger_noid(level, module) HCP_Logger(level, module, 0)
// 删除使用TSP_RequestID
#define HCP_Log(level, module) HCP_Logger(level, module, 0)
#define HCPENDLOG ""

namespace Sensitive {
    inline bool Match(const std::string& patten, const std::string& value)
    {
        std::regex reg(patten);
        std::smatch match;
        std::regex_search(value, match, reg);
        return !match.empty();
    }

    const std::string FuzzPattern[25] = {
        "[Pp][Aa][Ss][Ss]",
        "[Pp][Ww][Dd]", "[Kk][Ee][Yy]", "[Cc][Rr][Yy][Pp][Tt][Oo]", "[Ss][Ee][Ss][Ss][Ii][Oo][Nn]",
        "[Tt][Oo][Kk][Ee][Nn]", "[Ff][Ii][Nn][Gg][Ee][Rr][Pp][Rr][Ii][Nn][Tt]", "[Aa][Uu][Tt][Hh]",
        "[Ee][Nn][Cc]", "[Dd][Ee][Cc]", "[Tt][Gg][Tt]", "[Ii][Qq][Nn]", "[Ii][Nn][Ii][Tt][Ii][Aa][Tt][Oo][Rr]",
        "[Ss][Ee][Cc][Rr][Ee][Tt]", "[Cc][Ee][Rr][Tt]", "^[Ss][Kk]$", "^[Ii][Vv]$", "[Ss][Aa][Ll][Tt]", "^[Mm][Kk]$",
        "[Pp][Rr][Ii][Vv][Aa][Tt][Ee]", "[Uu][Ss][Ee][Rr][_][Ii][Nn][Ff][Oo]", "[Ee][Mm][Aa][Ii][Ll]",
        "[Pp][Hh][Oo][Nn][Ee]", "[Rr][Aa][Nn][Dd]", "[Vv][Ee][Rr][Ff][Ii][Yy][Cc][Oo][Dd][Ee]" };

    const int FuzzSize = sizeof(FuzzPattern) / sizeof(FuzzPattern[0]);

    inline std::string WipeSensitive(const std::string& name, const std::string& value)
    {
        for (std::size_t i = 0; i < FuzzSize; ++i) {
            if (Match(FuzzPattern[i], name)) {
                return "*";
            }
            if (Match(FuzzPattern[i], value)) {
                return "*";
            }
        }
        return value;
    }

    inline bool ISensitive(const std::string& name, const std::string& value)
    {
        for (std::size_t i = 0; i < FuzzSize; ++i) {
            if (Match(FuzzPattern[i], name)) {
                return true;
            }
            if (Match(FuzzPattern[i], value)) {
                return true;
            }
        }
        return false;
    }
}

namespace {
    typedef void (*CallbackWriteLogPtr)(int32_t level, const std::string& filePathName, int32_t lineNum,
                                        const std::string& funcName, const std::string& logString);
}

#define VARNAME(x) (#x)
#define WIPE_SENSITIVE(v) Module::WipeSensitiveDataForLog(Module::Trans2KV(VARNAME(v), (v)))

#ifdef WIN32
    AGENT_API std::string WipeSensitiveDataForLog(const std::string& str);
#else
    std::string WipeSensitiveDataForLog(const std::string& str);
#endif

template<typename T>
inline std::string Trans2KV(const char * varName, const T & var)
{
    std::ostringstream oss;
    oss << varName << ": " << var;
    return oss.str();
}

class AGENT_API CLogger {
public:
    ~CLogger();

    static CLogger& GetInstance();

    void Init(const char* pcLogFileName, std::string strFilePath);
    void Init(const char* pcLogFileName, std::string strFilePath, int iLogLevel, int iLogCount, int iLogMaxSize);

    int SetLogLevel(int iLogLevel);
    int SetLogCount(int iLogCount);
    // 打开日志缓存
    void OpenLogCache();
    // 关闭日志缓存
    int CloseLogCache();
    // void ReadLevelAndCount();
    // void ReadMaxSizeAndKeepTime();
    // 临时放到public,为了UT打桩方便
    // void ReadLogCacheThreshold();
    std::string GetUserString();
    std::string GetModuleName();
    std::string FilterModleNamePath(const std::string& str);
    int InitLogContent(const int iLevel, const int iFileLine, const std::string& pszFileName,
        const std::string& pszFuncction, const std::string pszFormat, va_list& pszArgp, std::ostringstream& strMsg);
    void Log(const int iLevel, const int iFileLine, const std::string& pszFileName,
        const std::string& pszFuncction, const std::string& pszFormat, ...);

    void SetWriteLogFunc(CallbackWriteLogPtr writeLogFunc);
    int SetLogConf(int iLogLevel, int iLogCount, int iLogMaxSize, int iLogKeepTime, int iLogCacheThreshold);
    int SetLogConf(int iLogLevel, int iLogCount, int iLogMaxSize);
    std::string GetLogRootPath() const;

private:
    CLogger();

    typedef struct _CacheData {
        int codeType;
        std::string logCache;
        std::wstring logCacheW;
        uint32_t cacheLen;
    } CacheData;

    static CLogger m_instance;  // 单例对象
    // 日志类线程互斥变量
    static std::mutex m_mutex;
    // 日志文件名
    std::string m_strFileName;
    // 日志文件路径
    std::string m_strFilePath;
    // 日志最大大小
    int m_iMaxSize;
    // 日志记录的当前级别
    int m_iLogLevel;
    // 日志个数
    int m_iLogCount;
    // 日志保留时间
    int m_iLogKeepTime;
    // 是否开启日志缓存功能
    int m_cacheFlg;
    // 日志缓存阈值大小(单位字节，配置文件单位为M)
    uint64_t m_cacheThreshold;
    // 当前日志缓存大小(单位字节)
    uint64_t m_cacheSize;
    // 日志缓存
    std::queue<CacheData> m_cacheContent;
    // 日志缓存打开计数器
    int m_OpenCacheNum;
    // 日志写函数，如果设置了日志函数，直接调用函数指针完成日志打印
    CallbackWriteLogPtr m_funcWriteLog;

    int MkHead(int iLevel, char pszHeadBuf[], int iBufLen);
    int SwitchLogFile(
        const std::string& pszLogPath, const std::string& pszLogName, int iLogCount, time_t LogKeepTime);
    int AsyncSwitchLogFile(
        const std::string& logFileName, const std::string& tmpLogFile, int iLogCount, time_t LogKeepTime);
    int HandleEachLogFile(const std::string& OldLogNameStr, const std::string& NewLogNameStr, int LogNum,
    int MaxLogCount, time_t MaxKeepTime);
    int HandleCurrentLogFile(const std::string& LogPath, const std::string& LogName);
    int CreateLogFile(const std::string& pszLogFile);
    FILE* GetLogFile();
#ifdef WIN32
    int ExecWinCmd(std::string pszCmdBuf, uint32_t& uiRetCode);
    void WriteLog2Cache(std::wostringstream& strWMsg);
#endif

    void WriteLog2Cache(std::ostringstream& strMsg);
    int ZipLogFile(std::string strCommand, std::string strLogFile);
};

inline std::string GetFunctionName(const std::string& prettyFunction)
{
    std::size_t end = prettyFunction.find("(");
    if (end == std::string::npos) {
        return prettyFunction;
    }
    std::size_t begin = prettyFunction.substr(0, end).rfind(" ");
    (begin == std::string::npos) ? (begin = 0) : (begin = begin + 1);
    end = end - begin;
    return prettyFunction.substr(begin, end) + "()";
}

class AGENT_API CLogGuard {
        public:
        CLogGuard(int iLine, std::string strFunctionName, std::string strFileName, const std::string& pszFormat, ...);
        ~CLogGuard();

        private:
        int m_iLine;
        std::string m_strFunctionName;
        std::string m_strFileName;
};

class LogProxy : public std::ostringstream {
public:
    LogProxy(const int level, const std::string &moduleName, const uint64_t requestID, const std::string &functionName,
        const unsigned int line)
        : m_logLevel(level), m_moduleName(moduleName), m_requestID(requestID), m_funcName(functionName), m_line(line)
    {}
    virtual ~LogProxy()
    {
        std::string msg = str();
        std::replace(msg.begin(), msg.end(), '%', ' ');

        CLogger::GetInstance().Log(m_logLevel, m_line, m_moduleName.c_str(), m_funcName.c_str(), msg);
    }

private:
    int m_logLevel {-1};
    std::string m_moduleName;
    uint64_t m_requestID {0};
    std::string m_funcName;
    unsigned int m_line {0};
};

} // namespace Module

class AGENT_API HCPTSP {
public:
    void reset(std::size_t x);
    std::size_t* get();
    static HCPTSP& getInstance();
private:
    HCPTSP(){};
    static HCPTSP instance;
};

#endif  // LOG_UNTL_H