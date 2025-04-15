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

import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * TidbTableBackupInterceptorTest
 *
 */
@RunWith(PowerMockRunner.class)
public class TidbTableBackupInterceptorTest {
    @Mock
    private TidbService tidbService;

    @Mock
    private ResourceService resourceService;

    private TidbTableBackupInterceptor tidbTableBackupInterceptor;

    @Mock
    private DefaultProtectAgentSelector defaultSelector;

    @Before
    public void setUp() {
        tidbTableBackupInterceptor = new TidbTableBackupInterceptor(tidbService, resourceService,
            new TidbAgentProvider(tidbService), defaultSelector);
    }

    @Test
    public void applicable_test() {
        Assert.assertTrue(tidbTableBackupInterceptor.applicable(ResourceSubTypeEnum.TIDB_TABLE.getType()));
    }

    @Test
    public void supplyAgent_test() {
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

        TaskResource taskResource = new TaskResource();
        taskResource.setParentUuid("123123");
        backupTask.setProtectObject(taskResource);

        ProtectedResource dbResource = new ProtectedResource();
        dbResource.setParentUuid("123123");
        extendInfo.put(TidbConstants.DATABASE_NAME, "name");
        dbResource.setExtendInfo(extendInfo);
        taskResource.setExtendInfo(extendInfo);

        PowerMockito.when(tidbService.getResourceByCondition(anyString())).thenReturn(dbResource);
        tidbTableBackupInterceptor.supplyAgent(backupTask);
    }

    @Test
    public void isSupportDataAndLogParallelBackup() {
        Assert.assertTrue(tidbTableBackupInterceptor.isSupportDataAndLogParallelBackup(new ProtectedResource()));
    }
}