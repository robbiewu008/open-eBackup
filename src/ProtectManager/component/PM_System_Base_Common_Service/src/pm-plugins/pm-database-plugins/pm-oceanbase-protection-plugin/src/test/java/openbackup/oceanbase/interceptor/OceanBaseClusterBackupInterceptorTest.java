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
package openbackup.oceanbase.interceptor;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.oceanbase.provider.OceanBaseAgentProvider;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-25
 */
@RunWith(PowerMockRunner.class)
public class OceanBaseClusterBackupInterceptorTest {
    private OceanBaseClusterBackupInterceptor backupInterceptor;

    private OceanBaseService oceanBaseService;

    private OceanBaseAgentProvider oceanBaseAgentProvider;

    private DeployTypeService deployTypeService;

    @Before
    public void init() {
        oceanBaseService = Mockito.mock(OceanBaseService.class);
        deployTypeService = Mockito.mock(DeployTypeService.class);
        oceanBaseAgentProvider = new OceanBaseAgentProvider(oceanBaseService);
        backupInterceptor = new OceanBaseClusterBackupInterceptor(oceanBaseService, oceanBaseAgentProvider,
            deployTypeService);
    }

    /**
     * 用例场景：OB 下发备份任务 applicable 校验
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void check_applicable_success() {
        boolean applicable = backupInterceptor.applicable(ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType());
        Assert.assertTrue(applicable);
    }

    /**
     * 用例场景：初始化仓库成功
     * 前置条件：OceanProtect服务正常
     * 检查点：检查仓库类型
     */
    @Test
    public void check_supply_backup_task_success() {
        BackupTask backupTask = new BackupTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(OBConstants.KEY_CLUSTER_INFO,
            "{\"obServerAgents\":[{\"parentUuid\":\"111-222-111\"},{\"parentUuid\":\"111-222-222\"},{\"parentUuid\":\"111-111-111\"}],\"obClientAgents\":[{\"parentUuid\":\"111-111-111\"},{\"parentUuid\":\"111-111-222\"}]}");
        taskEnvironment.setExtendInfo(extendInfo);
        backupTask.setProtectEnv(taskEnvironment);
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setType(1);
        backupTask.setRepositories(Lists.newArrayList(storageRepository));

        backupInterceptor.supplyBackupTask(backupTask);
        List<StorageRepository> list = backupTask.getRepositories();

        Assert.assertEquals(backupTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.DISTRIBUTED.getType());
        Assert.assertEquals(list.size(), 3);
        Assert.assertEquals(list.get(0).getType().intValue(), 1);
        Assert.assertEquals(list.get(1).getType().intValue(), 3);
        Assert.assertEquals(list.get(2).getType().intValue(), 2);
    }

    @Test
    public void isSupportDataAndLogParallelBackup() {
        Assert.assertTrue(backupInterceptor.isSupportDataAndLogParallelBackup(new ProtectedResource()));
    }
}
