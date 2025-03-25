#ifndef AGENT_LOG_H
#define AGENT_LOG_H

#include <stdarg.h>
#include <list>
#include <queue>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <regex>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include "common/Types.h"
#include "common/LogCode.h"
#include "common/CMpThread.h"
#include "common/File.h"
#include "common/CMpTime.h"
#include "common/MpString.h"

// 日志级别
static const mp_int32 OS_LOG_DEBUG = 0;
static const mp_int32 OS_LOG_INFO = 1;
static const mp_int32 OS_LOG_WARN = 2;
static const mp_int32 OS_LOG_ERROR = 3;
static const mp_int32 OS_LOG_CRI = 4;

static const mp_uchar MAX_HEAD_SIZE = 16;
static const mp_uint32 MAX_MSG_SIZE = 4096000;  // 1024*1000*4
static const mp_uchar LOG_HEAD_LENGTH = 10;
static const mp_uchar DEFAULT_LOG_COUNT = 5;
static const mp_uint32 UNIT_K = 1024;
static const mp_uint32 UNIT_M = 1048576;
static const mp_uint32 DEFAULT_LOG_SIZE = 52428800;       // 50M
static const mp_uint32 MAX_LOG_FILE_SIZE = 52428800;      // 50M
static const mp_uint32 DEFAULT_LOG_KEEP_TIME = 15552000;  // 180 days
#define ADDLOG(x) ADDLLU(x)
#define ADDLLU(x) x##LLU
static const mp_uchar LOG_CACHE_ON = 1;
static const mp_uchar LOG_CACHE_OFF = 0;
static const mp_uchar LOG_CACHE_ASCII = 1;
static const mp_uchar LOG_CACHE_UNICODE = 0;
static const mp_uchar LOG_CACHE_MAXSIZE = 30;

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

#ifdef  WIN32
#define FUNCTION_NAME __FUNCTION__
#define WFUNCTION WIDEN(__FUNCTION__)
#elif defined SOLARIS
#define FUNCTION_NAME __FUNCTION__
#elif defined(_AIX)
#define FUNCTION_NAME __FUNCTION__
#else
#define FUNCTION_NAME __PRETTY_FUNCTION__
#endif
#define LOG_FUNCTION_NAME GetFunctionName(FUNCTION_NAME)
#define LOG_FUNCTION_NAMEW GetFunctionNameW(WFUNCTION)

#ifdef WIN32
#define LOGGUARD(pszFormat, ...) CLogGuard logGuard(__LINE__, __FUNCTION__, __FILE__, pszFormat, __VA_ARGS__)
#define COMMLOG(iLevel, pszFormat, ...)                                                                                \
    CLogger::GetInstance().Log(iLevel, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, __VA_ARGS__)
#define COMMLOGW(iLevel, pszFormat, ...)                                                                               \
    CLogger::GetInstance().LogW(iLevel, __LINE__, __WFILE__, LOG_FUNCTION_NAMEW, pszFormat, __VA_ARGS__)
#define DBGLOG(pszFormat, ...)                                                                                         \
    CLogger::GetInstance().Log(OS_LOG_DEBUG, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, __VA_ARGS__)
#define INFOLOG(pszFormat, ...)                                                                                        \
    CLogger::GetInstance().Log(OS_LOG_INFO, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, __VA_ARGS__)
#define WARNLOG(pszFormat, ...)                                                                                        \
    CLogger::GetInstance().Log(OS_LOG_WARN, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, __VA_ARGS__)
#define ERRLOG(pszFormat, ...)                                                                                         \
    CLogger::GetInstance().Log(OS_LOG_ERROR, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, __VA_ARGS__)
#elif defined SOLARIS
#define LOGGUARD(...) CLogGuard logGuard(__LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
#define COMMLOG(iLevel, ...) CLogger::GetInstance().Log(iLevel, __LINE__, __FILE__, LOG_FUNCTION_NAME, __VA_ARGS__)
#define DBGLOG(...) CLogger::GetInstance().Log(OS_LOG_DEBUG, __LINE__, __FILE__, LOG_FUNCTION_NAME, __VA_ARGS__)
#define INFOLOG(...) CLogger::GetInstance().Log(OS_LOG_INFO, __LINE__, __FILE__, LOG_FUNCTION_NAME, __VA_ARGS__)
#define WARNLOG(...) CLogger::GetInstance().Log(OS_LOG_WARN, __LINE__, __FILE__, LOG_FUNCTION_NAME, __VA_ARGS__)
#define ERRLOG(...) CLogger::GetInstance().Log(OS_LOG_ERROR, __LINE__, __FILE__, LOG_FUNCTION_NAME, __VA_ARGS__)
#else
#define COMMLOG(iLevel, pszFormat, args...)                                                                            \
    CLogger::GetInstance().Log(iLevel, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, ##args)
#define LOGGUARD(pszFormat, args...) CLogGuard logGuard(__LINE__, __FUNCTION__, __FILE__, pszFormat, ##args)
#define DBGLOG(pszFormat, args...)                                                                                     \
    CLogger::GetInstance().Log(OS_LOG_DEBUG, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, ##args)
#define INFOLOG(pszFormat, args...)                                                                                    \
    CLogger::GetInstance().Log(OS_LOG_INFO, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, ##args)
#define WARNLOG(pszFormat, args...)                                                                                    \
    CLogger::GetInstance().Log(OS_LOG_WARN, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, ##args)
#define ERRLOG(pszFormat, args...)                                                                                     \
    CLogger::GetInstance().Log(OS_LOG_ERROR, __LINE__, __FILE__, LOG_FUNCTION_NAME, pszFormat, ##args)
#endif


namespace Sensitive {
inline bool Match(const mp_string& patten, const mp_string& value)
{
    std::regex reg(patten);
    std::smatch match;
    std::regex_search(value, match, reg);
    return !match.empty();
}

static const mp_string FuzzPattern[25] = { "[Pp][Aa][Ss][Ss]",
    "[Pp][Ww][Dd]", "[Kk][Ee][Yy]", "[Cc][Rr][Yy][Pp][Tt][Oo]", "[Ss][Ee][Ss][Ss][Ii][Oo][Nn]",
    "[Tt][Oo][Kk][Ee][Nn]", "[Ff][Ii][Nn][Gg][Ee][Rr][Pp][Rr][Ii][Nn][Tt]", "[Aa][Uu][Tt][Hh]",
    "[Ee][Nn][Cc]", "[Dd][Ee][Cc]", "[Tt][Gg][Tt]", "[Ii][Qq][Nn]", "[Ii][Nn][Ii][Tt][Ii][Aa][Tt][Oo][Rr]",
    "[Ss][Ee][Cc][Rr][Ee][Tt]", "[Cc][Ee][Rr][Tt]", "^[Ss][Kk]$", "^[Ii][Vv]$", "[Ss][Aa][Ll][Tt]", "^[Mm][Kk]$",
    "[Pp][Rr][Ii][Vv][Aa][Tt][Ee]", "[Uu][Ss][Ee][Rr][_][Ii][Nn][Ff][Oo]", "[Ee][Mm][Aa][Ii][Ll]",
    "[Pp][Hh][Oo][Nn][Ee]", "[Rr][Aa][Nn][Dd]", "[Vv][Ee][Rr][Ff][Ii][Yy][Cc][Oo][Dd][Ee]" };
static const mp_int32 FuzzSize = sizeof(FuzzPattern) / sizeof(FuzzPattern[0]);

inline mp_string WipeSensitive(const mp_string& name, const mp_string& value)
{
    for (mp_size i = 0; i < FuzzSize; ++i) {
        if (Match(FuzzPattern[i], name)) {
            return "*";
        }
        if (Match(FuzzPattern[i], value)) {
            return "*";
        }
    }
    return value;
}

inline mp_bool ISensitive(const mp_string& name, const mp_string& value)
{
    for (mp_size i = 0; i < FuzzSize; ++i) {
        if (Match(FuzzPattern[i], name)) {
            return MP_TRUE;
        }
        if (Match(FuzzPattern[i], value)) {
            return MP_TRUE;
        }
    }
    return MP_FALSE;
}
}

#define DBGSTR(x) #x "=" << Sensitive::WipeSensitive(#x, x) << " "
#define DBG(x) #x "=" << (x) << " "

#define HEX(x) "0x" << std::hex << x << std::dec
#define HEXDBG(x) " " #x "=0x" << std::hex << x << std::dec
#define DPLOG(level) LogProxy(level, __FILE__, LOG_FUNCTION_NAME, __LINE__) << ""
#define DBGLOG_EX() DPLOG(OS_LOG_DEBUG)
#define INFOLOG_EX() DPLOG(OS_LOG_INFO)
#define WARNLOG_EX() DPLOG(OS_LOG_WARN)
#define ERRLOG_EX() DPLOG(OS_LOG_ERROR)

namespace {
typedef void (*CallbackWriteLogPtr)(int32_t level, const std::string& filePathName, int32_t lineNum,
    const std::string& funcName, const std::string& logString);
}

class AGENT_API CLogger {
public:
    ~CLogger();
    static CLogger& GetInstance();

    void Init(const char* pcLogFileName, const mp_string& strFilePath);
    mp_int32 SetLogLevel(mp_int32 iLogLevel);
    mp_int32 SetLogCount(mp_int32 iLogCount);
    // 打开日志缓存
    mp_void OpenLogCache();
    // 关闭日志缓存
    mp_int32 CloseLogCache();
    mp_void GetLogCfgInfo();
    mp_void LoadLogCfg();
    mp_void ReadLevelAndCount();
    mp_void ReadMaxSizeAndKeepTime();
    // 临时放到public,为了UT打桩方便
    mp_void ReadLogCacheThreshold();
    mp_string GetUserString();
    mp_string GetModuleName();
    mp_string FilterModleNamePath(const mp_string& str);
    mp_int32 GetLogMaxSize();
    mp_int32 GetLogCount();
    mp_int32 GetLogKeepTime();
    mp_int32 SwitchLogFile(
        const mp_string& pszLogPath, const mp_string& pszLogName, mp_int32 iLogCount, mp_time LogKeepTime);
    mp_int32 InitLogContent(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
        const mp_string& pszFuncction, const mp_string& pszFormat, va_list& pszArgp, std::ostringstream& strMsg);
    mp_void Log(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
        const mp_string& pszFuncction, const mp_string& pszFormat, ...);
#ifdef WIN32
    mp_void LogW(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_wstring& pszFileName,
        const mp_wstring& pszFuncction, const mp_wstring pszFormat, ...);
    mp_int32 ExecWinCmd(mp_string pszCmdBuf, mp_uint32& uiRetCode);
#endif
    mp_void SetWriteLogFunc(CallbackWriteLogPtr writeLogFunc);

private:
    CLogger();

    typedef struct _CacheData {
        mp_int32 codeType;
        mp_string logCache;
        mp_wstring logCacheW;
        mp_uint32 cacheLen;
    } CacheData;

    static CLogger m_instance;  // 单例对象
    // 日志类线程互斥变量
    thread_lock_t m_tLock;
    // 日志文件名
    mp_string m_strFileName;
    // 日志文件路径
    mp_string m_strFilePath;
    // 日志最大大小
    mp_int32 m_iMaxSize;
    // 日志记录的当前级别
    mp_int32 m_iLogLevel;
    // 日志个数
    mp_int32 m_iLogCount;
    // 日志保留时间
    mp_int32 m_iLogKeepTime;
    // 是否开启日志缓存功能
    mp_int32 m_cacheFlg;
    // 日志缓存阈值大小(单位字节，配置文件单位为M)
    mp_uint64 m_cacheThreshold;
    // 当前日志缓存大小(单位字节)
    mp_uint64 m_cacheSize;
    // 日志缓存
    std::queue<CacheData> m_cacheContent;
    // 日志缓存打开计数器
    mp_int32 m_OpenCacheNum;
    // 日志写函数，如果设置了日志函数，直接调用函数指针完成日志打印
    CallbackWriteLogPtr m_funcWriteLog;
    // 日志配置信息读取线程
    std::unique_ptr<std::thread> m_getLogCfgThread;
    // 日志配置信息读取线程停止条件变量
    std::condition_variable m_logCfgThreadCV;
    // 日志配置信息读取线程停止标志
    std::atomic<bool> m_stopFlag { false };
    // 日志配置信息读取线程停止条件变量互斥锁
    std::mutex m_logCfgThreadCVLock;

    mp_int32 MkHead(mp_int32 iLevel, mp_char pszHeadBuf[], mp_int32 iBufLen);
    mp_int32 HandleEachLogFile(const mp_string& OldLogNameStr, const mp_string& NewLogNameStr, mp_int32 LogNum,
        mp_int32 MaxLogCount, mp_time MaxKeepTime);
    mp_int32 HandleCurrentLogFile(const mp_string& LogPath, const mp_string& LogName);
    mp_int32 CreateLogFile(const mp_string& pszLogFile);
    EXTER_ATTACK FILE* GetLogFile();
    mp_bool IsInAgent();
    mp_int32 GetTestCfgInfo(mp_string& strSceneType);
    mp_int32 GetInAgentLogLevel(mp_string& logLevelInfo);
    mp_void StrReplaceSpecSymbols(mp_string &logContent);
#ifdef WIN32
    mp_wstring GetUserStringW();
    mp_wstring FilterModleNamePath(const mp_wstring& str);
    mp_int32 InitLogContentW(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_wstring& pszFileName,
        const mp_wstring& pszFuncction, const mp_wstring pszFormat, va_list& pszArgp, std::wostringstream& strMsg);
    mp_void WriteLog2Cache(std::wostringstream& strWMsg);
#endif

    mp_void WriteLog2Cache(std::ostringstream& strMsg);
    mp_int32 ZipLogFile(const mp_string& strCommand, const mp_string& strLogFile);
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
inline mp_wstring GetFunctionNameW(const mp_wstring& prettyFunction)
{
    std::size_t end = prettyFunction.find(L"(");
    if (end == std::string::npos) {
        return prettyFunction;
    }
    std::size_t begin = prettyFunction.substr(0, end).rfind(L" ");
    if (begin == std::string::npos) {
        begin = 0;
    } else {
        begin = begin + 1;
    }
    end = end - begin;
    return prettyFunction.substr(begin, end) + L"()";
}

class AGENT_API CLogGuard {
public:
    CLogGuard(mp_int32 iLine, const mp_string& strFunctionName,
        const mp_string& strFileName, const mp_string& pszFormat, ...);
    ~CLogGuard();

private:
    mp_int32 m_iLine;
    mp_string m_strFunctionName;
    mp_string m_strFileName;
};

class LogProxy : public std::ostringstream {
public:
    LogProxy(const int level, const mp_string& fileName, const mp_string& funcnName, const unsigned int line)
        : mLogLevel(level), mFileName(fileName), mRequestID(0), mFuncName(funcnName), mLine(line)
    {}
    virtual ~LogProxy()
    {
        mp_string msg = str();
        std::replace(msg.begin(), msg.end(), '%', ' ');

        CLogger::GetInstance().Log(mLogLevel, mLine, mFileName.c_str(), mFuncName.c_str(), msg);
    }

private:
    int mLogLevel;
    mp_string mFileName;
    uint64_t mRequestID;
    mp_string mFuncName;
    unsigned int mLine;
};

#endif  // _AGENT_LOG_H_
