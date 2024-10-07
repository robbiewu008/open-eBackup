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
#include "Utils.h"
#include "log/Log.h"
#include "PluginTypes.h"
#include "param_checker/ParamChecker.h"
#include "ApplicationProtectFramework_types.h"
#include "config_reader/ConfigIniReader.h"

using namespace AppProtect;

void ParamCheck(const std::string& name, const Json::Value argJv)
{
    int isNeedCheck = Module::ConfigReader::getInt("General", "AllowParamCheck");
    if (isNeedCheck == 0) {
        WARNLOG("AllowParamCheck is set to 0, Paramchecker is disabled!");
        return;
    }
    bool ret = Module::StructChecker::Instance().Check(name, argJv);
    if (!ret) {
        AppProtectPluginException exception;
        exception.code = -1;
        exception.message = "check param failed";
        throw exception;
    }
}

void ParamCheck(const std::unordered_map<std::string, Json::Value>& params)
{
    int isNeedCheck = Module::ConfigReader::getInt("General", "AllowParamCheck");
    if (isNeedCheck == 0) {
        WARNLOG("AllowParamCheck is set to 0, Paramchecker is disabled!");
        return;
    }
    for (auto iter = params.begin(); iter != params.end(); ++iter) {
        bool ret = Module::StructChecker::Instance().Check(iter->first, iter->second);
        if (!ret) {
            AppProtectPluginException exception;
            exception.code = -1;
            exception.message = "check param failed";
            throw exception;
        }
    }
}

bool ParamCheck(const std::string& name, const Json::Value argJv, ActionResult& returnValue)
{
    int isNeedCheck = Module::ConfigReader::getInt("General", "AllowParamCheck");
    if (isNeedCheck == 0) {
        WARNLOG("AllowParamCheck is set to 0, Paramchecker is disabled!");
        return true;
    }
    bool ret = Module::StructChecker::Instance().Check(name, argJv);
    if (!ret) {
        returnValue.__set_code(ERROR_COMMON_INVALID_PARAM);
    }
    return ret;
}

bool ParamCheck(const std::unordered_map<std::string, Json::Value>& params, ActionResult& returnValue)
{
    int isNeedCheck = Module::ConfigReader::getInt("General", "AllowParamCheck");
    if (isNeedCheck == 0) {
        WARNLOG("AllowParamCheck is set to 0, Paramchecker is disabled!");
        return true;
    }
    for (auto iter = params.begin(); iter != params.end(); ++iter) {
        bool ret = Module::StructChecker::Instance().Check(iter->first, iter->second);
        if (!ret) {
            returnValue.__set_code(ERROR_COMMON_INVALID_PARAM);
            return false;
        }
    }
    return true;
}
