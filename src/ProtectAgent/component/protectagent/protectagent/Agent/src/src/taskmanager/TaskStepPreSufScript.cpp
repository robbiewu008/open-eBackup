/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepPreSufScript.cpp
 * @brief  Contains function declarations for TaskStepPreSufScript
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "taskmanager/TaskStepPreSufScript.h"
#include "common/Log.h"
#include "common/JsonUtils.h"
#include "common/File.h"
#include "apps/oracle/OracleDefines.h"

TaskStepPreScript::TaskStepPreScript(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepScript(id, taskId, name, ratio, order)
{}

TaskStepPreScript::~TaskStepPreScript()
{}

mp_int32 TaskStepPreScript::Init(const Json::Value& param)
{
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to init pre script.", m_taskId.c_str());
    static const mp_string keyPreScriptName = "preScript";

    if (!param.isObject() || !param.isMember(keyPreScriptName)) {
        COMMLOG(OS_LOG_WARN, "initialize backup step script failed, have no key %s.", keyPreScriptName.c_str());
        return MP_SUCCESS;
    }

    GET_JSON_STRING(param, keyPreScriptName, scriptName);
    CHECK_FAIL_EX(CheckParamStringEnd(scriptName, 0, ORACLE_PLUGIN_MAX_SCRIPT));
    COMMLOG(OS_LOG_DEBUG, "scriptname is %s,", scriptName.c_str());
    auto ret = CMpFile::CheckFileName(scriptName);
    if (ret != MP_SUCCESS) {
        ERRLOG("Script name is invailed, filename is %s", scriptName.c_str());
        return ret;
    }
    // 初始化参数，当前不支持stop，后面支持如果stop为空，则
    Init3rdScript(scriptName, "", "", "", MP_TRUE);
    return MP_SUCCESS;
}

TaskStepPostScript::TaskStepPostScript(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepScript(id, taskId, name, ratio, order)
{}

TaskStepPostScript::~TaskStepPostScript()
{}

mp_int32 TaskStepPostScript::Init(const Json::Value& param)
{
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to init post script.", m_taskId.c_str());
    static const mp_string keySufScriptName = "postScript";

    if (!param.isObject() || !param.isMember(keySufScriptName)) {
        COMMLOG(OS_LOG_WARN, "initialize backup step script failed, have no key %s.", keySufScriptName.c_str());
        return MP_SUCCESS;
    }

    GET_JSON_STRING(param, keySufScriptName, scriptName);
    CHECK_FAIL_EX(CheckParamStringEnd(scriptName, 0, ORACLE_PLUGIN_MAX_SCRIPT));
    COMMLOG(OS_LOG_DEBUG, "script is %s,", scriptName.c_str());
    auto ret = CMpFile::CheckFileName(scriptName);
    if (ret != MP_SUCCESS) {
        ERRLOG("Script name is invailed, filename is %s", scriptName.c_str());
        return ret;
    }
    // 初始化参数，当前不支持stop，后面支持如果stop为空，则
    Init3rdScript(scriptName, "", "", "", MP_FALSE);
    return MP_SUCCESS;
}


TaskStepFailPostScript::TaskStepFailPostScript(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepScript(id, taskId, name, ratio, order)
{}

TaskStepFailPostScript::~TaskStepFailPostScript()
{}

mp_int32 TaskStepFailPostScript::Init(const Json::Value& param)
{
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to init fail post script.", m_taskId.c_str());
    static const mp_string keyScriptName = "failPostScript";

    if (!param.isObject() || !param.isMember(keyScriptName)) {
        COMMLOG(OS_LOG_WARN, "initialize backup step script failed, have no key %s.", keyScriptName.c_str());
        return MP_SUCCESS;
    }

    GET_JSON_STRING(param, keyScriptName, scriptName);
    CHECK_FAIL_EX(CheckParamStringEnd(scriptName, 0, ORACLE_PLUGIN_MAX_SCRIPT));
    COMMLOG(OS_LOG_DEBUG, "script is %s,", scriptName.c_str());
    auto ret = CMpFile::CheckFileName(scriptName);
    if (ret != MP_SUCCESS) {
        ERRLOG("Script name is invailed, filename is %s", scriptName.c_str());
        return ret;
    }
    // 初始化参数，当前不支持stop，后面支持如果stop为空，则
    Init3rdScript(scriptName, "", "", "", MP_FALSE);
    return MP_SUCCESS;
}
