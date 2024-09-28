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
package openbackup.db2.protection.access.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.service.Db2InstanceService;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * {@link Db2TablespaceServiceImpl} 测试类
 *
 */
public class Db2TablespaceServiceImplTest {
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final AgentUnifiedService agentService = PowerMockito.mock(AgentUnifiedService.class);

    private final Db2InstanceService db2instanceService = PowerMockito.mock(Db2InstanceService.class);

    private Db2TablespaceServiceImpl db2TablespaceServiceImpl = new Db2TablespaceServiceImpl(resourceService,
        agentService, db2instanceService);

    /**
     * 用例场景：设置表空间锁定状态
     * 前置条件：表空间被注册
     * 检查点：返回锁定状态为true
     */
    @Test
    public void execute_set_tablespace_locked_Status_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockPagedResources());
        PageListResponse<ProtectedResource> tablespace = mockTablespace();
        db2TablespaceServiceImpl.setTablespaceLockedStatus(UUIDGenerator.getUUID(), tablespace);
        Assert.assertEquals("true",
            tablespace.getRecords().get(0).getExtendInfoByKey(DatabaseConstants.EXTEND_INFO_KEY_IS_LOCKED));
    }

    /**
     * 用例场景：查询集群环境下的表空间
     * 前置条件：参数及服务正常
     * 检查点：返回表空间数正确
     */
    @Test
    public void query_cluster_tablespace_success() {
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockClusterResource());
        PowerMockito.when(agentService.getDetailPageList(any(), any(), any(), any())).thenReturn(mockTablespace());
        PageListResponse<ProtectedResource> response = db2TablespaceServiceImpl.queryClusterTablespace(
            mockEnv(ResourceSubTypeEnum.DB2_CLUSTER.getType()), mockConditions());
        Assert.assertEquals(1, response.getTotalCount());
    }

    /**
     * 用例场景：查询单机环境下的表空间
     * 前置条件：参数及服务正常
     * 检查点：返回表空间数正确
     */
    @Test
    public void query_single_tablespace_success() {
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockSingleResource());
        PowerMockito.when(agentService.getDetailPageList(any(), any(), any(), any())).thenReturn(mockTablespace());
        PageListResponse<ProtectedResource> response = db2TablespaceServiceImpl.querySingleTablespace(
            mockEnv(ResourceSubTypeEnum.U_BACKUP_AGENT.getType()), mockConditions());
        Assert.assertEquals(1, response.getTotalCount());
    }

    private PageListResponse<ProtectedResource> mockPagedResources() {
        ProtectedResource tablespace = new ProtectedResource();
        tablespace.setExtendInfoByKey(DatabaseConstants.TABLESPACE_KEY, "test1");
        List<ProtectedResource> list = new ArrayList<>();
        list.add(tablespace);
        PageListResponse<ProtectedResource> data = new PageListResponse<>();
        data.setRecords(list);
        return data;
    }

    private PageListResponse<ProtectedResource> mockTablespace() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        ProtectedResource tablespace = new ProtectedResource();
        tablespace.setName("test1");
        List<ProtectedResource> list = new ArrayList<>();
        list.add(tablespace);
        response.setRecords(list);
        return response;
    }

    private ProtectedEnvironment mockEnv(String subType) {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(UUID.randomUUID().toString());
        environment.setSubType(subType);
        return environment;
    }

    private BrowseEnvironmentResourceConditions mockConditions() {
        BrowseEnvironmentResourceConditions conditions = new BrowseEnvironmentResourceConditions();
        conditions.setPageNo(0);
        conditions.setPageSize(100);
        conditions.setResourceType(ResourceSubTypeEnum.DB2_TABLESPACE.getType());
        return conditions;
    }

    private Optional<ProtectedResource> mockSingleResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(UUID.randomUUID().toString());
        resource.setName("testDB");
        resource.setParentName("db2inst");
        Authentication auth = new Authentication();
        auth.setAuthKey("db2inst1");
        auth.setAuthPwd("db2inst1");
        resource.setAuth(auth);
        return Optional.ofNullable(resource);
    }

    private Optional<ProtectedResource> mockClusterResource() {
        return Optional.ofNullable(queryClusterInstance().getRecords().get(0));
    }

    private PageListResponse<ProtectedResource> queryClusterInstance() {
        PageListResponse<ProtectedResource> result = new PageListResponse<>();
        result.setTotalCount(1);
        ProtectedResource clusterInstance = new ProtectedResource();
        clusterInstance.setUuid(UUID.randomUUID().toString());
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setEndpoint("127.0.0.1");
        agent.setPort(50000);
        agent.setExtendInfoByKey(ResourceConstants.AGENT_IP_LIST, "127.0.0.1");
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(agent);
        Map<String, List<ProtectedResource>> agentMap = new HashMap<>();
        agentMap.put(DatabaseConstants.AGENTS, agents);
        ProtectedResource instance = new ProtectedResource();
        instance.setDependencies(agentMap);
        List<ProtectedResource> children = new ArrayList<>();
        children.add(instance);
        Map<String, List<ProtectedResource>> childrenMap = new HashMap<>();
        childrenMap.put(DatabaseConstants.CHILDREN, children);
        clusterInstance.setDependencies(childrenMap);
        clusterInstance.setExtendInfoByKey(Db2Constants.CATALOG_IP_KEY, "127.0.0.1");
        result.setRecords(Collections.singletonList(clusterInstance));
        return result;
    }
}
