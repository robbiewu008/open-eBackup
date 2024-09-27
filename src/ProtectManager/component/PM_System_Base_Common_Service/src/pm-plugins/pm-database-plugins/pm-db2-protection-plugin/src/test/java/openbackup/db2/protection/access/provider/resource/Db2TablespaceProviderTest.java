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
package openbackup.db2.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2TablespaceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.UUID;

/**
 * {@link Db2TablespaceProvider} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-06
 */
public class Db2TablespaceProviderTest {
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);

    private final Db2TablespaceService db2TablespaceService = PowerMockito.mock(Db2TablespaceService.class);

    private final Db2TablespaceProvider db2TablespaceProvider = new Db2TablespaceProvider(resourceService,
        environmentService, db2TablespaceService);

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源
     * 检查点：是否返回true
     */
    @Test
    public void applicable_db2_tablespace_provider_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.DB2_TABLESPACE.getType());
        Assert.assertTrue(db2TablespaceProvider.applicable(resource));
        resource.setSubType(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType());
        Assert.assertFalse(db2TablespaceProvider.applicable(resource));
    }

    /**
     * 用例场景：创建表空间集前检查
     * 前置条件：输入表空间集信息
     * 检查点：无异常抛出
     */
    @Test
    public void execute_before_create_check_when_create_db2_tablespace() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockPagedResources());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockEnvResource());
        PowerMockito.when(environmentService.getEnvironmentById(any())).thenReturn(mockEnv());
        PowerMockito.when(db2TablespaceService.queryClusterTablespace(any(), any())).thenReturn(mockTablespace());
        db2TablespaceProvider.beforeCreate(mockTablespaceResource());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：创建表空间集前检查
     * 前置条件：表空间不存在
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_tablespace_is_not_exist_when_create_db2_tablespace() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockPagedResources());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockEnvResource());
        PowerMockito.when(environmentService.getEnvironmentById(any())).thenReturn(mockEnv());
        PowerMockito.when(db2TablespaceService.queryClusterTablespace(any(), any())).thenReturn(mockTablespace());
        ProtectedResource tablespaceResource = mockTablespaceResource();
        tablespaceResource.setExtendInfoByKey(DatabaseConstants.TABLESPACE_KEY, "test2");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2TablespaceProvider.beforeCreate(tablespaceResource));
        Assert.assertEquals(CommonErrorCode.OBJ_NOT_EXIST, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：更新表空间集前检查
     * 前置条件：输入表空间集信息
     * 检查点：无异常抛出
     */
    @Test
    public void execute_before_update_check_when_update_db2_tablespace() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockPagedResources());
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(Optional.ofNullable(mockTablespaceResource()));
        PowerMockito.when(environmentService.getEnvironmentById(any())).thenReturn(mockEnv());
        PowerMockito.when(db2TablespaceService.queryClusterTablespace(any(), any())).thenReturn(mockTablespace());
        db2TablespaceProvider.beforeUpdate(mockTablespaceResource());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：更新表空间集时被注册
     * 前置条件：表空间被注册
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_tablespace_registered_when_before_create() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockRegisteredResources());
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(Optional.ofNullable(mockOldTablespace()));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2TablespaceProvider.beforeUpdate(mockTablespaceResource()));
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：创建表空间集时表空间为空
     * 前置条件：表空间为空
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_tablespace_is_empty_when_before_create() {
        ProtectedResource tablespaceResource = mockTablespaceResource();
        tablespaceResource.setExtendInfoByKey(DatabaseConstants.TABLESPACE_KEY, "");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2TablespaceProvider.beforeCreate(tablespaceResource));
        Assert.assertEquals(CommonErrorCode.OBJ_NOT_EXIST, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：创建表空间集时选择了hadr集群
     * 前置条件：选择了hadr集群
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_cluster_type_is_hadr_when_before_create() {
        ProtectedResource tablespaceResource = mockTablespaceResource();
        tablespaceResource.getEnvironment()
            .setExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.HADR.getType());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2TablespaceProvider.beforeCreate(tablespaceResource));
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    private ProtectedResource mockTablespaceResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setExtendInfoByKey(DatabaseConstants.TABLESPACE_KEY, "test,test1");
        resource.setEnvironment(mockEnv());
        return resource;
    }

    private PageListResponse<ProtectedResource> mockPagedResources() {
        List<ProtectedResource> list = new ArrayList<>();
        PageListResponse<ProtectedResource> data = new PageListResponse<>();
        data.setRecords(list);
        return data;
    }

    private Optional<ProtectedResource> mockEnvResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setPath("127.0.0.1");
        return Optional.ofNullable(resource);
    }

    private PageListResponse<ProtectedResource> mockRegisteredResources() {
        ProtectedResource resource = new ProtectedResource();
        resource.setExtendInfoByKey(DatabaseConstants.TABLESPACE_KEY, "test");
        List<ProtectedResource> list = new ArrayList<>();
        list.add(resource);
        PageListResponse<ProtectedResource> data = new PageListResponse<>();
        data.setRecords(list);
        return data;
    }

    private ProtectedResource mockOldTablespace() {
        ProtectedResource resource = new ProtectedResource();
        resource.setExtendInfoByKey(DatabaseConstants.TABLESPACE_KEY, "test1");
        return resource;
    }

    private ProtectedEnvironment mockEnv() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(UUID.randomUUID().toString());
        environment.setEndpoint("127.0.0.1");
        environment.setSubType(ResourceSubTypeEnum.DB2_CLUSTER.getType());
        return environment;
    }

    private PageListResponse<ProtectedResource> mockTablespace() {
        ProtectedResource tablespaceOne = new ProtectedResource();
        tablespaceOne.setName("test");
        ProtectedResource tablespaceTwo = new ProtectedResource();
        tablespaceTwo.setName("test1");
        List<ProtectedResource> list = new ArrayList<>();
        list.add(tablespaceOne);
        list.add(tablespaceTwo);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        response.setRecords(list);
        return response;
    }
}