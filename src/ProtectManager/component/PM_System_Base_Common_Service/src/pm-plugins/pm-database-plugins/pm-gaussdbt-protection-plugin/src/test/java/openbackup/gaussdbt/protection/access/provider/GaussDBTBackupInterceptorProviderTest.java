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

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * GaussDBT备份拦截器Provider测试类
 *
 */
public class GaussDBTBackupInterceptorProviderTest {
    private final GaussDBTBackupInterceptorProvider gaussDBTBackupInterceptorProvider
        = new GaussDBTBackupInterceptorProvider();

    /**
     * 用例场景：框架调用backup intercept.
     * 前置条件：backup task填写task info正确。
     * 检查点：填写正确
     */
    @Test
    public void gauss_dbt_backup_supply_backup_task_success() {
        BackupTask backupTask = new BackupTask();
        StorageRepository dataStorageRepository = new StorageRepository();
        dataStorageRepository.setType(RepositoryTypeEnum.DATA.getType());
        backupTask.addRepository(dataStorageRepository);
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.AGENTS, "test_uuid");
        taskEnvironment.setExtendInfo(extendInfo);
        backupTask.setProtectEnv(taskEnvironment);
        gaussDBTBackupInterceptorProvider.supplyBackupTask(backupTask);
        Assert.assertEquals(3, backupTask.getRepositories().size());
        Assert.assertEquals("false", backupTask.getAdvanceParams().get(GaussDBTConstant.MULTI_FILE_SYSTEM_KEY));
        Assert.assertEquals(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat(), backupTask.getCopyFormat());
        Assert.assertEquals("true", backupTask.getAdvanceParams().get(DatabaseConstants.MULTI_POST_JOB));
    }

    /**
     * 用例场景：框架调用backup intercept.
     * 前置条件：backup task填写supply nodes正确。
     * 检查点：填写正确
     */
    @Test
    public void gauss_dbt_backup_supply_nodes_success() {
        BackupTask backupTask = new BackupTask();
        TaskEnvironment environment = new TaskEnvironment();
        environment.setName("node01");
        List<TaskEnvironment> environments = Collections.singletonList(environment);
        TaskEnvironment environment1 = new TaskEnvironment();
        environment1.setNodes(environments);
        backupTask.setProtectEnv(environment1);
        TaskResource taskResource = new TaskResource();
        Map<String, String> map = new HashMap<>();
        String nodeString = "[{\"uuid\":\"123456\",\"name\":\"node01\",\"extendInfo\":{\"role\":\"1\"}}]";
        map.put(GaussDBTConstant.NODES_KEY, nodeString);
        taskResource.setExtendInfo(map);
        backupTask.setProtectObject(taskResource);
        gaussDBTBackupInterceptorProvider.supplyNodes(backupTask);
        Assert.assertEquals(1, backupTask.getProtectEnv().getNodes().size());
    }

    /**
     * 用例场景：框架调用applicable
     * 前置条件：类型填写正确
     * 检查点：填写正确
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(gaussDBTBackupInterceptorProvider.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
    }
}