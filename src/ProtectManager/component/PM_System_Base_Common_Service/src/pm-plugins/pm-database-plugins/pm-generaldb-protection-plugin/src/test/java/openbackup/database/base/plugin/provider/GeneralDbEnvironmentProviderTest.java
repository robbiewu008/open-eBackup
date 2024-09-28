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
package openbackup.database.base.plugin.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.util.TestConfHelper;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * GeneralDbEnvironmentProvider测试类
 *
 */
@SpringBootTest(classes = {
    GeneralDbEnvironmentProvider.class, UnifiedConnectionCheckProvider.class, GeneralDbResourceConnectionChecker.class
})
@RunWith(SpringRunner.class)
public class GeneralDbEnvironmentProviderTest {
    @Autowired
    private GeneralDbEnvironmentProvider generalDbEnvironmentProvider;

    @Autowired
    private UnifiedConnectionCheckProvider unifiedConnectionCheckProvider;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    @Qualifier("unifiedResourceConnectionChecker")
    private UnifiedResourceConnectionChecker unifiedResourceConnectionChecker;

    @MockBean
    private PluginConfigManager pluginConfigManager;

    @MockBean
    private AgentUnifiedService agentUnifiedService;

    @Autowired
    private GeneralDbResourceConnectionChecker generalDbResourceConnectionChecker;

    @MockBean
    private ProtectedEnvironmentRetrievalsService environmentRetrievalsService;

    @Before
    public void init() {
        Mockito.when(providerManager.findProviderOrDefault(Mockito.eq(ProtectedResourceChecker.class), Mockito.any(),
            Mockito.any())).thenReturn(generalDbResourceConnectionChecker);

        AgentBaseDto agentBaseDto1 = new AgentBaseDto();
        agentBaseDto1.setErrorCode("0");
        Mockito.when(agentUnifiedService.checkApplication(Mockito.any(), Mockito.any())).thenReturn(agentBaseDto1);

        generalDbEnvironmentProvider.register(null);
        Assert.assertTrue(generalDbEnvironmentProvider.applicable(ResourceSubTypeEnum.GENERAL_DB.getType()));
    }

    /**
     * 用例场景：通用数据库健康检查
     * 前置条件：无
     * 检查点：当host在线时，健康检查通过
     */
    @Test
    public void health_check_should_success() {
        ProtectedEnvironment environment = TestConfHelper.mockInstance(true);
        environment.setExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_CONF, TestConfHelper.getHanaConf());
        generalDbEnvironmentProvider.validate(environment);
        Mockito.verify(agentUnifiedService, Mockito.times(1)).checkApplication(Mockito.any(), Mockito.any());
    }
}
