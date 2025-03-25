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
#ifndef MOUDLE_UTILS_H
#define MOUDLE_UTILS_H

#include "define/Defines.h"
#include "define/Types.h"
#include "common/Thread.h"
#include <vector>
#include <set>
#ifndef WIN32
#include <dlfcn.h>
#endif

namespace Module {

AGENT_API int SafeStoi(const std::string& str, int defaultValue = FAILED);
AGENT_API long SafeStol(const std::string& str, long defaultValue = FAILED);
AGENT_API long long SafeStoll(const std::string& str, long long defaultValue = FAILED);
AGENT_API float SafeStof(const std::string& str, float defaultValue = FAILED);
AGENT_API unsigned long long SafeStoUll(const std::string& str, unsigned long long defaultValue = FAILED);

// 不要直接使用signal注册信号
typedef void (*signal_proc)(int);

AGENT_API void DoSleep(uint32_t ms);

AGENT_API int CheckCmdDelimiter(std::string& str);
AGENT_API int GetOSError();

AGENT_API char* GetOSStrErr(int err, char buf[], std::size_t buf_len);

AGENT_API int GetCurrentUserName(std::string& strUserName, unsigned long& iErrCode);
#ifdef WIN32
AGENT_API int GetCurrentUserNameWin(std::string& strUserName, unsigned long& iErrCode);
#elif defined SOLARIS
AGENT_API int GetCurrentUserNameSol(std::string& strUserName, unsigned long& iErrCode);
#else
AGENT_API int GetCurrentUserNameOther(std::string& strUserName, unsigned long& iErrCode);
#endif

#ifndef WIN32
AGENT_API int GetUidByUserName(std::string strUserName, int& uid, int& gid);
AGENT_API int ChownFile(std::string strFileName, int uid, int gid);
AGENT_API int ChmodFile(std::string strFileName, uint32_t mode);
#endif

AGENT_API size_t GetMachineId();


AGENT_API const std::string BaseFileName(const std::string& pszFileName);

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
            pObj = NULL;                                                                                               \
        }                                                                                                              \
        if (!pObj) {                                                                                                   \
            COMMLOG(OS_LOG_ERROR, "pObj is NULL.");                                                                    \
            return FAILED;                                                                                          \
        }                                                                                                              \
    } while (0);

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
    } while (0);

#define NEW_ARRAY_CATCH_RETURN_FAILED(pObj, CobjClassName, iNum) do                                                    \
    {                                                                                                                  \
        try {                                                                                                          \
            pObj = new CobjClassName[iNum];                                                                                \
        } catch (...) {                                                                                                    \
            COMMLOG(OS_LOG_ERROR, "New %s failed", #CobjClassName);                                                        \
            pObj = NULL;                                                                                                   \
        }                                                                                                                  \
        if (!pObj) {                                                                                                       \
            COMMLOG(OS_LOG_ERROR, "pObj is NULL.");                                                                        \
            return FAILED;                                                                                              \
        }                                                                                                                  \
    } while (0);

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
    thread_lock_t lockVal;
};

} // namespace Module

#endif // MOUDLE_UTILS_H