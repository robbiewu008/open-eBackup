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
#ifndef MODULE_SECURITY_CMD_CMDPOLICY_H
#define MODULE_SECURITY_CMD_CMDPOLICY_H

#include <vector>
#include <unordered_set>
#include <string>
#include <iostream>
#include <thread>
#include "security/cmd/CmdParam.h"

namespace Module {

class CmdPolicy {
public:
    CmdPolicy() = default;
    virtual ~CmdPolicy() = default;

    /**
     *  @brief 使用者向框架注册命令白名单和路径白名单
     *
     *  @param cmdWhitelist  [IN]系统命令白名单，如'ps -ef 2>&1 >/dev/null'中的ps
     *  @param pathWhitelist [IN]二进制或脚本路径白名单，如'/opt/script/test.sh'中的/opt/script
     *  @return 成功返回true，失败返回false
     */
    static bool RegisterWhitelist(
        const std::unordered_set<std::string>& cmdWhitelist, const std::unordered_set<std::string>& pathWhitelist);

    /**
     *  @brief 检查通用的系统命令是否符合要求：在命令白名单中，且满足字母、数字、下划线、中划线的要求
     *
     *  @param value  [IN,OUT]命令名
     *  @return 成功返回true，失败返回false
     */
    bool CheckCommonCmdName(std::string& value);

    /**
     *  @brief 检查带完整路径的二进制命令名 、脚本命令名以及路径参数是否符合要求：
                路径要能标准化且所在路径（即父目录）必须位于路径白名单中
     *
     *  @param cmdPath  [IN,OUT]带路径的二进制程序、脚本或参数
     *  @return 成功返回true，失败返回false
     */
    bool CheckCmdPath(std::string& cmdPath);

    /**
     *  @brief 校验命令选项，格式为字母、数字、下划线或中划线
     *
     *  @param value  [IN,OUT]命令选项
     *  @return 成功返回true，失败返回false
     */
    bool CheckCmdOptionParam(std::string& value);

    /**
     *  @brief 校验命令参数并进行转义
     *
     *  @param value  [IN,OUT]命令参数
     *  @return 成功返回true，失败返回false
     */
    bool CheckCommonParam(std::string& value);

    /**
     *  @brief 构建参数
     *
     *  @param cmdParamInfo  [IN]命令参数
     *  @param outParam      [OUT]转换后的命令字符串
     *  @return 成功返回true，失败返回false
     */
    bool BuildParam(const CmdParam& cmdParamInfo, std::string& outParam);

    /**
     *  @brief 设置线程上下文有效的路径白名单，仅调用线程有效
     *
     *  @param tlsPathWhitelist  [IN]路径白名单
     *  @return 成功返回true，失败返回false
     */
    bool SetTlsPathWhitelist(const std::unordered_set<std::string>& tlsPathWhitelist);

protected:
    static std::unordered_set<std::string> m_extCmdWhitelist;
    static std::unordered_set<std::string> m_extPathWhitelist;
    std::unordered_set<std::string> m_tlsPathWhitelist;   // 仅调用线程上下文生效的路径白名单
};

}

#endif
