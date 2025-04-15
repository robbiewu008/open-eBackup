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
#ifndef AGENT_SERVICE_PLUGIN_H
#define AGENT_SERVICE_PLUGIN_H

#include <map>
#include <vector>

#include "common/Log.h"
#include "common/Types.h"
#include "message/rest/message_process.h"
#include "message/tcp/CDppMessage.h"
#include "message/Message.h"
#include "plugins/DataPathProcessClient.h"
#include "common/ErrorCode.h"

// rest请求的注册函数
template<class T>
class CRestActionMap {
public:
    typedef mp_int32 (T::*ACT)(CRequestMsg&, CResponseMsg&);
    typedef struct tag_rest_action {
        mp_string url;
        mp_string method;
        ACT action;
        tag_rest_action& operator=(tag_rest_action&& other)
        {
            url = std::move(other.url);
            method = std::move(other.method);
            action = other.action;
            other.action = nullptr;
            return *this;
        }
        tag_rest_action& operator=(const tag_rest_action& other)
        {
            url = other.url;
            method = other.method;
            action = other.action;
            return *this;
        }
    } rest_action_t;
    typedef std::map<mp_string, rest_action_t> ACTIONS_MAP;

public:
    CRestActionMap(){};
    virtual ~CRestActionMap(){};

    mp_void Add(const mp_string& pszUrl, const mp_string& pszHttpMethod, ACT act)
    {
        rest_action_t restAction;
        mp_string strKey;

        strKey = pszUrl;
        strKey += "_";
        strKey += pszHttpMethod;
        restAction.url = pszUrl;
        restAction.method = pszHttpMethod;
        restAction.action = act;
        m_mapActions[strKey] = std::move(restAction);
        INFOLOG("Add  action, key = %s.", strKey.c_str());
    }

    virtual mp_int32 GetAction(mp_string& strUrl, const mp_string& strMethod, rest_action_t& restAction)
    {
        mp_string strKey = strUrl + "_" + strMethod;
        DBGLOG("Get action, key = %s.", strKey.c_str());
        typename std::map<mp_string, rest_action_t>::iterator iter = m_mapActions.find(strKey);
        if (iter == m_mapActions.end()) {
            return MP_FAILED;
        }

        restAction = iter->second;
        return MP_SUCCESS;
    }

    mp_void PrintMap()
    {
    }

protected:
    ACTIONS_MAP m_mapActions;
};

// dpp协议的请求函数
template<class T>
class CDppActionMap {
public:
    typedef mp_int32 (T::*ACT)(CDppMessage&, CDppMessage&);
    typedef struct tag_dpp_action {
        mp_uint32 manageCmd;
        mp_string method;
        ACT action;
        tag_dpp_action& operator=(tag_dpp_action&& other)
        {
            manageCmd = std::move(other.manageCmd);
            method = std::move(other.method);
            action = other.action;
            other.action = nullptr;
            return *this;
        }
        tag_dpp_action& operator=(const tag_dpp_action& other)
        {
            manageCmd = other.manageCmd;
            method = other.method;
            action = other.action;
            return *this;
        }
    } dpp_action_t;
    typedef std::map<mp_uint32, dpp_action_t> ACTIONS_MAP;

public:
    CDppActionMap(){};
    ~CDppActionMap(){};

    mp_void Add(mp_uint32 manageCmd, ACT act, const mp_string funcName)
    {
        dpp_action_t dppAction = {manageCmd, funcName, act};
        m_mapActions[manageCmd] = std::move(dppAction);
    }

    mp_int32 GetAction(mp_uint32 manageCmd, dpp_action_t& dppAction)
    {
        typename std::map<mp_uint32, dpp_action_t>::iterator iter = m_mapActions.find(manageCmd);
        if (iter == m_mapActions.end()) {
            return MP_FAILED;
        }

        dppAction = iter->second;
        return MP_SUCCESS;
    }

    mp_void PrintMap()
    {
    }

private:
    ACTIONS_MAP m_mapActions;
};

// 业务插件基础类，具体插件实现继承该类
class CServicePlugin : public IPlugin {
public:
    enum { APP_PUGIN_ID = 2001 };

public:
    // 插件框架动态升级功能预留
    mp_int32 GetTypeId() const
    {
        return APP_PUGIN_ID;
    }
    mp_int32 Initialize(std::vector<mp_uint32>& cmds);
    mp_int32 Destroy();
    mp_void SetOption(mp_string pszName, mp_string pvVal);
    mp_bool GetOption(mp_string pszName, mp_string& pvVal);
    mp_void* CreateObject(mp_string pszName);
    mp_int32 GetClasses(IPlugin::DYN_CLASS& pClasses, mp_int32 sz);
    mp_string GetName();
    mp_string GetVersion();
    std::size_t GetSCN();

    // 通过rest命令调用插件，执行请求
    virtual mp_int32 Invoke(CRequestMsg& req, CResponseMsg& rsp);
    // 通过tcp命令调用插件，执行请求
    virtual mp_int32 Invoke(CDppMessage& req, CDppMessage& rsp);
    // 插件类需要实现该方法进行初始化
    virtual mp_int32 Init(std::vector<mp_uint32>& cmds);
    // 具体插件实现类需要实现rest处理方法
    virtual mp_int32 DoAction(CRequestMsg& req, CResponseMsg& rsp)
    {
        return MP_SUCCESS;
    }
    // 具体插件实现类需要实现tcp处理方法
    virtual mp_int32 DoAction(CDppMessage& req, CDppMessage& rsp)
    {
        return MP_SUCCESS;
    }
};

// 插件注册宏定义
// 注册插件
#define REGISTER_PLUGIN(clsname)                                                                                       \
    extern "C" AGENT_EXPORT IPlugin* QueryInterface()                                                              \
    {                                                                                                              \
        COMMLOG(OS_LOG_INFO, "Create new plugin obj %s.", #clsname);                                               \
        return new (std::nothrow) clsname();                                                                       \
    }                                                                                                              \
    static CRestActionMap<clsname> restActionMap;                                                                  \
    static CDppActionMap<clsname> dppActionMap

// 注册插件rest的Action
#define REGISTER_ACTION(url, httpmethod, act) restActionMap.Add(url, httpmethod, act)
// 注册插件dpp的Action
#define REGISTER_DPP_ACTION(manageCmd, act) dppActionMap.Add(manageCmd, act, #act)

// 获取注册rest的Action
#define GET_REQUEST_ACTION(url, http_method, restaction)                                                               \
    do {                                                                                                               \
        mp_int32 iRetx = restActionMap.GetAction(url, http_method, restaction);                                        \
        if (MP_SUCCESS != iRetx) {                                                                                     \
            COMMLOG(OS_LOG_ERROR, "Unimplement action, url %s, http method %s.", url.c_str(), http_method.c_str());    \
            return ERROR_COMMON_FUNC_UNIMPLEMENT;                                                                      \
        }                                                                                                              \
    } while (0)

// 执行rest请求的特定Action
#define DO_ACTION(clsname, req, rsp)                                                                                   \
    do {                                                                                                               \
        mp_string strUrl;                                                                                              \
        mp_string strMethod;                                                                                           \
        CRestActionMap<clsname>::rest_action_t restAction;                                                             \
        DBGLOG("Begin do rest action.");                                                                               \
        restActionMap.PrintMap();                                                                                      \
        strUrl = req.GetURL().GetProcURL();                                                                            \
        strMethod = req.GetHttpReq().GetMethod();                                                                      \
        GET_REQUEST_ACTION(strUrl, strMethod, restAction);                                                             \
        mp_int32 iRet = (this->*restAction.action)(req, rsp);                                                          \
        if (MP_SUCCESS != iRet) {                                                                                      \
            ERRLOG("Do rest action failed, url %s, method %s, iRet %d.", strUrl.c_str(), strMethod.c_str(), iRet);     \
            return iRet;                                                                                               \
        }                                                                                                              \
        DBGLOG("Do rest action succ.");                                                                                \
        return MP_SUCCESS;                                                                                             \
    } while (0)

// 获取注册dpp的Action
#define GET_REQUEST_DPP_ACTION(manageCmd, dppAction)                                                                   \
    do {                                                                                                               \
        mp_int32 iRetx = dppActionMap.GetAction(manageCmd, dppAction);                                                 \
        if (MP_SUCCESS != iRetx) {                                                                                     \
            COMMLOG(OS_LOG_ERROR, "Unimplement action, manageCmd %u.", manageCmd);                                     \
            return ERROR_COMMON_FUNC_UNIMPLEMENT;                                                                      \
        }                                                                                                              \
    } while (0)
// 执行dpp的请求特定Action
#define DO_DPP_ACTION(clsname, req, rsp)                                                                               \
    do {                                                                                                               \
        mp_uint32 manageCmd;                                                                                           \
        CDppActionMap<clsname>::dpp_action_t dppAction;                                                                \
        COMMLOG(OS_LOG_DEBUG, "Begin do dpp action.");                                                                 \
        dppActionMap.PrintMap();                                                                                       \
        manageCmd = req.GetManageCmd();                                                                                \
        if (manageCmd == MANAGE_CMD_NO_INVALID) {                                                                      \
            COMMLOG(OS_LOG_ERROR, "requestmsg can't find manage cmd.");                                                \
            return MP_FAILED;                                                                                          \
        }                                                                                                              \
        GET_REQUEST_DPP_ACTION(manageCmd, dppAction);                                                                  \
        mp_int32 iRet = (this->*dppAction.action)(req, rsp);                                                           \
        if (MP_SUCCESS != iRet) {                                                                                      \
            mp_int32 iNewRet = (iRet == MP_FAILED) ? ERROR_AGENT_INTERNAL_ERROR : iRet;                                \
            COMMLOG(OS_LOG_ERROR, "Do dpp action failed, manageCmd %u, iRet=%d newRet=%d.", manageCmd, iRet, iNewRet); \
            Json::Value rspBodyParams;                                                                                 \
            Json::Value rspBody;                                                                                       \
            if ((rsp.GetManageBody(rspBodyParams) == MP_SUCCESS) && rspBodyParams.isMember(MANAGECMD_KEY_BODY)) {      \
                rspBody[MANAGECMD_KEY_BODY] = rspBodyParams[MANAGECMD_KEY_BODY];                                       \
            }                                                                                                          \
            rspBody[MANAGECMD_KEY_CMDNO] = manageCmd + 1;                                                              \
            rspBody[MANAGECMD_KEY_ERRORCODE] = iNewRet;                                                                \
            rsp.SetMsgBody(rspBody);                                                                                   \
            return iRet;                                                                                               \
        }                                                                                                              \
        COMMLOG(OS_LOG_DEBUG, "Do dpp action succ.");                                                                  \
        return MP_SUCCESS;                                                                                             \
    } while (0)

#endif  // _AGENT_SERVICE_PLUGIN_H_
