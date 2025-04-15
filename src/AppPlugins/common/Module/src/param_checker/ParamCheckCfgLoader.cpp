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
#include "ParamCheckCfgLoader.h"
#include <boost/algorithm/string.hpp>
#include "common/File.h"
#include "log/Log.h"
#include "param_checker/BuildQueryCondition.h"

namespace Module {
using namespace std;
using namespace tinyxml2;

using ParamTypeLoader = std::shared_ptr<RuleChecker>(ParamCheckCfgLoader::*)(XMLElement*, const std::string&);
int ParamCheckCfgLoader::Load(const std::string& xmlFile)
{
    InitInternalChecker();

    if (!Module::CFile::FileExist(xmlFile.c_str())) {
        ERRLOG("%s not exist.", xmlFile.c_str());
        return Module::FAILED;
    }

    if (m_doc.LoadFile(xmlFile.c_str()) != XML_SUCCESS) {
        ERRLOG("Failed to load %s, err: %s", xmlFile.c_str(), m_doc.ErrorStr());
        return Module::FAILED;
    }

    XMLHandle hd(&m_doc);
    if (!ParseParamCheck(hd)) {
        ERRLOG("Failed to load %s, err: %s", xmlFile.c_str(), m_doc.ErrorStr());
        m_doc.Clear();
        return Module::FAILED;
    }

    m_doc.Clear();
    INFOLOG("Success to load %s", xmlFile.c_str());
    return Module::SUCCESS;
}

bool ParamCheckCfgLoader::ParseParamCheck(XMLHandle& hd)
{
    bool ret = true;
    XMLHandle topEhd = hd.FirstChildElement("rest_define");
    XMLElement* elm = topEhd.FirstChildElement("param_type_define").FirstChildElement("param_type").ToElement();
    while (elm && ret) {
        ret = LoadParamTypeDefine(elm);
        elm = elm->NextSiblingElement("param_type");
    }

    elm = topEhd.FirstChildElement("struct_define").ToElement();
    while (elm && ret) {
        ret = LoadMessageDefine(elm);
        elm = elm->NextSiblingElement("struct_define");
    }
    return ret;
}

bool ParamCheckCfgLoader::LoadParamTypeDefine(XMLElement* ele)
{
    const char* id = ele->Attribute("id");
    if (id == nullptr) {
        ERRLOG("Attribuite id missing. err: %s", m_doc.ErrorStr());
        return false;
    }

    auto ck = LoadParamType(ele);
    if (!ck) {
        return false;
    }

    auto it = m_paramTypes.find(id);
    if (it == m_paramTypes.end()) {
        m_paramTypes[id] = ck;
    } else {
        ERRLOG("Duplicate param type %s", id);
        return false;
    }

    return true;
}

std::string ParamCheckCfgLoader::GetChildText(XMLElement* ele, const std::string& name)
{
    if ((ele == nullptr) || name.empty()) {
        return "";
    }

    XMLElement* ce = ele->LastChildElement(name.c_str());
    if (ce) {
        return ce->GetText();
    }

    return "";
}

std::shared_ptr<RuleChecker> ParamCheckCfgLoader::LoadParamType(XMLElement* ele)
{
    std::string type = GetChildText(ele, "type");
    std::string cond = GetChildText(ele, "cond");
    if (type.empty() || cond.empty()) {
        ERRLOG("Attribuite type or cond missing. err: %s", m_doc.ErrorStr());
        return nullptr;
    }

    static std::unordered_map<std::string, ParamTypeLoader> loaderMap = {
        {"int", &ParamCheckCfgLoader::LoadIntType}, {"length", &ParamCheckCfgLoader::LoadLengthType},
        {"string", &ParamCheckCfgLoader::LoadStringType}, {"internal", &ParamCheckCfgLoader::LoadInternalType},
        // {"sql",      &ParamCheckCfgLoader::LoadSqlType}
    };

    auto h = loaderMap[type];
    if (!h) {
        ERRLOG("Not support param type: %s", type.c_str());
        return nullptr;
    }
    DBGLOG("Load param type: %s", type.c_str());

    return (this->*h)(ele, cond);
}

std::shared_ptr<RuleChecker> ParamCheckCfgLoader::LoadIntType(XMLElement* ele, const std::string& cond)
{
    std::string tmp(cond);
    std::string from;
    std::string to;
    if (!GetRange(tmp, from, to)) {
        ERRLOG("Invalid int param cond: %s", WIPE_SENSITIVE(cond).c_str());
        return nullptr;
    }

    long long ifrom = LLONG_MIN;
    long long ito = LLONG_MAX;
    std::istringstream isfrom(from);
    std::istringstream isto(to);
    if (!from.empty()) {
        isfrom >> ifrom;
    }
    if (!to.empty()) {
        isto >> ito;
    }

    return std::make_shared<IntRuleChecker>(ifrom, ito);
}

std::shared_ptr<RuleChecker> ParamCheckCfgLoader::LoadLengthType(XMLElement* ele, const std::string& cond)
{
    std::string tmp(cond);
    std::string from;
    std::string to;
    if (!GetRange(tmp, from, to)) {
        ERRLOG("Invalid int param cond: %s", WIPE_SENSITIVE(cond).c_str());
        return nullptr;
    }

    long long ifrom = LLONG_MIN;
    long long ito = LLONG_MAX;
    std::istringstream isfrom(from);
    std::istringstream isto(to);
    if (!from.empty()) {
        isfrom >> ifrom;
    }
    if (!to.empty()) {
        isto >> ito;
    }

    return std::make_shared<LengthRuleChecker>(ifrom, ito);
}

std::shared_ptr<RuleChecker> ParamCheckCfgLoader::LoadStringType(XMLElement* ele, const std::string& cond)
{
    return std::make_shared<StrRuleChecker>(cond);
}

std::shared_ptr<RuleChecker> ParamCheckCfgLoader::LoadInternalType(XMLElement* ele, const std::string& cond)
{
    auto it = m_internalCheckers.find(cond);
    if (it != m_internalCheckers.end()) {
        return it->second;
    }

    ERRLOG("Unknown internal paramtype: %s", WIPE_SENSITIVE(cond).c_str());
    return nullptr;
}

bool ParamCheckCfgLoader::LoadMessageDefine(XMLElement* ele)
{
    bool ret = true;
    XMLElement* subE = ele->FirstChildElement("struct");
    while (subE && ret) {
        const char* id = subE->Attribute("id");
        if (id == nullptr) {
            ERRLOG("Attribuite id missing. err: %s", m_doc.ErrorStr());
            return false;
        }

        auto msgChecker = LoadMessage(subE);
        if (!msgChecker) {
            return false;
        }

        if (m_messages.find(id) == m_messages.end()) {
            m_messages[id] = msgChecker;
        }

        subE = subE->NextSiblingElement("struct");
    }

    return ret;
}

std::shared_ptr<MemberCheck> ParamCheckCfgLoader::LoadMessage(XMLElement* ele)
{
    auto msgChecker = std::make_shared<MemberCheck>();
    XMLElement* pe = (XMLElement*)ele->FirstChildElement("param");
    while (pe) {
        auto paramChecker = LoadParam(pe);
        if (!paramChecker) {
            return nullptr;
        }
        DBGLOG("Add param name: %s to message checker!", paramChecker->Name().c_str());
        msgChecker->Add(paramChecker->Name(), paramChecker);  // 重复配置将覆盖
        pe = pe->NextSiblingElement("param");
    }

    pe = (XMLElement*)ele->FirstChildElement("subStruct");
    while (pe) {
        std::string subStructName = GetChildText(pe, "usestruct");
        std::string subName = GetChildText(pe, "name");
        if (subStructName.empty() || subName.empty()) {
            WARNLOG("Get sub struct name if empty");
        } else {
            DBGLOG("Add substruct %s to %s", subStructName.c_str(), ele->Attribute("id"));
            auto it = m_messages.find(subStructName);
            if (it != m_messages.end()) {
                msgChecker->Add(subName, subStructName);
                DBGLOG("Add substruct %s to %s success", subStructName.c_str(), ele->Attribute("id"));
            }
        }
        pe = pe->NextSiblingElement("subStruct");
    }
    return msgChecker;
}

std::shared_ptr<ParamChecker> ParamCheckCfgLoader::LoadParam(XMLElement* ele)
{
    std::string name = GetChildText(ele, "name");
    if (name.empty()) {
        ERRLOG("Attribuite name missing. err: %s", m_doc.ErrorStr());
        return nullptr;
    }

    auto paramChecker = std::make_shared<ParamChecker>(name);

    /* 允许为一个参数指定多个已经定义的检查规则，这些检查规则是已经在param_type_define中定义的 */
    for (XMLElement* pUseType = (XMLElement*)ele->FirstChildElement("usetype"); pUseType != nullptr;
         pUseType = pUseType->NextSiblingElement("usetype")) {
        std::string useType = pUseType->GetText();
        if (useType.empty()) {
            ERRLOG("Attribuite usetype missing. err: %s", m_doc.ErrorStr());
            return nullptr;
        }

        /* 检查是否为已经定义的规则检查器 */
        auto it = m_paramTypes.find(useType);
        if (it == m_paramTypes.end()) {
            ERRLOG("%s is not a defined check rule", useType.c_str());
            return nullptr;
        }
        DBGLOG("Add %s checker to %s", useType.c_str(), name.c_str());
        paramChecker->Add(it->second);
    }

    /* 允许在message中定义新的检查规则，与param_type_define中定义的一样 */
    if (ele->LastChildElement("type")) {
        paramChecker->Add(LoadParamType(ele));
    }

    return paramChecker;
}

void ParamCheckCfgLoader::InitInternalChecker()
{
}
}