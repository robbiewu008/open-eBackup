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
#include "ParseConfigFile.h"
#include "log/Log.h"
#include "common/File.h"
#include "DBPluginPath.h"
#include "common/JsonHelper.h"

using namespace std;
using namespace GeneralDB;

namespace {
#ifdef WIN32
    const mp_string SYSTEM = "win32";
#else
    const mp_string SYSTEM = "linux";
#endif
    const mp_string ACTION = "action";
    const mp_string PROGRESS = "progress";
}

std::shared_ptr<ParseConfigFile> ParseConfigFile::GetInstance()
{
    static std::shared_ptr<ParseConfigFile> g_instance = std::make_shared<ParseConfigFile>();
    return g_instance;
}

mp_int32 ParseConfigFile::GetExectueCmd(const Param &comParam, mp_string &actionConf, mp_string &progessConf)
{
    Json::Value jsValue;
    if (LoadAppConfFile(comParam.appType, jsValue) != MP_SUCCESS) {
        ERRLOG("Failed to load app conf file, appType=%s.", comParam.appType.c_str());
        return MP_FAILED;
    }
    if (!jsValue.isMember(comParam.cmdType) || !jsValue[comParam.cmdType].isObject()) {
        ERRLOG("Configuration file have no %s, appType=%s.", comParam.cmdType.c_str(), comParam.appType.c_str());
        return MP_FAILED;
    }

    if (jsValue[comParam.cmdType][SYSTEM].isObject() &&
        !jsValue[comParam.cmdType][SYSTEM][ACTION].isNull() &&
        jsValue[comParam.cmdType][SYSTEM][ACTION].isString()) {
        actionConf = jsValue[comParam.cmdType][SYSTEM][ACTION].asString();
        if (actionConf.empty()) {
            ERRLOG("The action of %s is empty.", comParam.cmdType.c_str());
            return MP_FAILED;
        } else {
            actionConf = DBPluginPath::GetInstance()->GetScriptPath() + actionConf;
            ReplaceScriptPath(comParam.scriptDir, actionConf);
        }
    } else {
        ERRLOG("The %s is not exist.", comParam.cmdType.c_str());
        return MP_FAILED;
    }
    if (!jsValue[comParam.cmdType][SYSTEM][PROGRESS].isNull() &&
        jsValue[comParam.cmdType][SYSTEM][PROGRESS].isString()) {
        progessConf = jsValue[comParam.cmdType][SYSTEM][PROGRESS].asString();
        if (!progessConf.empty()) {
            progessConf = DBPluginPath::GetInstance()->GetScriptPath() + progessConf;
            ReplaceScriptPath(comParam.scriptDir, progessConf);
        }
    }
    return MP_SUCCESS;
}

mp_int32 ParseConfigFile::LoadAppConfFile(const mp_string &appType, Json::Value &jsValue)
{
    auto it = m_appConfMap.find(appType);
    if (it != m_appConfMap.end()) {
        jsValue = it->second;
        return MP_SUCCESS;
    }
    if (appType.find("/") != mp_string::npos || appType.find("\\") != mp_string::npos) {
        ERRLOG("Apptype parameter is illegitimate.");
        return MP_FAILED;
    }
    mp_string confFilePath = DBPluginPath::GetInstance()->GetGeneraldbConfPath()  + appType + ".conf";
    if (!Module::CFile::FileExist(confFilePath.c_str())) {
        ERRLOG("Application configuration file not exists, appType=%s.", appType.c_str());
        return MP_FAILED;
    }
    if (GetJsonFileContent(confFilePath, jsValue) != MP_SUCCESS) {
        ERRLOG("Read application configuration file failed, appType=%s.", appType.c_str());
        return MP_FAILED;
    }
    if (!jsValue.isObject()) {
        ERRLOG("App conf option is not json object, appType=%s.", appType.c_str());
        return MP_FAILED;
    }
    m_appConfMap.insert(pair<mp_string, Json::Value>(appType, jsValue));
    return MP_SUCCESS;
}

mp_int32 ParseConfigFile::GetJsonFileContent(const mp_string &filePath, Json::Value &jsValue)
{
    mp_string fileContent;
    if (Module::CFile::ReadFile(filePath, fileContent) != MP_SUCCESS) {
        ERRLOG("Read application configuration file failed.");
        return MP_FAILED;
    }
    if (!Module::JsonHelper::JsonStringToJsonValue(fileContent, jsValue)) {
        ERRLOG("Failed to convert string into json.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 ParseConfigFile::GetGenJobConfHandle(Json::Value &retVal)
{
    if (!m_genJobConfHandle.empty()) {
        retVal = m_genJobConfHandle;
        return MP_SUCCESS;
    }
    Json::Value jsValue;
    if (GetJsonFileContent(DBPluginPath::GetInstance()->GetGeneraldbGensubPath(), jsValue) != MP_SUCCESS) {
        ERRLOG("Read application configuration file failed.");
        return MP_FAILED;
    }
    m_genJobConfHandle = jsValue;
    retVal = m_genJobConfHandle;
    return MP_SUCCESS;
}

mp_void ParseConfigFile::ReplaceScriptPath(const mp_string &scriptDir, mp_string &fullPath)
{
    if (scriptDir.empty()) {
        return;
    }
    if (scriptDir.find("/") != mp_string::npos || scriptDir.find("\\") != mp_string::npos) {
        ERRLOG("The path format is wrong.");
        return;
    }
    mp_string subStr = "{scriptDir}";
    fullPath = fullPath.replace(fullPath.find(subStr), subStr.length(), scriptDir);
}
