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
#ifndef PARAM_HANDLER_H
#define PARAM_HANDLER_H

#include <vector>
#include <map>
#include "define/Types.h"
#include "common/JsonUtils.h"
#include "common/Defines.h"

namespace GeneralDB {
class ParamHandler {
public:
    ParamHandler()
    {}

    ~ParamHandler()
    {}

    mp_int32 Exec(Json::Value &inputValue, std::map<mp_string, mp_string> &sensitiveInfo);
private:
    mp_int32 ParamVerify(const Json::Value &inputValue);
    mp_int32 WipeSensitiveInfo(Json::Value &jsonVal);
    mp_int32 NonSensitiveInfo(Json::Value &nonSensitiveInfo, const mp_string &pid);
    mp_void WipeJsonArray(Json::Value& jsonValArray, const mp_string &key);
    mp_void WipeJsonObject(Json::Value& jsonValObject, const mp_string &key);
    mp_void WipeCopiesInfo(Json::Value& jsonValArray, const mp_string &key);
    mp_bool IsSensitiveInfo(const mp_string& name);
    mp_void RemoveJsonMember(Json::Value &jsonVal, const mp_string &member, const mp_string &key);
private:
    std::map<mp_string, mp_string> m_sensitiveInfo;
};
}
#endif