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

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.UUID;

/**
 * mysql集群实例恢复任务下发provider test
 *
 */
public class MysqlClusterInstanceRestoreProviderTest {
    private final MysqlBaseService mysqlBaseService = mock(MysqlBaseService.class);

    private final CopyRestApi copyRestApi = mock(CopyRestApi.class);

    private final ResourceService resourceService = mock(ResourceService.class);

    private final MysqlClusterInstanceRestoreProvider mysqlRestoreInterceptor = new MysqlClusterInstanceRestoreProvider(
        mysqlBaseService, resourceService, copyRestApi);

    /**
     * 用例场景：下发集群实例恢复任务
     * 前置条件：1. 资源是集群实例
     * 检 查 点：1. 恢复参数是否正确
     */
    @Test
    public void cluster_instance_restore_intercept_success() {
        RestoreTask task = MysqlBaseMock.getDatabaseRestoreTask(ResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE);
        ProtectedResource protectedResource = mockGetSingleRes(task);
        MysqlBaseMock.mockGetAgentBySingleInstanceRes(protectedResource, mysqlBaseService);
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(MysqlBaseMock.getCopy());
        PowerMockito.when(mysqlBaseService.getResource(any())).thenReturn(protectedResource);
        PowerMockito.when(mysqlBaseService.supplyExtendInfo(any(), any())).thenReturn(new HashMap<>());
        PowerMockito.when(mysqlBaseService.checkMySQLDatabaseNameInvalidCharacters(any())).thenReturn(false);
        PowerMockito.doNothing().when(mysqlBaseService).checkSubType(any(), any());
        PowerMockito.doNothing().when(mysqlBaseService).checkVersion(any(), any());
        PowerMockito.doNothing().when(mysqlBaseService).checkNewDatabaseName(any());
        PowerMockito.doNothing().when(mysqlBaseService).checkClusterType(any(), any());
        mysqlRestoreInterceptor.supplyRestoreTask(task);
        Assert.assertEquals(task.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.AP.getType());
    }

    @Test
    public void ap_cluster_instance_intercept_success(){
        RestoreTask task = MysqlBaseMock.getDatabaseRestoreTask(ResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE);
        task.setCopyId("testCopyId");
        ProtectedResource protectedResource = mockGetSingleRes(task);
        MysqlBaseMock.mockGetAgentBySingleInstanceRes(protectedResource, mysqlBaseService);
        Copy copy = MysqlBaseMock.getCopy();
        copy.setUuid("CopyId");
        copy.setResourceId(task.getTargetObject().getUuid());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        PowerMockito.when(mysqlBaseService.getResource(any())).thenReturn(protectedResource);
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.AP);
        PowerMockito.when(mysqlBaseService.supplyExtendInfo(any(), any())).thenReturn(extendInfo);
        PowerMockito.when(mysqlBaseService.checkMySQLDatabaseNameInvalidCharacters(any())).thenReturn(false);
        PowerMockito.doNothing().when(mysqlBaseService).checkSubType(any(), any());
        PowerMockito.doNothing().when(mysqlBaseService).checkVersion(any(), any());
        PowerMockito.doNothing().when(mysqlBaseService).checkNewDatabaseName(any());
        PowerMockito.doNothing().when(mysqlBaseService).checkClusterType(any(), any());
        mysqlRestoreInterceptor.supplyRestoreTask(task);
    }

    private ProtectedResource mockGetSingleRes(RestoreTask task) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(UUID.randomUUID().toString());
        protectedResource.setVersion("111");
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(MysqlConstants.MYSQL_SERVICE_NAME, "mysql.service");
        extendInfo.put(MysqlConstants.MYSQL_SYSTEM_SERVICE_TYPE, "systemctl");
        protectedResource.setExtendInfo(extendInfo);
        PowerMockito.when(mysqlBaseService.getSingleInstanceByClusterInstance(task.getTargetObject().getUuid()))
            .thenReturn(Arrays.asList(protectedResource));
        return protectedResource;
    }

    /**
     * 用例场景：恢复的后置操作里寻找需要设置下一次备份为全量备份的资源
     * 前置条件：1. 无
     * 检 查 点：1. 需要设置下一次备份为全量备份的资源符合预期
     */
    @Test
    public void find_associated_resources_to_set_next_full_success() {
        RestoreTask task = MysqlBaseMock.getDatabaseRestoreTask(ResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE);
        ProtectedResource protectedResource = mockGetSingleRes(task);
        Set<String> databases = new HashSet<>();
        databases.add(UUID.randomUUID().toString());
        PowerMockito.when(resourceService.queryRelatedResourceUuids(protectedResource.getUuid(), new String[] {}))
            .thenReturn(databases);
        final List<String> associatedResources = mysqlRestoreInterceptor.findAssociatedResourcesToSetNextFull(task);
        Assert.assertEquals(associatedResources.size(), databases.size() + 1);
        Assert.assertEquals(associatedResources.get(0), databases.toArray()[0]);
        Assert.assertEquals(associatedResources.get(1), task.getTargetObject().getUuid());
    }
}
