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
#include <fstream>
#include "tools/agentcli/UpgradeJsonConf.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/File.h"

mp_int32 UpgradeJsonConf::GetConfigInfo(Json::Value &jsonValue, const mp_string &strFileName)
{
    std::ifstream infile;
    infile.open(strFileName.c_str(), std::ifstream::in);
    if ((infile.fail() && infile.bad()) || ((infile.rdstate() & std::ifstream::failbit) != 0)) {
        COMMLOG(OS_LOG_ERROR, "Open file %s failed, failed[%d], bad[%d]. errno[%d]:%s.", strFileName.c_str(),
            infile.fail(), infile.bad(), errno, strerror(errno));
        infile.close();
        return MP_FAILED;
    }

    Json::Reader jsonReader;
    if (!jsonReader.parse(infile, jsonValue)) {
        COMMLOG(OS_LOG_ERROR, "strFileName[%s] JsonData is invalid.", strFileName.c_str());
        infile.close();
        return MP_FAILED;
    }
    infile.close();
    return MP_SUCCESS;
}

void UpgradeJsonConf::MergeJsonConf(Json::Value &oldPluginInfo, Json::Value &newPluginInfo)
{
    for (auto& item: oldPluginInfo.getMemberNames()) {
        if (newPluginInfo.isMember(item)) {
            if (oldPluginInfo[item].isObject() && newPluginInfo[item].isObject()) {
                MergeJsonConf(oldPluginInfo[item], newPluginInfo[item]);
            } else {
                newPluginInfo[item] = oldPluginInfo[item];
            }
        }
    }
}

 /* ------------------------------------------------------------
Description  : 合并升级前后插件配置
Return       : MP_SUCCESS -- 成功
------------------------------------------------------------- */
mp_int32 UpgradeJsonConf::Handle(const mp_string& oldPluginJsonConf, const mp_string& newPluginJsonConf)
{
    // 获取插件特性配置文件信息
    Json::Value oldPluginInfo;
    Json::Value newPluginInfo;
    if (GetConfigInfo(oldPluginInfo, oldPluginJsonConf) != MP_SUCCESS || GetConfigInfo(newPluginInfo, newPluginJsonConf) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to obtain configuration file information.");
        return MP_FAILED;
    }

    // 合并json内容
    MergeJsonConf(oldPluginInfo, newPluginInfo);
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "    ";
    builder["enableYAMLCompatibility"] = true;
    mp_string mergedPluginInfo = Json::writeString(builder, newPluginInfo);
        
    std::ofstream outputFile(newPluginJsonConf, std::ios::out | std::ios::trunc);
    if (!outputFile.is_open()) {
        COMMLOG(OS_LOG_ERROR, "Failed to open file for writing: %s", newPluginJsonConf.c_str());
        return MP_FAILED;
    }

    outputFile << mergedPluginInfo;
    outputFile.close();
    COMMLOG(OS_LOG_INFO, "Upgrade conf %s success.", newPluginJsonConf.c_str());
    return MP_SUCCESS;
}