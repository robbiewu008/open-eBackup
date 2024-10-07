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
#ifndef PARAM_CHECKER_H
#define PARAM_CHECKER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <boost/algorithm/string.hpp>
#include "json/json.h"
#include "define/Defines.h"

namespace Module {
/*
 * 规则检查器，用于检查参数的取值规则
 */
class RuleChecker {
public:
    virtual ~RuleChecker()
    {}

    virtual bool Check(const std::string& targetParam) = 0;
};

class IntRuleChecker : public RuleChecker {
public:
    IntRuleChecker(long long f, long long t) : from(f), to(t)
    {}

    ~IntRuleChecker() override
    {}

    bool Check(const std::string& targetParam) override;

private:
    long long from;
    long long to;
};

class LengthRuleChecker : public RuleChecker {
public:
    LengthRuleChecker(long long f, long long t) : from(f), to(t)
    {}

    ~LengthRuleChecker() override
    {}

    bool Check(const std::string& targetParam) override;

private:
    long long from;
    long long to;
};

class StrRuleChecker : public RuleChecker {
public:
    explicit StrRuleChecker(const std::string& p) : pattern(p)
    {}

    ~StrRuleChecker() override
    {}

    bool Check(const std::string& targetParam) override;

private:
    std::string pattern;
};

/*
 * 参数检查器，一个参数可能对应多种规则的检查器
 */
class ParamChecker {
public:
    explicit ParamChecker(const std::string& name) : m_name(name)
    {}

    virtual ~ParamChecker()
    {}

    void Add(const std::shared_ptr<RuleChecker>& ck)
    {
        if (ck) {
            m_ruleCheckers.emplace_back(ck);
        }
    }

    const std::string& Name()
    {
        return m_name;
    }

    bool Check(const Json::Value& msgArgsJv);

private:
    bool ElememtCheck(const Json::Value& msgArgsJv);

private:
    std::string m_name;                                        // 参数名
    std::vector<std::shared_ptr<RuleChecker>> m_ruleCheckers;  // 该参数对应的各种规则检查器
};

/*
 * 消息检查器，包含消息中各参数的检查器，用于检查REST接口中各参数的合法性
 */
class MemberCheck {
public:
    bool Check(const Json::Value& msgArgs);

    void Add(const std::string& param, const std::shared_ptr<ParamChecker>& ck)
    {
        if (ck) {
            std::string name(param);
            m_paramCheckers[name] = ck;
        }
    }

    void Add(const std::string& name, const std::string& subStructName)
    {
        m_subStructs[name] = subStructName;
    }

private:
    bool ElememtCheck(const Json::Value& msgArgsJv);

private:
    std::unordered_map<std::string, std::shared_ptr<ParamChecker>> m_paramCheckers;  // 参数及对应的检查器，参数大写
    std::unordered_map<std::string, std::string> m_subStructs;  // 子结构体对应的检查器，参数大写
};

/* 对DB Rest接口进行检查，包括参数约束和合法性检查等 */
class AGENT_API StructChecker {
public:
	static StructChecker& Instance();
    void Init(const std::string& xmlPath);
    bool Check(const std::string& key, const Json::Value& args);
private:
	bool m_button{ false };
};
}
#endif
