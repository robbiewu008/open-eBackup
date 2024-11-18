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
package openbackup.mysql.resources.access.interceptor;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.mock;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.UUID;

/**
 * mysql单实例恢复任务下发provider test
 *
 */
public class MysqlSingleInstanceRestoreProviderTest {
    private final MysqlBaseService mysqlBaseService = mock(MysqlBaseService.class);

    private final CopyRestApi copyRestApi = mock(CopyRestApi.class);

    private final EncryptorService encryptorService = mock(EncryptorService.class);

    private final ResourceService resourceService = mock(ResourceService.class);

    private final MysqlSingleInstanceRestoreProvider mysqlRestoreInterceptor = new MysqlSingleInstanceRestoreProvider(
        mysqlBaseService, copyRestApi, encryptorService, resourceService);

    /**
     * 用例场景：下发单实例恢复任务
     * 前置条件：1. 资源是单实例
     * 检 查 点：1. 恢复参数是否正确
     */
    @Test
    public void single_instance_restore_success() {
        RestoreTask task = MysqlBaseMock.getDatabaseRestoreTask(ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE);
        ProtectedResource resource = new ProtectedResource();
        resource.setVersion("111");
        PowerMockito.when(mysqlBaseService.getResource(task.getTargetObject().getUuid())).thenReturn(resource);

        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setUuid(UUID.randomUUID().toString());
        agentEnv.setEndpoint("8.40.99.101");
        agentEnv.setPort(2181);
        PowerMockito.when(mysqlBaseService.getEnvironmentById(task.getTargetObject().getParentUuid())).thenReturn(agentEnv);

        Endpoint endpoint = new Endpoint();
        endpoint.setIp(agentEnv.getEndpoint());
        endpoint.setPort(agentEnv.getPort());
        PowerMockito.when(mysqlBaseService.getResource(task.getTargetObject().getUuid()))
            .thenReturn(resource);
        PowerMockito.when(mysqlBaseService.supplyExtendInfo(any(), any())).thenReturn(task.getTargetObject().getExtendInfo());
        PowerMockito.when(mysqlBaseService.getAgentEndpoint(agentEnv)).thenReturn(endpoint);
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(MysqlBaseMock.getCopy());
        PowerMockito.doNothing().when(mysqlBaseService).checkSubType(any(),any());
        PowerMockito.doNothing().when(mysqlBaseService).checkVersion(any(),any());
        PowerMockito.doNothing().when(mysqlBaseService).checkNewDatabaseName(any());
        PowerMockito.doNothing().when(mysqlBaseService).checkClusterType(any(),any());
        PowerMockito.when(encryptorService.decrypt("123")).thenReturn("456");
        final RestoreTask restoreTask = mysqlRestoreInterceptor.supplyRestoreTask(task);
        Assert.assertEquals(restoreTask.getAgents().get(0).getIp(), agentEnv.getEndpoint());
        Assert.assertEquals(restoreTask.getAgents().get(0).getPort(), (int) agentEnv.getPort());
        Assert.assertEquals(restoreTask.getTargetObject().getExtendInfo().get(DatabaseConstants.DATA_DIR), "456");
    }

    /**
     * 用例场景：恢复的后置操作里寻找需要设置下一次备份为全量备份的资源
     * 前置条件：1. 无
     * 检 查 点：1. 需要设置下一次备份为全量备份的资源符合预期
     */
    @Test
    public void find_associated_resources_to_set_next_full_success() {
        RestoreTask task = MysqlBaseMock.getDatabaseRestoreTask(ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE);
        Set<String> databases = new HashSet<>();
        databases.add(UUID.randomUUID().toString());
        PowerMockito.when(resourceService.queryRelatedResourceUuids(task.getTargetObject().getUuid(), new String[]{})).thenReturn(databases);
        final List<String> associatedResources = mysqlRestoreInterceptor.findAssociatedResourcesToSetNextFull(task);
        Assert.assertEquals(associatedResources.size(), databases.size() + 1);
        Assert.assertEquals(associatedResources.get(0), databases.toArray()[0]);
        Assert.assertEquals(associatedResources.get(1), task.getTargetObject().getUuid());
    }
}