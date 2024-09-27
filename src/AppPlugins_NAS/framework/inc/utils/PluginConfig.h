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
#ifndef PLUGIN_UTILS_H
#define PLUGIN_UTILS_H

#include <string>

enum class PluginUsageScene {
    EXTERNAL = 0,
    INTERNAL
};

class PluginConfig {
public:
    PluginConfig(const PluginConfig&) = delete;
    PluginConfig& operator=(const PluginConfig&) = delete;

    static PluginConfig& GetInstance()
    {
        static PluginConfig instance;
        return instance;
    }

    PluginUsageScene m_scene; // 插件使用场景：内置、外置
    std::string m_libName; // 插件名
    std::string m_pluginName; // 动态库全名

private:
    PluginConfig() {};
    ~PluginConfig() {};
};

#endif