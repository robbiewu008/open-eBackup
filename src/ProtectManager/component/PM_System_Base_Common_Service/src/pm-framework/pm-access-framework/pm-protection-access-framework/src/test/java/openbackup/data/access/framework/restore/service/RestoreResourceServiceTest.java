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
package openbackup.data.access.framework.restore.service;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.access.framework.agent.ProtectAgentSelector;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.mocks.CreateRestoreTaskRequestMocker;
import openbackup.data.access.framework.protection.mocks.ProtectedResourceMocker;
import openbackup.data.access.framework.protection.mocks.TokenMocker;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsInAnyOrder;
import static org.hamcrest.Matchers.hasSize;
import static org.hamcrest.Matchers.is;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

/**
 * 恢复资源服务测试用例集合
 *
 **/
@RunWith(PowerMockRunner.class)
@PrepareForTest({TokenBo.class})
public class RestoreResourceServiceTest {
    private final ResourceService resourceService = mock(ResourceService.class);
    private final ProtectedEnvironmentService protectedEnvironmentService = mock(ProtectedEnvironmentService.class);
    private final ProviderManager providerManager = mock(ProviderManager.class);
    private final DefaultProtectAgentSelector defaultProtectAgentSelector = mock(DefaultProtectAgentSelector.class);
    private final RestoreResourceService restoreResourceService = new RestoreResourceService(resourceService,
        protectedEnvironmentService, providerManager, defaultProtectAgentSelector);

    @Rule
    public final ExpectedException exception = ExpectedException.none();

    /**
     * 用例名称：验证当agent信息全部存在时，查询Endpoint方法返回正确<br/>
     * 前置条件：无<br/>
     * check点：返回数量一致，并且对象属性一致<br/>
     */
    @Test
    public void should_success_when_queryEndpoints_given_agent_ids_existed() {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId("1");
        TokenBo tokenBo = TokenMocker.getMockedTokenBo();
        PowerMockito.mockStatic(TokenBo.class);
        PowerMockito.when(TokenBo.get()).thenReturn(tokenBo);
        // Given
        List<Endpoint> endpointList = new ArrayList<>();
        endpointList.add(new Endpoint("22222", "192.128.1.2", 1002));
        endpointList.add(new Endpoint("33333", "192.128.1.3", 1003));
        endpointList.add(new Endpoint("44444", "192.128.1.4", 1004));
        final DefaultProtectAgentSelector mockSelector = mock(DefaultProtectAgentSelector.class);
        given(providerManager.findProvider(any(), any(), any())).willReturn(mockSelector);
        given(mockSelector.select(any(), any())).willReturn(endpointList);
        given(providerManager.findProviderOrDefault(eq(ProtectAgentSelector.class), any(), any()))
                .willReturn(mockSelector);
        given(mockSelector.select(any(), any())).willReturn(endpointList);
        // When
        ProtectedEnvironment targetEnv = new ProtectedEnvironment();
        Map<String, String> advanceParams = new HashMap<>();
        final List<Endpoint> endpoints = restoreResourceService.queryEndpoints(advanceParams,
            "DBBackupAgent", "Host", targetEnv);
        // Then
        assertThat(endpoints, hasSize(endpointList.size()));
        assertThat(endpoints, containsInAnyOrder(endpointList.get(0), endpointList.get(1), endpointList.get(2)));
    }

    /**
     * 用例名称：验证恢复成功时，刷新资源时异常，不影响主流程<br/>
     * 前置条件：无<br/>
     * check点：刷新资源函数抛出异常的时候，调用未抛出异常<br/>
     */
    @Test
    public void should_not_throw_Exception_when_refreshTargetEnv_given_restore_success_and_service_throw_exception() {
        // Given
        String requestId = UUID.randomUUID().toString();
        final String envId = "1111";
        when(protectedEnvironmentService.getEnvironmentById(eq(envId))).thenThrow(new LegoCheckedException(""));
        // When and Then
        restoreResourceService.refreshTargetEnv(requestId, envId, ProviderJobStatusEnum.SUCCESS);
        Mockito.verify(protectedEnvironmentService, Mockito.times(1)).refreshEnvironment(envId);
    }

    /**
     * 用例名称：验证恢复失败是时，未执行刷新资源<br/>
     * 前置条件：无<br/>
     * check点：刷新资源方法未执行<br/>
     */
    @Test
    public void should_not_execute_when_refreshTargetEnv_given_restore_fail() {
        // Given
        String requestId = UUID.randomUUID().toString();
        final String envId = "1111";
        // When
        restoreResourceService.refreshTargetEnv(requestId, envId, ProviderJobStatusEnum.FAIL);
        // Then
        verify(protectedEnvironmentService, never()).getEnvironmentById(envId);
    }

    /**
     * 用例名称：验证当资源信息存在时，查询目标资源信息方法正常返回<br/>
     * 前置条件：无<br/>
     * check点：返回的资源信息与期望的信息一致<br/>
     */
    @Test
    public void should_success_when_queryTaskResource_given_target_object_existed() {
        // Given
        final ProtectedResource protectedResource = ProtectedResourceMocker.mockTaskResource();
        final String targetObject = protectedResource.getUuid();
        given(resourceService.getResourceById(eq(targetObject))).willReturn(Optional.of(protectedResource));
        // When
        final TaskResource taskResource = restoreResourceService.buildTaskResourceByTargetObject(targetObject);
        // Then
        assertThat(taskResource.getUuid(), is(targetObject));
        assertThat(taskResource.getName(), is(protectedResource.getName()));
        assertThat(taskResource.getType(), is(protectedResource.getType()));
        assertThat(taskResource.getSubType(), is(protectedResource.getSubType()));
        assertThat(taskResource.getParentName(), is(protectedResource.getParentName()));
        assertThat(taskResource.getParentUuid(), is(protectedResource.getParentUuid()));
        assertThat(taskResource.getExtendInfo(), is(protectedResource.getExtendInfo()));
    }

    /**
     * 用例名称：验证当资源信息resourceId为空时，查询目标环资源息抛出异常<br/>
     * 前置条件：无<br/>
     * check点：异常信息与期望一致，异常信息与期望一致<br/>
     */
    @Test
    public void should_throw_IllegalArgumentException_when_queryTaskResource_given_target_object_empty() {
        // Given
        // Then
        exception.expect(IllegalArgumentException.class);
        exception.expectMessage("param targetObject is empty");
        // When
        restoreResourceService.buildTaskResourceByTargetObject("");
    }

    /**
     * 用例名称：验证当资源信息不存在时，查询目标资源信息抛出异常<br/>
     * 前置条件：无<br/>
     * check点：异常信息与期望一致<br/>
     */
    @Test
    public void should_return_default_resource_when_queryTaskResource_given_resource_target_object_not_existed() {
        // Given
        final String targetObject = "/A/B";
        given(resourceService.getResourceById(eq(targetObject))).willReturn(Optional.empty());
        // When
        final TaskResource taskResource = restoreResourceService.buildTaskResourceByTargetObject(targetObject);
        // Then
        Assert.assertNotNull(taskResource);
    }

    /**
     * 用例名称：验证恢复任务目标环境不存在时，抛出正确的异常<br/>
     * 前置条件：无<br/>
     * check点：1.抛出异常  2.错误码为期望错误码<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_init_given_target_env_does_not_exist() {
        // Given
        final CreateRestoreTaskRequest taskRequest = CreateRestoreTaskRequestMocker.buildWithParams(RestoreTypeEnum.CR,
            RestoreLocationEnum.ORIGINAL);
        String targetEnv = taskRequest.getTargetEnv();
        given(protectedEnvironmentService.getEnvironmentById(targetEnv)).willThrow(
            new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected environment is not exists!"));
        // When and Then
        Assert.assertThrows("Restore target env does not exist.", LegoCheckedException.class,
                () -> restoreResourceService.queryProtectedEnvironment(taskRequest, any()));
    }
}