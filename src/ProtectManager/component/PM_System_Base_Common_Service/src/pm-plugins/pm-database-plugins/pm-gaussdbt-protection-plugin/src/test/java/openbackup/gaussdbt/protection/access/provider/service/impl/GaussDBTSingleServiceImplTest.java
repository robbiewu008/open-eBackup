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
package openbackup.gaussdbt.protection.access.provider.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.service.DatabaseRestoreService;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTReleaseTypeEnum;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTSingleStateEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * {@link GaussDBTSingleServiceImpl} 测试类
 *
 */
public class GaussDBTSingleServiceImplTest {
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = PowerMockito.mock(
        ResourceConnectionCheckProvider.class);

    private final InstanceProtectionService instanceProtectionService = PowerMockito.mock(
        InstanceProtectionService.class);

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final DatabaseRestoreService databaseRestoreService = PowerMockito.mock(DatabaseRestoreService.class);

    private GaussDBTSingleServiceImpl gaussDBTSingleService = new GaussDBTSingleServiceImpl(resourceService,
        providerManager, instanceProtectionService, copyRestApi, databaseRestoreService);

    /**
     * 用例场景：校验GaussDBT单机资源是否被注册
     * 前置条件：已经被注册
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_registered_when_check_gaussdbt_single() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockGaussDBTSingleNums(1));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> gaussDBTSingleService.checkSingleIsRegistered(mockResource()));
        Assert.assertEquals(CommonErrorCode.RESOURCE_IS_REGISTERED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：校验GaussDBT单机资源是否被注册
     * 前置条件：未被注册
     * 检查点：无异常抛出
     */
    @Test
    public void check_gaussdbt_single_is_registered_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockGaussDBTSingleNums(0));
        gaussDBTSingleService.checkSingleIsRegistered(mockResource());
    }

    /**
     * 用例场景：校验GaussDBT单机资源的用户是否被修改
     * 前置条件：用户被修改
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_user_changed_when_check_gaussdbt_single() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockGaussDBTSingleNums(0));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> gaussDBTSingleService.checkSingleInstallUserIsChanged(mockResource()));
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：校验GaussDBT单机资源的用户是否被修改
     * 前置条件：用户未被修改
     * 检查点：无异常抛出
     */
    @Test
    public void check_single_install_user_is_changed_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockGaussDBTSingleNums(1));
        gaussDBTSingleService.checkSingleInstallUserIsChanged(mockResource());
    }

    /**
     * 用例场景：GaussDBT单机资源联调性检查
     * 前置条件：连通正常
     * 检查点：无异常抛出
     */
    @Test
    public void check_connection_success() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any())).thenReturn(mockContext());
        ProtectedEnvironment resource = mockResource();
        gaussDBTSingleService.checkConnection(resource);
    }

    /**
     * 用例场景：GaussDBT单机资源联调性检查
     * 前置条件：检查结果为空
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_result_is_empty_when_check_connection() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        ResourceCheckContext context = mockContext();
        context.setActionResults(new ArrayList<>());
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any())).thenReturn(context);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> gaussDBTSingleService.checkConnection(mockResource()));
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：GaussDBT单机资源联调性检查
     * 前置条件：检查结果为失败
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_result_is_failed_when_check_connection() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        ResourceCheckContext context = mockContext();
        context.getActionResults().get(IsmNumberConstant.ZERO).setCode(IsmNumberConstant.TWO);
        context.getActionResults().get(IsmNumberConstant.ZERO).setBodyErr("1677929475");
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any())).thenReturn(context);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> gaussDBTSingleService.checkConnection(mockResource()));
        Assert.assertEquals(1677929475L, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：GaussDBT单机资源参数填充
     * 前置条件：资源检查通过
     * 检查点：参数值填充正确
     */
    @Test
    public void fillGaussDBTSingleProperties_success() {
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(mockResource()));
        ProtectedEnvironment resource = mockResource();
        gaussDBTSingleService.fillGaussDBTSingleProperties(resource, mockResult());
        Assert.assertEquals("GaussDB_T_1.2.1", resource.getVersion());
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(),
            resource.getExtendInfoByKey(DatabaseConstants.DEPLOY_TYPE));
        Assert.assertEquals(GaussDBTSingleStateEnum.NORMAL.getState(),
            resource.getExtendInfoByKey(GaussDBTConstant.CLUSTER_STATE_KEY));
        Assert.assertEquals("test_user", resource.getExtendInfoByKey(GaussDBTConstant.INSTALL_USER_KEY));
        Assert.assertEquals(GaussDBTReleaseTypeEnum.STAND_ALONE.getType(),
            resource.getExtendInfoByKey(GaussDBTConstant.RELEASE_TYPE_KEY));
        Assert.assertEquals("127.0.0.1", resource.getPath());
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), resource.getLinkStatus());
        Assert.assertEquals("test_agent_id", resource.getExtendInfoByKey(DatabaseConstants.HOST_ID));
    }

    /**
     * 用例场景：GaussDBT单机状态更新
     * 前置条件：资源检查通过
     * 检查点：更新状态
     */
    @Test
    public void checkGaussDTSingleStatus_suucess() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any())).thenReturn(mockContext());
        gaussDBTSingleService.checkGaussDTSingleStatus(mockResource());
    }

    /**
     * 用例场景：获取环境的nodes
     * 前置条件：资源正常
     * 检查点：获取到nodes值
     */
    @Test
    public void should_return_nodes_when_execute_getEnvNodes() {
        PowerMockito.when(instanceProtectionService.extractEnvNodesBySingleInstance(any())).thenReturn(mockNodes());
        List<TaskEnvironment> nodes = gaussDBTSingleService.getEnvNodes(new ProtectedResource());
        Assert.assertEquals(IsmNumberConstant.ONE, nodes.size());
    }

    /**
     * 用例场景：获取资源的agents
     * 前置条件：资源正常
     * 检查点：获取到agents值
     */
    @Test
    public void should_return_agents_when_execute_getAgents() {
        PowerMockito.when(instanceProtectionService.extractEnvNodesBySingleInstance(any())).thenReturn(mockNodes());
        List<Endpoint> agents = gaussDBTSingleService.getAgents(new ProtectedResource());
        Assert.assertEquals(IsmNumberConstant.ONE, agents.size());
    }

    /**
     * 用例场景：检查是否支持恢复任务
     * 前置条件：参数正常
     * 检查点：无异常抛出
     */
    @Test
    public void execute_check_support_restore_success() {
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(mockCopy());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(mockResource()));
        gaussDBTSingleService.checkSupportRestore(mockRestoreTask());
    }

    private ProtectedEnvironment mockResource() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setParentUuid(UUID.randomUUID().toString());
        environment.setName("GaussDBT_test_database");
        environment.setSubType("GaussDBT-single");
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid("test_agent_id");
        agent.setEndpoint("127.0.0.1");
        agent.setPort(59527);
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(agent);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, agents);
        environment.setDependencies(dependencies);
        environment.setEnvironment(agent);
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.NO_AUTH);
        auth.setAuthKey("test_user");
        environment.setAuth(auth);
        environment.setEndpoint("127.0.0.1");
        return environment;
    }

    private PageListResponse<ProtectedResource> mockGaussDBTSingleNums(int singleNum) {
        PageListResponse<ProtectedResource> resources = new PageListResponse<>();
        resources.setTotalCount(singleNum);
        return resources;
    }

    private ResourceCheckContext mockContext() {
        Map<String, String> map = new HashMap<>();
        map.put(DatabaseConstants.VERSION, "GaussDB_T_1.2.1");
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(DatabaseConstants.SUCCESS_CODE);
        actionResult.setMessage(JSONObject.fromObject(map).toString());
        List<ActionResult> list = new ArrayList<>();
        list.add(actionResult);
        ResourceCheckContext context = new ResourceCheckContext();
        context.setActionResults(list);
        return context;
    }

    private String mockResult() {
        Map<String, String> map = new HashMap<>();
        map.put(DatabaseConstants.VERSION, "GaussDB_T_1.2.1");
        return JSONObject.fromObject(map).toString();
    }

    private List<TaskEnvironment> mockNodes() {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid(UUID.randomUUID().toString());
        taskEnvironment.setEndpoint("127.0.0.1");
        taskEnvironment.setPort(59521);
        return Collections.singletonList(taskEnvironment);
    }

    private Copy mockCopy() {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setResourceSubType(ResourceSubTypeEnum.GAUSSDBT_SINGLE.getType());
        return copy;
    }

    private RestoreTask mockRestoreTask() {
        RestoreTask task = new RestoreTask();
        task.setCopyId(UUIDGenerator.getUUID());
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid(UUIDGenerator.getUUID());
        taskResource.setSubType(ResourceSubTypeEnum.GAUSSDBT_SINGLE.getType());
        task.setTargetObject(taskResource);
        return task;
    }
}