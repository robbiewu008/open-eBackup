/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * @file AutoReleasePlugin.h
 * @brief  The implemention about AutoReleasePlugin.h
 * @version 1.0.0.0
 * @date 2022-1-24
 * @author c00511255
 */
#ifndef AUTO_RELEASE_PLUGIN_H_
#define AUTO_RELEASE_PLUGIN_H_
#include "pluginfx/ExternalPluginManager.h"

class AutoReleasePlugin {
public:
    explicit AutoReleasePlugin(const mp_string& strAppType)
        : m_strAppType(strAppType)
    {}
    
    ~AutoReleasePlugin()
    {
        if (!m_strAppType.empty()) {
            ExternalPluginManager::GetInstance().ReleasePluginByRest(m_strAppType);
        }
    }
private:
    mp_string m_strAppType;
};
#endif