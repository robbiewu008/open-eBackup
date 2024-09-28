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
#include "Dlib.h"
#include "securec.h"
#include "define/Types.h"
#include "log/Log.h"
#include "common/Utils.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

namespace Module {

using namespace std;

handle_t DlibOpen(const string& pszLibName)
{
    LOGGUARD("Lib name is %s", pszLibName.c_str());
#ifdef WIN32
    handle_t h_dlib = LoadLibrary(pszLibName.c_str());
    return h_dlib;
#else
    return DlibOpenEx(pszLibName, true);
#endif
}

/* ------------------------------------------------------------
Description  :打开lib
Input        : pszLibName -- lib名
------------------------------------------------------------- */
handle_t DlibOpenEx(const string& pszLibName, bool bLocal)
{
    LOGGUARD("Lib name is %s", pszLibName.c_str());
#ifdef WIN32
    return DlibOpen(pszLibName);
#else
    /*lint -e835*/
    int flag = bLocal ? DFLG_LOCAL : DFLG_GLOBAL;
    // Coverity&Fortify误报:FORTIFY.Process_Control
    // pszLibName引用出传递都是绝对路径
    return dlopen(pszLibName.c_str(), flag);
#endif
}

/* ------------------------------------------------------------
Description  :关闭lib
Input        : hDlib -- 打开lib时的句柄
------------------------------------------------------------- */
void DlibClose(handle_t hDlib)
{
    LOGGUARD("Close Lib");
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
#ifdef WIN32
AGENT_API void* DlibDlsym(handle_t hDlib, const string& pszFname)
#else
void* DlibDlsym(handle_t hDlib, const string& pszFname)
#endif
{
    LOGGUARD("Function name is %s", pszFname.c_str());
#ifdef WIN32
    return GetProcAddress(hDlib, pszFname.c_str());
#else
    return dlsym(hDlib, pszFname.c_str());
#endif
}

/* ------------------------------------------------------------
Description  :取得lib里方法执行出错信息
------------------------------------------------------------- */
#ifdef WIN32
AGENT_API const char* DlibError(char szMsg[], uint32_t isz)
#else
const char* DlibError(char szMsg[], uint32_t isz)
#endif
{
#ifdef WIN32
    int iErr = GetOSError();
    return GetOSStrErr(iErr, szMsg, isz);
#else

    CThreadAutoLock cLock(&DlibErrorMutex::GetInstance().GetValue());
    char* errMsg = dlerror();
    if (errMsg == nullptr) {
        return nullptr;
    } 
    const string pszErr = errMsg;

    if (pszErr.empty()) {
        szMsg[0] = 0;
        return nullptr;
    }

    /*lint -e713*/
    int iRet = strncpy_s(szMsg, isz, pszErr.c_str(), isz - 1);
    if (iRet != EOK) {
        return nullptr;
    }
    szMsg[isz - 1] = 0;
    return szMsg;
#endif
}

} // namespace Module
