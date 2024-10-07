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
#ifndef MODULE_SECURITY_CMD_CMDWHITELIST_H
#define MODULE_SECURITY_CMD_CMDWHITELIST_H

#include <string>
#include <unordered_map>

namespace Module {

#define CMD_NAME(x) (#x)

/**
 *【使用说明】：硬编码的命令为代码中写死的不依赖外部参数的命令，在执行时不需要框架对其进行校验。
 * 由于安全原因，使用配置文件或者函数接口的方式传递白名单命令都存在风险，因此该类硬编码命令需在编译时就完全确定。
 * 本文件用于记录各业务使用模块需要用到的硬编码命令，按模块进行分类。命令常量名规则：CMD_{模块}_{命令功能}，全大写
 *【使用步骤】：
 * 1、定义命令常量，如CMD_AGENT_GET_MEMORY
 * 2、建立命令名和命令之间的映射关系，即CMD_WHITELIST，key为步骤1定义的常量名对应的字符串，value为实际的命令
 */

/*=============================================================================
                       Agent框架使用到的硬编码命令在此定义
=============================================================================*/
const std::string CMD_AGENT_GET_MEMORY =
    R"(cat /proc/meminfo | grep -e 'MemTotal' -e 'MemFree' -e 'SwapTotal' -e 'SwapFree')";

/*=============================================================================
                       文件集备份插件使用到的硬编码命令在此定义
=============================================================================*/


/* ！！！const对象，不允许运行时修改该map，不提供对外修改接口 */
static const std::unordered_map<std::string, std::string> CMD_WHITELIST{
    {CMD_NAME(CMD_AGENT_GET_MEMORY), CMD_AGENT_GET_MEMORY},
};

}

#endif
