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
#include "plugins/ServicePlugin.h"
#include <vector>
#include "common/Log.h"
#include "common/ErrorCode.h"

using namespace std;

// ------------------------------------------------------------
// Description  : 初始化插件，实现IPlugin接口方
// Return       : MP_SUCCESS -- 成功
//                 非MP_SUCCESS -- 失败，返回特定错误码
// Create By    : yangwenjun 00275736
// -------------------------------------------------------------
mp_int32 CServicePlugin::Initialize(vector<mp_uint32> &cmds)
{
    COMMLOG(OS_LOG_DEBUG, "Begin initialize service plugin.");
    // 调用具体插件Init重载方法进行各插件初始化
    mp_int32 iRet = Init(cmds);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_DEBUG, "Invoke Init failed failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Initialize service plugin succ.");
    return MP_SUCCESS;
}

mp_int32 CServicePlugin::Destroy()
{
    return MP_SUCCESS;
}

mp_void CServicePlugin::SetOption(mp_string pszName, mp_string pvVal)
{
    (mp_void)pszName;
    (mp_void)pvVal;
    return;
}

mp_bool CServicePlugin::GetOption(mp_string pszName, mp_string& pvVal)
{
    (mp_void)pszName;
    (mp_void)pvVal;
    return MP_TRUE;
}

mp_void* CServicePlugin::CreateObject(mp_string pszName)
{
    (mp_void)pszName;
    return NULL;
}

mp_int32 CServicePlugin::GetClasses(IPlugin::DYN_CLASS& pClasses, mp_int32 sz)
{
    (mp_void)pClasses;
    (mp_void)sz;
    return MP_SUCCESS;
}

mp_string CServicePlugin::GetName()
{
    return "";
}

mp_string CServicePlugin::GetVersion()
{
    return "";
}

std::size_t CServicePlugin::GetSCN()
{
    return 0;
}

// ------------------------------------------------------------
// Description  : 初始化插件，需要进行初始化操作的插件类实现该方法
// Return       : MP_SUCCESS -- 成功
//                 非MP_SUCCESS -- 失败，返回特定错误码
// Create By    : yangwenjun 00275736
// -------------------------------------------------------------
mp_int32 CServicePlugin::Init(vector<mp_uint32> &cmds)
{
    return MP_SUCCESS;
}

// ------------------------------------------------------------
// Description  : 调用插件，执行请求
// Input        : req -- 请求消息
// Output       : rsp -- 响应消息
// Return       : MP_SUCCESS -- 成功
//                 非MP_SUCCESS -- 失败，返回特定错误码
// Create By    : yangwenjun 00275736
// -------------------------------------------------------------
mp_int32 CServicePlugin::Invoke(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_int32 iRet;

    COMMLOG(OS_LOG_DEBUG, "Begin invoke service plugin by rest channel.");
    iRet = DoAction(req, rsp);
    Json::Value& rspData = rsp.GetJsonValueRef();
    if (iRet != MP_SUCCESS) {
        rsp.SetRetCode((mp_int64)iRet);
        rspData["errorCode"] = (Json::UInt64)rsp.GetRetCode();
        COMMLOG(OS_LOG_DEBUG, "Do rest action failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "Invoke service plugin succ");
    return MP_SUCCESS;
}

// ------------------------------------------------------------
// Description  : 调用插件，执行请求
// Input        : req -- 请求消息
// Output       : rsp -- 响应消息
// Return       : MP_SUCCESS -- 成功
//                 非MP_SUCCESS -- 失败，返回特定错误码
// Create By    : yangwenjun 00275736
// -------------------------------------------------------------
mp_int32 CServicePlugin::Invoke(CDppMessage& req, CDppMessage& rsp)
{
    COMMLOG(OS_LOG_DEBUG, "Begin invoke service plugin by tcp channel.");
    mp_int32 iRet = DoAction(req, rsp);
    if (iRet != MP_SUCCESS) {
        rsp.SetRetCode((mp_int64)iRet);
        COMMLOG(OS_LOG_DEBUG, "Do request action failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "Invoke service plugin succ");
    return MP_SUCCESS;
}
