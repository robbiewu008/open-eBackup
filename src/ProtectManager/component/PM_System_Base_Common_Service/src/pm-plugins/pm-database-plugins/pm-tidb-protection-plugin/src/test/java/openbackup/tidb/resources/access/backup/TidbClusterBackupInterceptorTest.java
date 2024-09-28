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
package openbackup.tidb.resources.access.backup;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;
import openbackup.tidb.resources.access.util.TidbUtil;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Maps;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * TidbClusterBackupInterceptorTest
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(TidbUtil.class)
public class TidbClusterBackupInterceptorTest {
    @Mock
    private TidbService tidbService;

    @Mock
    private ResourceService resourceService;

    private TidbClusterBackupInterceptor tidbClusterBackupInterceptor;

    @Mock
    private DefaultProtectAgentSelector defaultSelector;

    @Before
    public void setUp() {
        tidbClusterBackupInterceptor = new TidbClusterBackupInterceptor(tidbService, resourceService,
            new TidbAgentProvider(tidbService), defaultSelector);
    }

    @Test
    public void applicable_test() {
        Assert.assertTrue(tidbClusterBackupInterceptor.applicable(ResourceSubTypeEnum.TIDB_CLUSTER.getType()));
    }

    @Test
    public void checkConnention_success() {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        TaskResource protectObject = new TaskResource();
        protectObject.setVersion("6.2.0");
        backupTask.setProtectObject(protectObject);
        tidbClusterBackupInterceptor.checkConnention(backupTask);
        Assert.assertTrue(true);
    }

    @Test
    public void checkConnention_error() {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        TaskResource protectObject = new TaskResource();
        protectObject.setVersion("5.2.0");
        protectObject.setExtendInfo(ImmutableMap.of(DatabaseConstants.VERSION, "5.2.0"));
        backupTask.setProtectObject(protectObject);
        Assert.assertThrows(LegoCheckedException.class, () -> tidbClusterBackupInterceptor.checkConnention(backupTask));
    }

    @Test
    public void supplyBackupTask_test() throws Exception {
        // PowerMockito.doNothing().when(tidbService).checkResourceStatus(any());
        BackupTask backupTask = new BackupTask();
        backupTask.setCopyFormat(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());

        TaskEnvironment taskEnvironment = new TaskEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        taskEnvironment.setExtendInfo(extendInfo);
        backupTask.setProtectEnv(taskEnvironment);

        List<StorageRepository> repositoryList = new ArrayList<>();
        StorageRepository storageRepository = new StorageRepository();
        repositoryList.add(storageRepository);
        backupTask.setRepositories(repositoryList);
        TaskResource protectObject = new TaskResource();
        protectObject.setUuid("1");
        protectObject.setRootUuid("1");
        protectObject.setExtendInfo(Maps.newHashMap());
        backupTask.setProtectObject(protectObject);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.anyString()))
            .thenReturn(Optional.of(protectedEnvironment));
        PowerMockito.mockStatic(TidbUtil.class);
        PowerMockito.doNothing().when(TidbUtil.class, "setTiupUuid", any(), anyString(), any(), any(), any());
        tidbClusterBackupInterceptor.supplyBackupTask(backupTask);
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);

        tidbClusterBackupInterceptor.supplyBackupTask(backupTask);
    }

    @Test
    public void isSupportDataAndLogParallelBackup() {
        Assert.assertTrue(tidbClusterBackupInterceptor.isSupportDataAndLogParallelBackup(new ProtectedResource()));
    }
}
