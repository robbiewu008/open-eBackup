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
#include <sstream>
#include "define/Defines.h"
#include "log/Log.h"
#include "common/JsonUtils.h"

using namespace std;

namespace Module {
/*------------------------------------------------------------
Function Name: GetJsonString
Description  : 获取JSON对象某个元素的字符串值
                jsValue:JSON数据，示例{"data":"chengdu"}
                strKey:JSON数据的key值,示例中的"data"
                strValue:key对应的字符串值
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetJsonString(const Json::Value& jsValue, string strKey, string& strValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return FAILED;
    }

    if (jsValue.isMember(strKey)) {
        if (jsValue[strKey].isString()) {
            strValue = jsValue[strKey].asString();
            return SUCCESS;
        } else {
            COMMLOG(OS_LOG_INFO, "The value Json key \"%s\" is not string.", strKey.c_str());
            return FAILED;
        }
    } else {
        strValue = "";
        COMMLOG(OS_LOG_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return FAILED;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonInt32
Description  : 获取JSON对象某个元素的int32值
               jsValue:JSON数据，示例 {"data":2015}
               strKey:JSON数据的key值,示例中的"data"
               iValue:key对应的int32值
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetJsonInt32(const Json::Value& jsValue, string strKey, int& iValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_ERROR, "Json is Array");
        return FAILED;
    }

    if (jsValue.isMember(strKey)) {
        if (jsValue[strKey].isInt()) {
            iValue = jsValue[strKey].asInt();
            return SUCCESS;
        } else {
            COMMLOG(OS_LOG_ERROR, "The value of Json key \"%s\" is not int.", strKey.c_str());
            return FAILED;
        }
    } else {
        iValue = FAILED;
        COMMLOG(OS_LOG_ERROR, "Json key \"%s\" does not exist.", strKey.c_str());
        return FAILED;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonUInt32
Description  : 获取JSON对象某个元素的uint32值
               jsValue:JSON数据，示例 {"data":2015}
               strKey:JSON数据的key值,示例中的"data"
               iValue:key对应的uint32值
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetJsonUInt32(const Json::Value& jsValue, string strKey, uint32_t& iValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return FAILED;
    }

    if (jsValue.isMember(strKey)) {
        if (jsValue[strKey].isUInt()) {
            iValue = jsValue[strKey].asUInt();
            return SUCCESS;
        } else {
            COMMLOG(OS_LOG_ERROR, "The value of Json key \"%s\" is not int.", strKey.c_str());
            return FAILED;
        }
    } else {
        iValue = 0;
        COMMLOG(OS_LOG_ERROR, "Json key \"%s\" does not exist.", strKey.c_str());
        return FAILED;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonInt64
Description  : 获取JSON对象某个元素的int64值
               jsValue:JSON数据，示例 {"data":2015}
               strKey:JSON数据的key值,示例中的"data"
               lValue:key对应的int64值
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetJsonInt64(const Json::Value& jsValue, string strKey, int64_t& lValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return FAILED;
    }

    if (jsValue.isMember(strKey)) {
        if (jsValue[strKey].isInt()) {
            lValue = jsValue[strKey].asInt64();
            return SUCCESS;
        } else {
            COMMLOG(OS_LOG_INFO, "The value of Json key \"%s\" is not int.", strKey.c_str());
            return FAILED;
        }
    } else {
        lValue = FAILED;
        COMMLOG(OS_LOG_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return FAILED;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonUInt64
Description  : 获取JSON对象某个元素的uint64值
               jsValue:JSON数据，示例 {"data":2015}
               strKey:JSON数据的key值,示例中的"data"
               lValue:key对应的uint64值
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetJsonUInt64(const Json::Value& jsValue, string strKey, uint64_t& lValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return FAILED;
    }

    if (jsValue.isMember(strKey)) {
        if (jsValue[strKey].isUInt64()) {
            lValue = jsValue[strKey].asUInt64();
            return SUCCESS;
        } else {
            COMMLOG(OS_LOG_INFO, "The value of Json key \"%s\" is not uint.", strKey.c_str());
            return FAILED;
        }
    } else {
        COMMLOG(OS_LOG_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return FAILED;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonArrayInt32
Description  : JSON对象某个元素为int32数组
               jsValue:JSON数据，示例 {"data":[2015,2016,2017]}
               strKey:JSON数据的key值，示例中的"data"
               vecValue:key对应的int32数组
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetJsonArrayInt32(const Json::Value& jsValue, string strKey, vector<int>& vecValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return FAILED;
    }

    if (jsValue.isMember(strKey) && jsValue[strKey].isArray()) {
        uint32_t uiSize = jsValue[strKey].size();
        for (uint32_t i = 0; i < uiSize; i++) {
            if (!jsValue[strKey][i].isObject() && !jsValue[strKey][i].isArray() && jsValue[strKey][i].isInt()) {
                vecValue.push_back(jsValue[strKey][i].asInt());
            } else {
                COMMLOG(OS_LOG_INFO, "The value of Json key \"%s\" is not int.", strKey.c_str());
                vecValue.clear();
                return FAILED;
            }
        }
        return SUCCESS;
    } else {
        COMMLOG(OS_LOG_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return FAILED;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonArrayInt64
Description  : JSON对象某个元素为int64数组
               jsValue:JSON数据，示例 {"data":[2015,2016,2017]}
               strKey:JSON数据的key值，示例中的"data"
               vecValue:key对应的int64数组
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetJsonArrayInt64(const Json::Value& jsValue, string strKey, vector<int64_t>& vecValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return FAILED;
    }

    if (jsValue.isMember(strKey) && jsValue[strKey].isArray()) {
        uint32_t uiSize = jsValue[strKey].size();
        for (uint32_t i = 0; i < uiSize; i++) {
            if (!jsValue[strKey][i].isObject() && !jsValue[strKey][i].isArray() && jsValue[strKey][i].isInt()) {
                vecValue.push_back(jsValue[strKey][i].asInt64());
            } else {
                COMMLOG(OS_LOG_INFO, "The value of Json key \"%s\" is not int.", strKey.c_str());
                vecValue.clear();
                return FAILED;
            }
        }
        return SUCCESS;
    } else {
        COMMLOG(OS_LOG_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return FAILED;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonArrayString
Description  : JSON对象某个元素为string数组
               jsValue:JSON数据，示例 {"data":["chengdu","chongqing","nanchong"]}
               strKey:JSON数据的key值，示例中的"data"
               vecValue:key对应的string数组
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetJsonArrayString(const Json::Value& jsValue, string strKey, vector<string>& vecValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_ERROR, "Json is Array");
        return FAILED;
    }

    if (jsValue.isMember(strKey) && jsValue[strKey].isArray()) {
        uint32_t uiSize = jsValue[strKey].size();
        for (uint32_t i = 0; i < uiSize; i++) {
            if (!jsValue[strKey][i].isObject() && !jsValue[strKey][i].isArray() && jsValue[strKey][i].isString()) {
                vecValue.push_back(jsValue[strKey][i].asString());
            } else {
                COMMLOG(OS_LOG_ERROR, "The value of Json key \"%s\" is not int.", strKey.c_str());
                vecValue.clear();
                return FAILED;
            }
        }
        return SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR, "Json key \"%s\" does not exist.", strKey.c_str());
        return FAILED;
    }
}
/*------------------------------------------------------------
Function Name: GetJsonArrayJson
Description  : jsValue: Json数组
               vecVale: 对应的json vector
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetJsonArrayJson(const Json::Value& jsValue, vector<Json::Value>& vecValue)
{
    if (jsValue.isArray()) {
        uint32_t uiSize = jsValue.size();
        for (uint32_t i = 0; i < uiSize; i++) {
            if (jsValue[i].isObject() && !jsValue[i].isArray()) {
                vecValue.push_back(jsValue[i]);
            } else {
                COMMLOG(OS_LOG_INFO, "The value is not Json object.");
                vecValue.clear();
                return FAILED;
            }
        }
        return SUCCESS;
    }
    COMMLOG(OS_LOG_ERROR, "current json is not a jsonArray!");
    return FAILED;
}

/*------------------------------------------------------------
Function Name: GetJsonArrayJson
Description  : JSON对象某个元素为Json数组
               jsValue:JSON数据，示例 {"data":[{"city":"chengdu"},{"city":"chongqing"}]}
               strKey:JSON数据的key值，示例中的"data"
               vecValue:key对应的json数组,数组单元素为json对象，类似于{"city":"chengdu"}
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetJsonArrayJson(const Json::Value& jsValue, string strKey, vector<Json::Value>& vecValue)
{
    if (jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is Array");
        return FAILED;
    }

    if (jsValue.isMember(strKey) && jsValue[strKey].isArray()) {
        uint32_t uiSize = jsValue[strKey].size();
        for (uint32_t i = 0; i < uiSize; i++) {
            if (jsValue[strKey][i].isObject() && !jsValue[strKey][i].isArray()) {
                vecValue.push_back(jsValue[strKey][i]);
            } else {
                COMMLOG(OS_LOG_INFO, "The value of Json key \"%s\" is not Json object.", strKey.c_str());
                vecValue.clear();
                return FAILED;
            }
        }
        return SUCCESS;
    } else {
        COMMLOG(OS_LOG_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return FAILED;
    }
}
/*------------------------------------------------------------
Function Name: GetArrayInt32
Description  : 该JSON对象为int32数组，且没有key值
               jsValue:JSON数据，示例 {[1,2,3]}
               vecValue:转换成的int32数组
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetArrayInt32(const Json::Value& jsValue, vector<int>& vecValue)
{
    if (!jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is not array");
        return FAILED;
    }

    uint32_t uiSize = jsValue.size();
    for (uint32_t i = 0; i < uiSize; i++) {
        if (!jsValue[i].isObject() && !jsValue[i].isArray()) {
            vecValue.push_back(jsValue[i].asInt());
        } else {
            COMMLOG(OS_LOG_INFO, "Json value is array or object.");
            return FAILED;
        }
    }

    return SUCCESS;
}
/*------------------------------------------------------------
Function Name: GetArrayInt64
Description  : 该JSON对象为int64数组，且没有key值
               jsValue:JSON数据，示例 {[1,2,3]}
               vecValue:转换成的int64数组
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetArrayInt64(const Json::Value& jsValue, vector<int64_t>& vecValue)
{
    if (!jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is not array");
        return FAILED;
    }

    uint32_t uiSize = jsValue.size();
    for (uint32_t i = 0; i < uiSize; i++) {
        if (!jsValue[i].isObject() && !jsValue[i].isArray()) {
            vecValue.push_back(jsValue[i].asInt64());
        } else {
            COMMLOG(OS_LOG_INFO, "Json value is array or object.");
            return FAILED;
        }
    }

    return SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetArrayStringWithoutBraces
Description  : 该JSON对象为string数组，且没有key值，不是json对象（没有大括号）
               jsValue:JSON数据，示例 没有大括号
                "notifyInfo" : ["127.0.0.1", "127.0.0.2"]
                vecValue:转换成的string数组
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetArrayStringWithoutBraces(const Json::Value& jsValue, vector<string>& vecValue)
{
    if (!jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is not array");
        return FAILED;
    }

    for (auto iter = jsValue.begin(); iter != jsValue.end(); ++iter) {
        if (iter->isString()) {
            vecValue.push_back(iter->asString());
        } else {
            COMMLOG(OS_LOG_INFO, "Json value is not string.");
            return FAILED;
        }
    }
    return SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetArrayInt32WithoutBraces
Description  : 该JSON对象为string数组，且没有key值，不是json对象（没有大括号）
               jsValue:JSON数据，示例 没有大括号
                "notifyInfo" : [12, 34]
                vecValue:转换成的int32数组
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetArrayInt32WithoutBraces(const Json::Value& jsValue, vector<int>& vecValue)
{
    if (!jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is not array");
        return FAILED;
    }

    for (auto iter = jsValue.begin(); iter != jsValue.end(); ++iter) {
        if (iter->isInt()) {
            vecValue.push_back(iter->asInt());
        } else {
            COMMLOG(OS_LOG_INFO, "Json value is not int.");
            return FAILED;
        }
    }
    return SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetArrayString
Description  : 该JSON对象为string数组，且没有key值
               jsValue:JSON数据，示例 {["chengdu","chongqing","nanchong"]}
               vecValue:转换成的string数组
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetArrayString(const Json::Value& jsValue, vector<string>& vecValue)
{
    if (!jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is not array");
        return FAILED;
    }

    uint32_t uiSize = jsValue.size();
    for (uint32_t i = 0; i < uiSize; i++) {
        if (!jsValue[i].isObject() && !jsValue[i].isArray()) {
            vecValue.push_back(jsValue[i].asString());
        } else {
            COMMLOG(OS_LOG_INFO, "Json value is array or object.");
            return FAILED;
        }
    }

    return SUCCESS;
}
/*------------------------------------------------------------
Function Name: GetArrayJson
Description  : 该JSON对象为json数组，且没有key值
               jsValue:JSON数据，示例 {[{"city":"chengdu"},{"city":"chonqing"}]}
               vecValue:转换成的json数组
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetArrayJson(const Json::Value& jsValue, vector<Json::Value>& vecValue)
{
    if (!jsValue.isArray()) {
        COMMLOG(OS_LOG_INFO, "Json is not array");
        return FAILED;
    }

    uint32_t uiSize = jsValue.size();
    for (uint32_t i = 0; i < uiSize; i++) {
        if (jsValue[i].isObject() && !jsValue[i].isArray()) {
            vecValue.push_back(jsValue[i]);
        } else {
            COMMLOG(OS_LOG_INFO, "Json value is not object.");
            return FAILED;
        }
    }

    return SUCCESS;
}
/*------------------------------------------------------------
Function Name: GetJsonKeyString
Description  : 获取JSON对象某个元素的字符串值,并且按照指定格式拼接参数
           jsValue:JSON数据，示例"paras": {"key":"chengdu","key2":"chengdu2","key3":"chengdu3"...}
           strKey:JSON数据的key值,示例中的"paras"
           strValue:key对应的字符串值 (key=chengdu:key2=chengdu2:key3=chengdu3:)
Others       :-------------------------------------------------------------*/
int CJsonUtils::GetJsonKeyString(const Json::Value& jsValue, string jsKey, string& strValue)
{
    if (jsValue.isMember(jsKey)) {
        const Json::Value& paraJv = jsValue[jsKey];
        Json::Value::Members members = paraJv.getMemberNames();  // 获取所有key的值
        // 遍历每个key
        for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); iterMember++) {
            string strKey = *iterMember;
            if (paraJv[strKey.c_str()].isString()) {
                string tempValue = paraJv[strKey.c_str()].asString();
                string strKeyValue = strKey + string(STR_EQUAL) + tempValue + STR_COLON;
                strValue.append(strKeyValue);
            } else {
                COMMLOG(OS_LOG_INFO, "The value of Json key \"%s\" is not string.", strKey.c_str());
                return FAILED;
            }
        }
    } else {
        COMMLOG(OS_LOG_ERROR, "Json key \"%s\" does not exist.", jsKey.c_str());
        return FAILED;
    }

    COMMLOG(OS_LOG_INFO, "GetJsonKeyString success.");
    return SUCCESS;
}

int CJsonUtils::ConvertStringtoJson(const string& rawBuffer, Json::Value& jsValue)
{
    COMMLOG(OS_LOG_DEBUG, "ConvertStringtoJson:buffer.size[%d]", rawBuffer.size());
    if (rawBuffer.empty()) {
        COMMLOG(OS_LOG_ERROR, "rawBuffer is empty");
        return FAILED;
    }

    Json::CharReaderBuilder charReaderBuilder;
    Json::CharReader *pCharReader(charReaderBuilder.newCharReader());
    if (!pCharReader) {
        COMMLOG(OS_LOG_ERROR, "pCharReader is null");
        return FAILED;
    }

    string strError;
    bool ret = false;
    try {
        ret = pCharReader->parse(rawBuffer.c_str(), rawBuffer.c_str() + rawBuffer.length(), &jsValue, &strError);
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "message buffer is json format, bufferSize[%d] errorMsg:%s",
            rawBuffer.length(), strError.c_str());
        delete pCharReader;
        pCharReader = NULL;
        return FAILED;
    }

    if (!ret) {
        COMMLOG(OS_LOG_ERROR, "Invalid json data: %s, length: %u", rawBuffer.c_str(), rawBuffer.length());
        delete pCharReader;
        pCharReader = NULL;
        return FAILED;
    }
    delete pCharReader;
    pCharReader = NULL;
    return SUCCESS;
}

std::string CJsonUtils::ConvertJsonToString(const Json::Value& jsValue)
{
    Json::FastWriter fastWriterValue;
    std::string sValue = fastWriterValue.write(jsValue);
    std::size_t len = sValue.length();
    std::size_t pos = sValue.find("\n");
    if (pos == (std::size_t)(len - 1)) {
        sValue = sValue.substr(0, pos);
    }

    return sValue;
}

std::string CJsonUtils::JsonToString(const Json::Value& jsValue)
{
    Json::FastWriter fastWriterValue;
    std::string sValue = fastWriterValue.write(jsValue);
    std::size_t len = sValue.length();
    std::size_t pos = sValue.find("\n");  //delete the '\n'
    if(pos == (std::size_t)(len - 1))
    {
        sValue = sValue.substr(0,pos);
    }

    return sValue;
}

bool CJsonUtils::StringToJson(const std::string &sJosn, Json::Value& jsValue)
{
    Json::Reader reader;

    if(!reader.parse(sJosn ,jsValue))
    {
        COMMLOG(OS_LOG_ERROR, "StringToJson Json reader parse failed !");
        return false;
    }

    return true;
}

} // namespace
