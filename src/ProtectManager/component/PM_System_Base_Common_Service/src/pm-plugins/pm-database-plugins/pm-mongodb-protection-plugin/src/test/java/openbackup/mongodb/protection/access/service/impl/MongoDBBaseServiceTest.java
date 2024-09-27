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
package openbackup.mongodb.protection.access.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mongodb.protection.access.constants.MongoDBConstants;
import openbackup.mongodb.protection.access.mock.MongoDBMockBean;

import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * mongodb 实际业务service 测试类
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */
public class MongoDBBaseServiceTest {
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private final ProtectedEnvironmentService protectedEnvironmentService = Mockito.mock(
        ProtectedEnvironmentService.class);

    private final MongoDBMockBean mongoDBMockBean = new MongoDBMockBean();

    private final MongoDBBaseServiceImpl mongoDBBaseService = new MongoDBBaseServiceImpl(resourceService,
        agentUnifiedService, protectedEnvironmentService);

    /**
     * 用例场景：MongoDB 根据资源id获取对象成功和失败场景
     * 前置条件：无
     * 检查点：正常通过和正常报错
     */
    @Test
    public void check_get_resource_by_id_success() {
        given(resourceService.getResourceById(eq("uuid"))).willReturn(Optional.of(new ProtectedResource()));
        mongoDBBaseService.getResource("uuid");
        given(resourceService.getResourceById(eq("uuid"))).willReturn(Optional.ofNullable(null));
        Assert.assertThrows(LegoCheckedException.class, () -> mongoDBBaseService.getResource("uuid"));
    }

    /**
     * 用例场景：MongoDB 根据环境id获取对象成功
     * 前置条件：无
     * 检查点：正常通过
     */
    @Test
    public void check_get_environment_by_id_success() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setName("uuid");
        given(protectedEnvironmentService.getEnvironmentById(eq("uuid"))).willReturn(protectedEnvironment);
        ProtectedEnvironment environment = mongoDBBaseService.getEnvironmentById("uuid");
        Assert.assertEquals("uuid", environment.getName());
    }

    /**
     * 用例场景：MongoDB 根据环境对象和资源对象获取Agent返回对象成功
     * 前置条件：无
     * 检查点：正常通过
     */
    @Test
    public void check_get_app_env_agent_info_success() {
        given(agentUnifiedService.getClusterInfo(any(), any())).willReturn(new AppEnvResponse());
        AppEnvResponse appEnvAgentInfo = mongoDBBaseService.getAppEnvAgentInfo(new ProtectedResource(), new ProtectedEnvironment());
        Assert.assertNull(appEnvAgentInfo.getName());
    }

    /**
     * 用例场景：MongoDB 更新数据库成功
     * 前置条件：无
     * 检查点：正常通过
     */
    @Test
    public void check_update_resource_service_success() {
        mongoDBBaseService.updateResourceService(new ProtectedEnvironment());
        Assert.assertNotNull(mongoDBBaseService);
    }

    /**
     * 用例场景：MongoDB 根据环境对象和资源对象获取Agent返回对象成功
     * 前置条件：无
     * 检查点：正常通过
     */
    @Test
    public void check_check_mongodb_node_size_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType("MongoDB");
        protectedResource.setType("database");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(MongoDBConstants.SERVICE_IP, "192.168.100.10");
        extendInfo.put(MongoDBConstants.SERVICE_PORT, "25080");
        protectedResource.setExtendInfo(extendInfo);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(IsmNumberConstant.ONE);
        HashMap<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.RESOURCE_TYPE, "database");
        filter.put(DatabaseConstants.SUB_TYPE, "MongoDB");
        filter.put(MongoDBConstants.SERVICE_IP, "192.168.100.10");
        filter.put(MongoDBConstants.SERVICE_PORT, "25080");
        given(resourceService.query(IsmNumberConstant.ZERO, IsmNumberConstant.ONE, filter)).willReturn(
            pageListResponse);
        mongoDBBaseService.checkMongoDBEnvironmentSize(protectedResource, false);
        Assert.assertThrows(LegoCheckedException.class,
            () -> mongoDBBaseService.checkMongoDBEnvironmentSize(protectedResource, true));
    }

    /**
     * 用例场景：MongoDB 更新数据库成功
     * 前置条件：无
     * 检查点：正常通过
     */
    @Test
    public void check_build_backup_task_nodes_success() {
        ProtectedResource protectedResource = mongoDBMockBean.getMongoDBProtectedEnvironment();
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedResource protectedResource1 = new ProtectedResource();
        protectedResource1.setExtendInfo(new HashMap<>());
        List<ProtectedResource> listA = new ArrayList<>();
        listA.add(protectedResource1);
        protectedResource1.setDependencies(new HashMap<>());
        protectedResource1.getDependencies().put(DatabaseConstants.AGENTS, listA);
        list.add(protectedResource1);
        protectedResource.getDependencies().put(DatabaseConstants.CHILDREN, list);
        given(resourceService.getResourceById(eq("uuid"))).willReturn(Optional.of(protectedResource));
        List<TaskEnvironment> taskEnvironments = mongoDBBaseService.buildBackupTaskNodes("uuid");
        Assert.assertEquals(taskEnvironments.size(), listA.size());
    }

    /**
     * 用例场景：MongoDB 根据资源id获取当前锁情况
     * 前置条件：无
     * 检查点：正常通过
     */
    @Test
    public void get_restore_lock_resource_success() {
        given(resourceService.queryRelatedResourceUuids("uuid", new String[0])).willReturn(new HashSet<>());
        List<LockResourceBo> lockResources = mongoDBBaseService.getRestoreLockResource("uuid");
        Assert.assertEquals(lockResources.size(), 1);
    }
}
