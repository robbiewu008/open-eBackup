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
package openbackup.database.base.plugin.interceptor;

import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.copy.CopyRestApi;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 描述
 *
 */
@SpringBootTest(classes = {
    DefaultDbBackupInterceptor.class, DefaultDbRestoreInterceptorProvider.class, TestBaseCopyDeleteInterceptor.class
})
@RunWith(SpringRunner.class)
@Ignore
public class DbInterceptorTest {
    @Autowired
    private DefaultDbBackupInterceptor defaultDbBackupInterceptor;

    @Autowired
    private DefaultDbRestoreInterceptorProvider defaultDbRestoreInterceptorProvider;

    @Autowired
    private TestBaseCopyDeleteInterceptor baseDbCopyDeleteInterceptor;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private AgentUnifiedService agentUnifiedService;

    @MockBean(name = "unifiedConnectionCheckProvider")
    private ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    @MockBean(name = "unifiedResourceConnectionChecker")
    private ProtectedResourceChecker protectedResourceChecker;

    @MockBean
    private PluginConfigManager pluginConfigManager;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private JobService jobService;

    @Before
    public void init() {
        Mockito.when(jobService.queryJob(Mockito.any())).thenReturn(new JobBo());
    }

    @Test
    @Ignore
    public void interceptor_should_supply_backupTask() {
        BackupTask backupTask = create_backupTask();
        create_mockito_condition();
        defaultDbBackupInterceptor.initialize(backupTask);
        Assert.assertTrue(backupTask.getAgents().size() > 0);
        Assert.assertTrue(backupTask.getProtectEnv().getNodes().size() > 0);
        String ipList = backupTask.getProtectEnv()
            .getNodes()
            .get(0)
            .getExtendInfo()
            .get(ResourceConstants.AGENT_IP_LIST);
        Assert.assertNotNull(ipList);
    }

    @Test
    @Ignore
    public void interceptor_should_success_to_restore() {
        RestoreTask restoreTask = create_restoreTask();
        create_mockito_condition();
        defaultDbRestoreInterceptorProvider.initialize(restoreTask);
        Mockito.verify(resourceService, Mockito.times(1)).getResourceById(Mockito.any());
    }

    private void create_mockito_condition() {
        Mockito.when(resourceService.getResourceById(Mockito.any()))
            .thenReturn(Optional.of(create_ProtectedResource()));
        Mockito.when(
            providerManager.findProviderOrDefault(Mockito.eq(ProtectedResourceChecker.class), Mockito.any(),
                Mockito.any())).thenReturn(protectedResourceChecker);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = new HashMap<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint("1.1.1.1");
        protectedEnvironment.setUuid("xxxxxxxxxxxxxxx");
        protectedEnvironment.setPort(3000);
        protectedResourceMap.put(new ProtectedResource(), Collections.singletonList(protectedEnvironment));
        Mockito.when(protectedResourceChecker.collectConnectableResources(Mockito.any()))
            .thenReturn(protectedResourceMap);
        Mockito.when(
            providerManager.findProviderOrDefault(Mockito.eq(ResourceConnectionCheckProvider.class), Mockito.any(),
                Mockito.any())).thenReturn(resourceConnectionCheckProvider);
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(0);
        ResourceCheckContext resourceCheckContext = new ResourceCheckContext();
        resourceCheckContext.setActionResults(Collections.singletonList(actionResult));
        Mockito.when(resourceConnectionCheckProvider.checkConnection(Mockito.any()))
            .thenReturn(resourceCheckContext);
        HostDto hostDto = new HostDto();
        hostDto.setEndpoint("1.1.1.1");
        hostDto.setPort(3000);
        hostDto.setExtendInfo(
            "{ \"agentIpList\" : \"8.40.118.23,8.42.118.23,192.168.118.23,fe80::c7c9:c5cc:343f:933b,"
                + "fe80::ae42:1032:3c67:4988,fe80::ee31:be1:f2b9:d798,fe80::bed8:fa3c:8d1f:d3b6,"
                + "fe80::20a5:eda1:f1cb:2a2d,fe80::3823:de95:af53:7fc9,fe80::c524:a1db:4882:5f75,"
                + "fe80::b281:45d4:7ab5:9c53,fe80::c358:41d0:bc40:7254\" }");
        Mockito.when(agentUnifiedService.getHost(Mockito.any(), Mockito.any())).thenReturn(hostDto);
    }

    private BackupTask create_backupTask() {
        BackupTask backupTask = new BackupTask();
        backupTask.setProtectObject(new TaskResource());
        backupTask.getProtectObject().setUuid("backupTask-uuid");
        backupTask.setAgents(new ArrayList<>());
        backupTask.getAgents().add(new Endpoint());
        backupTask.setProtectEnv(new TaskEnvironment());
        return backupTask;
    }

    private RestoreTask create_restoreTask() {
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setTargetEnv(new TaskEnvironment());
        restoreTask.getTargetEnv().setUuid("restore-target-env-uuid");
        return restoreTask;
    }

    private ProtectedResource create_ProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("protectedResource-uuid");
        return protectedResource;
    }

    /**
     * 测试场景：下发副本删除任务拦截
     * 前置条件：无
     * 检查点：任务参数填充：agent、forceDelete、associateDelete
     */
    @Test
    @Ignore
    public void delete_copy_should_intercepte_succees() {
        create_mockito_condition();
        DeleteCopyTask task = new DeleteCopyTask();
        task.setRequestId("request-id");
        RMap redis = Mockito.mock(RMap.class);
        Mockito.when(redis.get("is_associated")).thenReturn("true");
        Mockito.when(redis.get("user_id")).thenReturn("userId");
        Mockito.when(redissonClient.getMap(Mockito.any(), Mockito.any(StringCodec.class))).thenReturn(redis);
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        copyInfoBo.setResourceId("protectedResource-uuid");
        baseDbCopyDeleteInterceptor.initialize(task, copyInfoBo);
        // associate delete
        Mockito.verify(copyRestApi, Mockito.times(2))
            .deleteCopy(Mockito.any(), Mockito.any(), Mockito.any(), Mockito.eq(false), Mockito.any());
        // agent
        Assert.assertTrue(task.getAgents().size() > 0);
    }
}