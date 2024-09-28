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
package openbackup.oracle.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class OracleClusterDatabaseProviderTest {
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
    private final OracleBaseService oracleBaseService = Mockito.mock(OracleBaseService.class);

    private final OracleClusterDatabaseProvider oracleClusterDatabaseProvider = new OracleClusterDatabaseProvider(
            resourceService, agentUnifiedService, oracleBaseService);

    /**
     * 用例场景：oracle集群环境检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.ORACLE_CLUSTER.getType());
        Assert.assertTrue(oracleClusterDatabaseProvider.applicable(protectedResource));
    }

    /**
     * 用例场景：健康检查
     * 前置条件：无
     * 检查点: 检查成功
     */
    @Test
    public void health_check_success() {
        ProtectedResource resource = mockClusterDatabase();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(mockAgent());
        environment.setDependencies(new HashMap<>());
        environment.getDependencies().put(DatabaseConstants.AGENTS, agents);
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(environment);
        Mockito.when(agentUnifiedService.checkApplication(any(),any())).thenReturn(agentBaseDto);
        oracleClusterDatabaseProvider.healthCheck(resource);
        Mockito.verify(oracleBaseService, Mockito.times(1)).getOracleClusterHosts(resource);
    }

    /**
     * 用例场景：
     * 前置条件：无
     * 检查点: 检查成功
     */
    @Test
    public void check_success_when_have_existed_oracle_res_and_no_repeat(){
        ProtectedResource resource = mockClusterDatabase();
        PageListResponse<ProtectedResource> existsResource = new PageListResponse<>();
        existsResource.setRecords(new ArrayList<>());

        ProtectedEnvironment environment = new ProtectedEnvironment();
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(mockAgent());
        environment.setDependencies(new HashMap<>());
        environment.getDependencies().put(DatabaseConstants.AGENTS, agents);

        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");

        AgentDetailDto agentDetailDto = new AgentDetailDto();
        AppResource appResource = new AppResource();
        appResource.setExtendInfo(new HashMap<>());
        appResource.getExtendInfo().put(OracleConstants.CLUSTER_NAME, "hwdb1");
        List<AppResource> list = new ArrayList<>();
        list.add(appResource);
        agentDetailDto.setResourceList(list);

        Mockito.when(oracleBaseService.getOracleClusterHosts(any())).thenReturn(Collections.singletonList(mockAgent()));
        Mockito.when(resourceService.query(anyInt(),anyInt(),anyMap())).thenReturn(existsResource);
        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(environment);
        Mockito.when(agentUnifiedService.checkApplicationNoRetry(any(),any())).thenReturn(agentBaseDto);
        Mockito.when(agentUnifiedService.getDetail(any(),any(),any(),any())).thenReturn(agentDetailDto);
        PageListResponse<ProtectedResource> resources = new PageListResponse<>();
        Mockito.when(resourceService.query(anyInt(),anyInt(),any())).thenReturn(resources);
        oracleClusterDatabaseProvider.beforeCreate(resource);
        Mockito.verify(oracleBaseService, Mockito.times(1)).getOracleClusterHosts(resource);
    }

    /**
     * 用例场景：注册之前检查
     * 前置条件：存储信息多条
     * 检查点: 异常
     */
    @Test(expected = LegoCheckedException.class)
    public void check_before_create_fail_when_storage_over_limit(){
        ProtectedResource resource = mockClusterDatabase();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("storages", "[{'username':'admin','password':'Admin@storage1','port':'8088','ip':'5555.665.3.3',"
            + "'enableCert':'0'}, {'username':'admin','password':'Admin@storage1','port':'8088','ip':'5555.665.3.3',\"\n"
            + "            + \"'enableCert':'0'}]");
        resource.setExtendInfo(extendInfo);
        PageListResponse<ProtectedResource> existsResource = new PageListResponse<>();
        existsResource.setRecords(new ArrayList<>());
        Mockito.when(resourceService.query(anyInt(),anyInt(),anyMap())).thenReturn(existsResource);
        oracleClusterDatabaseProvider.beforeCreate(resource);

    }

    /**
     * 用例场景：注册之前检查
     * 前置条件：存储信息多条
     * 检查点: 异常
     */
    @Test(expected = LegoCheckedException.class)
    public void check_before_create_fail_when_ip_format_error(){
        ProtectedResource resource = mockClusterDatabase();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("storages", "[{'username':'admin','password':'Admin@storage1','port':'8088','ip':'5555.665.3.3',"
            + "'enableCert':'0'}]");
        resource.setExtendInfo(extendInfo);
        PageListResponse<ProtectedResource> existsResource = new PageListResponse<>();
        existsResource.setRecords(new ArrayList<>());
        Mockito.when(resourceService.query(anyInt(),anyInt(),anyMap())).thenReturn(existsResource);
        oracleClusterDatabaseProvider.beforeCreate(resource);
    }

    /**
     * 用例场景：注册之前检查
     * 前置条件：存储信息多条
     * 检查点: 异常
     */
    @Test(expected = LegoCheckedException.class)
    public void check_before_create_fail_when_storage_connect_error() {
        ProtectedResource resource = mockClusterDatabase();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("storages",
            "[{'username':'admin','password':'Admin@storage1','port':'8088','ipList':'127.0.0.1, 8.40.97.11',"
                + "'enableCert':'0'}]");
        resource.setExtendInfo(extendInfo);
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("-1");
        String errMessage = "{\"bodyErr\": 1577209919}";
        agentBaseDto.setErrorMessage(errMessage);
        PageListResponse<ProtectedResource> existsResource = new PageListResponse<>();
        existsResource.setRecords(new ArrayList<>());
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(existsResource);
        Mockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(agentBaseDto);
        List<ProtectedEnvironment> clusterHosts = new ArrayList<>();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("127.0.0.1");
        clusterHosts.add(environment);
        Mockito.when(oracleBaseService.getOracleClusterHosts(any())).thenReturn(clusterHosts);
        oracleClusterDatabaseProvider.beforeCreate(resource);
    }

    @Test
    public void test_get_resource_feature_success() {
        Assert.assertTrue(oracleClusterDatabaseProvider.getResourceFeature().isShouldCheckEnvironmentNameDuplicate());
    }


    /**
     * 用例场景：当接口返回任务不可删除时，任务删除失败
     * 前置条件：接口返回任务不可删除
     * 检查点: 抛出异常
     */
    @Test
    public void test_delete_pre_handle_failed(){
        PowerMockito.when(oracleBaseService.isAnonymizationDeletable(anyString())).thenReturn(false);
        Assert.assertThrows(LegoCheckedException.class,
            () -> oracleClusterDatabaseProvider.preHandleDelete(mockClusterDatabase()));
    }

    private ProtectedResource mockClusterDatabase(){
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("1234567");
        resource.setParentUuid("7654321");
        resource.setName("hwdb");
        resource.setSubType(ResourceSubTypeEnum.ORACLE_CLUSTER.getType());
        return resource;
    }

    private ProtectedEnvironment mockAgent(){
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid("fc2d213a-482c-4e3b-b72d-9c166bc8f952");
        agent.setEndpoint("192.168.111.180");
        agent.setPort(59536);
        return agent;
    }
}
