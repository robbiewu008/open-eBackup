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
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * mysql数据库恢复任务下发provider test
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/6/23
 */
public class MysqlDatabaseRestoreProviderTest {
    private static final String MYSQL_INSTANCE_UUID = "mysqlInstanceUuid";

    private final MysqlBaseService mysqlBaseService = mock(MysqlBaseService.class);

    private final CopyRestApi copyRestApi = mock(CopyRestApi.class);

    private final EncryptorService encryptorService = mock(EncryptorService.class);

    private final MysqlDatabaseRestoreProvider mysqlRestoreInterceptor = new MysqlDatabaseRestoreProvider(
        mysqlBaseService, copyRestApi, encryptorService);

    /**
     * 用例场景：下发单实例数据库恢复任务
     * 前置条件：1. 资源是单实例数据库
     * 检 查 点：1. 恢复参数是否正确
     */
    @Test
    public void single_instance_database_restore_intercept_success() {
        RestoreTask task = MysqlBaseMock.getDatabaseRestoreTask(ResourceSubTypeEnum.MYSQL_DATABASE);
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(MysqlBaseMock.getCopy());
        final ProtectedResource resource = MysqlBaseMock.mockGetGrantParentResource(task, "0", mysqlBaseService);
        MysqlBaseMock.mockGetAgentBySingleInstanceRes(resource, mysqlBaseService);
        final ProtectedResource protectedResource = new ProtectedResource();
        Map<String, String> map = new HashMap<>();
        map.put(DatabaseConstants.DATA_DIR, "111");
        protectedResource.setUuid(UUID.randomUUID().toString());
        protectedResource.setExtendInfo(map);
        PowerMockito.when(mysqlBaseService.getResource(task.getTargetObject().getUuid())).thenReturn(protectedResource);
        PowerMockito.when(mysqlBaseService.supplyExtendInfo(any(), any())).thenReturn(new HashMap<>());
        PowerMockito.doNothing().when(mysqlBaseService).checkSubType(any(),any());
        PowerMockito.doNothing().when(mysqlBaseService).checkVersion(any(),any());
        PowerMockito.doNothing().when(mysqlBaseService).checkNewDatabaseName(any());
        PowerMockito.doNothing().when(mysqlBaseService).checkClusterType(any(),any());
        PowerMockito.when(encryptorService.decrypt(any())).thenReturn("xxxx");
        RestoreTask restoreTask = mysqlRestoreInterceptor.supplyRestoreTask(task);
        Assert.assertEquals(restoreTask.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.SINGLE.getType());
        Assert.assertEquals(restoreTask.getTargetObject().getExtendInfo().get(DatabaseConstants.DATA_DIR), "xxxx");
        Assert.assertEquals(restoreTask.getAdvanceParams().get(MYSQL_INSTANCE_UUID), resource.getUuid());
    }

    /**
     * 用例场景：下发集群实例数据库恢复任务
     * 前置条件：1. 资源是集群实例数据库
     * 检 查 点：1. 恢复参数是否正确
     */
    @Test
    public void cluster_instance_database_restore_intercept_success() {
        RestoreTask task = MysqlBaseMock.getDatabaseRestoreTask(ResourceSubTypeEnum.MYSQL_DATABASE);
        ProtectedResource singleInstanceRes = MysqlBaseMock.mockGetGrantParentResource(task, "1", mysqlBaseService);
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(MysqlBaseMock.getCopy());
        PowerMockito.when(mysqlBaseService.getSingleInstanceByClusterInstance(singleInstanceRes.getParentUuid())).thenReturn(Arrays.asList(singleInstanceRes));
        MysqlBaseMock.mockGetAgentBySingleInstanceRes(singleInstanceRes, mysqlBaseService);
        PowerMockito.when(mysqlBaseService.getResource(task.getTargetObject().getUuid())).thenReturn(new ProtectedResource());
        PowerMockito.when(mysqlBaseService.supplyExtendInfo(any(), any())).thenReturn(new HashMap<>());
        PowerMockito.doNothing().when(mysqlBaseService).checkSubType(any(),any());
        PowerMockito.doNothing().when(mysqlBaseService).checkVersion(any(),any());
        PowerMockito.doNothing().when(mysqlBaseService).checkNewDatabaseName(any());
        PowerMockito.doNothing().when(mysqlBaseService).checkClusterType(any(),any());
        final RestoreTask supplyRestoreTask = mysqlRestoreInterceptor.supplyRestoreTask(task);
        Assert.assertEquals(singleInstanceRes.getExtendInfo().get(DatabaseConstants.CLUSTER_TYPE),
                supplyRestoreTask.getTargetObject().getExtendInfo().get(DatabaseConstants.CLUSTER_TYPE));
        Assert.assertEquals(supplyRestoreTask.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE),
            DatabaseDeployTypeEnum.AP.getType());
        Assert.assertEquals(supplyRestoreTask.getAdvanceParams().get(MYSQL_INSTANCE_UUID), singleInstanceRes.getParentUuid());
    }

    /**
     * 用例场景：恢复的后置操作里寻找需要设置下一次备份为全量备份的资源
     * 前置条件：1. 无
     * 检 查 点：1. 需要设置下一次备份为全量备份的资源符合预期
     */
    @Test
    public void find_associated_resources_to_set_next_full_success() {
        RestoreTask task = MysqlBaseMock.getDatabaseRestoreTask(ResourceSubTypeEnum.MYSQL_DATABASE);
        final Map<String, String> params = new HashMap<>();
        params.put(MYSQL_INSTANCE_UUID, UUID.randomUUID().toString());
        task.setAdvanceParams(params);
        final List<String> associatedResources = mysqlRestoreInterceptor.findAssociatedResourcesToSetNextFull(task);
        Assert.assertEquals(associatedResources.size(), 2);
        Assert.assertEquals(associatedResources.get(0), task.getTargetObject().getUuid());
        Assert.assertEquals(associatedResources.get(1), params.get(MYSQL_INSTANCE_UUID));
    }
}
