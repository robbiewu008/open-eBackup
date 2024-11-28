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
#ifndef MODULE_JSON_HELPER_H
#define MODULE_JSON_HELPER_H
#include <memory>
#include <list>
#include <set>
#include "json/json.h"
#include "log/Log.h"

namespace Module {

#define BEGIN_SERIAL_MEMEBER                                                                                           \
    void Serial(Json::Value& jsonValue, const bool serial) {

#define END_SERIAL_MEMEBER }

#define SERIAL_MEMEBER(name)                                                                                           \
    if (serial) {                                                                                                      \
        if (jsonValue.isMember(#name)) {                                                                               \
            Module::JsonHelper::JsonValueToType(name, jsonValue[#name]);                                                       \
        }                                                                                                              \
    } else {                                                                                                           \
        Module::JsonHelper::TypeToJsonValue(name, jsonValue[#name]);                                                           \
    }

#define SERIAL_MEMBER_TO_SPECIFIED_NAME(value, name)                                                                   \
    if (serial) {                                                                                                      \
        if (jsonValue.isMember(#name)) {                                                                               \
            Module::JsonHelper::JsonValueToType(value, jsonValue[#name]);                                                      \
        }                                                                                                              \
    } else {                                                                                                           \
        Module::JsonHelper::TypeToJsonValue(value, jsonValue[#name]);                                                          \
    }

#define SERIAL_MEMBER_TO_STRING_SPECIFIED_NAME(value, name, useNull)                                                   \
    if (serial) {                                                                                                      \
        if (jsonValue.isMember(#name)) {                                                                               \
            Module::JsonHelper::JsonValueToType(value, jsonValue[#name]);                                              \
        }                                                                                                              \
    } else {                                                                                                           \
        Module::JsonHelper::StringTypeToJsonValue(value, jsonValue[#name], useNull);                                   \
    }

namespace JsonHelper {
template<class T>
inline bool JsonValueToStruct(Json::Value& jsonValue, T& t)
{
    T tmpValue {};
    try {
        tmpValue.Serial(jsonValue, true);
    } catch (std::exception& e) {
        COMMLOG(OS_LOG_ERROR, "Json value to struct throw a exception. %s", e.what());
        return false;
    }
    t = tmpValue;
    return true;
}

inline bool JsonStringToJsonValue(const std::string& jsonString, Json::Value& value)
{
    if (jsonString.empty()) {
        COMMLOG(OS_LOG_WARN, "jsonString is empty.");
        return false;
    }
    Json::CharReaderBuilder charReaderBuilder;
    std::unique_ptr<Json::CharReader> pCharReader(charReaderBuilder.newCharReader());
    if (!pCharReader) {
        COMMLOG(OS_LOG_ERROR, "CharReaderBuilder is null.");
        return false;
    }
    const char* begin = jsonString.c_str();
    const char* end = begin + jsonString.size();
    std::string strError;
    bool ret = false;
    try {
        ret = pCharReader->parse(begin, end, &value, &strError);
    } catch (std::exception& e) {
        COMMLOG(OS_LOG_ERROR, "failed to parse, throw a exception. %s", e.what());
        return false;
    }
    if (!ret) {
        COMMLOG(OS_LOG_ERROR, "Json parse failed, error: %s", strError.c_str());
        return false;
    }
    return true;
}

template<class T>
inline bool JsonStringToStruct(const std::string& jsonString, T& t)
{
    if (jsonString.empty()) {
        COMMLOG(OS_LOG_ERROR, "Json is empty, return.");
        return false;
    }
    Json::CharReaderBuilder charReaderBuilder;
    std::shared_ptr<Json::CharReader> pCharReader(charReaderBuilder.newCharReader());
    if (pCharReader.get() == nullptr) {
        COMMLOG(OS_LOG_ERROR, "CharReaderBuilder is null.");
        return false;
    }
    const char* begin = jsonString.c_str();
    const char* end = begin + jsonString.length();
    std::string strError;
    Json::Value root;
    bool ret = false;
    try {
        ret = pCharReader->parse(begin, end, &root, &strError);
    } catch (std::exception& e) {
        COMMLOG(OS_LOG_ERROR, "failed to parse, throw a exception. %s", e.what());
        return false;
    }
    if (!ret) {
        COMMLOG(OS_LOG_ERROR, "Json parse failed, error: %s", strError.c_str());
        return false;
    }
    return JsonValueToStruct(root, t);
}

template<class T>
inline auto StructToJsonValue(T& t, Json::Value& jsonValue) -> decltype(t.Serial(jsonValue, false), bool())
{
	try {
		t.Serial(jsonValue, false);
	}
	catch (std::exception& e) {
		COMMLOG(OS_LOG_ERROR, "struct to json value throw a exception. %s", e.what());
		return false;
	}
	return true;
}


template<class T>
inline auto StructToJsonValue(T& t, Json::Value& jsonValue) -> decltype(t->Serial(jsonValue, false), bool())
{
	try {
		t->Serial(jsonValue, false);
	}
	catch (std::exception& e) {
		COMMLOG(OS_LOG_ERROR, "struct to json value throw a exception. %s", e.what());
		return false;
	}
	return true;
}

template<class T>
inline bool StructToJsonString(T& t, std::string& jsonString)
{
    Json::Value root;
    bool ret = StructToJsonValue(t, root);
    if (!ret) {
        COMMLOG(OS_LOG_ERROR, "struct to json value failed.");
        return false;
    }
	Json::FastWriter jsonWriter;
	jsonString = jsonWriter.write(root);
    return true;
}

template<class T>
inline auto JsonValueToType(T& value, Json::Value& jsonValue) -> decltype(value.Serial(jsonValue, false))
{
    value.Serial(jsonValue, true);
}

template<class T>
inline auto JsonValueToType(T& value, Json::Value& jsonValue) -> decltype(value->Serial(jsonValue, false))
{
	value->Serial(jsonValue, true);
}

template <typename T, typename std::enable_if<std::is_same<T, bool>::value>::type* = nullptr>
inline void JsonValueToType(T& value, Json::Value& jsonValue)
{
    if (jsonValue.isBool()) {
        value = jsonValue.asBool();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, bool>::value>::type* = nullptr>
inline void JsonValueToType(T& value, const Json::Value& jsonValue)
{
    if (jsonValue.isBool()) {
        value = jsonValue.asBool();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value>::type* = nullptr>
inline void JsonValueToType(T& value, Json::Value& jsonValue)
{
    if (jsonValue.isUInt()) {
        value = static_cast<T>(jsonValue.asUInt());
    }
}

template <typename T, typename std::enable_if<std::is_same<T, int32_t>::value>::type* = nullptr>
inline void JsonValueToType(T& value, Json::Value& jsonValue)
{
    if (jsonValue.isInt()) {
        value = jsonValue.asInt();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, uint32_t>::value>::type* = nullptr>
inline void JsonValueToType(T& value, Json::Value& jsonValue)
{
    if (jsonValue.isUInt()) {
        value = jsonValue.asUInt();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, int64_t>::value || std::is_same<T, long>::value>::type* = nullptr>
inline void JsonValueToType(T& value, Json::Value& jsonValue)
{
    if (jsonValue.isInt64()) {
        value = jsonValue.asInt64();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, uint64_t>::value>::type* = nullptr>
inline void JsonValueToType(T& value, Json::Value& jsonValue)
{
    if (jsonValue.isUInt64()) {
        value = jsonValue.asUInt64();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, std::string>::value>::type* = nullptr>
inline void JsonValueToType(T& value, Json::Value& jsonValue)
{
    if (jsonValue.isString()) {
        value = jsonValue.asString();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, double>::value>::type* = nullptr>
inline void JsonValueToType(T& value, Json::Value& jsonValue)
{
    if (jsonValue.isDouble()) {
        value = jsonValue.asDouble();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, float>::value>::type* = nullptr>
inline void JsonValueToType(T& value, Json::Value& jsonValue)
{
    if (jsonValue.isDouble()) {
        value = jsonValue.asFloat();
    }
}

template<class T>
inline void JsonValueToType(std::vector<T>& value, Json::Value& jsonValue)
{
    for (Json::ArrayIndex index = 0; index < jsonValue.size(); ++index) {
        T ele;
        JsonValueToType(ele, jsonValue[index]);
        value.push_back(ele);
    }
}

template<class T>
inline void JsonValueToType(std::set<T>& value, Json::Value& jsonValue)
{
    for (Json::ArrayIndex index = 0; index < jsonValue.size(); ++index) {
        T ele;
        JsonValueToType(ele, jsonValue[index]);
        value.insert(ele);
    }
}

template<class T>
inline void JsonValueToType(std::list<T>& value, Json::Value& jsonValue)
{
    for (Json::ArrayIndex index = 0; index < jsonValue.size(); ++index) {
        T ele;
        JsonValueToType(ele, jsonValue[index]);
        value.push_back(ele);
    }
}

template<class T>
inline auto TypeToJsonValue(T& value, Json::Value& jsonValue) -> decltype(value.Serial(jsonValue, true))
{
    value.Serial(jsonValue, false);
}

template<class T>
inline auto TypeToJsonValue(T& value, Json::Value& jsonValue) -> decltype(value->Serial(jsonValue, true))
{
	value->Serial(jsonValue, false);
}

template <typename T,
	typename std::enable_if<
	(std::is_integral<T>::value ||
	std::is_same<std::string, T>::value ||
	std::is_same<const std::string, T>::value ||
	std::is_floating_point<T>::value) &&
	(!std::is_same<T, int64_t>::value && !std::is_same<T, uint64_t>::value)
	>::type* = nullptr>
inline void TypeToJsonValue(T& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

// TODO:: i think these two template is unnecessay and can be merged to above
template <typename T, typename std::enable_if<std::is_same<T, int64_t>::value>::type* = nullptr>
inline void TypeToJsonValue(T& value, Json::Value& jsonValue)
{
    jsonValue = (Json::Int64)value;
}

template <typename T, typename std::enable_if<std::is_same<T, uint64_t>::value>::type* = nullptr>
inline void TypeToJsonValue(T& value, Json::Value& jsonValue)
{
    jsonValue = (Json::UInt64)value;
}

template<class T>
inline void TypeToJsonValue(std::vector<T>& value, Json::Value& jsonValue)
{
    for (auto it = value.begin(); it != value.end(); ++it) {
        Json::Value temp;
        TypeToJsonValue(*it, temp);
        jsonValue.append(temp);
    }
}

template<class T>
inline void TypeToJsonValue(std::set<T>& value, Json::Value& jsonValue)
{
    for (auto it = value.begin(); it != value.end(); ++it) {
        Json::Value temp;
        TypeToJsonValue(*it, temp);
        jsonValue.append(temp);
    }
}

template<class T>
inline auto JsonValueToType(T& value, const Json::Value& jsonValue) -> decltype(value.Serial(jsonValue, true))
{
    value.Serial(jsonValue, true);
}

template<class T>
inline auto JsonValueToType(T& value, const Json::Value& jsonValue) -> decltype(value->Serial(jsonValue, true))
{
	value->Serial(jsonValue, true);
}

template <typename T, typename std::enable_if<std::is_same<T, uint8_t>::value>::type* = nullptr>
inline void JsonValueToType(T& value, const Json::Value& jsonValue)
{
    if (jsonValue.isUInt()) {
        value = (uint8_t)jsonValue.asUInt();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, int32_t>::value>::type* = nullptr>
inline void JsonValueToType(T& value, const Json::Value& jsonValue)
{
    if (jsonValue.isInt()) {}
    value = jsonValue.asInt();
}

template <typename T, typename std::enable_if<std::is_same<T, uint32_t>::value>::type* = nullptr>
inline void JsonValueToType(T& value, const Json::Value& jsonValue)
{
    if (jsonValue.isUInt()) {
        value = jsonValue.asUInt();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, int64_t>::value>::type* = nullptr>
inline void JsonValueToType(T& value, const Json::Value& jsonValue)
{
    if (jsonValue.isInt64()) {
        value = jsonValue.asInt64();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, uint64_t>::value>::type* = nullptr>
inline void JsonValueToType(T& value, const Json::Value& jsonValue)
{
    if (jsonValue.isUInt64()) {
        value = jsonValue.asUInt64();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, std::string>::value>::type* = nullptr>
inline void JsonValueToType(T& value, const Json::Value& jsonValue)
{
    if (jsonValue.isString()) {
        value = jsonValue.asString();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, double>::value>::type* = nullptr>
inline void JsonValueToType(T& value, const Json::Value& jsonValue)
{
    if (jsonValue.isDouble()) {
        value = jsonValue.asDouble();
    }
}

template <typename T, typename std::enable_if<std::is_same<T, float>::value>::type* = nullptr>
inline void JsonValueToType(T& value, const Json::Value& jsonValue)
{
    if (jsonValue.isDouble()) {
        value = jsonValue.asFloat();
    }
}

template<class T>
inline void JsonValueToType(std::vector<T>& value, const Json::Value& jsonValue)
{
    for (Json::ArrayIndex index = 0; index < jsonValue.size(); ++index) {
        T ele;
        JsonValueToType(ele, jsonValue[index]);
        value.push_back(ele);
    }
}

template<class T>
inline void TypeToJsonValue(const T& value, Json::Value& jsonValue)
{
    value.Serial(jsonValue, false);

}

template<>
inline void TypeToJsonValue<bool>(const bool& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<uint8_t>(const uint8_t& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<int32_t>(const int32_t& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<uint32_t>(const uint32_t& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<int64_t>(const int64_t& value, Json::Value& jsonValue)
{
    jsonValue = (Json::Int64)value;
}

template<>
inline void TypeToJsonValue<uint64_t>(const uint64_t& value, Json::Value& jsonValue)
{
    jsonValue = (Json::UInt64)value;
}

template<>
inline void TypeToJsonValue<std::string>(const std::string& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<double>(const double& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

inline void StringTypeToJsonValue(const std::string& value, Json::Value& jsonValue, bool useNull)
{
    if (value.empty() && useNull) {
        jsonValue = Json::Value::null;
    } else {
        jsonValue = value;
    }
}

template<>
inline void TypeToJsonValue<float>(const float& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<class T>
inline void TypeToJsonValue(const std::vector<T>& value, Json::Value& jsonValue)
{
    for (auto it = value.begin(); it != value.end(); ++it) {
        Json::Value temp;
        TypeToJsonValue(*it, temp);
        jsonValue.append(temp);
    }
}

template<class T>
inline void TypeToJsonValue(const std::set<T>& value, Json::Value& jsonValue)
{
    for (auto it = value.begin(); it != value.end(); ++it) {
        Json::Value temp;
        TypeToJsonValue(*it, temp);
        jsonValue.append(temp);
    }
}

template<class T>
inline void TypeToJsonValue(std::list<T>& value, Json::Value& jsonValue)
{
    for (auto it = value.begin(); it != value.end(); ++it) {
        Json::Value temp;
        TypeToJsonValue(*it, temp);
        jsonValue.append(temp);
    }
}

template<class T>
inline bool JsonStringToType(const std::string& jsonString, std::vector<T>& t)
{
    Json::Reader jsonReader;
    Json::Value root;
    bool ret = jsonReader.parse(jsonString, root);
    if (!ret) {
        COMMLOG(OS_LOG_ERROR, "Json string to struct failed : %s", jsonString.c_str());
        return false;
    }
    JsonValueToType(t, root);
    return true;
}

template<class T>
inline void JsonToStructByName(T& v, const std::string& name, Json::Value& jsonValue)
{
    if (jsonValue.isMember(name)) {
        JsonHelper::JsonValueToType(v, jsonValue[name]);
    }
}

template<class T>
inline void JsonToStructByName(T& v, const std::string& name, const Json::Value& jsonValue)
{
    if (jsonValue.isMember(name)) {
        JsonHelper::JsonValueToType(v, jsonValue[name]);
    }
}
}  // namespace JsonHelper

} // namespace Module

#endif  // JSON_HELPER_HPP
