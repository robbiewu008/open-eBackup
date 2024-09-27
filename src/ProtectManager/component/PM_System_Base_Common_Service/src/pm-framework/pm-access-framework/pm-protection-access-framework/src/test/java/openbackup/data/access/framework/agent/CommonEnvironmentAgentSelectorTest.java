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
package openbackup.data.access.framework.agent;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.agent.CommonEnvironmentAgentSelector;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.HashMap;
import java.util.List;

/**
 * {@link CommonEnvironmentAgentSelector} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-31
 */
public class CommonEnvironmentAgentSelectorTest {
    private final ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);

    private CommonEnvironmentAgentSelector selector = new CommonEnvironmentAgentSelector(environmentService);

    /**
     * 用例场景：框架调applicable接口
     * 前置条件：资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_common_environment_agent_selector_success() {
        Assert.assertTrue(selector.applicable(ResourceTypeEnum.HOST.getType()));
        Assert.assertFalse(selector.applicable(ResourceSubTypeEnum.MYSQL.getType()));
    }

    /**
     * 用例场景：获取资源的环境信息
     * 前置条件：环境为空
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_environment_is_null_when_select() {
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> selector.select(new ProtectedResource(), new HashMap<>()));
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：类型为Host时走到该selector
     * 前置条件：环境为空
     * 检查点: 返回空endpoint
     */
    @Test
    public void should_throw_LegoCheckedException_if_environment_is_null_when_reources_type_is_Host() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType(ResourceTypeEnum.HOST.getType());
        List<Endpoint> endpoints = selector.select(protectedResource, new HashMap<>());
        Assert.assertTrue(VerifyUtil.isEmpty(endpoints.get(0).getId()));
    }

    /**
     * 用例场景：获取资源的环境信息
     * 前置条件：环境为不为空
     * 检查点: 获取agent信息正确
     */
    @Test
    public void execute_select_success() {
        PowerMockito.when(environmentService.getEnvironmentById(any())).thenReturn(mockEnvironment());
        List<Endpoint> endpointList = selector.select(mockProtectedResource(), new HashMap<>());
        Assert.assertEquals(IsmNumberConstant.ONE, endpointList.size());
        Assert.assertEquals(endpointList.get(0).getAgentOS(), "windows");
    }

    private ProtectedResource mockProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setEnvironment(mockEnvironment());
        return protectedResource;
    }

    private ProtectedEnvironment mockEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(UUIDGenerator.getUUID());
        environment.setEndpoint("127.0.0.1");
        environment.setPort(50000);
        environment.setOsType("windows");
        return environment;
    }
}