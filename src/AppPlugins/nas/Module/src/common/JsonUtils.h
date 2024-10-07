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
#ifndef MODULE_JSON_UTILS_H
#define MODULE_JSON_UTILS_H
#include "define/Types.h"
#include "common/MpString.h"
#include "json/value.h"
#include "json/json.h"

namespace Module {

// 将调用返回码返回
#define JSON_CHECK_FAIL(Call)                                                                                          \
    {                                                                                                                  \
        int iCheckNotOkRet = Call;                                                                                     \
        if (SUCCESS != iCheckNotOkRet) {                                                                               \
            COMMLOG(OS_LOG_ERROR, "Call %s failed, ret %d.", #Call, iCheckNotOkRet);                                   \
            return iCheckNotOkRet;                                                                                     \
        }                                                                                                              \
    }                                                                                                                  

class AGENT_API CJsonUtils
{
public:
    static int GetJsonString(const Json::Value& jsValue, std::string strKey, std::string& strValue);
    static int GetJsonInt32(const Json::Value& jsValue, std::string strKey, int& iValue);
    static int GetJsonUInt32(const Json::Value& jsValue, std::string strKey, uint32_t& iValue);
    static int GetJsonInt64(const Json::Value& jsValue, std::string strKey, int64_t& lValue);
    static int GetJsonUInt64(const Json::Value& jsValue, std::string strKey, uint64_t& lValue);
    static int GetJsonArrayInt32(const Json::Value& jsValue, std::string strKey, std::vector<int>& vecValue);
    static int GetJsonArrayInt64(const Json::Value& jsValue, std::string strKey, std::vector<int64_t>& vecValue);
    static int GetJsonArrayString(const Json::Value& jsValue, std::string strKey, std::vector<std::string>& vecValue);
    static int GetJsonArrayJson(const Json::Value& jsValue, std::vector<Json::Value>& vecValue);
    static int GetJsonArrayJson(const Json::Value& jsValue, std::string strKey, std::vector<Json::Value>& vecValue);
    static int GetArrayInt32(const Json::Value& jsValue, std::vector<int>& vecValue);
    static int GetArrayInt64(const Json::Value& jsValue, std::vector<int64_t>& vecValue);
    static int GetArrayString(const Json::Value& jsValue, std::vector<std::string>& vecValue);
    static int GetArrayJson(const Json::Value& jsValue, std::vector<Json::Value>& vecValue);
    static int GetJsonKeyString(const Json::Value& jsValue, std::string jsKey, std::string& strValue);
    static int GetArrayStringWithoutBraces(const Json::Value& jsValue, std::vector<std::string>& vecValue);
    static int GetArrayInt32WithoutBraces(const Json::Value& jsValue, std::vector<int>& vecValue);
    static int ConvertStringtoJson(const std::string& rawBuffer, Json::Value& jsValue);
    static std::string ConvertJsonToString(const Json::Value& jsValue);
    static std::string JsonToString(const Json::Value& jsValue);
    static bool StringToJson(const std::string &sJosn, Json::Value& jsValue);
};

// 便于检查返回结果，定义部分宏定义
#define GET_JSON_STRING(jsValue, strKey, strValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetJsonString(jsValue, strKey, strValue))
#define GET_JSON_INT32(jsValue, strKey, iValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetJsonInt32(jsValue, strKey, iValue))
#define GET_JSON_UINT32(jsValue, strKey, iValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetJsonUInt32(jsValue, strKey, iValue))
#define GET_JSON_INT64(jsValue, strKey, lValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetJsonInt64(jsValue, strKey, lValue))
#define GET_JSON_UINT64(jsValue, strKey, lValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetJsonUInt64(jsValue, strKey, lValue))     
#define GET_JSON_ARRAY_STRING(jsValue, strKey, vecValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetJsonArrayString(jsValue, strKey, vecValue))
#define GET_JSON_ARRAY_INT32(jsValue, strKey, vecValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetJsonArrayInt32(jsValue, strKey, vecValue))
#define GET_JSON_ARRAY_INT64(jsValue, strKey, vecValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetJsonArrayInt64(jsValue, strKey, vecValue))   
#define GET_JSON_ARRAY_JSON(jsValue, strKey, vecValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetJsonArrayJson(jsValue, strKey, vecValue)) 
#define GET_ARRAY_STRING(jsValue, vecValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetArrayString(jsValue, vecValue))
#define GET_ARRAY_INT32(jsValue, vecValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetArrayInt32(jsValue, vecValue))
#define GET_ARRAY_INT64(jsValue, vecValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetArrayInt64(jsValue, vecValue))
#define GET_ARRAY_JSON(jsValue, vecValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetArrayJson(jsValue, vecValue))
#define GET_JSON_KEY_STRING(jsValue, strKey, strValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetJsonKeyString(jsValue, strKey, strValue))

#define GET_ARRAY_STRING_WITHOUT_BRACES(jsValue, vecValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetArrayStringWithoutBraces(jsValue, vecValue))
#define GET_ARRAY_INT32_WITHOUT_BRACES(jsValue, vecValue) \
    JSON_CHECK_FAIL(CJsonUtils::GetArrayInt32WithoutBraces(jsValue, vecValue))

#define GET_JSON_STRING_OPTION(jsValue, strKey, strValue) \
    if (jsValue.isMember(strKey)) \
    {  \
        JSON_CHECK_FAIL(CJsonUtils::GetJsonString(jsValue, strKey, strValue)); \
    }  \

#define GET_JSON_INT32_OPTION(jsValue, strKey, iValue) \
    if (jsValue.isMember(strKey)) \
    {  \
        JSON_CHECK_FAIL(CJsonUtils::GetJsonInt32(jsValue, strKey, iValue)); \
    }  \

#define GET_JSON_UINT32_OPTION(jsValue, strKey, iValue) \
    if (jsValue.isMember(strKey)) \
    {  \
        JSON_CHECK_FAIL(CJsonUtils::GetJsonUInt32(jsValue, strKey, iValue)); \
    }  \

#define GET_JSON_INT64_OPTION(jsValue, strKey, lValue) \
    if (jsValue.isMember(strKey)) \
    {  \
        JSON_CHECK_FAIL(CJsonUtils::GetJsonInt64(jsValue, strKey, lValue)); \
    }  \

#define GET_JSON_UINT64_OPTION(jsValue, strKey, lValue) \
    if (jsValue.isMember(strKey)) \
    {  \
        JSON_CHECK_FAIL(CJsonUtils::GetJsonUInt64(jsValue, strKey, lValue)); \
    }  \

#define GET_JSON_ARRAY_STRING_OPTION(jsValue, strKey, vecValue) \
    if (jsValue.isMember(strKey)) \
    {  \
        JSON_CHECK_FAIL(CJsonUtils::GetJsonArrayString(jsValue, strKey, vecValue)); \
    }  \

#define GET_JSON_ARRAY_INT32_OPTION(jsValue, strKey, vecValue) \
    if (jsValue.isMember(strKey)) \
    {  \
        JSON_CHECK_FAIL(CJsonUtils::GetJsonArrayInt32(jsValue, strKey, vecValue)); \
    }  \

#define GET_JSON_ARRAY_INT64_OPTION(jsValue, strKey, vecValue) \
    if (jsValue.isMember(strKey)) \
    {  \
        JSON_CHECK_FAIL(CJsonUtils::GetJsonArrayInt64(jsValue, strKey, vecValue)); \
    }  \

#define GET_JSON_ARRAY_JSON_OPTION(jsValue, strKey, vecValue) \
    if (jsValue.isMember(strKey)) \
    {  \
        JSON_CHECK_FAIL(CJsonUtils::GetJsonArrayJson(jsValue, strKey, vecValue)); \
    }  \

// 检查json元素是否存在
#define MODULE_CHECK_JSON_VALUE(jsValue, strKey) \
    if (!jsValue.isMember(strKey)) \
    { \
        COMMLOG(OS_LOG_ERROR, "Key \"%s\" is not exist.", strKey); \
        return ERROR_COMMON_INVALID_PARAM; \
    } \

// 检查json对象是否为数组
#define MODULE_CHECK_JSON_ARRAY(jsValue) do \
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
} // namespace
#endif // MODULE_JSON_UTILS_H
