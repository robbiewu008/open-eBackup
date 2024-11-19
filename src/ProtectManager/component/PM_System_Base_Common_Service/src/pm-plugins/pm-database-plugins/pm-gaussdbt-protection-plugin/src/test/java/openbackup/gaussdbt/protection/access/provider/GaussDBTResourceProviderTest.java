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
package openbackup.gaussdbt.protection.access.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

/**
 * GaussDBTResourceProvider测试类
 *
 */
public class GaussDBTResourceProviderTest {
    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private final PluginConfigManager pluginConfigManager = PowerMockito.mock(PluginConfigManager.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private GaussDBTResourceProvider gaussDBTResourceProvider = new GaussDBTResourceProvider(providerManager,
        pluginConfigManager, resourceService);

    /**
     * 用例场景：gaussDBT类型识别
     * 前置条件：类型参数为GaussDBT
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.GAUSSDBT.getType());
        Assert.assertTrue(gaussDBTResourceProvider.applicable(resource));
    }

    /**
     * 用例场景：GaussDBT健康检查更新节点信息，不使用数据库框架再次校验
     * 前置条件：更新存在的GaussDBT资源
     * 检查点: 调用成功
     */
    @Test
    public void before_update_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.GAUSSDBT.getType());
        resource.setUuid("test_uuid");
        gaussDBTResourceProvider.beforeUpdate(resource);
    }
}
