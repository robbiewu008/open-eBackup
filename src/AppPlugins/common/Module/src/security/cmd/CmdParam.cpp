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
#include "security/cmd/CmdParam.h"
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include "log/Log.h"

namespace Module {

const static std::unordered_map<CmdParamType, std::string> specialParams = {{CONTINUOUS_PARAM, ";"},
    {PIPELINE_PARAM, "|"},
    {BACKGROUND_PARAM, "&"},
    {LOGICAND_PARAM, "&&"},
    {LOGICOR_PARAM, "||"},
    {VAR_SUBS_PARAM, "$"},
    {APPEND_PARAM, ">>"},
    {SILENCE_PARAM, ">/dev/null 2>&1"},
    {REDIRECTOUT_PARAM, ">"},
    {REDIRECTIN_PARAM, "<"},
    {BACKTICKS_PARAM, "`"},
    {BACKSLASH_PARAM, "\\"},
    {NEGATE_PARAM, "!"},
    {HARD_QUOTE_PARAM, "\'"},
    {OUTPUT_PARAM, "2>&1"},
    {ENDOFFILE_PARAM, "<<EOF"},
    {NEWLINE_PARAM, "\n"},
    {CURDIR_PARAM, "."},
    {WILDCARD_CHAR_PARAM, "*"}};

const static std::unordered_set<CmdParamType> commonParams = {
    COMMON_PARAM, COMMON_CMD_NAME, BIN_CMD_NAME, SCRIPT_CMD_NAME, CMD_OPTION_PARAM, PATH_PARAM};

CmdParam::CmdParam(const char* param, bool withPreSpace)
{
    std::string outParam = param;
    Init(COMMON_PARAM, outParam, withPreSpace);
}

CmdParam::CmdParam(const std::string& param, bool withPreSpace)
{
    std::string outParam = param;
    Init(COMMON_PARAM, outParam, withPreSpace);
}

CmdParam::CmdParam(CmdParamType typeId, std::string const& param, bool withPreSpace)
{
    Init(typeId, param, withPreSpace);
}

CmdParam::~CmdParam() noexcept
{}

std::string const& CmdParam::Name() const
{
    return m_name;
}

CmdParamType CmdParam::TypeId() const
{
    return m_typeId;
}

std::string const& CmdParam::Value(void) const
{
    return m_defaultValue;
}

bool CmdParam::WithPreSpace() const
{
    return m_withPreSpace;
}

void CmdParam::Print(std::ostream& os) const
{
    os << "cmdParam:{ " << "typeId: " << m_typeId << ", " << "value: " << m_defaultValue  << " }" << std::endl;
}

std::string CmdParam::GetParam(CmdParamType typeId)
{
    auto iter = specialParams.find(typeId);
    if (iter != specialParams.end()) {
        return iter->second;
    }
    return "";
}

void CmdParam::Init(CmdParamType typeId, std::string const& param, bool withPreSpace)
{
    m_withPreSpace = withPreSpace;
    m_typeId = typeId;
    auto iter = commonParams.find(typeId);
    if (iter != commonParams.end()) {
        m_name = "";
        m_defaultValue = param;
    } else {
        m_name = GetParam(typeId);
        m_defaultValue = GetParam(typeId);
    }
}

}
