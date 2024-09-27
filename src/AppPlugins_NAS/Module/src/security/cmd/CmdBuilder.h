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
#ifndef MODULE_SECURITY_CMD_CMDBUILDER_H
#define MODULE_SECURITY_CMD_CMDBUILDER_H

#include <unordered_set>
#include <vector>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include "define/Defines.h"
#include "security/cmd/CmdParam.h"
#include "security/cmd/CmdPolicy.h"

namespace Module {

struct CmdRunUser {
    std::string username;
    std::string passwd;
    std::string domain;
};

class AGENT_API CmdBuilder {
public:
    /**
     *  @brief 构造函数，仅包含命令名
     *
     *  @param cmdName  [IN]命令名，后续的命令选项、参数以及其它复合命令均可通过流操作符的方式传递
     */
    explicit CmdBuilder(const std::string &cmdName);

    /**
     *  @brief 构造普通型命令，将命令名称、选项、参数都当做普通参数，如'ls -lth /opt/tram/'，不支持特殊字符
               注意：命令不支持特殊字符，普通参数中如果包含空格、特殊字符将进行转义
     *
     *  @param cmdName [IN]主命令名，即命令串中的首个命令字，如ls
     *  @param cmds [IN]命令参数列表，包括主命令名、命令选项、命令参数，普通参数中如果包含空格、特殊字符将进行转义
     */
    CmdBuilder(const std::string& cmdName, const std::vector<std::string>& cmds);

    /**
     *  @brief 【推荐】构造增强型命令，将命令名称、选项、参数区分类型，命令可以是多命令、复杂命令，支持特殊要求的参数
               如'ls -lth /opt/tram/ | grep -2'，
               注意：命令支持特殊要求的参数，需要指定特定参数类型，普通参数中如果包含空格、特殊字符将进行转义
     *
     *  @param cmdName [IN]主命令名，即命令串中的首个命令字，如ls
     *  @param cmds [IN]命令参数列表，包括主命令名、命令选项、命令参数，普通参数中如果包含空格、特殊字符将进行转义
     */
    CmdBuilder(const std::string& cmdName, const std::vector<CmdParam>& cmds);

    // 使用流操作符构建命令
    CmdBuilder& operator<<(std::string const& param);

    // 使用流操作符构建命令
    CmdBuilder& operator<<(const char* param);

    // 【推荐】使用流操作符构建命令
    CmdBuilder& operator<<(CmdParam const& cmdParamInfo);

    // 初始化，根据构造函数传入的参数完成命令的校验、拼接。
    bool Init();

    // 设置限本命令生效的路径白名单
    void SetPathWhitelist(const std::unordered_set<std::string>& pathWhitelist);

    // 设置命令执行的用户信息
    void SetRunUser(const CmdRunUser& cmdUser);

    // 打印构建的命令
    void Print(std::ostream& os) const;

    // 获取构建的命令
    std::string GetCmdString();

    // 获取用于打印的命令
    std::string GetPrintCmdString();

#ifdef WIN32
    // Windows下切用户执行命令，仅限Windows使用
    int RunCmdWithWinLogon(std::vector<std::string>& result);
#endif

#if defined(_AIX) || defined (SOLARIS)
    int RunCmdWithProcessV1(
        const boost::filesystem::path& exe, const std::vector<std::string>& args, std::vector<std::string>& result);
#else
    int RunCmdWithProcessV2(
        const boost::filesystem::path& exe, const std::vector<std::string>& args, std::vector<std::string>& result);
#endif

    // 执行命令
    int Run(std::vector<std::string>& result);

    virtual ~CmdBuilder() = default;

private:
    std::string m_cmdName;
    std::string m_cmdStr;
    std::string m_printCmd;  // 打印日志时使用该变量，不要直接使用m_cmdStr
    std::vector<CmdParam> m_params;
    CmdRunUser m_cmdRunUser;
    CmdPolicy m_policyChecker;
};

}

#endif
