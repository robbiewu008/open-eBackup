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
#ifndef PARAM_CHECK_CFG_LOADER_H
#define PARAM_CHECK_CFG_LOADER_H

#include <unordered_map>
#include <tinyxml2.h>
#include "ParamChecker.h"

namespace Module {
using RestMsgCheckers =
    std::unordered_map<std::string, std::shared_ptr<MemberCheck>>;  // Rest接口检查器，key为method#url
class ParamCheckCfgLoader {
public:
    static ParamCheckCfgLoader& Instance()
    {
        static ParamCheckCfgLoader loader;
        return loader;
    }

    int Load(const std::string& xmlFile);

    const RestMsgCheckers& GetMsgCheckers()
    {
        return m_messages;
    }

protected:
    void InitInternalChecker();
    bool ParseParamCheck(tinyxml2::XMLHandle& hd);
    bool LoadParamTypeDefine(tinyxml2::XMLElement* ele);
    bool LoadMessageDefine(tinyxml2::XMLElement* ele);

private:
    std::string GetChildText(tinyxml2::XMLElement* ele, const std::string& name);
    std::shared_ptr<RuleChecker> LoadParamType(tinyxml2::XMLElement* ele);
    std::shared_ptr<RuleChecker> LoadIntType(tinyxml2::XMLElement* ele, const std::string& cond);
    std::shared_ptr<RuleChecker> LoadLengthType(tinyxml2::XMLElement* ele, const std::string& cond);
    std::shared_ptr<RuleChecker> LoadStringType(tinyxml2::XMLElement* ele, const std::string& cond);
    std::shared_ptr<RuleChecker> LoadInternalType(tinyxml2::XMLElement* ele, const std::string& cond);

    std::shared_ptr<MemberCheck> LoadMessage(tinyxml2::XMLElement* ele);
    std::shared_ptr<ParamChecker> LoadParam(tinyxml2::XMLElement* ele);

private:
    tinyxml2::XMLDocument m_doc;
    std::unordered_map<std::string, std::shared_ptr<RuleChecker>>
        m_internalCheckers;  // 自定义检查器名称和检查器之间的映射关系
    std::unordered_map<std::string, std::shared_ptr<RuleChecker>> m_paramTypes;  // param_type和检查器为1:1的关系，key为param_type
    std::unordered_map<std::string, std::shared_ptr<MemberCheck>> m_messages;  // message和检查器为1:1的关系，key为message
};
}
#endif
