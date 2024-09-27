#ifndef AGENT_UTILS_H
#define AGENT_UTILS_H
#include <vector>
#include <set>
#include <memory>
#ifndef WIN32
#include <dlfcn.h>
#endif

#include "common/Types.h"
#include "common/Defines.h"
#include "common/CMpThread.h"
#include "common/Log.h"

#ifdef SANCLIENT_AGENT
static const mp_string AGENT_RUNNING_USER = "sanclient";
static const mp_string EXAGENT_RUNNING_USER = "exrdadmin";
#else
static const mp_string AGENT_RUNNING_USER = "rdadmin";
static const mp_string EXAGENT_RUNNING_USER = "exrdadmin";
#endif
#ifdef WIN32
static const mp_string AGENT_ROOT_USER = "local_system";
#else
static const mp_string AGENT_ROOT_USER = "root";
#endif
static const mp_int32 WINDOWS_USERNAME_LEN = 512;
static const mp_int32 LINUX_USERNAME_LEN = 1024;
static const mp_uchar HOST_OS_UNKNOWN = 0;
static const mp_uchar HOST_OS_WINDOWS = 1;
static const mp_uchar HOST_OS_REDHAT = 2;
static const mp_uchar HOST_OS_HP_UX = 3;
static const mp_uchar HOST_OS_SOLARIS = 4;
static const mp_uchar HOST_OS_AIX = 5;
static const mp_uchar HOST_OS_SUSE = 6;
static const mp_uchar HOST_OS_ORACLE_LINUX = 7;
static const mp_uchar HOST_OS_OTHER_LINUX = 8;

// task step usage
static const mp_int32 TASK_STEP_INTERVAL_FIVE = 5;
static const mp_int32 TASK_STEP_INTERVAL_TEN = 10;
static const mp_int32 TASK_STEP_INTERVAL_FIFTEEN = 15;
static const mp_int32 TASK_STEP_INTERVAL_TWENTY = 20;
static const mp_int32 TASK_STEP_INTERVAL_EIGHTY = 80;
static const mp_int32 TASK_STEP_INTERVAL_NINETY = 90;
static const mp_int32 TASK_STEP_INTERVAL_HUNDERED = 100;

// vm backup level
static const mp_uint32 VMWARE_VM_INCR_BACKUP = 1;
static const mp_uint32 VMWARE_VM_FULL_BACKUP = 2;

// vddk version usage
static const mp_string AVAILABLE_STRING = "0123456789.";
static const mp_string DIGITAL_STRING = "0123456789";
static const mp_int32 TARGET_STRLEN = 3;

// common sleep config
static const int COMMON_LOW_RETRY_TIMES = 3;
static const int COMMON_LOW_RETRY_INTERVAL = 1000;     // 1000 ms
static const int COMMON_MID_RETRY_TIMES = 12;
static const int COMMON_MID_RETRY_INTERVAL = 5000;     // 5000 ms = 5s
static const int COMMON_HIGH_RETRY_TIMES = 30;
static const int COMMON_HIGH_RETRY_INTERVAL = 30000;     // 30000 ms = 30s

// Host deploy env
const mp_string HOST_ENV_DEPLOYTYPE_A8000 = "a8000"; // a8000
const mp_string HOST_ENV_DEPLOYTYPE_X8000 = "d0";    // x8000
const mp_string HOST_ENV_DEPLOYTYPE_X6000 = "d1";    // x6000
const mp_string HOST_ENV_DEPLOYTYPE_X3000 = "d2";    // x3000
const mp_string HOST_ENV_DEPLOYTYPE_CLOUD_BACKUP = "d3";         // cloud_backup
const mp_string HOST_ENV_DEPLOYTYPE_HYPERDETECT = "hyperdetect"; // 防勒索, 属于d4
const mp_string HOST_ENV_DEPLOYTYPE_HYPERDETECT_NO_BRAND = "d4"; // 白牌防勒索
const mp_string HOST_ENV_DEPLOYTYPE_HYPERDETECT_CYBER_ENGINE = "d5"; // 安全一体机
const mp_string HOST_ENV_DEPLOYTYPE_X9000 = "d6";                    // x9000
const mp_string HOST_ENV_DEPLOYTYPE_E6000 = "d7";             // e6000, 分布式一体机
const mp_string HOST_ENV_DEPLOYTYPE_DATABACKUP = "d8";             // 软硬解耦, dependent

// host install type
const mp_int32 AGENT_INSTALL_TYPE_EXTERNAL = 0;
const mp_int32 AGENT_INSTALL_TYPE_INTERNAL = 1;

// label
static const mp_string DATATURBO_FAILED_LABEL = "agent_execute_mount_dataturbo_fail_label";

static const mp_string AGENT_ACCESS_REMOTE_STORAGE_FAILED_LABEL = "agent_access_remote_storage_fail_label";

// awk column
const mp_int32 AWK_COL_LAST_3 = -3;
const mp_int32 AWK_COL_LAST_2 = -2;
const mp_int32 AWK_COL_LAST_1 = -1;
const mp_int32 AWK_COL_FIRST_0 = 0;
const mp_int32 AWK_COL_FIRST_1 = 1;
const mp_int32 AWK_COL_FIRST_2 = 2;
const mp_int32 AWK_COL_FIRST_3 = 3;
const mp_int32 AWK_COL_FIRST_4 = 4;
const mp_int32 AWK_COL_FIRST_5 = 5;
const mp_int32 AWK_COL_FIRST_6 = 6;
const mp_int32 AWK_COL_FIRST_7 = 7;
const mp_int32 AWK_COL_FIRST_8 = 8;
const mp_int32 AWK_COL_FIRST_9 = 9;

// 获取用户UUID的互斥类
class UuidByUNameMutex {
public:
    static UuidByUNameMutex& GetInstance()
    {
        static UuidByUNameMutex uuidByUNameMutex;
        return uuidByUNameMutex;
    }

    thread_lock_t& GetValue()
    {
        return lockVal;
    }

private:
    UuidByUNameMutex()
    {
        CMpThread::InitLock(&lockVal);
    }

    ~UuidByUNameMutex()
    {
        CMpThread::DestroyLock(&lockVal);
    }

private:
    thread_lock_t lockVal;
};
// 获取lib的错误信息互斥类
class DlibErrorMutex {
public:
    static DlibErrorMutex& GetInstance()
    {
        static DlibErrorMutex dlibErrorMutex;
        return dlibErrorMutex;
    }

    thread_lock_t& GetValue()
    {
        return lockVal;
    }

private:
    DlibErrorMutex()
    {
        CMpThread::InitLock(&lockVal);
    }

    ~DlibErrorMutex()
    {
        CMpThread::DestroyLock(&lockVal);
    }

private:
    thread_lock_t lockVal;
};

// 不要直接使用signal注册信号
typedef mp_void (*signal_proc)(mp_int32);
using Defer = std::shared_ptr<void>;
AGENT_API mp_int32 SignalRegister(mp_int32 signo, signal_proc func);
AGENT_API mp_void DoSleep(mp_uint32 ms);
AGENT_API mp_int32 CheckCmdDelimiter(const mp_string& str);
AGENT_API mp_int32 GetOSError();
AGENT_API mp_char* GetOSStrErr(mp_int32 err, mp_char buf[], mp_size buf_len);
AGENT_API mp_int32 InitCommonModules(const mp_string& pszFullBinPath);
AGENT_API mp_int32 GetHostName(mp_string& strHostName);
AGENT_API mp_uint32 MakeLogUpperBound(mp_uint64 value);

// 动态库操作相关
#define DFLG_LOCAL (RTLD_NOW | RTLD_LOCAL)
#define DFLG_GLOBAL (RTLD_NOW | RTLD_GLOBAL)
AGENT_API mp_handle_t DlibOpen(const mp_string& pszLibName);
AGENT_API mp_handle_t DlibOpenEx(const mp_string& pszLibName, mp_bool bLocal);
AGENT_API mp_void DlibClose(mp_handle_t hDlib);
AGENT_API mp_void* DlibDlsym(mp_handle_t hDlib, const mp_string& pszFname);
AGENT_API const mp_char* DlibError(mp_char szMsg[], mp_uint32 isz);
AGENT_API mp_int32 GetCurrentUserName(mp_string& strUserName, mp_ulong& iErrCode);
#ifdef WIN32
AGENT_API mp_int32 GetCurrentUserNameWin(mp_string& strUserName, mp_ulong& iErrCode);
#elif defined SOLARIS
AGENT_API mp_int32 GetCurrentUserNameSol(mp_string& strUserName, mp_ulong& iErrCode);
#else
AGENT_API mp_int32 GetCurrentUserNameOther(mp_string& strUserName, mp_ulong& iErrCode);
#endif
AGENT_API const mp_string BaseFileName(const mp_string& pszFileName);
AGENT_API mp_void RemoveFullPathForLog(mp_string strCmd, mp_string& strLogCmd);

#ifdef WIN32
AGENT_API mp_int32 GetCurrentUserNameW(mp_wstring& strUserName, mp_ulong& iErrCode);
AGENT_API const mp_wstring BaseFileNameW(const mp_wstring& pszFileName);
#else
AGENT_API mp_int32 GetUidByUserName(const mp_string& strUserName, mp_int32& uid, mp_int32& gid);
AGENT_API mp_int32 ChownFile(const mp_string& strFileName, const mp_int32& uid, mp_int32& gid);
AGENT_API mp_int32 ChmodFile(const mp_string& strFileName, mp_uint32 mode);
AGENT_API mp_int32 GetFileOwnerName(const mp_string& strFileName, mp_string& owner);
#endif
AGENT_API std::vector<mp_string> GrepE(const std::vector<mp_string>& vecInput, const mp_string& strCondition);
AGENT_API std::vector<mp_string> GrepE(const std::vector<mp_string>& vecInput,
    const std::vector<mp_string>& vecCondition);
AGENT_API std::vector<mp_string> GrepV(const std::vector<mp_string>& vecInput,
    const std::vector<mp_string>& vecCondition);
AGENT_API std::vector<mp_string> GrepV(const std::vector<mp_string>& vecInput, const mp_string& strCondition);
AGENT_API std::vector<mp_string> GrepW(const std::vector<mp_string>& vecInput, const mp_string& strCondition);
AGENT_API std::vector<mp_string> Awk(const std::vector<mp_string>& vecInput, int nPrint, char cSep = ' ');
AGENT_API std::vector<mp_string> Awk(const std::vector<mp_string>& vecInput,
    int nPos, const mp_string& strCondition, int nPrint, char cSep = ' ');

#ifdef WIN32
AGENT_API mp_int32 SendIOControl(const mp_string& strPath, DWORD dwCmd, mp_void* pData, mp_uint32 uiLen);
#else
AGENT_API mp_int32 SendIOControl(const mp_string& strPath, mp_int32 iCmd, mp_void* pData, mp_uint32 uiLen);
#endif

// 以下临时使用固定函数实现，后续开源软件选型后采用开源软件优化
AGENT_API mp_int32 CheckParamString(const mp_string& paramValue,
    mp_int32 lenBeg, mp_int32 lenEnd, const mp_string& strInclude, const mp_string& strExclude);
AGENT_API mp_int32 CheckParamString(const mp_string& paramValue,
    mp_int32 lenBeg, mp_int32 lenEnd, const mp_string& strPre);
AGENT_API mp_int32 CheckParamStringEnd(
    mp_string& paramValue, mp_int32 lenBeg, mp_int32 lenEnd, const mp_string& strEnd = mp_string());
AGENT_API mp_int32 CheckParamInteger32(mp_int32 paramValue, mp_int32 begValue, mp_int32 endValue,
    const std::vector<mp_int32>& vecExclude = std::vector<mp_int32>());
AGENT_API mp_int32 CheckParamInteger64(
    mp_int64 paramValue, mp_int64 begValue, mp_int64 endValue, std::vector<mp_int64>& vecExclude);
AGENT_API mp_int32 CheckParamStringIsIP(const mp_string& paramValue);
AGENT_API mp_int32 CheckParamStringIsIPv6(const mp_string& paramValue);
AGENT_API mp_int32 CheckIsIpv6(const mp_string& strIpv6, const mp_string& paramValue);
AGENT_API mp_int32 CheckPathString(mp_string& pathValue);
AGENT_API mp_int32 CheckPathString(mp_string& pathValue, const mp_string& strPre);
AGENT_API mp_int32 CheckPathTraversal(mp_string& pathValue);
AGENT_API mp_int32 CheckFileSysMountParam(const mp_string& strDeviceName,
    mp_int32 volumeType, const mp_string& strMountPoint);
#ifdef WIN32
AGENT_API mp_int32 CheckFileSysMountParamWin32(mp_string strDeviceName, mp_int32 volumeType, mp_string strMountPoint);
#endif
AGENT_API mp_int32 CheckFileSysFreezeParam(const mp_string& strDiskNames);

AGENT_API mp_bool IsHuweiStorage(mp_string arrayVendor);
// 敏感信息匿名处理
AGENT_API mp_int32 WipeSensitiveForJsonData(const mp_string& rawBuffer, mp_string& strValue);
AGENT_API mp_void WipeJsonStringValue(Json::Value& v, const std::set<mp_string>& excludedKey = std::set<mp_string>());

AGENT_API void ClearString(mp_string& strValue);
AGENT_API bool GetNginxListenIP(mp_string& strIP, mp_int32& nPort);
AGENT_API mp_int32 CheckParamValid(mp_string& param);
AGENT_API mp_string CheckParamInvalidReplace(mp_string& param);
AGENT_API mp_int32 CheckEscapeCht(const mp_string& param);
AGENT_API mp_int32 CheckIpAddressValid(mp_string& ipAddress);
AGENT_API mp_int32 CalibrationFormatString(mp_string &params);
AGENT_API mp_int32 CalibrationFormatTaskId(mp_string &params);
AGENT_API mp_int32 CalibrationFormatErrDetail(mp_string &params);
AGENT_API mp_int32 CalibrationFormatStorProtocol(mp_uint64 params);
AGENT_API mp_int32 ModifyLineData(const mp_string& fileName, const mp_string& key, const mp_string& value);

#ifndef WIN32
AGENT_API mp_int32 ChangeGmonDir();
AGENT_API mp_int32 GetProgressInfo(std::vector<mp_string> vecResult);
#endif
#define NEW_CATCH(pObj, CobjClassName)                                                                                 \
    try {                                                                                                              \
        pObj = new CobjClassName;                                                                                      \
    } catch (...) {                                                                                                    \
        COMMLOG(OS_LOG_ERROR, "New %s failed", #CobjClassName);                                                        \
        pObj = NULL;                                                                                                   \
    }

#define NEW_CATCH_RETURN_FAILED(pObj, CobjClassName) do                                                                \
    {                                                                                                                  \
        try {                                                                                                          \
            pObj = new CobjClassName;                                                                                  \
        } catch (...) {                                                                                                \
            COMMLOG(OS_LOG_ERROR, "New %s failed", #CobjClassName);                                                    \
            if (pObj) {                                                                                                \
                delete pObj;                                                                                           \
            }                                                                                                          \
            pObj = NULL;                                                                                               \
        }                                                                                                              \
        if (!pObj) {                                                                                                   \
            COMMLOG(OS_LOG_ERROR, "pObj is NULL.");                                                                    \
            return MP_FAILED;                                                                                          \
        }                                                                                                              \
    } while (0)

#define NEW_ARRAY_CATCH(pObj, CobjClassName, iNum)                                                                     \
    try {                                                                                                              \
        pObj = new CobjClassName[iNum];                                                                                \
    } catch (...) {                                                                                                    \
        COMMLOG(OS_LOG_ERROR, "New %s failed", #CobjClassName);                                                        \
        pObj = NULL;                                                                                                   \
    }

#define DELETE_AND_SET_NULL(obj) do                                                                                    \
    {                                                                                                                  \
        delete[] obj;                                                                                                  \
        obj = NULL;                                                                                                    \
    } while (0)

#define NEW_ARRAY_CATCH_RETURN_FAILED(pObj, CobjClassName, iNum) do \
    {   \
        try {   \
            pObj = new CobjClassName[iNum]; \
        } catch (...) { \
            COMMLOG(OS_LOG_ERROR, "New %s failed", #CobjClassName); \
            pObj = NULL;    \
        }   \
        if (!pObj) {    \
            COMMLOG(OS_LOG_ERROR, "pObj is NULL."); \
            return MP_FAILED;   \
        }   \
    } while (0)

#define MEMORY_GUARD(p) std::shared_ptr<void> p##p((p), [](void* (p)) {                                                \
    delete p;                                                                                                         \
    p = nullptr;                                                                                                      \
})

#endif  // __AGENT_UTILS_H__
