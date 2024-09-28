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
#include "ParamChecker.h"
#include <map>
#include <set>
#include <vector>
#include <regex>
#include <codecvt>
#include <tinyxml2.h>
#include "common/File.h"
#include "log/Log.h"
#include "define/Types.h"
#include "ParamCheckCfgLoader.h"

namespace Module {
using namespace std;
using namespace tinyxml2;
bool IntRuleChecker::Check(const std::string& targetParam)
{
    std::string strParam;
    if (targetParam[0] == '-') {
        strParam = targetParam.substr(1);
    } else {
        strParam = targetParam;
    }

    if (!boost::all(strParam, boost::is_digit())) {
        ERRLOG("IntRuleChecker falied, target param: [%s] is not a digit", strParam.c_str());
        return false;
    }

    if ((targetParam[0] == '0') && (targetParam.length() != 1)) {
        ERRLOG("IntRuleChecker failed, target param: [%s] begin with 0 but not 0", targetParam.c_str());
        return false;
    }

    long long v;
    std::istringstream iss(targetParam);
    iss >> v;
    bool ret = (v >= from && v <= to);
    if (!ret) {
        ERRLOG("Check failed, v: %lld, from: %lld, to: %lld", v, from, to);
    }
    return ret;
}

bool LengthRuleChecker::Check(const std::string& targetParam)
{
    if (targetParam.length() >= static_cast<unsigned long long>(from)) {
        if (targetParam.length() <= static_cast<unsigned long long>(to)) {
            return true;
        }
    }
    ERRLOG("Check failed, targetParam.length()=%llu, from: %lld, to: %lld", targetParam.length(), from, to);
    return false;
}

bool StrRuleChecker::Check(const std::string& targetParam)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wstrTargetParam = converter.from_bytes(targetParam);
    std::wstring wPattern = converter.from_bytes(pattern);
    std::wregex wReg(wPattern);
    bool ret = std::regex_match(wstrTargetParam, wReg);
    if (!ret) {
        ERRLOG("Check failed, pattern: %s, value: %s", pattern.c_str(), WIPE_SENSITIVE(targetParam).c_str());
    }
    return ret;
}

bool ParamChecker::Check(const Json::Value& msgArgsJv)
{
    if (!msgArgsJv.isArray()) {
        return ElememtCheck(msgArgsJv);
    }
    for (auto eleJv : msgArgsJv) {
        if (!ElememtCheck(eleJv)) {
            return false;
        }
    }
    return true;
}

bool ParamChecker::ElememtCheck(const Json::Value& msgArgsJv)
{
    if (m_ruleCheckers.empty()) {
        ERRLOG("No rule checker for %s", m_name.c_str());
        return false;
    }

    if (msgArgsJv.type() == Json::stringValue && msgArgsJv.asString().empty()) {
        DBGLOG("Check param success, name: %s has empty value!", m_name.c_str());
        return true;
    }

    std::string argValue;
    if (msgArgsJv.type() == Json::stringValue) {
        argValue = msgArgsJv.asString();
    } else {
        Json::StreamWriterBuilder builder;
        argValue = Json::writeString(builder, msgArgsJv);
    }

    for (auto& ck : m_ruleCheckers) {
        if (!ck->Check(argValue)) {
            ERRLOG("param: [%s] check failed, value: %s", m_name.c_str(), argValue.c_str());
            return false;
        }
    }
    return true;
}

bool MemberCheck::Check(const Json::Value& msgArgsJv)
{
    if (!msgArgsJv.isArray()) {
        return ElememtCheck(msgArgsJv);
    }
    for (auto eleJv : msgArgsJv) {
        if (!ElememtCheck(eleJv)) {
            return false;
        }
    }
    return true;
}

bool MemberCheck::ElememtCheck(const Json::Value& msgArgsJv)
{
    Json::Value::Members members = msgArgsJv.getMemberNames();  // 获取所有key的值
    for (const std::string& argName : members) {
        if (m_paramCheckers.count(argName) != 0) {
            if (msgArgsJv[argName].empty()) {
                DBGLOG("Check struct success, name: %s has empty value!", argName.c_str());
                continue;
            }
            if (!m_paramCheckers[argName]->Check(msgArgsJv[argName])) {
                ERRLOG("Param check failed, name: %s", argName.c_str());
                return false;
            }
        }
        if (m_subStructs.count(argName) != 0) {
            if (!StructChecker::Instance().Check(m_subStructs[argName], msgArgsJv[argName])) {
                ERRLOG("Check sub struct failed, name: %s", argName.c_str());
                return false;
            }
        }
    }
    return true;
}

StructChecker& StructChecker::Instance()
{
    static StructChecker checker;
    return checker;
}

void StructChecker::Init(const std::string& xmlPath)
{
    if (ParamCheckCfgLoader::Instance().Load(xmlPath) != Module::SUCCESS) {
        WARNLOG("Initialize StructChecker failed, disable parameter checker.");
        m_button = false;
        return;
    }
    DBGLOG("Initialize StructChecker success, enable parameter checker.");
    m_button = true;
    return;
}

bool StructChecker::Check(const std::string& key, const Json::Value& args)
{
    if (!m_button) {
        WARNLOG("Paramcheck file is not exist. Paramchecker is disabled!");
        return true;
    }
    DBGLOG("Enter StructChecker, key: %s, parameter: %s", key.c_str(), WIPE_SENSITIVE(args.toStyledString()).c_str());
    auto megCheckers = ParamCheckCfgLoader::Instance().GetMsgCheckers();
    if (megCheckers.count(key) == 0) {
        WARNLOG("GetMsgCheckers for: %s failed!", key.c_str());
        return false;
    }
    if (!megCheckers[key]->Check(args)) {
        ERRLOG("StructChecker check failed for key: %s", key.c_str());
        return false;
    }
    return true;
}
}
