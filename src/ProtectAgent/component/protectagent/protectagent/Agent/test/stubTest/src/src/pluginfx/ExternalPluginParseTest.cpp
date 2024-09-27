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
#include "pluginfx/ExternalPluginParseTest.h"
#include <string>
#include <fstream>
#include <dirent.h>
#include "pluginfx/ExternalPluginParse.h"
#include "common/Types.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/JsonUtils.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/File.h"

using namespace std;

mp_void LogReturn(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...)
{
    return;
}

mp_int32 ParsePluginListFailed()
{
    return MP_FAILED;
}

mp_int32 ParsePluginListSucc()
{
    return MP_SUCCESS;
}

mp_int32 GetPluginConfigPathFailed(const mp_string &strPluginPath, mp_string &strPluginConfigName)
{
    return MP_FAILED;
}

mp_int32 GetPluginConfigPathSucc(const mp_string &strPluginPath, mp_string &strPluginConfigName)
{
    strPluginConfigName = "plugin_attribute_1.1.0.json";
    return MP_SUCCESS;
}

mp_int32 GetConfigInfoSucc(plugin_info &pluginInfo)
{
    std::vector<mp_string> appTypeVec;
    appTypeVec.push_back("APPPlugin_NAS1");
    appTypeVec.push_back("APPPlugin_NAS2");

    pluginInfo.application = appTypeVec;
    pluginInfo.name = "Nas";
    pluginInfo.application_version["APPPlugin_NAS1"].minVer = "11.0";
    pluginInfo.application_version["APPPlugin_NAS1"].maxVer = "13.0";
    return MP_SUCCESS;
}

mp_void CreateTemConfig(mp_string fileName)
{
    CMpFile::CreateFile(fileName);
}

mp_void ClearTemConfig(mp_string fileName)
{
    CMpFile::DelFile(fileName);
}

mp_int32 JudgePluginVersion(mp_string pluginVersion)
{
    if (pluginVersion == "1.1.0") {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}
/*
 * 用例名称：解析插件特性文件
 * 前置条件：外部插件目录Plugins存在
 * check点：能否正确解析文件
 */
TEST_F(ExternalPluginParseTest, ParsePluginAttributeTest)
{
    plugin_info pluginInfo;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    ExternalPluginParse externalPluginParse;

    mp_int32 iRet = externalPluginParse.ParsePluginAttribute();
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(ADDR(ExternalPluginParse, GetConfigInfo), GetConfigInfoSucc);
    iRet = externalPluginParse.ParsePluginAttribute();
    EXPECT_EQ(iRet, MP_SUCCESS);
}
/*
 * 用例名称：获取json配置文件字段
 * 前置条件：外部插件目录Plugins存在
 * check点：打开文件返回值是否正确 
 */
/*TEST_F(ExternalPluginParseTest, GetConfigInfoTest)
{
    plugin_info pluginInfo;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    ExternalPluginParse externalPluginParse;
    mp_string strFileName = "/opt/plugin_attribute_1.1.0.json";
    (mp_void)CreateTemConfig(strFileName);

    EXPECT_EQ(MP_FAILED, externalPluginParse.ParsePluginAttribute());

    (mp_void)ClearTemConfig(strFileName);
}*/
/*
 * 用例名称：检查解析的插件版本是否正确
 * 前置条件：插件名称为plugin_attribute_1.1.0.json
 * check点：能否解析插件版本1.1.0
 */
TEST_F(ExternalPluginParseTest, GetPluginConfigVersionTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    ExternalPluginParse externalPluginParse;

    mp_string pluginConfigName = "plugin_attribute_";
    mp_string strPluginConfigName = "plugin_attribute_1.1.0.json";
    mp_string strVersion;

    mp_string::size_type start = pluginConfigName.size();
    mp_string::size_type end = strPluginConfigName.find(".json");
    strVersion = strPluginConfigName.substr(start, end - start);

    mp_int32 iRet = JudgePluginVersion(strVersion);
    EXPECT_EQ(iRet, MP_SUCCESS);
}