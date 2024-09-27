/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file JsonUtils.cpp
 * @brief  Contains function declarations for Josn files
 * @version 0.1
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include <sstream>
#include "common/Defines.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/JsonUtils.h"

using namespace std;

/*------------------------------------------------------------
Function Name: GetJsonString
Description  : 获取JSON对象某个元素的字符串值
                jsValue:JSON数据，示例{"data":"chengdu"}
                strKey:JSON数据的key值,示例中的"data"
                strValue:key对应的字符串值
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonString(const Json::Value& jsValue, const mp_string& strKey, mp_string& strValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isObject() && jsValue.isMember(strKey)) {
        if (jsValue[strKey].isString()) {
            strValue = jsValue[strKey].asString();
            return MP_SUCCESS;
        } else {
            COMMLOG(OS_LOG_INFO, "The value Json key \"%s\" is not string.", strKey.c_str());
            return ERROR_COMMON_INVALID_PARAM;
        }
    } else {
        strValue = "";
        COMMLOG(OS_LOG_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonBool
Description  : 获取JSON对象某个元素的布朗值
                jsValue:JSON数据，示例{"data":ture}
                strKey:JSON数据的key值,示例中的"data"
                bValue:key对应的布朗值
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonBool(const Json::Value& jsValue, const mp_string& strKey, mp_bool& bValue)
{
    if (jsValue.isArray()) {
        INFOLOG("Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isObject() && jsValue.isMember(strKey)) {
        if (jsValue[strKey].isBool()) {
            bValue = jsValue[strKey].asBool();
            return MP_SUCCESS;
        } else {
            INFOLOG("The value Json key \"%s\" is not Bool.", strKey.c_str());
            return ERROR_COMMON_INVALID_PARAM;
        }
    } else {
        INFOLOG("Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonInt32
Description  : 获取JSON对象某个元素的int32值
               jsValue:JSON数据，示例 {"data":2015}
               strKey:JSON数据的key值,示例中的"data"
               iValue:key对应的int32值
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonInt32(const Json::Value& jsValue, const mp_string& strKey, mp_int32& iValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_ERROR, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isObject() && jsValue.isMember(strKey)) {
        if (jsValue[strKey].isInt()) {
            iValue = jsValue[strKey].asInt();
            return MP_SUCCESS;
        } else {
            COMMLOG(OS_LOG_ERROR, "The value of Json key \"%s\" is not int.", strKey.c_str());
            return ERROR_COMMON_INVALID_PARAM;
        }
    } else {
        iValue = MP_FAILED;
        COMMLOG(OS_LOG_WARN, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonUInt32
Description  : 获取JSON对象某个元素的uint32值
               jsValue:JSON数据，示例 {"data":2015}
               strKey:JSON数据的key值,示例中的"data"
               iValue:key对应的uint32值
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonUInt32(const Json::Value& jsValue, const mp_string& strKey, mp_uint32& iValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isObject() && jsValue.isMember(strKey)) {
        if (jsValue[strKey].isUInt()) {
            iValue = jsValue[strKey].asUInt();
            return MP_SUCCESS;
        } else {
            COMMLOG(OS_LOG_ERROR, "The value of Json key \"%s\" is not int.", strKey.c_str());
            return ERROR_COMMON_INVALID_PARAM;
        }
    } else {
        iValue = 0;
        COMMLOG(OS_LOG_WARN, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonInt64
Description  : 获取JSON对象某个元素的int64值
               jsValue:JSON数据，示例 {"data":2015}
               strKey:JSON数据的key值,示例中的"data"
               lValue:key对应的int64值
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonInt64(const Json::Value& jsValue, const mp_string& strKey, mp_int64& lValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isObject() && jsValue.isMember(strKey)) {
        if (jsValue[strKey].isInt()) {
            lValue = jsValue[strKey].asInt64();
            return MP_SUCCESS;
        } else {
            COMMLOG(OS_LOG_INFO, "The value of Json key \"%s\" is not int.", strKey.c_str());
            return ERROR_COMMON_INVALID_PARAM;
        }
    } else {
        lValue = MP_FAILED;
        COMMLOG(OS_LOG_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonUInt64
Description  : 获取JSON对象某个元素的uint64值
               jsValue:JSON数据，示例 {"data":2015}
               strKey:JSON数据的key值,示例中的"data"
               lValue:key对应的uint64值
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonUInt64(const Json::Value& jsValue, const mp_string& strKey, mp_uint64& lValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isObject() && jsValue.isMember(strKey)) {
        if (jsValue[strKey].isUInt64()) {
            lValue = jsValue[strKey].asUInt64();
            return MP_SUCCESS;
        } else {
            COMMLOG(OS_LOG_INFO, "The value of Json key \"%s\" is not uint.", strKey.c_str());
            return ERROR_COMMON_INVALID_PARAM;
        }
    } else {
        COMMLOG(OS_LOG_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonArrayInt32
Description  : JSON对象某个元素为int32数组
               jsValue:JSON数据，示例 {"data":[2015,2016,2017]}
               strKey:JSON数据的key值，示例中的"data"
               vecValue:key对应的int32数组
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonArrayInt32(const Json::Value& jsValue, const mp_string& strKey, vector<mp_int32>& vecValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isObject() && jsValue.isMember(strKey) && jsValue[strKey].isArray()) {
        mp_uint32 uiSize = jsValue[strKey].size();
        for (mp_uint32 i = 0; i < uiSize; i++) {
            if (!jsValue[strKey][i].isObject() && !jsValue[strKey][i].isArray() && jsValue[strKey][i].isInt()) {
                vecValue.push_back(jsValue[strKey][i].asInt());
            } else {
                COMMLOG(OS_LOG_INFO, "The value of Json key \"%s\" is not int.", strKey.c_str());
                vecValue.clear();
                return ERROR_COMMON_INVALID_PARAM;
            }
        }
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonArrayInt64
Description  : JSON对象某个元素为int64数组
               jsValue:JSON数据，示例 {"data":[2015,2016,2017]}
               strKey:JSON数据的key值，示例中的"data"
               vecValue:key对应的int64数组
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonArrayInt64(const Json::Value& jsValue, const mp_string& strKey, vector<mp_int64>& vecValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isObject() && jsValue.isMember(strKey) && jsValue[strKey].isArray()) {
        mp_uint32 uiSize = jsValue[strKey].size();
        for (mp_uint32 i = 0; i < uiSize; i++) {
            if (!jsValue[strKey][i].isObject() && !jsValue[strKey][i].isArray() && jsValue[strKey][i].isInt()) {
                vecValue.push_back(jsValue[strKey][i].asInt64());
            } else {
                COMMLOG(OS_LOG_INFO, "The value of Json key \"%s\" is not int.", strKey.c_str());
                vecValue.clear();
                return ERROR_COMMON_INVALID_PARAM;
            }
        }
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonArrayString
Description  : JSON对象某个元素为string数组
               jsValue:JSON数据，示例 {"data":["chengdu","chongqing","nanchong"]}
               strKey:JSON数据的key值，示例中的"data"
               vecValue:key对应的string数组
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonArrayString(const Json::Value& jsValue, const mp_string& strKey,
    vector<mp_string>& vecValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_ERROR, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isObject() && jsValue.isMember(strKey) && jsValue[strKey].isArray()) {
        mp_uint32 uiSize = jsValue[strKey].size();
        for (mp_uint32 i = 0; i < uiSize; i++) {
            if (!jsValue[strKey][i].isObject() && !jsValue[strKey][i].isArray() && jsValue[strKey][i].isString()) {
                vecValue.push_back(jsValue[strKey][i].asString());
            } else {
                COMMLOG(OS_LOG_ERROR, "The value of Json key \"%s\" is not int.", strKey.c_str());
                vecValue.clear();
                return ERROR_COMMON_INVALID_PARAM;
            }
        }
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_WARN, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
}

/*------------------------------------------------------------
Function Name: GetJsonArrayJson
Description  : JSON对象某个元素为Json数组
               jsValue:JSON数据，示例 {"data":[{"city":"chengdu"},{"city":"chongqing"}]}
               strKey:JSON数据的key值，示例中的"data"
               vecValue:key对应的json数组,数组单元素为json对象，类似于{"city":"chengdu"}
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonArrayJson(const Json::Value& jsValue, const mp_string& strKey,
    vector<Json::Value>& vecValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isObject() && jsValue.isMember(strKey) && jsValue[strKey].isArray()) {
        mp_uint32 uiSize = jsValue[strKey].size();
        for (mp_uint32 i = 0; i < uiSize; i++) {
            if (jsValue[strKey][i].isObject() && !jsValue[strKey][i].isArray()) {
                vecValue.push_back(jsValue[strKey][i]);
            } else {
                COMMLOG(OS_LOG_INFO, "The value of Json key \"%s\" is not Json object.", strKey.c_str());
                vecValue.clear();
                return ERROR_COMMON_INVALID_PARAM;
            }
        }
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
}
/*------------------------------------------------------------
Function Name: GetArrayInt32
Description  : 该JSON对象为int32数组，且没有key值
               jsValue:JSON数据，示例 {[1,2,3]}
               vecValue:转换成的int32数组
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetArrayInt32(const Json::Value& jsValue, vector<mp_int32>& vecValue)
{
    if (!jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is not array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_uint32 uiSize = jsValue.size();
    for (mp_uint32 i = 0; i < uiSize; i++) {
        if (!jsValue[i].isObject() && !jsValue[i].isArray()) {
            vecValue.push_back(jsValue[i].asInt());
        } else {
            COMMLOG(OS_LOG_INFO, "Json value is array or object.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------
Function Name: GetArrayInt64
Description  : 该JSON对象为int64数组，且没有key值
               jsValue:JSON数据，示例 {[1,2,3]}
               vecValue:转换成的int64数组
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetArrayInt64(const Json::Value& jsValue, vector<mp_int64>& vecValue)
{
    if (!jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is not array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_uint32 uiSize = jsValue.size();
    for (mp_uint32 i = 0; i < uiSize; i++) {
        if (!jsValue[i].isObject() && !jsValue[i].isArray()) {
            vecValue.push_back(jsValue[i].asInt64());
        } else {
            COMMLOG(OS_LOG_INFO, "Json value is array or object.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetArrayStringWithoutBraces
Description  : 该JSON对象为string数组，且没有key值，不是json对象（没有大括号）
               jsValue:JSON数据，示例 没有大括号
                "notifyInfo" : ["127.0.0.1", "127.0.0.2"]
                vecValue:转换成的string数组
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetArrayStringWithoutBraces(const Json::Value& jsValue, vector<mp_string>& vecValue)
{
    if (!jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is not array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    for (auto iter = jsValue.begin(); iter != jsValue.end(); ++iter) {
        if (iter->isString()) {
            vecValue.push_back(iter->asString());
        } else {
            COMMLOG(OS_LOG_INFO, "Json value is not string.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetArrayInt32WithoutBraces
Description  : 该JSON对象为string数组，且没有key值，不是json对象（没有大括号）
               jsValue:JSON数据，示例 没有大括号
                "notifyInfo" : [12, 34]
                vecValue:转换成的int32数组
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetArrayInt32WithoutBraces(const Json::Value& jsValue, vector<mp_int32>& vecValue)
{
    if (!jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is not array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    for (auto iter = jsValue.begin(); iter != jsValue.end(); ++iter) {
        if (iter->isInt()) {
            vecValue.push_back(iter->asInt());
        } else {
            COMMLOG(OS_LOG_INFO, "Json value is not int.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetArrayString
Description  : 该JSON对象为string数组，且没有key值
               jsValue:JSON数据，示例 {["chengdu","chongqing","nanchong"]}
               vecValue:转换成的string数组
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetArrayString(const Json::Value& jsValue, vector<mp_string>& vecValue)
{
    if (!jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is not array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_uint32 uiSize = jsValue.size();
    for (mp_uint32 i = 0; i < uiSize; i++) {
        if (!jsValue[i].isObject() && !jsValue[i].isArray()) {
            vecValue.push_back(jsValue[i].asString());
        } else {
            COMMLOG(OS_LOG_INFO, "Json value is array or object.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------
Function Name: GetArrayJson
Description  : 该JSON对象为json数组，且没有key值
               jsValue:JSON数据，示例 {[{"city":"chengdu"},{"city":"chonqing"}]}
               vecValue:转换成的json数组
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetArrayJson(const Json::Value& jsValue, vector<Json::Value>& vecValue)
{
    if (!jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is not array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_uint32 uiSize = jsValue.size();
    for (mp_uint32 i = 0; i < uiSize; i++) {
        if (jsValue[i].isObject() && !jsValue[i].isArray()) {
            vecValue.push_back(jsValue[i]);
        } else {
            COMMLOG(OS_LOG_INFO, "Json value is not object.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------
Function Name: GetJsonKeyString
Description  : 获取JSON对象某个元素的字符串值,并且按照指定格式拼接参数
           jsValue:JSON数据，示例"paras": {"key":"chengdu","key2":"chengdu2","key3":"chengdu3"...}
           strKey:JSON数据的key值,示例中的"paras"
           strValue:key对应的字符串值 (key=chengdu:key2=chengdu2:key3=chengdu3:)
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonKeyString(const Json::Value& jsValue, const mp_string& jsKey, mp_string& strValue)
{
    if (jsValue.isObject() && jsValue.isMember(jsKey) && jsValue[jsKey].isObject()) {
        const Json::Value& paraJv = jsValue[jsKey];
        Json::Value::Members members = paraJv.getMemberNames();  // 获取所有key的值
        // 遍历每个key
        for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); ++iterMember) {
            mp_string strKey = *iterMember;
            if (paraJv[strKey.c_str()].isString()) {
                mp_string tempValue = paraJv[strKey.c_str()].asString();
                mp_string strKeyValue = strKey + mp_string(STR_EQUAL) + tempValue + STR_COLON;
                strValue.append(strKeyValue);
            } else {
                COMMLOG(OS_LOG_INFO, "The value of Json key \"%s\" is not string.", strKey.c_str());
                return ERROR_COMMON_INVALID_PARAM;
            }
        }
    } else {
        COMMLOG(OS_LOG_WARN, "Json key \"%s\" does not exist.", jsKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

    COMMLOG(OS_LOG_INFO, "GetJsonKeyString success.");
    return MP_SUCCESS;
}

mp_int32 CJsonUtils::ConvertStringtoJson(const mp_string& rawBuffer, Json::Value& jsValue)
{
    COMMLOG(OS_LOG_DEBUG, "ConvertStringtoJson:buffer.size[%d]", rawBuffer.size());
    if (rawBuffer.empty()) {
        COMMLOG(OS_LOG_ERROR, "rawBuffer is empty");
        return MP_FAILED;
    }

    Json::CharReaderBuilder charReaderBuilder;
    Json::CharReader *pCharReader(charReaderBuilder.newCharReader());
    if (!pCharReader) {
        COMMLOG(OS_LOG_ERROR, "pCharReader is null");
        return MP_FAILED;
    }

    std::string strError;
    bool ret = false;
    try {
        ret = pCharReader->parse(rawBuffer.c_str(), rawBuffer.c_str() + rawBuffer.length(), &jsValue, &strError);
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "message buffer is json format, bufferSize[%d] errorMsg:%s",
            rawBuffer.length(), strError.c_str());
        delete pCharReader;
        pCharReader = NULL;
        return MP_FAILED;
    }

    if (!ret) {
        COMMLOG(OS_LOG_ERROR, "Invalid json data: %s, length: %u", rawBuffer.c_str(), rawBuffer.length());
        delete pCharReader;
        pCharReader = NULL;
        return MP_FAILED;
    }
    delete pCharReader;
    pCharReader = NULL;
    return MP_SUCCESS;
}