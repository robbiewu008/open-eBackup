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
package openbackup.data.access.framework.protection.plugin;

import openbackup.data.access.framework.core.plugin.DefaultPluginConfigManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;

import org.junit.Assert;
import org.junit.Test;

public class PluginConfigManagerTest {
    private static final String subType = "Mysql";

    private PluginConfigManager pluginConfigManager = new DefaultPluginConfigManager();

    @Test
    public void test_resolve_plugin_file() {
        pluginConfigManager.init();
        PluginConfig pluginConfig = pluginConfigManager.getPluginConfig(subType).orElse(null);
        Assert.assertNotNull(pluginConfig);
        Assert.assertEquals(pluginConfig.getSubType(), subType);
        Assert.assertTrue(pluginConfig.getConfigMap().size() > 0);
    }
}
