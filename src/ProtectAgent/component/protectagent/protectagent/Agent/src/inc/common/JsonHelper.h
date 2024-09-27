/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file JobServiceHandler.h
 * @brief  Factory for Service Center
 * @version 1.1.0
 * @date 2021-08-31
 * @author kangjiawei WX884906
 */
#ifndef JSON_HELPER_HPP
#define JSON_HELPER_HPP
#include <memory>
#include <list>
#include <set>
#include "jsoncpp/include/json/json.h"
#include "common/Log.h"

#define BEGIN_SERIAL_MEMEBER                                                                                           \
    void Serial(Json::Value& jsonValue, const bool serial) {
#define END_SERIAL_MEMEBER }

#define SERIAL_MEMEBER(name)                                                                                           \
    if (serial) {                                                                                                      \
        if (jsonValue.isObject() && jsonValue.isMember(#name)) {                                                       \
            JsonHelper::JsonValueToType(name, jsonValue[#name]);                                                       \
        }                                                                                                              \
    } else {                                                                                                           \
        JsonHelper::TypeToJsonValue(name, jsonValue[#name]);                                                           \
    }

#define SERIAL_MEMBER_TO_SPECIFIED_NAME(value, name)                                                                   \
    if (serial) {                                                                                                      \
        if (jsonValue.isObject() && jsonValue.isMember(#name)) {                                                       \
            JsonHelper::JsonValueToType(value, jsonValue[#name]);                                                      \
        }                                                                                                              \
    } else {                                                                                                           \
        JsonHelper::TypeToJsonValue(value, jsonValue[#name]);                                                          \
    }

namespace JsonHelper {
template<class T>
inline bool JsonValueToStruct(Json::Value& jsonValue, T& t)
{
    T tmpValue;
    try {
        tmpValue.Serial(jsonValue, true);
    } catch (std::exception& e) {
        COMMLOG(OS_LOG_ERROR, "Json value to struct throw a exception. %s", e.what());
        return false;
    }

    t = std::move(tmpValue);
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
    std::string strError;
    bool ret = false;
    try {
        const char* end = begin + jsonString.size();
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
    std::string strError;
    Json::Value root;
    bool ret = false;
    try {
        const char* end = begin + jsonString.length();
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
inline bool StructToJsonValue(T& t, Json::Value& jsonValue)
{
    try {
        t.Serial(jsonValue, false);
    } catch (std::exception& e) {
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
inline void JsonValueToType(T& value, Json::Value& jsonValue)
{
    value.Serial(jsonValue, true);
}

template<>
inline void JsonValueToType<bool>(bool& value, Json::Value& jsonValue)
{
    if (jsonValue.isBool()) {
        value = jsonValue.asBool();
    }
}

template<>
inline void JsonValueToType<uint8_t>(uint8_t& value, Json::Value& jsonValue)
{
    if (jsonValue.isUInt()) {
        value = (uint8_t)jsonValue.asUInt();
    }
}

template<>
inline void JsonValueToType<uint16_t>(uint16_t& value, Json::Value& jsonValue)
{
    if (jsonValue.isUInt()) {
        value = (uint16_t)jsonValue.asUInt();
    }
}

template<>
inline void JsonValueToType<int32_t>(int32_t& value, Json::Value& jsonValue)
{
    if (jsonValue.isInt()) {
        value = jsonValue.asInt();
    }
}

template<>
inline void JsonValueToType<uint32_t>(uint32_t& value, Json::Value& jsonValue)
{
    if (jsonValue.isUInt()) {
        value = jsonValue.asUInt();
    }
}

template<>
inline void JsonValueToType<int64_t>(int64_t& value, Json::Value& jsonValue)
{
    if (jsonValue.isInt64()) {
        value = jsonValue.asInt64();
    }
}

template<>
inline void JsonValueToType<uint64_t>(uint64_t& value, Json::Value& jsonValue)
{
    if (jsonValue.isUInt64()) {
        value = jsonValue.asUInt64();
    }
}

template<>
inline void JsonValueToType<std::string>(std::string& value, Json::Value& jsonValue)
{
    if (jsonValue.isString()) {
        value = jsonValue.asString();
    }
}

template<>
inline void JsonValueToType<double>(double& value, Json::Value& jsonValue)
{
    if (jsonValue.isDouble()) {
        value = jsonValue.asDouble();
    }
}

template<>
inline void JsonValueToType<float>(float& value, Json::Value& jsonValue)
{
    if (jsonValue.isDouble()) {
        value = jsonValue.asFloat();
    }
}

template<class T>
inline void JsonValueToType(std::vector<T>& value, Json::Value& jsonValue)
{
    value.reserve(sizeof(T) * jsonValue.size());
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
inline void TypeToJsonValue(T& value, Json::Value& jsonValue)
{
    value.Serial(jsonValue, false);
}

template<>
inline void TypeToJsonValue<bool>(bool& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<uint8_t>(uint8_t& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<uint16_t>(uint16_t& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<int32_t>(int32_t& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<uint32_t>(uint32_t& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<int64_t>(int64_t& value, Json::Value& jsonValue)
{
    jsonValue = (Json::Int64)value;
}

template<>
inline void TypeToJsonValue<uint64_t>(uint64_t& value, Json::Value& jsonValue)
{
    jsonValue = (Json::UInt64)value;
}

template<>
inline void TypeToJsonValue<std::string>(std::string& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<const std::string>(const std::string& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<double>(double& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<>
inline void TypeToJsonValue<float>(float& value, Json::Value& jsonValue)
{
    jsonValue = value;
}

template<class T>
inline void TypeToJsonValue(std::vector<T>& value, Json::Value& jsonValue)
{
    for (auto it = value.begin(); it != value.end(); ++it) {
        Json::Value temp;
        TypeToJsonValue(*it, temp);
        jsonValue.append(std::move(temp));
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
inline void JsonValueToType(T& value, const Json::Value& jsonValue)
{
    value.Serial(jsonValue, true);
}

template<>
inline void JsonValueToType<bool>(bool& value, const Json::Value& jsonValue)
{
    if (jsonValue.isBool()) {
        value = jsonValue.asBool();
    }
}

template<>
inline void JsonValueToType<uint8_t>(uint8_t& value, const Json::Value& jsonValue)
{
    if (jsonValue.isUInt()) {
        value = (uint8_t)jsonValue.asUInt();
    }
}

template<>
inline void JsonValueToType<int32_t>(int32_t& value, const Json::Value& jsonValue)
{
    if (jsonValue.isInt()) {}
    value = jsonValue.asInt();
}

template<>
inline void JsonValueToType<uint32_t>(uint32_t& value, const Json::Value& jsonValue)
{
    if (jsonValue.isUInt()) {
        value = jsonValue.asUInt();
    }
}

template<>
inline void JsonValueToType<int64_t>(int64_t& value, const Json::Value& jsonValue)
{
    if (jsonValue.isInt64()) {
        value = jsonValue.asInt64();
    }
}

template<>
inline void JsonValueToType<uint64_t>(uint64_t& value, const Json::Value& jsonValue)
{
    if (jsonValue.isUInt64()) {
        value = jsonValue.asUInt64();
    }
}

template<>
inline void JsonValueToType<std::string>(std::string& value, const Json::Value& jsonValue)
{
    if (jsonValue.isString()) {
        value = jsonValue.asString();
    }
}

template<>
inline void JsonValueToType<double>(double& value, const Json::Value& jsonValue)
{
    if (jsonValue.isDouble()) {
        value = jsonValue.asDouble();
    }
}

template<>
inline void JsonValueToType<float>(float& value, const Json::Value& jsonValue)
{
    if (jsonValue.isDouble()) {
        value = jsonValue.asFloat();
    }
}

template<class T>
inline void JsonValueToType(std::vector<T>& value, const Json::Value& jsonValue)
{
    value.reserve(sizeof(T) * jsonValue.size());
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
        jsonValue.append(std::move(temp));
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
    if (jsonValue.isObject() && jsonValue.isMember(name)) {
        JsonHelper::JsonValueToType(v, jsonValue[name]);
    }
}

template<class T>
inline void JsonToStructByName(T& v, const std::string& name, const Json::Value& jsonValue)
{
    if (jsonValue.isObject() && jsonValue.isMember(name)) {
        JsonHelper::JsonValueToType(v, jsonValue[name]);
    }
}
}  // namespace JsonHelper
#endif  // JSON_HELPER_HPP
