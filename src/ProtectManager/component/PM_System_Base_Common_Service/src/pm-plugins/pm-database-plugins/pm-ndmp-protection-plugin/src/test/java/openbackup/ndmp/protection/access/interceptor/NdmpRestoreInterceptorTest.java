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
package openbackup.ndmp.protection.access.interceptor;

import static org.assertj.core.api.Assertions.assertThatNoException;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.ndmp.protection.access.constant.NdmpConstant;
import openbackup.ndmp.protection.access.service.NdmpService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.enums.ProtectionStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
public class NdmpRestoreInterceptorTest {
    private ResourceService resourceService= PowerMockito.mock(ResourceService.class);

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final NdmpService ndmpService = Mockito.mock(NdmpService.class);

    private final NdmpRestoreInterceptor ndmpRestoreInterceptor = new NdmpRestoreInterceptor(ndmpService, copyRestApi,
        resourceService);

    @Test
    public void applicable() {
        Assert.assertTrue(ndmpRestoreInterceptor.applicable(ResourceSubTypeEnum.NDMP_BACKUPSET.getType()));
    }

    @Test
    public void supplyRestoreTask() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setSubType(ResourceSubTypeEnum.NDMP_BACKUPSET.getType());
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        environment.setRootUuid("env");
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> protectedResources = new ArrayList<>();
        ProtectedEnvironment protectedResource1 = new ProtectedEnvironment();
        protectedResource1.setUuid("123");
        protectedResources.add(protectedResource1);
        dependencies.put(NdmpConstant.AGENTS, protectedResources);
        environment.setDependencies(dependencies);
        environment.setExtendInfo(new HashMap<>());
        environment.setProtectionStatus(ProtectionStatusEnum.PROTECTED.getType());
        ProtectedObject protectedObject = new ProtectedObject();
        Map<String, Object> extendParam = new HashMap<>();
        extendParam.put(DatabaseConstants.AGENTS, "192.168.1.1");
        protectedObject.setExtParameters(extendParam);
        environment.setProtectedObject(protectedObject);
        Mockito.when(ndmpService.getEnvironmentById(anyString())).thenReturn(environment);
        Mockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(environment));
        Mockito.when(ndmpService.supplyNodes()).thenReturn(new ArrayList<>());
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = new HashMap<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("uuid");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8088);
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        Endpoint endpoint = new Endpoint();
        endpoint.setId("111111");
        List<Endpoint> endpoints = new ArrayList<>();
        endpoints.add(endpoint);
        PowerMockito.when(ndmpService.getAgents(anyString(), anyString())).thenReturn(endpoints);
        RestoreTask restoreTask = ndmpRestoreInterceptor.supplyRestoreTask(initRestoreTask());
        Assert.assertEquals("env", restoreTask.getTargetEnv().getUuid());
    }

    private RestoreTask initRestoreTask() {
        RestoreTask restoreTask = new RestoreTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setParentUuid("parentUuid");
        taskResource.setRootUuid("parentUuid");
        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put("agents", "agents");
        restoreTask.setAdvanceParams(advanceParams);
        taskResource.setUuid("env");
        restoreTask.setCopyId("copy_id");
        restoreTask.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        restoreTask.setTargetObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("env");
        taskEnvironment.setExtendInfo(new HashMap<>());
        restoreTask.setTargetEnv(taskEnvironment);
        restoreTask.setRepositories(new ArrayList<>());
        restoreTask.setAgents(Lists.newArrayList(new Endpoint("192.168.1.1", 152)));
        return restoreTask;
    }

    @Test
    public void test_supply_agent_success() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setSubType(ResourceSubTypeEnum.NDMP_BACKUPSET.getType());
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        environment.setRootUuid("env");
        environment.setExtendInfo(new HashMap<>());
        environment.setProtectionStatus(ProtectionStatusEnum.UNPROTECTED.getType());
        Mockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(environment));

        List<ProtectedResource> interAgents = new ArrayList<>();
        ProtectedResource interAgent = new ProtectedResource();
        interAgent.setEndpoint("192.168.12.12");
        interAgent.setPort(65535);
        interAgents.add(interAgent);
        Mockito.when(ndmpService.getInterAgents()).thenReturn(interAgents);

        RestoreTask task = initRestoreTask();
        assertThatNoException().isThrownBy(() -> Whitebox.invokeMethod(ndmpRestoreInterceptor, "supplyAgent", task));
    }

    @Test
    public void test_throw_exception_while_target_object_not_exist() {
        RestoreTask task = initRestoreTask();
        task.getAdvanceParams().put("agents", "");
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(ndmpRestoreInterceptor, "supplyAgent", task));
    }

    @Test
    public void test_supply_agent_success_while_user_select_agents() throws Exception {
        RestoreTask task = initRestoreTask();

        List<Endpoint> endpoints = new ArrayList<>();
        endpoints.add(new Endpoint("1", "129.168.1.2", 5955));
        Mockito.when(ndmpService.getAgents(anyString(), anyString())).thenReturn(endpoints);
        Whitebox.invokeMethod(ndmpRestoreInterceptor, "supplyAgent", task);
        Assert.assertEquals(task.getAgents(), endpoints);

    }
}