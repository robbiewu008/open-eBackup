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
package openbackup.gaussdbdws.protection.access.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

/**
 * DWS集群 资源测试类
 *
 */
public class GaussDBDWSClusterResourceProviderTest {
    private final ProviderManager providerManager = Mockito.mock(ProviderManager.class);

    private final PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);

    private final GaussDBDWSClusterResourceProvider gaussDBDWSClusterResourceProvider
        = new GaussDBDWSClusterResourceProvider(this.providerManager, pluginConfigManager);

    /**
     * 用例场景：检测DWS 集群 是否支持lanfree的应用
     * 前置条件：NA
     * 检查点: 不支持
     */
    @Test
    public void get_resource_feature_success() {
        Assert.assertFalse(gaussDBDWSClusterResourceProvider.getResourceFeature().isSupportedLanFree());
    }

}
