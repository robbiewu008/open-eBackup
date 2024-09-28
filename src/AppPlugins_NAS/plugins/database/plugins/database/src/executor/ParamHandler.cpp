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
#include "ParamHandler.h"

#include <regex>
#include <cstdlib>
#include "log/Log.h"
#include "common/File.h"
#include "DBPluginPath.h"

using namespace std;
using namespace GeneralDB;

namespace {
    const mp_string MODULE = "ParamHandler";
    const mp_string SensitiveFields[25] = { "[Pp][Aa][Ss][Ss]",
        "[Pp][Ww][Dd]", "[Kk][Ee][Yy]", "[Cc][Rr][Yy][Pp][Tt][Oo]", "[Ss][Ee][Ss][Ss][Ii][Oo][Nn]",
        "[Tt][Oo][Kk][Ee][Nn]", "[Ff][Ii][Nn][Gg][Ee][Rr][Pp][Rr][Ii][Nn][Tt]", "[Aa][Uu][Tt][Hh]",
        "[Ee][Nn][Cc]", "[Dd][Ee][Cc]", "[Tt][Gg][Tt]", "[Ii][Qq][Nn]", "[Ii][Nn][Ii][Tt][Ii][Aa][Tt][Oo][Rr]",
        "[Ss][Ee][Cc][Rr][Ee][Tt]", "[Cc][Ee][Rr][Tt]", "^[Ss][Kk]$", "^[Ii][Vv]$", "[Ss][Aa][Ll][Tt]", "^[Mm][Kk]$",
        "[Pp][Rr][Ii][Vv][Aa][Tt][Ee]", "[Uu][Ss][Ee][Rr][_][Ii][Nn][Ff][Oo]", "[Ee][Mm][Aa][Ii][Ll]",
        "[Pp][Hh][Oo][Nn][Ee]", "[Rr][Aa][Nn][Dd]", "[Vv][Ee][Rr][Ff][Ii][Yy][Cc][Oo][Dd][Ee]" };
    const mp_int32 SensitiveFieldsSize = sizeof(SensitiveFields) / sizeof(SensitiveFields[0]);
    const mp_string FILE_LIST_PARAMS = "filelistParams";
    const mp_string COPIES = "copies";
    const mp_string PROTECT_ENV = "protectEnv";
    const mp_string PROTECT_OBJECT = "protectObject";
    const mp_string REPOSITORIES = "repositories";
}

mp_int32 ParamHandler::Exec(Json::Value &inputValue, std::map<mp_string, mp_string> &sensitiveInfo)
{
    if (ParamVerify(inputValue) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Failed to verfiy param info." << HCPENDLOG;
        return MP_FAILED;
    }
    if (WipeSensitiveInfo(inputValue) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Failed to wipe sensitive info." << HCPENDLOG;
        return MP_FAILED;
    }
    sensitiveInfo = m_sensitiveInfo;
    return MP_SUCCESS;
}

mp_int32 ParamHandler::ParamVerify(const Json::Value &inputValue)
{
    return MP_SUCCESS;
}

mp_int32 ParamHandler::WipeSensitiveInfo(Json::Value &jsonVal)
{
    if (jsonVal.type() == Json::arrayValue) {
        WipeJsonArray(jsonVal, "");
    } else if (jsonVal.type() == Json::objectValue) {
        WipeJsonObject(jsonVal, "");
    } else {
        HCP_Log(ERR, MODULE) << "Failed to convert string into json." << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_void ParamHandler::WipeJsonArray(Json::Value& jsonValArray, const mp_string &key)
{
    mp_string tmpMembers;
    for (mp_int32 i = 0; i < jsonValArray.size(); ++i) {
        if (key.empty()) {
            tmpMembers = to_string(i);
        } else {
            tmpMembers = key + "_" + to_string(i);
        }
        if (jsonValArray[i].type() == Json::objectValue) {
            WipeJsonObject(jsonValArray[i], tmpMembers);
        } else if (jsonValArray[i].type() == Json::arrayValue) {
            WipeJsonArray(jsonValArray[i], tmpMembers);
        }
    }
}

mp_void ParamHandler::WipeJsonObject(Json::Value& jsonValObject, const mp_string &key)
{
    Json::Value::Members members = jsonValObject.getMemberNames();
    for (auto &iter : members) {
        mp_string tmpKey;
        if (key.empty()) {
            tmpKey = iter;
        } else {
            tmpKey = key + "_" + iter;
        }
        /* 针对Oracle数据库进行特殊适配，filelistParams字段不进行脱敏，防止因该字段值较多导致脱敏超时 */
        if (iter == FILE_LIST_PARAMS) {
            continue;
        }
        /* copies字段保存副本信息，仅需要对其protectEnv和protectObject字段进行脱敏，减少参数脱敏时间 */
        if (iter == COPIES) {
            WipeCopiesInfo(jsonValObject[iter], tmpKey);
            continue;
        }
        if (jsonValObject[iter].type() == Json::objectValue) {
            WipeJsonObject(jsonValObject[iter], tmpKey);
        } else if (jsonValObject[iter].type() == Json::arrayValue) {
            WipeJsonArray(jsonValObject[iter], tmpKey);
        } else if (IsSensitiveInfo(iter)) {
            RemoveJsonMember(jsonValObject, iter, tmpKey);
        }
    }
}

mp_void ParamHandler::WipeCopiesInfo(Json::Value& jsonValArray, const mp_string &key)
{
    mp_string tmpMembers;
    for (mp_int32 i = 0; i < jsonValArray.size(); ++i) {
        tmpMembers = key + "_" + to_string(i);
        Json::Value::Members members = jsonValArray[i].getMemberNames();
        for (auto &iter : members) {
            if (iter == PROTECT_ENV || iter == PROTECT_OBJECT) {
                mp_string tmpKey = tmpMembers + "_" + iter;
                WipeJsonObject(jsonValArray[i][iter], tmpKey);
            } else if (iter == REPOSITORIES) {
                mp_string tmpKey = tmpMembers + "_" + iter;
                WipeJsonArray(jsonValArray[i][iter], tmpKey);
            }
        }
    }
}

mp_bool ParamHandler::IsSensitiveInfo(const mp_string& name)
{
    for (mp_size i = 0; i < SensitiveFieldsSize; ++i) {
        std::regex reg(SensitiveFields[i]);
        std::smatch match;
        std::regex_search(name, match, reg);
        if (!match.empty()) {
            return MP_TRUE;
        }
    }
    return MP_FALSE;
}

mp_void ParamHandler::RemoveJsonMember(Json::Value &jsonVal, const mp_string &member, const mp_string &key)
{
    if (jsonVal[member].type() == Json::stringValue) {
        mp_string value = jsonVal[member].asString();
        m_sensitiveInfo.insert(pair<mp_string, mp_string>(key, value));
    } else if (jsonVal[member].type() == Json::realValue) {
        mp_string value = to_string(jsonVal[member].asDouble());
        m_sensitiveInfo.insert(pair<mp_string, mp_string>(key, value));
    } else if (jsonVal[member].type() == Json::uintValue) {
        mp_string value = to_string(jsonVal[member].asUInt());
        m_sensitiveInfo.insert(pair<mp_string, mp_string>(key, value));
    } else if (jsonVal[member].type() == Json::intValue) {
        mp_string value = to_string(jsonVal[member].asInt());
        m_sensitiveInfo.insert(pair<mp_string, mp_string>(key, value));
    } else {
        return;
    }
    jsonVal.removeMember(member);
}
