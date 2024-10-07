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
#ifndef MODULE_SECURITY_CMD_CMDPARAM_H
#define MODULE_SECURITY_CMD_CMDPARAM_H

#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include "define/Defines.h"

namespace Module {

enum CmdParamType {
    COMMON_PARAM,        // 普通string参数
    CONTINUOUS_PARAM,    // ; 连续指令服务
    PIPELINE_PARAM,      // | 连续指令服务 管道：连接上个指令的标准输出，作为下个指令的标准输入
    BACKGROUND_PARAM,    // & 后台运行
    LOGICAND_PARAM,      // && 逻辑与，前一个命令成功执行后才会执行下一个命令
    LOGICOR_PARAM,       // || 逻辑或，前一个命令执行失败，则会执行下一个命令
    VAR_SUBS_PARAM,      // $ 变量替换
    APPEND_PARAM,        // >> 将输出以追加的方式重定向到 file。
    SILENCE_PARAM,       // 2>&1 >/dev/null //避免shell命令或者程序等运行中有内容输出
    REDIRECTOUT_PARAM,   // > 重定向输入
    REDIRECTIN_PARAM,    // < 目标文件内容发送到命令中
    BACKTICKS_PARAM,     // ` 返回当前执行结果
    BACKSLASH_PARAM,     // \ 作为连接符号用，或者转义用
    NEGATE_PARAM,        // ! 执行上一条shell命令
    HARD_QUOTE_PARAM,    // ' 单引号
    WILDCARD_CHAR_PARAM, // * 通配符
    OUTPUT_PARAM,        // 2>&1 错误输出2重定向到标准输出1
    COMMON_CMD_NAME,     // 普通命令名，例如：ps
    BIN_CMD_NAME,        // 带完整路径的内部二进制命令名，例如：/usr/bin/find  xxx
    SCRIPT_CMD_NAME,     // 带完整路径的脚本命令名，例如：/opt/up.sh
    CMD_OPTION_PARAM,    // 命令参数，对应 ls -lth 中的 -lth ，df -h   中的 -h
    ENDOFFILE_PARAM,     // <<EOF 文件结束符
    NEWLINE_PARAM,       // \n 新行
    CURDIR_PARAM,        // . 当前目录
    PATH_PARAM,          // 路径参数
};

class AGENT_API CmdParam {
public:
    /**
     *  @brief 构造函数
     *
     *  @param param         [IN]命令组成部分
     *  @param withPreSpace  [IN]默认为true，用于指示在命令拼接成字符串时是否需要加前置空格
                            命令拼接成字符串时，默认会在两个CmdParam间加空格，但有些场景不需要这个空格，
                            如awk -F' ' '{print $8}'，$会拆成特殊CmdParam，8}会拆成普通参数，但两者之间不需要空格
     *  @return 注册成功返回true，注册失败返回false
    */
    CmdParam(const char* param, bool withPreSpace = true);
    CmdParam(const std::string& param, bool withPreSpace = true);
    CmdParam(CmdParamType typeId, std::string const& param, bool withPreSpace = true);
    virtual ~CmdParam() noexcept;

    // 返回命令参数的名称
    virtual std::string const& Name(void) const;

    // 返回命令参数的类型
    virtual CmdParamType TypeId(void) const;

    // 返回命令参数的值
    virtual std::string const& Value(void) const;

    // 返回前置空格标记
    bool WithPreSpace() const;

    // 打印命令参数信息
    virtual void Print(std::ostream& os) const;
protected:
    void Init(CmdParamType typeId, std::string const& param, bool withPreSpace = true);
    std::string GetParam(CmdParamType typeId);
protected:
    std::string m_name;
    CmdParamType m_typeId;
    std::string m_defaultValue;
    bool m_withPreSpace = true;
};

}

#endif
