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
#include "security/cmd/CmdPolicy.h"
#include <set>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <regex>
#include <boost/filesystem.hpp>
#include "log/Log.h"

namespace Module {

using CmdChecker = bool (CmdPolicy::*)(std::string&);

/**
    通过对命令参数进行转义，防止命令注入
    参考PHP语言提供的escapeshellarg函数：https://www.php.net/manual/zh/function.escapeshellarg.php
    https://github.com/php/php-src/blame/PHP-7.3.10/ext/standard/exec.c#L392
    in Windows mode,
    1) Prepend a " character.
    2) Replace all ", %, and ! characters with  .
    3) If the end consists of an odd number of \ characters, add one \ character to the end. (Bug #69646)
    4) Append a " character.
    
    in other platforms mode,
    1) Prepend a ' character.
    2) Replace all ' characters with '\''
    3) Append a ' character.
*/
std::string EscapeShellArg(const std::string& arg)
{
    std::string escapedArg;

#ifdef _WIN32
    // Windows-specific escaping
    // 参数字符串用双引号进行包裹
    escapedArg += R"(")";

    // 对"、%、!转换成空格
    for (char c : arg) {
        if (c == '"' || c == '%' || c == '!') {
            escapedArg += " ";
        } else {
            escapedArg += c;
        }
    }

    // 如果参数是以奇数个\字符结尾，则需再加一个\作为转义
    if (escapedArg.back() == '\\') {
        const int evenBase = 2;
        int k = 0;
        int n = escapedArg.length() - 1;
        for (; n >= 0 && escapedArg[n]; n--, k++);
        if (k % evenBase) {
            escapedArg += R"(\)";
        }
    }

    escapedArg += R"(")";
#else
    // Other platforms (Linux, macOS, etc.) escaping
    // 参数字符串用单引号进行包裹
    escapedArg += R"(')";

    // 单引号替换成'\''
    for (char c : arg) {
        if (c == '\'') {
            escapedArg += R"('\'')";
        } else {
            escapedArg += c;
        }
    }

    escapedArg += R"(')";
#endif

    return escapedArg;
}

std::unordered_set<std::string> CmdPolicy::m_extCmdWhitelist;
std::unordered_set<std::string> CmdPolicy::m_extPathWhitelist;

bool CmdPolicy::RegisterWhitelist(
    const std::unordered_set<std::string>& cmdWhitelist, const std::unordered_set<std::string>& pathWhitelist)
{
    bool ret = true;
    m_extCmdWhitelist.clear();
    m_extPathWhitelist.clear();

    /* 检查注册的命令白名单中的命令是否满足字母、数字、下划线、中划线的基本要求 */
    for (auto& c : cmdWhitelist) {
        if (std::regex_match(c, std::regex("[0-9a-zA-Z_-]*"))) {
            DBGLOG("Ext whitelist command:%s", c.c_str());
            m_extCmdWhitelist.insert(c);
        } else {
            ERRLOG("Invalid command format in path whitelist:{%s}", c.c_str());
            ret = false;
        }
    }

    /* 检查注册的路径白名单中的路径是否满足标准化的要求 */
    for (auto& p : pathWhitelist) {
        boost::system::error_code ec;
        boost::filesystem::path dir(p);
        dir += boost::filesystem::path::preferred_separator;  // 在路径后统一追加路径分隔符，防止后续比较时路径格式不统一
        dir = boost::filesystem::weakly_canonical(dir, ec);
        if (dir.empty() || (ec.value() != boost::system::errc::success)) {
            ERRLOG("Invalid path foramt:{%s}, canonical failed: %s",
                p.c_str(),
                WIPE_SENSITIVE(ec.message()).c_str());
            ret = false;
        } else {
            DBGLOG("Ext whitelist path:%s", dir.string().c_str());
            m_extPathWhitelist.insert(dir.string());
        }
    }

    return ret;
}

bool CmdPolicy::SetTlsPathWhitelist(const std::unordered_set<std::string>& tlsPathWhitelist)
{
    bool ret = true;
    m_tlsPathWhitelist.clear();

    /* 检查注册的路径白名单中的路径是否满足标准化的要求 */
    for (auto& p : tlsPathWhitelist) {
        boost::system::error_code ec;
        boost::filesystem::path dir(p);
        dir += boost::filesystem::path::preferred_separator;  // 在路径后统一追加路径分隔符，防止后续比较时路径格式不统一
        dir = boost::filesystem::weakly_canonical(dir, ec);
        if (dir.empty() || (ec.value() != boost::system::errc::success)) {
            ERRLOG("Invalid path foramt:{%s}, canonical failed: %s",
                p.c_str(),
                WIPE_SENSITIVE(ec.message()).c_str());
            ret = false;
        } else {
            DBGLOG("Tls whitelist path:%s", dir.string().c_str());
            m_tlsPathWhitelist.insert(dir.string());
        }
    }

    return ret;
}

// 检查通用的系统命令是否符合要求：在命令白名单中，且满足字母、数字、下划线、中划线的要求
bool CmdPolicy::CheckCommonCmdName(std::string& value)
{
    // 检查系统命令是否在白名单中
    if (!m_extCmdWhitelist.count(value)) {
        WARNLOG("Command(%s) is not in cmdWhitelist", WIPE_SENSITIVE(value).c_str());
        return false;
    }

    return true;
}

// 检查带完整路径的二进制命令名 、脚本命令名以及路径参数是否符合要求
bool CmdPolicy::CheckCmdPath(std::string& value)
{
    // 检查路径能否标准化，canonical要求路径必须存在，框架可能无权限访问，所以使用weakly_canonical
    // 通过weakly_canonical来校验路径格式的合法性，如Windows、Linux、UNC等路径
    boost::system::error_code ec;
    boost::filesystem::path pathParam = boost::filesystem::weakly_canonical(value, ec);
    if (pathParam.empty() || (ec.value() != boost::system::errc::success)) {
        ERRLOG("Canonical path(%s) failed, error: %s",
            WIPE_SENSITIVE(value).c_str(),
            WIPE_SENSITIVE(ec.message()).c_str());
        return false;
    }

    auto IsWhitelistPath = [this](boost::filesystem::path dir) {
        dir += boost::filesystem::path::preferred_separator;  // 以路径分隔符结束
        dir = boost::filesystem::weakly_canonical(dir);
        DBGLOG("Check path:%s", dir.string().c_str());
        if (m_extPathWhitelist.count(dir.string()) || m_tlsPathWhitelist.count(dir.string())) {
            return true;
        } else {
            return false;
        }
    };
    // 校验全路径或父路径是否在白名单中
    if (IsWhitelistPath(pathParam) || IsWhitelistPath(pathParam.parent_path())) {
        value = EscapeShellArg(pathParam.string());
        return true;
    } else {
        ERRLOG("Path(%s) is not in whitelist", pathParam.string().c_str());
        return false;
    }

    return true;
}

// 校验命令选项
bool CmdPolicy::CheckCmdOptionParam(std::string& value)
{
    // 命令选项格式为字母、数字、下划线、中划线、斜杠(Windows用到，如ipconfig /all)、冒号
    if (!std::regex_match(value, std::regex(R"([0-9a-zA-Z:/_-]*)"))) {
        ERRLOG("Illegal character in command option(%s)", WIPE_SENSITIVE(value).c_str());
        return false;
    }

    return true;
}

// 对普通命令参数进行转义
bool CmdPolicy::CheckCommonParam(std::string& value)
{
    value = EscapeShellArg(value);

    return true;
}

bool CmdPolicy::BuildParam(const CmdParam& cmdParamInfo, std::string& outParam)
{
    CmdParamType typeId = cmdParamInfo.TypeId();
    std::string value = cmdParamInfo.Value();

    static std::unordered_map<CmdParamType, CmdChecker> cmdCheckers = {
        {COMMON_CMD_NAME, &CmdPolicy::CheckCommonCmdName},
        {BIN_CMD_NAME, &CmdPolicy::CheckCmdPath},
        {SCRIPT_CMD_NAME, &CmdPolicy::CheckCmdPath},
        {CMD_OPTION_PARAM, &CmdPolicy::CheckCmdOptionParam},
        {COMMON_PARAM, &CmdPolicy::CheckCommonParam},
        {PATH_PARAM, &CmdPolicy::CheckCmdPath}
    };

    CmdChecker h = cmdCheckers[typeId];
    if (!h || (this->*h)(value)) {
        outParam = value;
        return true;
    }

    return false;
}

}
