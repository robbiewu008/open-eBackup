/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file JsonUtils.h
 * @brief  处理消息体，作为agent内部通信消息
 * @version 1.0.0
 * @date 2020-08-01
 * @author yangwenjun 00275736
 */
#ifndef _AGENT_JSON_UTILS_H_
#define _AGENT_JSON_UTILS_H_
#include "common/Types.h"
#include "common/MpString.h"
#include "jsoncpp/include/json/value.h"
#include "jsoncpp/include/json/json.h"

class AGENT_API CJsonUtils
{
public:
    static mp_int32 GetJsonString(const Json::Value& jsValue, const mp_string& strKey, mp_string& strValue);
    static mp_int32 GetJsonBool(const Json::Value& jsValue, const mp_string& strKey, mp_bool& bValue);
    static mp_int32 GetJsonInt32(const Json::Value& jsValue, const mp_string& strKey, mp_int32& iValue);
    static mp_int32 GetJsonUInt32(const Json::Value& jsValue, const mp_string& strKey, mp_uint32& iValue);
    static mp_int32 GetJsonInt64(const Json::Value& jsValue, const mp_string& strKey, mp_int64& lValue);
    static mp_int32 GetJsonUInt64(const Json::Value& jsValue, const mp_string& strKey, mp_uint64& lValue);
    static mp_int32 GetJsonArrayInt32(const Json::Value& jsValue, const mp_string& strKey,
        std::vector<mp_int32>& vecValue);
    static mp_int32 GetJsonArrayInt64(const Json::Value& jsValue, const mp_string& strKey,
        std::vector<mp_int64>& vecValue);
    static mp_int32 GetJsonArrayString(const Json::Value& jsValue, const mp_string& strKey,
        std::vector<mp_string>& vecValue);
    static mp_int32 GetJsonArrayJson(const Json::Value& jsValue, const mp_string& strKey,
        std::vector<Json::Value>& vecValue);
    static mp_int32 GetArrayInt32(const Json::Value& jsValue, std::vector<mp_int32>& vecValue);
    static mp_int32 GetArrayInt64(const Json::Value& jsValue, std::vector<mp_int64>& vecValue);
    static mp_int32 GetArrayString(const Json::Value& jsValue, std::vector<mp_string>& vecValue);
    static mp_int32 GetArrayJson(const Json::Value& jsValue, std::vector<Json::Value>& vecValue);
    static mp_int32 GetJsonKeyString(const Json::Value& jsValue, const mp_string& jsKey, mp_string& strValue);
    static mp_int32 GetArrayStringWithoutBraces(const Json::Value& jsValue, std::vector<mp_string>& vecValue);
    static mp_int32 GetArrayInt32WithoutBraces(const Json::Value& jsValue, std::vector<mp_int32>& vecValue);
    static mp_int32 ConvertStringtoJson(const mp_string& rawBuffer, Json::Value& jsValue);
};

// 便于检查返回结果，定义部分宏定义
#define GET_JSON_STRING(jsValue, strKey, strValue) \
    CHECK_FAIL_EX(CJsonUtils::GetJsonString(jsValue, strKey, strValue))
#define GET_JSON_BOOL(jsValue, strKey, bValue) \
    CHECK_FAIL_EX(CJsonUtils::GetJsonBool(jsValue, strKey, bValue))
#define GET_JSON_INT32(jsValue, strKey, iValue) \
    CHECK_FAIL_EX(CJsonUtils::GetJsonInt32(jsValue, strKey, iValue))
#define GET_JSON_UINT32(jsValue, strKey, iValue) \
    CHECK_FAIL_EX(CJsonUtils::GetJsonUInt32(jsValue, strKey, iValue))
#define GET_JSON_INT64(jsValue, strKey, lValue) \
    CHECK_FAIL_EX(CJsonUtils::GetJsonInt64(jsValue, strKey, lValue))
#define GET_JSON_UINT64(jsValue, strKey, lValue) \
    CHECK_FAIL_EX(CJsonUtils::GetJsonUInt64(jsValue, strKey, lValue))
#define GET_JSON_ARRAY_STRING(jsValue, strKey, vecValue) \
    CHECK_FAIL_EX(CJsonUtils::GetJsonArrayString(jsValue, strKey, vecValue))
#define GET_JSON_ARRAY_INT32(jsValue, strKey, vecValue) \
    CHECK_FAIL_EX(CJsonUtils::GetJsonArrayInt32(jsValue, strKey, vecValue))
#define GET_JSON_ARRAY_INT64(jsValue, strKey, vecValue) \
    CHECK_FAIL_EX(CJsonUtils::GetJsonArrayInt64(jsValue, strKey, vecValue))
#define GET_JSON_ARRAY_JSON(jsValue, strKey, vecValue) \
    CHECK_FAIL_EX(CJsonUtils::GetJsonArrayJson(jsValue, strKey, vecValue))
#define GET_ARRAY_STRING(jsValue, vecValue) \
    CHECK_FAIL_EX(CJsonUtils::GetArrayString(jsValue, vecValue))
#define GET_ARRAY_INT32(jsValue, vecValue) \
    CHECK_FAIL_EX(CJsonUtils::GetArrayInt32(jsValue, vecValue))
#define GET_ARRAY_INT64(jsValue, vecValue) \
    CHECK_FAIL_EX(CJsonUtils::GetArrayInt64(jsValue, vecValue))
#define GET_ARRAY_JSON(jsValue, vecValue) \
    CHECK_FAIL_EX(CJsonUtils::GetArrayJson(jsValue, vecValue))
#define GET_JSON_KEY_STRING(jsValue, strKey, strValue) \
    CHECK_FAIL_EX(CJsonUtils::GetJsonKeyString(jsValue, strKey, strValue))

#define GET_ARRAY_STRING_WITHOUT_BRACES(jsValue, vecValue) \
    CHECK_FAIL_EX(CJsonUtils::GetArrayStringWithoutBraces(jsValue, vecValue))
#define GET_ARRAY_INT32_WITHOUT_BRACES(jsValue, vecValue) \
    CHECK_FAIL_EX(CJsonUtils::GetArrayInt32WithoutBraces(jsValue, vecValue))

#define GET_JSON_STRING_OPTION(jsValue, strKey, strValue) \
    if (jsValue.isObject() && jsValue.isMember(strKey)) \
    {  \
        CHECK_FAIL_EX(CJsonUtils::GetJsonString(jsValue, strKey, strValue)); \
    }  \

#define GET_JSON_INT32_OPTION(jsValue, strKey, iValue) \
    if (jsValue.isObject() && jsValue.isMember(strKey)) \
    {  \
        CHECK_FAIL_EX(CJsonUtils::GetJsonInt32(jsValue, strKey, iValue)); \
    }  \

#define GET_JSON_UINT32_OPTION(jsValue, strKey, iValue) \
    if (jsValue.isObject() && jsValue.isMember(strKey)) \
    {  \
        CHECK_FAIL_EX(CJsonUtils::GetJsonUInt32(jsValue, strKey, iValue)); \
    }  \

#define GET_JSON_INT64_OPTION(jsValue, strKey, lValue) \
    if (jsValue.isObject() && jsValue.isMember(strKey)) \
    {  \
        CHECK_FAIL_EX(CJsonUtils::GetJsonInt64(jsValue, strKey, lValue)); \
    }  \

#define GET_JSON_UINT64_OPTION(jsValue, strKey, lValue) \
    if (jsValue.isObject() && jsValue.isMember(strKey)) \
    {  \
        CHECK_FAIL_EX(CJsonUtils::GetJsonUInt64(jsValue, strKey, lValue)); \
    }  \

#define GET_JSON_ARRAY_STRING_OPTION(jsValue, strKey, vecValue) \
    if (jsValue.isObject() && jsValue.isMember(strKey)) \
    {  \
        CHECK_FAIL_EX(CJsonUtils::GetJsonArrayString(jsValue, strKey, vecValue)); \
    }  \

#define GET_JSON_ARRAY_INT32_OPTION(jsValue, strKey, vecValue) \
    if (jsValue.isObject() && jsValue.isMember(strKey)) \
    {  \
        CHECK_FAIL_EX(CJsonUtils::GetJsonArrayInt32(jsValue, strKey, vecValue)); \
    }  \

#define GET_JSON_ARRAY_INT64_OPTION(jsValue, strKey, vecValue) \
    if (jsValue.isObject() && jsValue.isMember(strKey)) \
    {  \
        CHECK_FAIL_EX(CJsonUtils::GetJsonArrayInt64(jsValue, strKey, vecValue)); \
    }  \

#define GET_JSON_ARRAY_JSON_OPTION(jsValue, strKey, vecValue) \
    if (jsValue.isObject() && jsValue.isMember(strKey)) \
    {  \
        CHECK_FAIL_EX(CJsonUtils::GetJsonArrayJson(jsValue, strKey, vecValue)); \
    }  \

// 检查json元素是否存在
#define CHECK_JSON_VALUE(jsValue, strKey) \
    if (!jsValue.isObject() || !jsValue.isMember(strKey)) \
    { \
        COMMLOG(OS_LOG_ERROR, "Key \"%s\" is not exist.", strKey); \
        return ERROR_COMMON_INVALID_PARAM; \
    } \

// 检查json对象是否为数组
#define CHECK_JSON_ARRAY(jsValue) do \
    { \
        if (!jsValue.isArray()) \
        { \
            COMMLOG(OS_LOG_ERROR, "Json value is not array."); \
            return ERROR_COMMON_INVALID_PARAM; \
        } \
        if (jsValue.size() == 0) \
        { \
            COMMLOG(OS_LOG_ERROR, "Size of Json array is 0."); \
            return ERROR_COMMON_INVALID_PARAM; \
        } \
    } while (0)

#endif // _AGENT_JSON_UTILS_H_

