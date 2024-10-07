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
#ifndef FRAMEWORK_UTILS_H
#define FRAMEWORK_UTILS_H
#include <string>
#include <unordered_map>
#include "define/Defines.h"
#include "json/json.h"
#include "ApplicationProtectPlugin_types.h"

AGENT_API void ParamCheck(const std::string& name, const Json::Value argJv);
AGENT_API void ParamCheck(const std::unordered_map<std::string, Json::Value>& params);
AGENT_API bool ParamCheck(const std::string& name, const Json::Value argJv, AppProtect::ActionResult& returnValue);
AGENT_API bool ParamCheck(
    const std::unordered_map<std::string, Json::Value>& params, AppProtect::ActionResult& returnValue);

#endif