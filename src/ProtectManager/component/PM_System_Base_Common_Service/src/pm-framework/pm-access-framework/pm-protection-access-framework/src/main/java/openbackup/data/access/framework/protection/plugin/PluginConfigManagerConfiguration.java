/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.data.access.framework.protection.plugin;

import openbackup.data.access.framework.core.plugin.DefaultPluginConfigManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

/**
 * 插件configurantion
 *
 * @since 2022-05-23
 */
@Configuration
public class PluginConfigManagerConfiguration {
    /**
     * PluginConfigManager Bean
     *
     * @return PluginConfigManager
     */
    @Bean
    PluginConfigManager getPluginConfigManager() {
        DefaultPluginConfigManager pluginConfigManager = new DefaultPluginConfigManager();
        pluginConfigManager.init();
        return pluginConfigManager;
    }
}
