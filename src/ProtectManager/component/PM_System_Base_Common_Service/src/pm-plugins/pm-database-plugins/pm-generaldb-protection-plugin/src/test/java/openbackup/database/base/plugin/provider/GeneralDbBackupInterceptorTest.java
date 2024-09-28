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

import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.common.GeneralDbErrorCode;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.database.base.plugin.util.TestConfHelper;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

/**
 * GeneralDbBackupInterceptor测试类
 *
 */
@SpringBootTest(classes = {GeneralDbBackupInterceptor.class, GeneralDbProtectAgentService.class})
@RunWith(SpringRunner.class)
public class GeneralDbBackupInterceptorTest {
    @Autowired
    private GeneralDbBackupInterceptor generalDbBackupInterceptor;

    @Autowired
    private GeneralDbProtectAgentService generalDbProtectAgentService;

    @MockBean
    protected ProviderManager providerManager;

    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private AgentUnifiedService agentUnifiedService;

    @MockBean
    private DataBaseAgentSelector dataBaseAgentSelector;

    @MockBean
    @Qualifier("unifiedConnectionCheckProvider")
    private ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    @MockBean
    @Qualifier("unifiedResourceConnectionChecker")
    private ProtectedResourceChecker protectedResourceChecker;

    @Before
    public void init() {
        ProtectedEnvironment singleDb = TestConfHelper.mockInstance(false);
        List<ProtectedEnvironment> hosts = TestConfHelper.mockHost();
        GeneralDbUtil.setProtectResourceFullHostInfo(singleDb,hosts);
        Mockito.when(resourceService.getResourceById(Mockito.any()))
            .thenReturn(Optional.of(singleDb));
        HostDto hostDto = new HostDto();
        hostDto.setUuid("host1");
        Mockito.when(agentUnifiedService.getHost(Mockito.any(), Mockito.any())).thenReturn(hostDto);
    }

    /**
     * 用例场景：通用数据库设置备份参数
     * 前置条件：无
     * 检查点：备份参数设置成功：format,agent,node,conf info,repository
     */
    @Test
    public void backup_interceptor_success() {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(DatabaseConstants.FULL_BACKUP_TYPE);
        backupTask.setProtectEnv(new TaskEnvironment());
        backupTask.setRepositories(new ArrayList<>(Collections.singletonList(new StorageRepository())));
        TaskResource protectObject = new TaskResource();
        protectObject.setUuid("testUuid");
        protectObject.setVersion("1.0");
        protectObject.setExtendInfo(new HashMap<>());
        protectObject.getExtendInfo().put(GeneralDbConstant.EXTEND_SCRIPT_CONF, TestConfHelper.getHanaConf());
        backupTask.setProtectObject(protectObject);

        generalDbBackupInterceptor.initialize(backupTask);

        // format
        Assert.assertEquals(backupTask.getCopyFormat(), 1);
        // agent
        Assert.assertEquals(backupTask.getAgents().get(0).getId(), "host1");
        // node
        Assert.assertEquals(backupTask.getProtectEnv().getNodes().get(0).getUuid(), "host1");
        // conf info
        Assert.assertEquals(backupTask.getAdvanceParams().get(TaskUtil.SPEED_STATISTICS), "1");
        Assert.assertEquals(backupTask.getAdvanceParams().get(DatabaseConstants.MULTI_POST_JOB), "true");
        // repository
        Assert.assertTrue(backupTask.getRepositories().size() > 1);
    }

    /**
     * 用例场景：通用数据库设置备份参数
     * 前置条件：无
     * 检查点：如果资源版本不匹配，则抛错
     */
    @Test
    public void throw_exception_when_version_not_match() {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        backupTask.setProtectEnv(new TaskEnvironment());
        backupTask.setRepositories(new ArrayList<>(Collections.singletonList(new StorageRepository())));
        TaskResource protectObject = new TaskResource();
        protectObject.setUuid("testUuid");
        protectObject.setVersion("1.0");
        protectObject.setExtendInfo(new HashMap<>());
        protectObject.getExtendInfo().put(GeneralDbConstant.EXTEND_SCRIPT_CONF, TestConfHelper.getHanaConf());
        backupTask.setProtectObject(protectObject);

        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> generalDbBackupInterceptor.initialize(backupTask));
        Assert.assertEquals(exception.getErrorCode(), GeneralDbErrorCode.VERSION_DO_NOT_SUPPORT_BACKUP);
    }
}
