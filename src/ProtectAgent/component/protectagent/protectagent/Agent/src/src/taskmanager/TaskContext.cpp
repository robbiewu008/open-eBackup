/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskContext.cpp
 * @brief  The implemention TaskContext
 * @version 1.0.0.0
 * @date 2019-11-15
 * @author wangguitao 00510599
 */
#include "taskmanager/TaskContext.h"

#include "common/CMpThread.h"
#include "common/JsonUtils.h"

TaskContext* TaskContext::m_pTaskContext = NULL;

TaskContext* TaskContext::GetInstance()
{
    if (NULL != m_pTaskContext) {
        return m_pTaskContext;
    }

    m_pTaskContext = new (std::nothrow) TaskContext();
    if (NULL == m_pTaskContext) {
        COMMLOG(OS_LOG_ERROR, "new TaskContext failed");
        return NULL;
    }

    return m_pTaskContext;
}

TaskContext::TaskContext()
{
    CMpThread::InitLock(&m_taskcontextLock);
}

TaskContext::~TaskContext()
{
    CMpThread::DestroyLock(&m_taskcontextLock);
}

mp_void TaskContext::SetJsonValue(const mp_string& taskID, const mp_string& strKey, const Json::Value& jsonVal)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        iter->second[strKey] = jsonVal;
    } else {
        Json::Value newJsonValue;
        newJsonValue[strKey] = jsonVal;
        m_taskContextJsonDate.insert(std::pair<mp_string, Json::Value>(taskID, newJsonValue));
    }
}

mp_void TaskContext::SetValueString(const mp_string& taskID, const mp_string& strKey, const mp_string& strValue)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        iter->second[strKey] = strValue;
    } else {
        Json::Value newJsonValue;
        newJsonValue[strKey] = strValue;
        m_taskContextJsonDate.insert(std::pair<mp_string, Json::Value>(taskID, newJsonValue));
    }
}

mp_void TaskContext::SetValueInt32(const mp_string& taskID, const mp_string& strKey, const mp_int32& iValue)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        iter->second[strKey] = iValue;
    } else {
        Json::Value newJsonValue;
        newJsonValue[strKey] = iValue;
        m_taskContextJsonDate.insert(std::pair<mp_string, Json::Value>(taskID, newJsonValue));
    }
}

mp_void TaskContext::SetValueUInt32(const mp_string& taskID, const mp_string& strKey, const mp_uint32& iValue)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        iter->second[strKey] = iValue;
    } else {
        Json::Value newJsonValue;
        newJsonValue[strKey] = iValue;
        m_taskContextJsonDate.insert(std::pair<mp_string, Json::Value>(taskID, newJsonValue));
    }
}

mp_int32 TaskContext::GetValueString(const mp_string& taskID, const mp_string& strKey, mp_string& strValue)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        GET_JSON_STRING(iter->second, strKey, strValue);
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR, "task %s get %s value failed.", taskID.c_str(), strKey.c_str());
        return MP_FAILED;
    }
}

mp_int32 TaskContext::GetValueJson(const mp_string& taskID, const mp_string& strKey, Json::Value& jsValue)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        jsValue = iter->second[strKey];
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR, "task %s get %s value failed.", taskID.c_str(), strKey.c_str());
        return MP_FAILED;
    }
}

mp_int32 TaskContext::GetValueStringOption(const mp_string& taskID, const mp_string& strKey, mp_string& strValue)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        GET_JSON_STRING_OPTION(iter->second, strKey, strValue);
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR, "task %s get %s value failed.", taskID.c_str(), strKey.c_str());
        return MP_FAILED;
    }
}

mp_int32 TaskContext::GetValueInt32(const mp_string& taskID, const mp_string& strKey, mp_int32& iValue)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        GET_JSON_INT32(iter->second, strKey, iValue);
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR, "task %s get %s value failed.", taskID.c_str(), strKey.c_str());
        return MP_FAILED;
    }
}

mp_int32 TaskContext::GetValueUInt32(const mp_string& taskID, const mp_string& strKey, mp_uint32& uiValue)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        GET_JSON_UINT32(iter->second, strKey, uiValue);
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR, "task %s get %s value failed.", taskID.c_str(), strKey.c_str());
        return MP_FAILED;
    }
}

mp_int32 TaskContext::GetValueVector(const mp_string& taskID, const mp_string& strKey, std::vector<mp_string>& vecValue)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        GET_JSON_ARRAY_STRING(iter->second, strKey, vecValue);
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR, "task %s get %s value failed.", taskID.c_str(), strKey.c_str());
        return MP_FAILED;
    }
}

mp_int32 TaskContext::GetValueInt32Option(const mp_string& taskID, const mp_string& strKey, mp_int32& iValue)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        GET_JSON_INT32_OPTION(iter->second, strKey, iValue);
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR, "task %s get %s value failed.", taskID.c_str(), strKey.c_str());
        return MP_FAILED;
    }
}

mp_int32 TaskContext::GetValueUInt32Option(const mp_string& taskID, const mp_string& strKey, mp_uint32& uiValue)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        GET_JSON_UINT32_OPTION(iter->second, strKey, uiValue);
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR, "task %s get %s value failed.", taskID.c_str(), strKey.c_str());
        return MP_FAILED;
    }
}

mp_int32 TaskContext::GetValueVectorOption(const mp_string &taskID, const mp_string &strKey,
    std::vector<mp_string> &vecValue)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        GET_JSON_ARRAY_STRING_OPTION(iter->second, strKey, vecValue);
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR, "task %s get %s value failed.", taskID.c_str(), strKey.c_str());
        return MP_FAILED;
    }
}

mp_void TaskContext::RemoveTaskContext(const mp_string& taskID)
{
    CThreadAutoLock tlock(&m_taskcontextLock);
    std::map<mp_string, Json::Value>::iterator iter = m_taskContextJsonDate.find(taskID);
    if (iter != m_taskContextJsonDate.end()) {
        m_taskContextJsonDate.erase(iter);
        COMMLOG(OS_LOG_DEBUG, "Remove content of task '%s' from cache successfully.", taskID.c_str());
    }
}
