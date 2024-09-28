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

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.ndmp.protection.access.constant.NdmpConstant;
import openbackup.ndmp.protection.access.service.impl.NdmpServiceImpl;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import openbackup.system.base.common.constants.LocalStorageInfoRes;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableMap;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 备份测试类
 *
 */
@RunWith(PowerMockRunner.class)
public class NdmpBackupInterceptorTest {
    private NdmpBackupInterceptor ndmpBackupInterceptor;

    private LocalStorageService localStorageService;

    @Mock
    private NdmpServiceImpl ndmpService;

    @Before
    public void init() throws IllegalAccessException {
        localStorageService = Mockito.mock(LocalStorageService.class);
        ndmpBackupInterceptor = new NdmpBackupInterceptor(localStorageService, ndmpService);
        MemberModifier.field(NdmpBackupInterceptor.class, "ndmpService").set(ndmpBackupInterceptor, ndmpService);
        LocalStorageInfoRes localStorageInfoRes = new LocalStorageInfoRes();
        localStorageInfoRes.setEsn("xxxxxxxxxxxxxxxxx");
        PowerMockito.when(localStorageService.getStorageInfo()).thenReturn(localStorageInfoRes);
    }

    /**
     * 用例场景：NDMP 下发备份任务 applicable 校验
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(ndmpBackupInterceptor.applicable(ResourceSubTypeEnum.NDMP_BACKUPSET.getType()));
    }

    /**
     * 用例场景：NDMP 更新task信息无异常
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void supplyAgent() {
        BackupTask backupTask = new BackupTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("aaaaaaaaaaaaaaaaa");
        backupTask.setProtectEnv(taskEnvironment);
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("aaaaaaaaaaaaaaaaa");
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setUuid("UUID");
        protectedResource.setEndpoint("123456");
        protectedResource.setPort(22);
        protectedResource.setUuid("123456");
        list.add(protectedResource);
        dependencies.put(NdmpConstant.AGENTS, list);
        resource.setDependencies(dependencies);
        PowerMockito.when(ndmpService.getEnvironmentById(any())).thenReturn(resource);
        TaskResource protectObject = new TaskResource();
        protectObject.setParentUuid("parentUuid");
        backupTask.setProtectObject(protectObject);
        backupTask.setAdvanceParams(ImmutableMap.of("agents", "agents"));
        Endpoint endpoint = new Endpoint();
        endpoint.setId("111111");
        List<Endpoint> endpoints = new ArrayList<>();
        endpoints.add(endpoint);
        PowerMockito.when(ndmpService.getAgents(anyString(), anyString())).thenReturn(endpoints);
        ndmpBackupInterceptor.supplyAgent(backupTask);
        Assert.assertEquals(1, backupTask.getAgents().size());
    }

    @Test
    public void supplyNodes() {
        BackupTask backupTask = new BackupTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("aaaaaaaaaaaaaaaaa");
        backupTask.setProtectEnv(taskEnvironment);
        List<TaskEnvironment> nodes = new ArrayList<>();
        TaskEnvironment clusterTaskEnvironment = new TaskEnvironment();
        clusterTaskEnvironment.setUuid("bbbbbbbbbbbbb");
        TaskEnvironment hostTaskEnvironment = new TaskEnvironment();
        hostTaskEnvironment.setUuid("ccccccccccc");
        nodes.add(clusterTaskEnvironment);
        nodes.add(hostTaskEnvironment);
        PowerMockito.when(ndmpService.supplyNodes()).thenReturn(nodes);
        ndmpBackupInterceptor.supplyNodes(backupTask);
    }

    @Test
    public void supplyBackupTask() {
        BackupTask backupTask = new BackupTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("uuid");
        backupTask.setProtectEnv(taskEnvironment);
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setEndpoint(new Endpoint());
        repositories.add(storageRepository);
        backupTask.setRepositories(repositories);
        TaskResource protectObject = new TaskResource();
        protectObject.setParentUuid("parentUuid");
        protectObject.setExtendInfo(ImmutableMap.of("fullName", "fullName"));
        backupTask.setProtectObject(protectObject);
        ndmpBackupInterceptor.supplyBackupTask(backupTask);
        Assert.assertEquals(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat(), backupTask.getCopyFormat());
    }
}
