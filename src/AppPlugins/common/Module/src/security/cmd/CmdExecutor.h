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
#ifndef MODULE_SECURITY_CMD_CMDEXECUTOR_H
#define MODULE_SECURITY_CMD_CMDEXECUTOR_H

#include <string>
#include <unordered_set>
#include <vector>
#include "define/Defines.h"
#include "security/cmd/CmdParam.h"
#include "security/cmd/CmdBuilder.h"

namespace Module {

/**
 *  @brief 向框架注册命令和路径白名单
 *
 *  @param cmdWhitelist   [IN]系统命令白名单，只有在该白名单中的系统命令才允许执行
                              注意：命令需满足[0-9a-zA-Z_-]正则要求，否则将注册失败
 *  @param pathWhitelist  [IN]二进制命令或脚本路径白名单，只有在该白名单路径下的二进制或脚本才允许执行
                              注意：路径需满足绝对化要求，否则将注册失败
 *  @return 注册成功返回true，注册失败返回false
 */
bool AGENT_API RegisterWhitelist(
    const std::unordered_set<std::string>& cmdWhitelist, const std::unordered_set<std::string>& pathWhitelist);

/**
 *  @brief 执行命令，命令不支持特殊字符，将作为普通字符串进行转义
 *
 *  @param cmdName   [IN]主命令名
 *  @param cmdVec    [IN]完整命令，不支持特殊字符，将转义
 *  @param result    [OUT]命令输出结果，按行输出
 *  @param cmdUser   [IN]命令执行用户，默认不指定
 *  @return 命令执行返回码
 */
int AGENT_API RunCommand(const std::string& cmdName, const std::vector<std::string>& cmdVec,
    std::vector<std::string>& result, const CmdRunUser& cmdUser = {});

/**
 *  @brief 执行命令，命令支持特殊字符，需明确指定特殊参数的类型
 *
 *  @param cmdName   [IN]主命令名
 *  @param cmdVec    [IN]完整命令，支持特殊字符，需明确指定特殊参数的类型
 *  @param result    [OUT]命令输出结果，按行输出
 *  @param cmdUser   [IN]命令执行用户，默认不指定
 *  @return 命令执行返回码
 */
int AGENT_API RunCommand(const std::string& cmdName, const std::vector<CmdParam>& cmdVec,
    std::vector<std::string>& result, const CmdRunUser& cmdUser = {});

/**
 *  @brief 执行命令，需要指定命令路径参数白名单
 *         当命令包含路径参数，且路径是动态生成时可指定路径白名单，该白名单仅本次命令生效
 *
 *  @param cmdName         [IN]主命令名
 *  @param cmdVec          [IN]完整命令，支持特殊字符，需明确指定特殊参数的类型
 *  @param result          [OUT]命令输出结果，按行输出
 *  @param pathWhitelist   [IN]路径白名单，仅本次命令生效
 *  @param cmdUser         [IN]命令执行用户，默认不指定
 *  @return 命令执行返回码
 */
int AGENT_API RunCommand(const std::string& cmdName, const std::vector<CmdParam>& cmdVec,
    std::vector<std::string>& result, const std::unordered_set<std::string>& pathWhitelist,
    const CmdRunUser& cmdUser = {});

/**
 *  @brief 执行命令，需调用者构造好CmdBuilder对象
 *
 *  @param builder   [IN]拼接好的builder，在builder内已经完成校验初始化
 *  @param result    [OUT]命令输出结果，按行输出
 *  @return 命令执行返回码
 */
int AGENT_API RunCommand(CmdBuilder& builder, std::vector<std::string>& result);

}

#endif
