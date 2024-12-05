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
#include "CmdBuilder.h"
#include <locale>
#include <codecvt>
#include <vector>
#include <fstream>
#include <boost/asio/read.hpp>
#include <boost/asio/readable_pipe.hpp>
#include <boost/process/shell.hpp>

#if defined(_AIX) || defined(SOLARIS)
#include <boost/process.hpp>
#else
#include <boost/process/v2/stdio.hpp>
#include <boost/process/v2/process.hpp>
#endif

#ifdef WIN32
#include <boost/process/v2/windows/with_logon_launcher.hpp>
#endif

#include "common/CleanMemPwd.h"
#include "log/Log.h"
#include "security/cmd/CmdWhitelist.h"

namespace Module {

CmdBuilder::CmdBuilder(const std::string& cmdName)
{
    m_cmdName = cmdName;
}

CmdBuilder::CmdBuilder(const std::string& cmdName, const std::vector<std::string>& params)
{
    m_cmdName = cmdName;
    for (auto& param : params) {
        CmdParam cmdParamInfo(COMMON_PARAM, param);
        m_params.push_back(cmdParamInfo);
    }
}

CmdBuilder::CmdBuilder(const std::string& cmdName, const std::vector<CmdParam>& params)
{
    m_cmdName = cmdName;
    m_params = params;
}

// 构建规范化命令
bool CmdBuilder::Init()
{
    // 如果命令在白名单则不用经过框架做校验，直接从命令白名单中获取对应的命令
    if (m_params.empty() && CMD_WHITELIST.count(m_cmdName)) {
        m_cmdStr = CMD_WHITELIST.at(m_cmdName);
        return true;
    }

    // 非硬编码的命令（即依赖外部参数的命令），需要校验命令或路径是否在白名单中
    if (m_policyChecker.CheckCommonCmdName(m_cmdName) || m_policyChecker.CheckCmdPath(m_cmdName)) {
        bool isFirst = true;
        for (auto& param : m_params) {
            std::string outParam;
            if (!m_policyChecker.BuildParam(param, outParam)) {
                return false;
            }

            if (isFirst || !param.WithPreSpace()) {
                m_cmdStr += outParam;
                isFirst = false;
            } else {
                m_cmdStr += " " + outParam;
            }

            if (param.TypeId() == COMMON_PARAM) {
                m_printCmd += " ?"; // 对于普通参数打印时将其匿名化
            } else {
                m_printCmd += " " + outParam;
            }
        }
    } else {
        ERRLOG("Invaild command: %s", WIPE_SENSITIVE(m_cmdName).c_str());
        return false;
    }
   
    return true;
}

CmdBuilder& CmdBuilder::operator<<(std::string const& param)
{
    CmdParam cmdParamInfo(COMMON_PARAM, param);
    m_params.push_back(cmdParamInfo);
    return *this;
}

CmdBuilder& CmdBuilder::operator<<(const char* param)
{
    std::string str(param);
    CmdParam cmdParamInfo(COMMON_PARAM, str);
    m_params.push_back(cmdParamInfo);
    return *this;
}

CmdBuilder& CmdBuilder::operator<<(CmdParam const& cmdParamInfo)
{
    m_params.push_back(cmdParamInfo);
    return *this;
}

void CmdBuilder::Print(std::ostream& os) const
{
    os << "Cmd: " << std::endl;
    os << " { " << std::endl;
    os << WIPE_SENSITIVE(m_printCmd) << std::endl;
    os << " }" << std::endl;
}

std::string CmdBuilder::GetCmdString()
{
    return m_cmdStr;
}

std::string CmdBuilder::GetPrintCmdString()
{
    return m_printCmd;
}

void CmdBuilder::SetPathWhitelist(const std::unordered_set<std::string>& pathWhitelist)
{
    m_policyChecker.SetTlsPathWhitelist(pathWhitelist);
}

void CmdBuilder::SetRunUser(const CmdRunUser& cmdUser)
{
    m_cmdRunUser = cmdUser;
}

#ifdef WIN32
int CmdBuilder::RunCmdWithWinLogon(std::vector<std::string>& result)
{
    int exitCode = -1;
    boost::system::error_code ec;
    boost::asio::io_context ctx;
    boost::asio::readable_pipe rp{ctx};
    std::vector<std::string> args = {"/c", m_cmdStr};

    try {
        // Windows切用户执行命令，命令解释器为cmd.exe，通过boost shell()获取，如C:\Windows\System32\cmd.exe
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        auto winLauncher = boost::process::v2::windows::with_logon_launcher(converter.from_bytes(m_cmdRunUser.username),
            converter.from_bytes(m_cmdRunUser.passwd),
            converter.from_bytes(m_cmdRunUser.domain));
        auto proc = winLauncher(
            ctx, ec, boost::process::shell(), args, boost::process::v2::process_stdio{nullptr, rp});

        // 获取命令的执行输出结果
        std::string retBuf;
        auto sz = boost::asio::read(rp, boost::asio::dynamic_buffer(retBuf), ec);
        while (ec == boost::asio::error::interrupted) {
            sz += boost::asio::read(rp, boost::asio::dynamic_buffer(retBuf), ec);
        }

        std::istringstream is(retBuf);
        std::string line;
        while (std::getline(is, line)) {
            DBGLOG("Command result: %s", WIPE_SENSITIVE(line).c_str());
            result.push_back(line);
        }

        exitCode = proc.wait();
    } catch (std::exception& e) {
        ERRLOG("Failed to run/wait processes: %s", WIPE_SENSITIVE(e.what()).c_str());
    }

    CleanMemoryPwd(m_cmdRunUser.passwd);

    INFOLOG("Command: %s, exit_code: %d", WIPE_SENSITIVE(m_printCmd).c_str(), exitCode);
    return exitCode;
}
#endif

#if defined(_AIX) || defined(SOLARIS)
int CmdBuilder::RunCmdWithProcessV1(
    const boost::filesystem::path& exe, const std::vector<std::string>& args, std::vector<std::string>& result)
{
    boost::process::ipstream is;
    boost::process::child c(exe, args, boost::process::std_out > is);

    std::string line;
    while (std::getline(is, line)) {
        result.push_back(line);
    }

    c.wait();
    return c.exit_code();
}
#else
int CmdBuilder::RunCmdWithProcessV2(
    const boost::filesystem::path& exe, const std::vector<std::string>& args, std::vector<std::string>& result)
{
    boost::system::error_code ec;
    boost::asio::io_context ctx;
    boost::asio::readable_pipe rp{ctx};

    boost::process::v2::process proc{ctx, exe, args, boost::process::v2::process_stdio{nullptr, rp}};

    std::string retBuf;
    auto sz = boost::asio::read(rp, boost::asio::dynamic_buffer(retBuf), ec);
    while (ec == boost::asio::error::interrupted) {
        sz += boost::asio::read(rp, boost::asio::dynamic_buffer(retBuf), ec);
    }

    std::istringstream is(retBuf);
    std::string line;
    while (std::getline(is, line)) {
        DBGLOG("Command result: %s", WIPE_SENSITIVE(line).c_str());
        result.push_back(line);
    }

    return proc.wait();
}
#endif

int CmdBuilder::Run(std::vector<std::string>& result)
{
    int exitCode = -1;
    boost::filesystem::path exe;
    std::vector<std::string> args;
    DBGLOG("Begin to run command: %s", WIPE_SENSITIVE(m_printCmd).c_str());

    try {
#ifdef WIN32
        m_cmdStr = R"(")" + m_cmdStr + R"(")";
        if (m_cmdRunUser.username.empty()) {
            exe = boost::process::shell();  // 如C:\Windows\System32\cmd.exe，通过boost shell()获取
            args = {"/c", m_cmdStr};
        } else { // Windows下切用户执行命令
            return RunCmdWithWinLogon(result);
        }
#else
        if (m_cmdRunUser.username.empty()) {
            exe = boost::process::shell(); // 如/bin/sh，通过boost shell()获取
            args = {"-c", m_cmdStr};
        } else { // Linux下切用户执行命令
            exe = "/bin/su";
            args = {"-", m_cmdRunUser.username, "-c", m_cmdStr};
        }
#endif

#if defined(_AIX) || defined(SOLARIS)  // AIX和Solaris使用Boost Process V1，V2暂不支持AIX/Solaris
        exitCode = RunCmdWithProcessV1(exe, args, result);
#else        // 其它平台使用Boost Process V2
        exitCode = RunCmdWithProcessV2(exe, args, result);
#endif
    } catch (std::exception &e) {
        ERRLOG("Failed to run/wait processes: %s", WIPE_SENSITIVE(e.what()).c_str());
    }

    CleanMemoryPwd(m_cmdRunUser.passwd);

    INFOLOG("Command: %s, exit_code: %d", WIPE_SENSITIVE(m_printCmd).c_str(), exitCode);
    return exitCode;
}

}
